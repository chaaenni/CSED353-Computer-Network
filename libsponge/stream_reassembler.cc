#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity),
_unassembled(), _unassembled_bytes(0), _if_eof(false), _next_index(0) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    if(eof){
        _if_eof = eof;
    }
    
    if(index + data.length() <= _next_index){
        if(_unassembled.begin()->first == _next_index){
            string assembled = "";
            size_t assembled_bytes = 0;
            for(auto i = _unassembled.begin(); i != _unassembled.end(); i++){
                assembled += i->second;
                assembled_bytes++;
                // _unassembled.erase(i->first);
            }
            _output.write(assembled);
            _next_index += assembled_bytes;
            _unassembled_bytes -= assembled_bytes;
            _unassembled.clear();
        }
        return;
    }
    size_t start_index = index;
    if(index < _next_index) start_index = _next_index;
    string subdata = data.substr(start_index);

    for(size_t i = 0; i < subdata.length() ; i++){
        _unassembled[start_index + i] = subdata[i];
        _unassembled_bytes++;
    }

    size_t occupied_capacity = _output.buffer_size() + _unassembled.size();
    if(occupied_capacity > _capacity){
        size_t end_index = _unassembled.end()->first;
        for(size_t i = end_index; i > end_index - (occupied_capacity - _capacity); i--){
            _unassembled.erase(i);
            _unassembled_bytes--;
        }
    }
    
    if(_unassembled.begin()->first == _next_index){
        string assembled = "";
        size_t assembled_bytes = 0;
        for(auto i = _unassembled.begin(); i != _unassembled.end(); i++){
            assembled += i->second;
            assembled_bytes++;
            // _unassembled.erase(i->first);
        }

        _output.write(assembled);
        _next_index += assembled_bytes;
        _unassembled_bytes -= assembled_bytes;
        _unassembled.clear();

    }

    if(_if_eof && _unassembled_bytes == 0) _output.end_input();

}


size_t StreamReassembler::unassembled_bytes() const { return _unassembled_bytes; }

bool StreamReassembler::empty() const { return _unassembled.size() == 0; }
