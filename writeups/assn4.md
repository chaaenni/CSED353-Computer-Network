Assignment 4 Writeup
=============

My name: [Chaeyeon Jang]

My POVIS ID: [jcy2749]

My student ID (numeric): [20200952]

This assignment took me about [12] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

Your benchmark results (without reordering, with reordering): [0.14, 0.04]

Program Structure and Design of the TCPConnection:
[

    For the TCPConnection class, I added the following private members:
    - bool _active: indicate the status of the connection(whether the connection is active or not)
    - size_t _time_since_last_segment_received: save the elapsed time since the last segment is received
    - void send_segments(): private function for sending segements kept in outbound's sending queue

    And the functions below are the important functions that are implemented in TCPConnection class. 

    (1) segment_received()
    - If the RST flag is set, it sets both the sender's and receiver's streams to the error state and marks the active status as false to perform an unclean shutdown.
    - The segment is delivered to the receiver through the _receiver.segment_received function, where the segment's seqno, SYN, payload, and FIN are checked, and the payload data is written to the receiver's stream via the push_substring function.
    - If the segment's ACK flag is set, the _sender.ack_received() function is called to deliver the ackno and window size to the sender. If the segment's payload is empty, the sender sends an empty segment using send_empty_segment.
    - If a keep-alive segment arrives, an empty segment is sent. If the receiver's stream ends before the sender has sent a FIN, the linger flag is set to false.

    (2) write()
    - If the size of the input data is 0, return 0.
    - Write the data to the stream using the _sender's outbound stream write() function, and store the number of bytes written in num_written.
    - Fill the _sender's window with segments to send using the fill_window() function, then send available segments from the _sender's _segments_out queue using send_segments(). During send_segments(), if the receiver's ackno is available, it updates the segment header accordingly.
    - Return num_written.

    (3) tick()
    - Update elapsed time by adding the input ms_since_last_tick to _time_since_last_segment_received, and execute the _sender's tick() function.
    - If the _sender's consecutive_retransmissions count exceeds MAX_RETX_ATTEMPTS, 
        1. Set both _sender's and _receiver's streams to error state.
        2. Set active to false.
        3. Use send_empty_segment() to queue an empty segment with RST flag set to true
        for unclean shutdown.
    - If the following conditions are met:
        1. The inbound stream has been fully assembled and has ended.
        2. The outbound stream has been ended by the local application and fully sent to the remote peer.
        3. The outbound stream has been fully acknowledged by the remote peer.

        Perform theses actions:
        1. If _linger_after_streams_finish is false (passive close) or _time_since_last_segment_received exceeds 10× initial retransmission timeout (active close condition), set _active to false to close the connection.
    - Send any remaining sender segments via send_segments().

    (4) connect()
    - If TCPConnection is not active, return.
    - Fill the _sender queue with segments to send using the _sender's fill_window() function, then send available segments from the sender's _segments_out queue using send_segments().

    (5) ~TCPConnection()
    - If the TCPConnection is currently active, mark both the _sender's and _receiver's streams as error state, and set active to false to perform an unclean shutdown.
    - Use the sender's send_empty_segment() function to place an empty segment in the sending queue, and set that segment's RST flag to true
    - If the receiver's ackno value exists, update the segment header's ack, ackno, and win values
    
    
]

Implementation Challenges:
[

    - While running test cases, I encountered exceptions in the rst and rst_win tests, which proved challenging to resolve. The debugging process was particularly difficult because I had to identify and fill logical gaps not only in the TCPConnection implementation but also in previously written code. Locating and fixing the faulty code took a significant amount of time.
    - In StreamReassembler's push_string, exception handling was missing for certain edge cases - such as when UINT64_MAX is passed as an index or when an index outside the window range is received. To handle these exceptions, I added a right_end_window variable to properly manage these cases.

]

Remaining Bugs:
[

    As far as I know, there are no remaining bugs. However, the CPU-limited throughput and CPU-limited throughput with reordering did not achieve high values. In particular, the CPU-limited throughput with reordering was below 0.10 Gbits/s, which I suspect is due to the implementation of the StreamReassembler. Currently, the StreamReassembler uses a map type to store unassembled substrings, but switching to a different container type might improve performance.

]