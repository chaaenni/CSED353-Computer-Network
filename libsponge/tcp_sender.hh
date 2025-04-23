#ifndef SPONGE_LIBSPONGE_TCP_SENDER_HH
#define SPONGE_LIBSPONGE_TCP_SENDER_HH

#include "byte_stream.hh"
#include "tcp_config.hh"
#include "tcp_segment.hh"
#include "wrapping_integers.hh"

#include <functional>
#include <queue>

//! \brief The "sender" part of a TCP implementation.

class Timer {
  private:
    unsigned int _elapsed_time;  // for saving elapsed time of the timer
    bool _if_run;                // if timer is running now

  public:
    Timer();
    void stop();
    void start();
    void increment(const size_t ms_since_last_tick);
    unsigned int elapsed_time();
    bool if_run();
};

//! Accepts a ByteStream, divides it up into segments and sends the
//! segments, keeps track of which segments are still in-flight,
//! maintains the Retransmission Timer, and retransmits in-flight
//! segments if the retransmission timer expires.
class TCPSender {
  private:
    //! our initial sequence number, the number for our SYN.
    WrappingInt32 _isn;

    //! outbound queue of segments that the TCPSender wants sent
    std::queue<TCPSegment> _segments_out{};

    //! retransmission timer for the connection
    unsigned int _initial_retransmission_timeout;

    //! outgoing stream of bytes that have not yet been sent
    ByteStream _stream;

    //! the (absolute) sequence number for the next byte to be sent
    uint64_t _next_seqno{0};

    std::queue<TCPSegment>
        _outstanding_segments{};   // queue for storing outstanding segments, that are sent but not acknowledged yet
    uint64_t _bytes_in_flight{0};  // for saving the number of bytes that are sent but not acknowledged yet

    bool _is_syn_set;  // if SYN flag has been set
    bool _is_fin_set;  // if FIN flag has been set

    unsigned int _consecutive_retransmissions;  // for saving the number of consecutive retransmissions
    uint64_t _recent_abs_ackno;                 // for saving recent absolute ackno
    uint16_t _window_size;   // for saving the window size received from the receiver, especially for fill_window()
    bool _is_window_size_0;  // if window size received from the receiver is 0
    unsigned int _RTO;  // retransmission timeout. Initialized with _initial_retransmission_timeout, and doubled when
                        // timer has been expired

    Timer _timer;  // additionally defined class for controlling timer and elapsed time

  public:
    //! Initialize a TCPSender
    TCPSender(const size_t capacity = TCPConfig::DEFAULT_CAPACITY,
              const uint16_t retx_timeout = TCPConfig::TIMEOUT_DFLT,
              const std::optional<WrappingInt32> fixed_isn = {});

    //! \name "Input" interface for the writer
    //!@{
    ByteStream &stream_in() { return _stream; }
    const ByteStream &stream_in() const { return _stream; }
    //!@}

    //! \name Methods that can cause the TCPSender to send a segment
    //!@{

    //! \brief A new acknowledgment was received
    void ack_received(const WrappingInt32 ackno, const uint16_t window_size);

    //! \brief Generate an empty-payload segment (useful for creating empty ACK segments)
    void send_empty_segment();

    //! \brief create and send segments to fill as much of the window as possible
    void fill_window();

    //! \brief Notifies the TCPSender of the passage of time
    void tick(const size_t ms_since_last_tick);
    //!@}

    //! \name Accessors
    //!@{

    //! \brief How many sequence numbers are occupied by segments sent but not yet acknowledged?
    //! \note count is in "sequence space," i.e. SYN and FIN each count for one byte
    //! (see TCPSegment::length_in_sequence_space())
    size_t bytes_in_flight() const;

    //! \brief Number of consecutive retransmissions that have occurred in a row
    unsigned int consecutive_retransmissions() const;

    //! \brief TCPSegments that the TCPSender has enqueued for transmission.
    //! \note These must be dequeued and sent by the TCPConnection,
    //! which will need to fill in the fields that are set by the TCPReceiver
    //! (ackno and window size) before sending.
    std::queue<TCPSegment> &segments_out() { return _segments_out; }
    //!@}

    //! \name What is the next sequence number? (used for testing)
    //!@{

    //! \brief absolute seqno for the next byte to be sent
    uint64_t next_seqno_absolute() const { return _next_seqno; }

    //! \brief relative seqno for the next byte to be sent
    WrappingInt32 next_seqno() const { return wrap(_next_seqno, _isn); }
    //!@}

    bool is_syn_sent() const;   // return whether the SYN segment has been sent
    bool is_fin_sent() const;   // return whether the FIN segment has been sent
    bool is_fin_acked() const;  // return whether the FIN segment has been sent and acknowlegded
};

#endif  // SPONGE_LIBSPONGE_TCP_SENDER_HH
