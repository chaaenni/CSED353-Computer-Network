#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

#include <iostream>
using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity),
_unassembled(), _unassembled_bytes(0), _if_eof(false), _next_index(0) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    if(eof){
        
        _if_eof = true;
    }

    // 1
    size_t start_index = index;
    string subdata = data;
    if(index < _next_index){
        start_index = _next_index;
        if(subdata.size() > start_index - index) subdata = data.substr(start_index - index);
        else subdata = "";
    }

    //2
    // string subdata = data;


    // for(size_t i = 0; i < subdata.length() ; i++){
    //     _unassembled[start_index + i] = subdata[i];
    //     _unassembled_bytes++;
    // }

    // size_t occupied_capacity = _output.buffer_size() + _unassembled.size();
    // if(occupied_capacity > _capacity){
    //     size_t end_index = _unassembled.end()->first;
    //     for(size_t i = end_index; i > end_index - (occupied_capacity - _capacity); i--){
    //         _unassembled.erase(i);
    //         _unassembled_bytes--;
    //     }
    // }

    size_t remaining_capacity = _capacity - _output.buffer_size();
    cout << "remaining capacity: " << remaining_capacity << "buffer size: " << endl;
    // size_t remaining_capacity = _capacity - _output.buffer_size() - _unassembled_bytes;
    // cout << "_unassembled_bytes: " << _unassembled_bytes << "capacity: " << _capacity << endl;

    //1
    if (subdata.size() > remaining_capacity) {
        subdata = subdata.substr(0, remaining_capacity);
    }
    
    for(size_t i = 0; i < subdata.length() ; i++){
        if(_unassembled.size() <= _capacity){
            if(_unassembled.find(start_index + i) == _unassembled.end()) _unassembled_bytes++;
            _unassembled[start_index + i] = subdata[i];
        }
    }
    // cout << "unassembled siz_before: " << _unassembled_bytes << endl;

    // //2
    // if (subdata.size() > remaining_capacity + (_next_index - index)) {
    //     subdata = subdata.substr(0, remaining_capacity + (_next_index - index));
    // }

    // for(size_t i = 0; i < subdata.length() ; i++){
    //     if(index + i < _capacity){
    //         _unassembled[index + i] = subdata[i];
    //         _unassembled_bytes++;
    //     }
    // }


    //debug
    // for(auto i = _unassembled.begin(); i != _unassembled.end(); i++){
    //     cout << i->first << ":" << i->second << endl;
    // }

    
    
    string assembled = "";
    size_t assembled_bytes = _output.buffer_size();

    for(auto i = _unassembled.begin(); i != _unassembled.end(); ){
        if((i->first == _next_index) && (assembled_bytes < _capacity)){
            assembled += i->second;
            _next_index++;
            _unassembled_bytes--;
            assembled_bytes++;
            i = _unassembled.erase(i);
        }else{
            i++;
        }
    }

    _output.write(assembled);
    cout << "unassembled siz_after: " << _unassembled_bytes << endl;
    // cout << "string: " << _output.peek_output(_output.buffer_size()) << endl;
    // cout << "num_written: " << bytes << endl;

    // for(auto i = _unassembled.begin(); i != _unassembled.end(); i++){
    //     cout << i->first << ":" << i->second << endl;
    // }


    if(_if_eof && _unassembled_bytes == 0) _output.end_input();

}


size_t StreamReassembler::unassembled_bytes() const { return _unassembled_bytes; }

bool StreamReassembler::empty() const { return _unassembled.size() == 0; }
