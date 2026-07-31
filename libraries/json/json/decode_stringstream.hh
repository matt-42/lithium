#pragma once

#if defined(_MSC_VER)
#include <ciso646>
#endif

#include <algorithm>
#include <cmath>
#include <string_view>

namespace li {

using std::string_view;

namespace internal {
// buffer_end bounds every read: str must never be dereferenced once it reaches
// the end of the underlying decode_stringstream buffer (e.g. a number with no
// trailing delimiter at the very end of an HTTP body must not read past it).
template <typename I> void parse_uint(I* val_, const char* str, const char* buffer_end, const char** end) {
  I& val = *val_;
  val = 0;
  // Compute the byte budget once instead of comparing str < buffer_end on every
  // iteration, so the bounds check costs nothing extra in the hot digit loop.
  int max_i = str < buffer_end ? int(std::min<long long>(40, buffer_end - str)) : 0;
  int i = 0;
  while (i < max_i) {
    char c = str[i];
    if (c < '0' or c > '9')
      break;
    val = val * 10 + c - '0';
    i++;
  }
  str += i;
  if (end)
    *end = str;
}

template <typename I> void parse_int(I* val, const char* str, const char* buffer_end, const char** end) {
  bool neg = false;

  if (str < buffer_end and str[0] == '-') {
    neg = true;
    str++;
  }
  parse_uint(val, str, buffer_end, end);
  if constexpr (!std::is_same<I, bool>::value) {
    if (neg)
      *val = -(*val);
  }
}

inline unsigned long long pow10(unsigned int e) {
  unsigned long long pows[] = {1,
                               10,
                               100,
                               1000,
                               10000,
                               100000,
                               1000000,
                               10000000,
                               100000000,
                               1000000000,
                               10000000000,
                               100000000000,
                               1000000000000,
                               10000000000000,
                               100000000000000,
                               1000000000000000,
                               10000000000000000,
                               100000000000000000};

  if (e < 18)
    return pows[e];
  else
    return 0;
}

template <typename F> void parse_float(F* f, const char* str, const char* buffer_end, const char** end) {
  // 1.234e-10
  // [sign][int][decimal_part][exp]

  const char* it = str;
  int integer_part;
  parse_int(&integer_part, it, buffer_end, &it);
  int sign = integer_part >= 0 ? 1 : -1;
  *f = integer_part;
  if (it < buffer_end and *it == '.') {
    it++;
    unsigned long long decimal_part;
    const char* dec_end;
    parse_uint(&decimal_part, it, buffer_end, &dec_end);

    if (dec_end > it)
      *f += (F(decimal_part) / pow10(dec_end - it)) * sign;

    it = dec_end;
  }

  if (it < buffer_end and (*it == 'e' || *it == 'E')) {
    it++;
    bool neg = false;
    if (it < buffer_end and *it == '-') {
      neg = true;
      it++;
    }

    unsigned int exp = 0;
    parse_uint(&exp, it, buffer_end, &it);
    if (neg)
      *f = *f / pow10(exp);
    else
      *f = *f * pow10(exp);
  }

  if (end)
    *end = it;
}

} // namespace internal

class decode_stringstream {
public:
  inline decode_stringstream(std::string_view buffer_)
      : cur(buffer_.empty() ? empty_sentinel : buffer_.data()), bad_(false),
        // An empty/default string_view (e.g. an empty HTTP body) has a null data()
        // pointer. Redirect cur/buffer to a real, null-terminated byte so peek()/eof()
        // never dereference nullptr and behave like parsing an empty std::string.
        buffer(buffer_.empty() ? std::string_view(empty_sentinel, 0) : buffer_) {}

  inline bool eof() const { return cur >= buffer.data() + buffer.size(); }
  // Never dereference past the buffer: malformed/truncated input (a JSON value
  // missing its closing token) must yield a decode error, not an out-of-bounds read.
  inline const char peek() const { return eof() ? '\0' : *cur; }
  inline const char get() {
    char c = peek();
    if (!eof())
      cur++;
    return c;
  }
  inline int bad() const { return bad_; }
  inline int good() const { return !bad_ && !eof(); }

  template <typename O, typename F> void copy_until(O& output, F until) {
    const char* start = cur;
    const char* end = cur;
    const char* buffer_end = buffer.data() + buffer.size();
    while (end < buffer_end && until(*end))
      end++;

    output.append(std::string_view(start, end - start));
    cur = end;
  }

  template <typename T> void operator>>(T& value) {
    eat_spaces();
    if constexpr (std::is_floating_point<T>::value) {
      // Decode floating point.
      eat_spaces();
      const char* end = nullptr;
      internal::parse_float(&value, cur, buffer.data() + buffer.size(), &end);
      if (end == cur)
        bad_ = true;
      cur = end;
    } else if constexpr (std::is_integral<T>::value) {
      // Decode integer.
      const char* end = nullptr;
      internal::parse_int(&value, cur, buffer.data() + buffer.size(), &end);
      if (end == cur)
        bad_ = true;
      cur = end;
    } else if constexpr (std::is_same<T, std::string>::value) {
      // Decode UTF8 string.
      json_to_utf8(*this, value);
    } else if constexpr (std::is_same<T, string_view>::value) {
      // Decoding to stringview does not decode utf8.

      if (get() != '"') {
        bad_ = true;
        return;
      }

      const char* start = cur;

      while (!eof() and peek() != '"') {
        if (peek() == '\\') {
          cur++;      // Skip the backslash.
          if (!eof())
            cur++; // Skip the escaped char so a literal \" does not end the scan early.
        } else
          cur++;
      }

      if (eof()) {
        // Missing closing quote: report an error instead of returning a
        // string_view that runs past the end of the input.
        bad_ = true;
        return;
      }

      const char* end = cur;
      value = string_view(start, end - start);

      if (get() != '"') {
        bad_ = true;
        return;
      }
    }
  }

private:
  inline void eat_spaces() {
    while (!eof() and peek() < 33)
      ++cur;
  }

  static constexpr char empty_sentinel[1] = {'\0'};

  int bad_;
  const char* cur;
  std::string_view buffer; //
};

} // namespace li
