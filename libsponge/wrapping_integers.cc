#include "wrapping_integers.hh"

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    uint64_t _2_32_with_64bit = 1ull << 32;
    return isn + n % _2_32_with_64bit;
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    uint64_t n_64 = n.raw_value();
    uint64_t isn_64 = isn.raw_value();
    uint64_t _2_32_with_64bit = 1ull << 32;

    uint64_t abs_seqno = (n_64 < isn_64) ? n_64 + _2_32_with_64bit - isn_64 : n_64 - isn_64;

    if (abs_seqno >= checkpoint)
        return abs_seqno;

    uint64_t left_dist = (checkpoint - abs_seqno) % _2_32_with_64bit;
    uint64_t right_dist = (abs_seqno - checkpoint) % _2_32_with_64bit;

    if (left_dist > right_dist) {
        abs_seqno = checkpoint + right_dist;
    } else {
        abs_seqno = checkpoint - left_dist;
    }

    return abs_seqno;
}
