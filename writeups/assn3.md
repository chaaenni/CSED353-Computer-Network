Assignment 3 Writeup
=============

My name: [Chaeyeon Jang]

My POVIS ID: [jcy2749]

My student ID (numeric): [20200952]

This assignment took me about [12] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the TCPSender:
[

    ##1. TCPSender

    For the TCPSender class, I added the following private members:
    - outstanding_segments: queue for storing outstanding segments, that are sent but not acknowledged yet
    - uint64_t _bytes_in_flight{0}: for saving the number of bytes that are sent but not acknowledged yet
    - bool _is_syn_set: if SYN flag has been set
    - bool _is_fin_set: if FIN flag has been set
    - unsigned int _consecutive_retransmissions: for saving the number of consecutive retransmissions
    - uint64_t _recent_abs_ackno: for saving recent absolute ackno 
    - uint16_t _window_size: for saving the window size received from the receiver, especially for fill_window()
    - bool _is_window_size_0: if window size received from the receiver is 0
    - unsigned int _RTO: retransmission timeout. Initialized with _initial_retransmission_timeout, and doubled when timer has been expired
    - Timer _timer: additionally defined class for controlling timer and elapsed time

    ###(1). fill_window()
    - While the condition that SYN hasn't been set yet or next_seqno is smaller than recent_abs_ackno + window_size is satisfied, create a segment and set its header and payload with the appropriate seqno and strings read from the stream, respectively. Then, put the segment into both the segments_out buffer and the outstanding_segments buffer. Also, update bytes_in_flight and next_seqno by adding the length in the sequence space of the TCP segment, and start the timer if it is not already running.

    ###(2). ack_received()
    - First of all, preprocess the window size received from the receiver and calculate the absolute ackno using the given ackno from the receiver. Then, while the outstanding_segments buffer is not empty, iterate the following process:

        - Get the frontmost segment from the outstanding_segments buffer and calculate the absolute seqno of the segment.
        - If abs_ackno < abs_seqno + segment.length_in_sequence_space() or abs_ackno > _next_seqno, break the while loop.
        - Pop the frontmost segment from the outstanding_segments buffer and update bytes_in_flight by subtracting the length in the sequence space of the TCP segment.
        - Set the RTO (Retransmission Timeout) with initial_retransmission_timeout and reset consecutive_retransmissions to 0, then restart the timer.

    - Set recent_abs_ackno as the calculated absolute ackno, and if the outstanding_segments buffer is empty, stop the timer.

    ###(3). tick()
    - If the timer is not running, return from the function.
    - Add ms_since_last_tick, which is the input of the function, to the elapsed time of the timer.
    - If the timer has expired, resend the frontmost TCPSegment from the outstanding_segments buffer. If the window_size is nonzero, increment consecutive_retransmissions and double the RTO.
    - Restart the timer.

    ###(4). send_empty_segment()
    - Create the TCPSegment with empty payload and set its seqno with next_seqno.
    - Push it to the segments_out buffer.


    ###(5). consecutive_retransmissions() and bytes_in_flight()
    - Each returns _consecutive_retransmissions and _bytes_in_flight.


    ##2. Timer
    I newly added the Timer class to manage time-related factors. For the Timer class, I added the following private members:
    - unsigned int _elapsed_time: for saving elapsed time of the timer
    - bool _if_run: if timer is running now

    ###(1). stop() and start()
    - stop() sets if_run false, and reset elapsed_time as 0.
    - start() sets if_run true, and reset elapsed_time as 0.

    ###(2). increment()
    - increment() adds ms_since_last_tick to elapsed_time if the timer is running. 

    ###(3). elapsed_time() and if_run()
    - Each function returns _elapsed_time and _if_run.


]

Implementation Challenges:
[

    - I failed the t_send_ack test case because I did not handle the case where the received ackno from the receiver is greater than _next_seqno. To fix this, I added the condition abs_ackno <= _next_seqno to the while loop in the ack_received function. This ensures the code only runs when the condition is met.

    - I encountered a segmentation fault in the t_send_extra test case, and finding the cause took a long time. I suspected that the while loop structure in the ack_received function could cause the segmentation fault. I modified the loop structure to avoid segmentation faults by using the condition !_outstanding_segments.empty(). If this condition is met, I calculate segment and abs_seqno, and break the loop if abs_ackno < abs_seqno + segment.length_in_sequence_space() or abs_ackno > _next_seqno. This made the code safer, and I was able to pass the test case.
]

Remaining Bugs:
[

    As far a I know, there’re no remaining bugs in my code.
    
]
