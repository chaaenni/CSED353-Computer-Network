Assignment 1 Writeup
=============

My name: [Chaeyeon Jang]

My POVIS ID: [jcy2749]

My student ID (numeric): [20200952]

This assignment took me about [12] hours to do (including the time on studying, designing, and writing the code).

Program Structure and Design of the StreamReassembler:
[For the private members of the StreamReassembler class, I've added _unassembled, _unassembled_bytes, _if_eof, and _next_index. The map<size_t, char> structure is used for _unassembled, which stores unassembled substrings. _unassembled_bytes stores the number of substrings saved in _unassembled that have not yet been reassembled. _if_eof saves whether the eof input of the push_substring function is true. _next_index stores the expected index of the next ordered substring.


For the public members of the StreamReassembler class, I've implemented the push_substring(), unassembled_bytes(), and empty() functions.


The push_substring() function accepts a substring (a segment of bytes), possibly out-of-order, from the logical stream. It assembles any newly contiguous substrings and writes them into the output stream in order. To implement this function, I first preprocessed the input data by slicing it to insert only the necessary part into the _unassembled map. If the input index is smaller than _next_index, I set the value of start_index to _next_index. If the size of the data is larger than (start_index - index), I sliced the data from (start_index - index). Here, (start_index - index) indicates the index where a new character starts, that is, the index of the first character not already in the _unassembled map. This preprocessing step is for slicing overlapping (unnecessary) characters.

Next, I sliced the subdata if its size exceeded the remaining capacity. Then, I inserted the preprocessed substring into the _unassembled map until the size of the map exceeded the capacity. If the key of the subdata to be inserted was not already in _unassembled, I incremented _unassembled_bytes. Subsequently, I inserted the unassembled substrings into the buffer (assembled buffer) if the following conditions were met:
1. The index (key) of the _unassembled element is equal to _next_index.
2. The total number of assembled substrings is below the capacity.

Starting from i = _unassembled.begin() and continuing until i reaches _unassembled.end(), if the conditions above were satisfied, I performed the following actions:
- Concatenated the value of the _unassembled element to the assembled string.
- Incremented _next_index and assembled_bytes.
- Decremented _unassembled_bytes.
- Updated i to _unassembled.erase(i).

After iterating through the loop, I wrote the final assembled string into the _output buffer. Finally, if _if_eof is true and _unassembled_bytes is 0, I set the _input_ended of the _output to true using _output.end_input().

For the unassembled_bytes() function, I returned _unassembled_bytes.

For the empty() function, I returned the result of _unassembled.size() == 0.
]

Implementation Challenges:
[For all subparts of the push_substring() function, setting the conditions for each action was the most challenging task I faced. This is because I had to consider all possible test cases when defining the conditions for each action. For instance, when setting the start_index and slicing the first part of the input data, defining conditions such as (index < _next_index) and (subdata.size() > start_index - index) was particularly tricky, as I needed to account for numerous exception cases.

By analyzing the code of the test cases that caused failures, I was able to revise my code to satisfy those test cases. For example, before revision, I set the condition for slicing input data with only (index < _next_index). However, after analyzing the test case code, I realized that I also needed to add the condition (subdata.size() > start_index - index) as well as (index < _next_index). During debugging, I printed some variables using cout and ultimately revised the code to pass all test cases.
]

Remaining Bugs:
[As far as I know, there are no remaining bugs in the code. However, one question still remains: Is a map the best structure for implementing the unassembled container?]