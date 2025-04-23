#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return _time_since_last_segment_received; }

void TCPConnection::segment_received(const TCPSegment &seg) {
    //if the RST flag is set, do unclean shutdown
    if(seg.header().rst){
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();

        _active = false;
    }

    _receiver.segment_received(seg);

    //tells the TCPSender about ackno and window_size
    if(seg.header().ack){
        _sender.ack_received(seg.header().ackno, seg.header().win);
        if(_sender.is_syn_sent()) _sender.fill_window();
    }

    if(seg.length_in_sequence_space()){
        if(seg.header().syn && !_sender.is_syn_sent()) _sender.fill_window();
        else _sender.send_empty_segment();
    }

    //respond to keep-alive segment
    if(_receiver.ackno().has_value() && seg.length_in_sequence_space() == 0 && (seg.header().seqno == _receiver.ackno().value() - 1)){
        _sender.send_empty_segment();
    }

    if(!_sender.is_fin_sent() && _receiver.stream_out().input_ended()){ //!_sender.is_fin_sent()
        _linger_after_streams_finish = false;
    }

    _time_since_last_segment_received = 0;
    send_segments();
}

bool TCPConnection::active() const { return _active; }

size_t TCPConnection::write(const string &data) {
    if(data.size() == 0) return 0;

    size_t num_written = _sender.stream_in().write(data);
    _sender.fill_window();

    send_segments();
    return num_written;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    _time_since_last_segment_received += ms_since_last_tick;
    _sender.tick(ms_since_last_tick);

    if(_sender.consecutive_retransmissions() > _cfg.MAX_RETX_ATTEMPTS){
        //do unclean shutdown
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();
        _active = false;

        _sender.send_empty_segment();
        TCPSegment rst_segment = _sender.segments_out().front();
        rst_segment.header().rst = true;

        if(_receiver.ackno().has_value()){
            rst_segment.header().ack = true;
            rst_segment.header().ackno = _receiver.ackno().value();
            rst_segment.header().win = min(_receiver.window_size(), size_t(numeric_limits<uint16_t>::max()));
        }

        _sender.segments_out().pop();
        _segments_out.push(rst_segment);
    }
    else if(_sender.stream_in().eof() && _receiver.stream_out().input_ended() && !bytes_in_flight() 
        && !unassembled_bytes() && _sender.is_fin_acked() && _sender.next_seqno_absolute() == _sender.stream_in().bytes_written() + 2){
            if(!_linger_after_streams_finish || _time_since_last_segment_received >= 10 * _cfg.rt_timeout){ 
                //if it is passive case or it has been at least 10 times the initial retransmission timeout in active case, finish TCP connection
                _active = false;
            }
            send_segments();
    }else send_segments();
}

void TCPConnection::end_input_stream() {
    _sender.stream_in().end_input();

    _sender.fill_window();
    send_segments();
}

void TCPConnection::connect() {
    if(!active()) return;

    _sender.fill_window();
    send_segments();
}

void TCPConnection::send_segments(){
    while(!_sender.segments_out().empty()){
        TCPSegment segment = _sender.segments_out().front();

        if(_receiver.ackno().has_value()){
            segment.header().ack = true;
            segment.header().ackno = _receiver.ackno().value();
            segment.header().win = min(_receiver.window_size(), size_t(numeric_limits<uint16_t>::max()));
        }

        _segments_out.push(segment);
        _sender.segments_out().pop();
    }
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";

            // Your code here: need to send a RST segment to the peer
            // do unclean shutdown
            _sender.stream_in().set_error();
            _receiver.stream_out().set_error();
            _active = false;

            _sender.send_empty_segment();
            TCPSegment rst_segment = _sender.segments_out().front();
            rst_segment.header().rst = true;

            if(_receiver.ackno().has_value()){
                rst_segment.header().ack = true;
                rst_segment.header().ackno = _receiver.ackno().value();
                rst_segment.header().win = min(_receiver.window_size(), size_t(numeric_limits<uint16_t>::max()));
            }

            _sender.segments_out().pop();
            _segments_out.push(rst_segment);
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}
