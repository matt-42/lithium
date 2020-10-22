#pragma once

#include <li/http_server/output_buffer.hh>

namespace li {

template <int CHUNK_SIZE = 2000>
struct growing_output_buffer {

  growing_output_buffer() :
    buffer_(sbo_, CHUNK_SIZE, [this] (const char* s, int size) {
      // append s to the growing buffer.
      int old_growing_size = growing_.size();
      growing_.resize(growing_.size() + size);
      memcpy(growing_.data() + old_growing_size, s, size);
    }) {
  }

  void reset() { growing_.clear(); buffer_.reset(); }
  std::size_t size() { return buffer_.size() + growing_.size(); }

  std::string_view to_string_view() {
    if (growing_.size() == 0) return buffer_.to_string_view();
    else {
      buffer_.flush();
      return std::string_view(growing_);
    }
  }

  template <typename T>
  growing_output_buffer& operator<<(T&& s) { buffer_ << s; return *this; }

  char sbo_[CHUNK_SIZE];
  std::string growing_;
  output_buffer buffer_;
};

}
