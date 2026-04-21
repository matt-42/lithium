#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>
#include <string>

namespace li {

namespace internal {

// Taken from from https://github.com/veorq/SipHash/blob/master/siphash.c

/* default: SipHash-2-4 */
#ifndef cROUNDS
#define cROUNDS 2
#endif
#ifndef dROUNDS
#define dROUNDS 4
#endif

#define ROTL(x, b) (uint64_t)(((x) << (b)) | ((x) >> (64 - (b))))

#define U32TO8_LE(p, v)                                                                            \
  (p)[0] = (uint8_t)((v));                                                                         \
  (p)[1] = (uint8_t)((v) >> 8);                                                                    \
  (p)[2] = (uint8_t)((v) >> 16);                                                                   \
  (p)[3] = (uint8_t)((v) >> 24);

#define U64TO8_LE(p, v)                                                                            \
  U32TO8_LE((p), (uint32_t)((v)));                                                                 \
  U32TO8_LE((p) + 4, (uint32_t)((v) >> 32));

#define U8TO64_LE(p)                                                                               \
  (((uint64_t)((p)[0])) | ((uint64_t)((p)[1]) << 8) | ((uint64_t)((p)[2]) << 16) |                 \
   ((uint64_t)((p)[3]) << 24) | ((uint64_t)((p)[4]) << 32) | ((uint64_t)((p)[5]) << 40) |          \
   ((uint64_t)((p)[6]) << 48) | ((uint64_t)((p)[7]) << 56))

#define SIPROUND                                                                                   \
  do {                                                                                             \
    v0 += v1;                                                                                      \
    v1 = ROTL(v1, 13);                                                                             \
    v1 ^= v0;                                                                                      \
    v0 = ROTL(v0, 32);                                                                             \
    v2 += v3;                                                                                      \
    v3 = ROTL(v3, 16);                                                                             \
    v3 ^= v2;                                                                                      \
    v0 += v3;                                                                                      \
    v3 = ROTL(v3, 21);                                                                             \
    v3 ^= v0;                                                                                      \
    v2 += v1;                                                                                      \
    v1 = ROTL(v1, 17);                                                                             \
    v1 ^= v2;                                                                                      \
    v2 = ROTL(v2, 32);                                                                             \
  } while (0)

/*
    Computes a SipHash value
    *in: pointer to input data (read-only)
    inlen: input data length in bytes (any size_t value)
    *k: pointer to the key data (read-only), must be 16 bytes
    *out: pointer to output data (write-only), outlen bytes must be allocated
    outlen: length of the output in bytes, must be 8 or 16
*/
inline int siphash(const void* in, const size_t inlen, const void* k, uint8_t* out,
                   const size_t outlen) {

  const unsigned char* ni = (const unsigned char*)in;
  const unsigned char* kk = (const unsigned char*)k;

  assert((outlen == 8) || (outlen == 16));
  uint64_t v0 = UINT64_C(0x736f6d6570736575);
  uint64_t v1 = UINT64_C(0x646f72616e646f6d);
  uint64_t v2 = UINT64_C(0x6c7967656e657261);
  uint64_t v3 = UINT64_C(0x7465646279746573);
  uint64_t k0 = U8TO64_LE(kk);
  uint64_t k1 = U8TO64_LE(kk + 8);
  uint64_t m;
  int i;
  const unsigned char* end = ni + inlen - (inlen % sizeof(uint64_t));
  const int left = inlen & 7;
  uint64_t b = ((uint64_t)inlen) << 56;
  v3 ^= k1;
  v2 ^= k0;
  v1 ^= k1;
  v0 ^= k0;

  if (outlen == 16)
    v1 ^= 0xee;

  for (; ni != end; ni += 8) {
    m = U8TO64_LE(ni);
    v3 ^= m;

    for (i = 0; i < cROUNDS; ++i)
      SIPROUND;

    v0 ^= m;
  }

  switch (left) {
  case 7:
    b |= ((uint64_t)ni[6]) << 48;
    /* FALLTHRU */
  case 6:
    b |= ((uint64_t)ni[5]) << 40;
    /* FALLTHRU */
  case 5:
    b |= ((uint64_t)ni[4]) << 32;
    /* FALLTHRU */
  case 4:
    b |= ((uint64_t)ni[3]) << 24;
    /* FALLTHRU */
  case 3:
    b |= ((uint64_t)ni[2]) << 16;
    /* FALLTHRU */
  case 2:
    b |= ((uint64_t)ni[1]) << 8;
    /* FALLTHRU */
  case 1:
    b |= ((uint64_t)ni[0]);
    break;
  case 0:
    break;
  }

  v3 ^= b;

  for (i = 0; i < cROUNDS; ++i)
    SIPROUND;

  v0 ^= b;

  if (outlen == 16)
    v2 ^= 0xee;
  else
    v2 ^= 0xff;

  for (i = 0; i < dROUNDS; ++i)
    SIPROUND;

  b = v0 ^ v1 ^ v2 ^ v3;
  U64TO8_LE(out, b);

  if (outlen == 8)
    return 0;

  v1 ^= 0xdd;

  for (i = 0; i < dROUNDS; ++i)
    SIPROUND;

  b = v0 ^ v1 ^ v2 ^ v3;
  U64TO8_LE(out + 8, b);

  return 0;
}

inline std::string generate_random_key(int length) {
    std::random_device rd;
    std::string key(length, '\0');
    // Fill with raw random bytes from random_device.
    for (int i = 0; i < length; i += 4) {
        uint32_t r = rd();
        for (int j = 0; j < 4 && i + j < length; ++j)
            key[i + j] = static_cast<char>((r >> (j * 8)) & 0xFF);
    }
    return key;
}

} // namespace internal

static const std::string siphashkey = internal::generate_random_key(16);

struct siphash {

  siphash() = default;
  siphash(const siphash&) = default;

  inline std::size_t operator()(const std::string& s) const {
    uint64_t out;
    internal::siphash(s.data(), s.size(), siphashkey.data(), (uint8_t*)&out, 8);
    return out;
  }
  inline std::size_t operator()(const std::string_view& s) const {
    uint64_t out;
    internal::siphash(s.data(), s.size(), siphashkey.data(), (uint8_t*)&out, 8);
    return out;
  }
};
} // namespace li
