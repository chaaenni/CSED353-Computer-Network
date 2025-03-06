#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity):
    buffer(), _capacity(capacity), _input_ended(false), _error(false), _num_written(0), _num_read(0) {}


size_t ByteStream::write(const string &data) {
    if(_input_ended) return 0;

    int num_writable = min(data.size(), remaining_capacity());

    for(int i = 0; i < num_writable ; i++){
        buffer.push_back(data[i]);
        _num_written++;
    }
    return num_writable;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    string peeked = "";
    int peek_len = min(len, buffer.size());
    for(int i = 0; i < peek_len ; i++){
        peeked += buffer[i];
    }
    // for(deque<char>::iterator buffer_ptr = buffer.begin(); buffer_ptr < buffer.begin() + len; buffer_ptr++){
    //     peeked += *buffer_ptr;
    // }

    return peeked;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    int peek_len = min(len, buffer.size());
    for(int i = 0; i < peek_len; i++){
        buffer.pop_front();
        _num_read ++;
    }

    return;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    string peeked = peek_output(len);
    pop_output(len);
    return peeked;
}

void ByteStream::end_input() {
    _input_ended = true;
}

bool ByteStream::input_ended() const { return _input_ended; }

size_t ByteStream::buffer_size() const { return buffer.size(); }

bool ByteStream::buffer_empty() const { return (buffer.size() == 0); }

bool ByteStream::eof() const { return _input_ended && buffer_empty(); }

size_t ByteStream::bytes_written() const { return _num_written; }

size_t ByteStream::bytes_read() const { return _num_read; }

size_t ByteStream::remaining_capacity() const {
    return _capacity - buffer.size();
 }
