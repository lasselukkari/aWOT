/*
  WebSocket handshake crypto: SHA-1 + Base64.

  SHA-1 implementation adapted from Steve Reid's public domain reference
  (100% public domain). Base64 encoder is a standard table lookup.
*/

#ifndef AWOT_WS_CRYPTO_H_
#define AWOT_WS_CRYPTO_H_

#include "aWOT_common.h"

namespace awot {
namespace ws {

struct Sha1Ctx {
  uint32_t state[5];
  uint32_t count[2];
  uint8_t buffer[64];
};

static inline uint32_t sha1_rol(uint32_t value, uint32_t bits) {
  return (value << bits) | (value >> (32 - bits));
}

static inline void sha1_transform(uint32_t state[5], const uint8_t buffer[64]) {
  uint32_t a, b, c, d, e;
  uint32_t block[16];

  for (int i = 0; i < 16; i++) {
    block[i] =
        ((uint32_t)buffer[i * 4] << 24) | ((uint32_t)buffer[i * 4 + 1] << 16) |
        ((uint32_t)buffer[i * 4 + 2] << 8) | ((uint32_t)buffer[i * 4 + 3]);
  }

  a = state[0];
  b = state[1];
  c = state[2];
  d = state[3];
  e = state[4];

  // A 16-word ring avoids an 80-word schedule, and the loop keeps flash use
  // small on AVR where unrolling all 80 rounds is prohibitively expensive.
  for (uint8_t i = 0; i < 80; ++i) {
    uint8_t slot = i & 15;
    if (i >= 16) {
      block[slot] = sha1_rol(block[(i + 13) & 15] ^ block[(i + 8) & 15] ^
                                 block[(i + 2) & 15] ^ block[slot],
                             1);
    }
    uint32_t next = sha1_rol(a, 5) + e + block[slot];
    if (i < 20) {
      next += (d ^ (b & (c ^ d))) + 0x5A827999UL;
    } else if (i < 40) {
      next += (b ^ c ^ d) + 0x6ED9EBA1UL;
    } else if (i < 60) {
      next += ((b & c) | (d & (b | c))) + 0x8F1BBCDCUL;
    } else {
      next += (b ^ c ^ d) + 0xCA62C1D6UL;
    }
    e = d;
    d = c;
    c = sha1_rol(b, 30);
    b = a;
    a = next;
  }

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
  state[4] += e;
}

static inline void sha1_init(Sha1Ctx* ctx) {
  ctx->state[0] = 0x67452301;
  ctx->state[1] = 0xEFCDAB89;
  ctx->state[2] = 0x98BADCFE;
  ctx->state[3] = 0x10325476;
  ctx->state[4] = 0xC3D2E1F0;
  ctx->count[0] = ctx->count[1] = 0;
}

static inline void sha1_update(Sha1Ctx* ctx, const uint8_t* data, size_t len) {
  uint32_t i;
  uint32_t j = (ctx->count[0] >> 3) & 63;

  if ((ctx->count[0] += (uint32_t)len << 3) < ((uint32_t)len << 3)) {
    ctx->count[1]++;
  }
  ctx->count[1] += (uint32_t)len >> 29;

  if ((j + len) > 63) {
    i = 64 - j;
    memcpy(&ctx->buffer[j], data, i);
    sha1_transform(ctx->state, ctx->buffer);
    for (; i + 63 < len; i += 64) {
      sha1_transform(ctx->state, &data[i]);
    }
    j = 0;
  } else {
    i = 0;
  }
  memcpy(&ctx->buffer[j], &data[i], len - i);
}

static inline void sha1_final(Sha1Ctx* ctx, uint8_t digest[20]) {
  uint8_t finalcount[8];
  for (int i = 0; i < 8; i++) {
    finalcount[i] =
        (uint8_t)((ctx->count[(i >= 4 ? 0 : 1)] >> ((3 - (i & 3)) * 8)) & 255);
  }
  uint8_t c = 0x80;
  sha1_update(ctx, &c, 1);
  while ((ctx->count[0] & 504) != 448) {
    c = 0;
    sha1_update(ctx, &c, 1);
  }
  sha1_update(ctx, finalcount, 8);
  for (int i = 0; i < 20; i++) {
    digest[i] = (uint8_t)((ctx->state[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);
  }
}

static inline void sha1(const uint8_t* data, size_t len, uint8_t digest[20]) {
  Sha1Ctx ctx;
  sha1_init(&ctx);
  sha1_update(&ctx, data, len);
  sha1_final(&ctx, digest);
}

// Encodes `inLen` bytes into standard Base64; writes ceil(inLen/3)*4 bytes.
// Does not null-terminate. Returns bytes written.
static inline size_t base64Encode(const uint8_t* in, size_t inLen, char* out) {
  static const char kAlpha[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  size_t o = 0;
  size_t i = 0;
  while (i + 3 <= inLen) {
    uint32_t v =
        ((uint32_t)in[i] << 16) | ((uint32_t)in[i + 1] << 8) | in[i + 2];
    out[o++] = kAlpha[(v >> 18) & 63];
    out[o++] = kAlpha[(v >> 12) & 63];
    out[o++] = kAlpha[(v >> 6) & 63];
    out[o++] = kAlpha[v & 63];
    i += 3;
  }
  if (i < inLen) {
    uint32_t v = (uint32_t)in[i] << 16;
    size_t rem = inLen - i;
    if (rem == 2) v |= (uint32_t)in[i + 1] << 8;
    out[o++] = kAlpha[(v >> 18) & 63];
    out[o++] = kAlpha[(v >> 12) & 63];
    out[o++] = (rem == 2) ? kAlpha[(v >> 6) & 63] : '=';
    out[o++] = '=';
  }
  return o;
}

// Computes Sec-WebSocket-Accept for a given Sec-WebSocket-Key.
// Writes exactly 28 chars + NUL into `out` (buffer must be >= 29 bytes).
static inline void computeAcceptKey(const char* key, char* out) {
  static const char kGuid[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
  while (*key == ' ' || *key == '\t') ++key;
  size_t length = strlen(key);
  while (length && (key[length - 1] == ' ' || key[length - 1] == '\t'))
    --length;
  Sha1Ctx ctx;
  sha1_init(&ctx);
  sha1_update(&ctx, (const uint8_t*)key, length);
  sha1_update(&ctx, (const uint8_t*)kGuid, sizeof(kGuid) - 1);
  uint8_t digest[20];
  sha1_final(&ctx, digest);
  size_t n = base64Encode(digest, 20, out);
  out[n] = '\0';
}

}  // namespace ws
}  // namespace awot

#endif
