#pragma once

#include <string_view>
#include <vector>
#include <memory>
#include <cassert>


namespace li {

struct input_buffer {

  std::vector<char> buffer_;
  int cursor = 0; // First index of the currently used buffer area
  int end = 0;    // Index of the last read character

  input_buffer() : buffer_(50 * 1024), cursor(0), end(0) {}

  // Free unused space in the buffer in [i1, i2[.
  // This may move data in [i2, end[ if needed.
  void free(int i1, int i2) {
    assert(i1 < i2);
    assert(i1 >= 0 and i1 < buffer_.size());
    assert(i2 > 0 and i2 <= buffer_.size());

    if (i1 == cursor and i2 == end) // eat the whole buffer.
      cursor = end = 0;
    else if (i1 == cursor) // eat the beggining of the buffer.
      cursor = i2;
    else if (i2 == end)
      end = i1;         // eat the end of the buffer.
    else if (i2 != end) // eat somewhere in the middle.
    {
      if (buffer_.size() - end < buffer_.size() / 4) {
        if (end - i2 > i2 - i1) // use memmove if overlap.
          std::memmove(buffer_.data() + i1, buffer_.data() + i2, end - i2);
        else
          std::memcpy(buffer_.data() + i1, buffer_.data() + cursor, end - cursor);
      }
    }
  }

  void free(const char* i1, const char* i2) {
    assert(i1 >= buffer_.data());
    assert(i1 <= &buffer_.back());
    // std::cout << (i2 - &buffer_.front()) << " " << buffer_.size() <<  << std::endl;
    assert(i2 >= buffer_.data() and i2 <= &buffer_.back() + 1);
    free(i1 - buffer_.data(), i2 - buffer_.data());
  }
  void free(const std::string_view& str) { free(str.data(), str.data() + str.size()); }

  // private: Read more data
  // Read from ptr until character x.
  // read n more characters at address ptr.
  // eat n first/last character.
  // free part of the buffer and more data if needed.

  // Read more data.
  // Return 0 on error.
  template <typename F> int read_more(F& fiber, int size = -1) {

    // If size is not specified, read potentially until the end of the buffer.
    if (size == -1)
      size = buffer_.size() - end;

    if (end == buffer_.size() || size > (buffer_.size() - end))
      throw std::runtime_error("Error: request too long, read buffer full.");

    int received = fiber.read(buffer_.data() + end, size);
    end = end + received;
    return received;
  }

  template <typename F> std::string_view read_more_str(F& fiber) {
    int l = read_more(fiber);
    return std::string_view(buffer_.data() + end - l);
  }

  template <typename F> std::string_view read_n(F&& fiber, const char* start, int size) {
    int str_start = start - buffer_.data();
    int str_end = size + str_start;
    if (end < str_end) {
      // Read more body on the socket.
      int current_size = end - str_start;
      while (current_size < size)
        current_size += read_more(fiber);
    }
    return std::string_view(start, size);
  }

  template <typename F> std::string_view read_until(F&& fiber, const char*& start, char delimiter) {
    const char* str_end = start;

    while (true) {
      const char* buffer_end = buffer_.data() + end;
      while (str_end < buffer_end and *str_end != delimiter)
        str_end++;

      if (*str_end == delimiter)
        break;
      else {
        if (!read_more(fiber))
          break;
      }
    }

    auto res = std::string_view(start, str_end - start);
    start = str_end + 1;
    return res;
  }

  bool empty() const { return cursor == end; }

  // Return the amount of data currently available to read.
  int current_size() { return end - cursor; }

  // Reset the buffer. Copy remaining data at the beggining if there is some.
  void reset() {
    assert(cursor <= end);
    if (cursor == end)
      end = cursor = 0;
    else {
      if (cursor > end - cursor) // use memmove if overlap.
        std::memmove(buffer_.data(), buffer_.data() + cursor, end - cursor);
      else
        std::memcpy(buffer_.data(), buffer_.data() + cursor, end - cursor);

      // if (
      // memcpy(buffer_.data(), buffer_.data() + cursor, end - cursor);

      end = end - cursor;
      cursor = 0;
    }
  }

  // On success return the number of bytes read.
  // On error return 0.
  char* data() { return buffer_.data(); }
};

} // namespace li
