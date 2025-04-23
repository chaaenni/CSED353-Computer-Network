#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

#include <iostream>

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    WrappingInt32 seqno = seg.header().seqno;
    _syn_flag = seg.header().syn;
    _fin_flag = seg.header().fin;

    if (!_if_syn_set && _syn_flag) {  // initialize ISN when SYN flag is true
        _isn = seqno;
        _if_syn_set = true;
    }

    if (!_if_fin_set && _fin_flag) {
        _abs_seqno_fin = unwrap(seqno + seg.length_in_sequence_space() - 1, _isn, _abs_checkpoint);
        _if_fin_set = true;
    }

    if (!seg.length_in_sequence_space() || !_if_syn_set)
        return;  // if the length of the TCP segment is 0 or SYN has not arrived, then just return the function

    uint64_t abs_seqno = unwrap(seqno, _isn, _abs_checkpoint);

    // cout << "abs_seqno: " << abs_seqno << endl;
    size_t stream_index =
        (!_syn_flag) ? abs_seqno - 1 : abs_seqno;  // if SYN flag is false(if segment is not SYN and just ordinary
                                                   // segment), get stream_index by calculating abs_seqno - 1
    _reassembler.push_substring(seg.payload().copy(), stream_index, _fin_flag);

    _abs_checkpoint = abs_seqno;
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (!_if_syn_set)
        return {};

    size_t ackno =
        _reassembler.get_next_index() +
        (_if_syn_set ? 1 : 0);  // if SYN flag is already set in TCP receiver, then add 1(for including SYN index)
    ackno +=
        (ackno == _abs_seqno_fin ? 1
                                 : 0);  // if ackno is equal to absolute seqno of the FIN, then increment ackno by 1.
    return wrap(ackno, _isn);           // convert to WrapingInt32(seqno style)
}

size_t TCPReceiver::window_size() const { return _capacity - _reassembler.stream_out().buffer_size(); }
