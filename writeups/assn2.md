Assignment 2 Writeup
=============

My name: [Chaeyeon Jang]

My POVIS ID: [jcy2749]

My student ID (numeric): [20200952]

This assignment took me about [12] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the TCPReceiver and wrap/unwrap routines:
[
    
**1. Wrapping Integers**

For the WrappingInt32 class, there are two main functions: wrap() and unwrap(). The wrap() function converts an absolute sequence number (abs_seqno) to a WrappingInt32 sequence number (seqno). For convenience, I will refer to the absolute sequence number as abs_seqno and the WrappingInt32 sequence number as seqno.

For the transformation, I calculated seqno by adding the ISN (Initial Sequence Number) and abs_seqno % 2^32. To obtain 2^32, I left-shifted the unsigned long long integer 1 by 32.

The unwrap() function converts seqno to abs_seqno. For this conversion, if seqno is lower than the ISN, I added 2^32 to seqno before subtracting the ISN. Otherwise, I subtracted the ISN directly from seqno. Therefore, the conversion from seqno to abs_seqno can be expressed as follows:

If seqno < ISN: abs_seqno = seqno + 2^32 - ISN
If seqno >= ISN: abs_seqno = seqno - ISN

For the unwrap() operation, I further checked whether the resulting abs_seqno was the nearest value to the checkpoint. By comparing whether abs_seqno was greater than the checkpoint or, if lower, by evaluating the left and right distances of the candidate abs_seqno values from the checkpoint, I returned the appropriate abs_seqno for each case.

**2. TCP Receiver**

For the TCPReceiver class, I added the following private members:

_if_syn_set: Verifies whether the SYN flag in the TCP segment has been set for the TCPReceiver.
_if_fin_set: Verifies whether the FIN flag in the TCP segment has been set for the TCPReceiver.
_syn_flag: Stores the value of the SYN flag in the TCP segment.
_fin_flag: Stores the value of the FIN flag in the TCP segment.
_abs_checkpoint: Stores the checkpoint for unwrapping seqno to abs_seqno.
_abs_seqno_fin: If the FIN flag is set in the TCP segment, calculates the absolute sequence number of the FIN sequence and saves it to _abs_seqno_fin.

The main functions in the TCPReceiver class are segment_received(), ackno(), and window_size().

(1) segment_received()

The segment_received() function retrieves the seqno, SYN flag, and FIN flag from the input TCP segment. If the SYN flag is true and the SYN has not yet been set in the TCPReceiver, it initializes the ISN as seqno and sets _if_syn_set to true. If the FIN flag is true and the FIN has not yet been set in the TCPReceiver, it calculates the abs_seqno of the FIN and sets _if_fin_set to true.
From the seqno in the TCP segment header, it calculates the abs_seqno and stream index, and pushes the payload to the stream index position of the stream buffer using the StreamReassembler.

(2) ackno()

The ackno() function returns the acknowledgment number (ackno). To obtain the ackno, I retrieved _next_index (a private member of the StreamReassembler class) by adding a public function in the StreamReassembler to access the private member. If _if_syn_set is true, I added 1 to _next_index.
If the calculated ackno matches the abs_seqno of the FIN, I added 1 to _next_index. Finally, I returned the ackno in WrappingInt32 format by wrapping the ackno.

(3) window_size()

The window_size() function returns the distance between the first unassembled index and the first unacceptable index. This is equivalent to the remaining capacity of the unassembled container, which can be calculated as capacity - size of the assembled buffer.
]

Implementation Challenges:
[
- It was tricky to devise a method for finding the nearest abs_seqno from the checkpoint in the unwrap() function. Specifically, calculating the distances between the candidate abs_seqno values and the checkpoint was the most challenging part.

- In the TCPReceiver, I initially implemented the segment_received() function without considering the abs_seqno of the FIN, which led to the failure of two test cases. Realizing that the abs_seqno of the FIN was necessary for calculating the ackno was somewhat difficult.
]

Remaining Bugs:
[
As far a I know, there’re no remaining bugs in my code.
]
