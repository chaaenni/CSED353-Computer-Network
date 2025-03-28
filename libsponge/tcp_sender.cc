#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>
using namespace std;

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , _is_syn_set(false)
    , _is_fin_set(false)
    , _consecutive_retransmissions(0)
    , _recent_abs_ackno(0)
    , _window_size(0)
    , _is_window_size_0(false)
    , _RTO(_initial_retransmission_timeout)
    , _timer() {}

uint64_t TCPSender::bytes_in_flight() const { return _bytes_in_flight; }

void TCPSender::fill_window() {
    while(!_is_syn_set || _next_seqno < _recent_abs_ackno + _window_size){
        TCPSegment segment;
        size_t remaining_capacity = _recent_abs_ackno + _window_size - _next_seqno;
        size_t read_size = min(remaining_capacity, static_cast<size_t>(TCPConfig::MAX_PAYLOAD_SIZE));
        segment.payload() = Buffer(_stream.read(read_size));
        segment.header().seqno = wrap(_next_seqno, _isn);
        if(!_next_seqno){ //set SYN
            _is_syn_set = true;
            segment.header().syn = true;
        }
        if(_stream.eof() && !_is_fin_set && remaining_capacity > segment.length_in_sequence_space()){ //set FIN
            _is_fin_set = true;
            segment.header().fin = true;
        }

        if(segment.length_in_sequence_space() == 0) break; //if payload is empty, do not push in _segments_out and _outstanding_segments

        _segments_out.push(segment);
        _outstanding_segments.push(segment);
        _bytes_in_flight += segment.length_in_sequence_space();
        _next_seqno += segment.length_in_sequence_space();

        if(!_timer.if_run()) _timer.start();
    }

    return;

}


//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    //preprocess window size
    if(window_size == 0){
        _window_size = 1;
        _is_window_size_0 = true;
    }
    else _window_size = max(window_size, uint16_t(1));

    uint64_t abs_ackno = unwrap(ackno, _isn, _recent_abs_ackno);

    TCPSegment segment = _outstanding_segments.front();
    uint64_t abs_seqno = unwrap(segment.header().seqno, _isn, _recent_abs_ackno);

    while(!_outstanding_segments.empty() &&
    abs_ackno >= abs_seqno + segment.length_in_sequence_space()){
        _outstanding_segments.pop();
        _bytes_in_flight -= segment.length_in_sequence_space();

        //update timer-related factors and the number of consecutive_retransmissions
        _RTO = _initial_retransmission_timeout;
        _consecutive_retransmissions = 0;

        _timer.start();

        if(!_outstanding_segments.empty()){
            segment = _outstanding_segments.front();
            abs_seqno = unwrap(segment.header().seqno, _isn, _recent_abs_ackno);
        }
    }

    _recent_abs_ackno = abs_ackno;

    if(_outstanding_segments.empty()) _timer.stop();
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    if(!_timer.if_run()) return;

    _timer.increment(ms_since_last_tick);

    if(_timer.elapsed_time() >= _RTO){ // if timer has been expired
        TCPSegment earliest_segment = _outstanding_segments.front();
        _segments_out.push(earliest_segment);

        if(!_is_window_size_0){
            _consecutive_retransmissions++;
            _RTO *= 2;
        }

        _timer.start();
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return _consecutive_retransmissions; }

void TCPSender::send_empty_segment() {
    TCPSegment segment;
    segment.payload() = Buffer();
    segment.header().seqno = wrap(_next_seqno, _isn);

    _segments_out.push(segment);
    
}

Timer::Timer():
    _elapsed_time(0)
    , _if_run(false) {}

void Timer::stop() {
    _if_run = false;
    _elapsed_time = 0;
}

void Timer::start(){
    _if_run = true;
    _elapsed_time = 0;
}

void Timer::increment(const size_t ms_since_last_tick){
    if(!_if_run) return;
    _elapsed_time += ms_since_last_tick;
}


unsigned int Timer::elapsed_time() { return _elapsed_time; }
bool Timer::if_run() { return _if_run; }