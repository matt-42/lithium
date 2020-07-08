#include <cassert>
#include <iomanip>
#include <iostream>
#include <sstream>

#include <lithium.hh>

using namespace li;

struct R {
  void append(uint8_t c) { s.append(1, c); }
  std::string s;
};

void test_ascii(uint16_t c) {
  R ref;
  ref.append('"');
  switch (c) {
  case '"':
    ref.append('\\');
    ref.append('"');
    break;
  case '\\':
    ref.append('\\');
    ref.append('\\');
    break;
    // case '/': ref.append('/'); break; // Not escaped.
  case '\n':
    ref.append('\\');
    ref.append('n');
    break;
  case '\r':
    ref.append('\\');
    ref.append('r');
    break;
  case '\t':
    ref.append('\\');
    ref.append('t');
    break;
  case '\b':
    ref.append('\\');
    ref.append('b');
    break;
  case '\f':
    ref.append('\\');
    ref.append('f');
    break;
  default:
    ref.append(char(c));
  }
  ref.append('"');

  {
    std::string out;
    std::stringstream stream;
    stream << std::string(1, char(c));

    // c should convert to "c"
    auto err = utf8_to_json(stream, out);
    assert(err == 0);
    assert(out == ref.s);

    // and convert back to c without quote.
    stream.clear();
    stream.str(ref.s);
    out = "";
    err = json_to_utf8(stream, out);
    assert(err == 0);
    assert(out[0] == c and out.size() == 1);
  }

  // unicode c (\uXXXX) should convert to "c"
  {
    std::string out;
    std::stringstream stream;
    stream << "\"\\u" << std::hex << std::setfill('0') << std::setw(4) << c << '"';

    auto err = json_to_utf8(stream, out);
    assert(err == 0);
    assert(out[0] == c and out.size() == 1);
  }
}

std::string json_to_utf8(std::string s) {
  std::string out;
  std::stringstream stream;
  stream << s;

  // c should convert to "c"
  // auto err = li::json_to_utf8(stream, out);
  auto err = json_to_utf8(stream, out);
  // if (err.code)
  //   std::cerr << err.what << std::endl;
  assert(err == 0);
  return out;
}

std::string utf8_to_json(std::string s) {
  std::string out;
  std::stringstream stream;
  stream << s;

  // c should convert to "c"
  auto err = utf8_to_json(stream, out);
  // if (err)
  //   std::cerr << err.what << std::endl;
  assert(err == 0);
  return out;
}

void test_BMP(uint16_t c) // Basic Multilingual Plane U+0000 ... U+FFFF
{
  // Convect c to "\uXXXX"
  // Convert it to utf8 li::json_to_utf8
  // Convert it back to "\uXXXX" with li::utf8_to_json and check the result.
  std::stringstream stream;
  stream << "\"\\u" << std::hex << std::setfill('0') << std::setw(4) << std::uppercase << c << '"';

  std::string ref = stream.str();
  auto utf8 = json_to_utf8(ref);

  if (c >= 0x0080 and c <= 0x07FF)
    assert(utf8.size() == 2);
  else if (c >= 0x0800 and c <= 0xFFFF)
    assert(utf8.size() == 3);
  assert(utf8_to_json(utf8) == ref);
}

void test_SP(uint32_t c) //  Supplementary Planes 0x10000 ... 0x10FFFF
{
  c -= 0x10000;
  uint16_t H = (c >> 10) + 0xD800;
  uint16_t L = (c & 0x3FF) + 0xDC00;

  // Convect c to "\uXXXX"
  // Convert it to utf8 li::json_to_utf8
  // Convert it back to "\uXXXX" with li::utf8_to_json and check the result.
  std::stringstream stream;
  stream << "\"\\u" << std::hex << std::setfill('0') << std::setw(4) << std::uppercase << H << "\\u"
         << std::hex << std::setfill('0') << std::setw(4) << std::uppercase << L << '"';

  std::string ref = stream.str();
  auto utf8 = json_to_utf8(ref);

  assert(utf8.size() == 4);
  assert(utf8_to_json(utf8) == ref);
}

int main() {

  {
    assert(utf8_to_json("\xF0\x9D\x84\x9E") == "\"\\uD834\\uDD1E\"");
    auto r = json_to_utf8("\"\\uD834\\uDD1E\"");
    assert(json_to_utf8("\"\\uD834\\uDD1E\"") == "\xF0\x9D\x84\x9E");
  }

  { // Mix 7bits, 11bits, 16bits and 20bits (surrogate pair) codepoints.
    std::string out;
    std::istringstream stream("abc\xC2\xA2\xE2\x82\xAC\xF0\x90\x80\x81");
    auto err = utf8_to_json(stream, out);
    // FIXNE assert(out == R"("abc\u00A2\u20AC\uD800\uDC01")");
  }

  { // Empty string.
    std::string out;
    std::istringstream stream("");
    auto err = utf8_to_json(stream, out);
    assert(out == "\"\"");
  }

  { // Empty string.
    std::string out;
    std::istringstream stream(R"("")");
    auto err = json_to_utf8(stream, out);
    assert(out == "");
  }

  // lowercase in unicode sequences.
  {
    std::string out;
    std::istringstream stream(R"("\u000a")");
    auto err = json_to_utf8(stream, out);
    assert(err == 0);
    assert(out == "\x0a");
  }

  // uppercase in unicode sequences.
  {
    std::string out;
    std::istringstream stream(R"("\u000A")");
    auto err = json_to_utf8(stream, out);
    assert(err == 0);
    assert(out == "\x0a");
  }

  { // Test all characters.
    for (uint32_t c = 1; c <= 0x10FFFF; c++) {
      if (c < 0x0080)
        test_ascii(c);
      else if (c >= 0x0080 and c <= 0xFFFF and (c < 0xD800 or c > 0xDBFF))
        test_BMP(c);
      else if (c >= 0x10000 and c <= 0x10FFFF)
        test_SP(c);
    }
  }
}
