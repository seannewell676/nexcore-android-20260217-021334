#include "nexcore/hash.h"
#include <array>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cstdint>

namespace nexcore {
namespace {
inline uint32_t rotr(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }
inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
inline uint32_t bsig0(uint32_t x) { return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22); }
inline uint32_t bsig1(uint32_t x) { return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25); }
inline uint32_t ssig0(uint32_t x) { return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3); }
inline uint32_t ssig1(uint32_t x) { return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10); }

const uint32_t K[64] = {
  0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,0x923f82a4u,0xab1c5ed5u,
  0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,
  0xe49b69c1u,0xefbe4786u,0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
  0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,0x06ca6351u,0x14292967u,
  0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,
  0xa2bfe8a1u,0xa81a664bu,0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
  0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,0x5b9cca4fu,0x682e6ff3u,
  0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u
};

std::array<uint8_t,32> sha256(const uint8_t* data, size_t len) {
  uint32_t h[8] = {0x6a09e667u,0xbb67ae85u,0x3c6ef372u,0xa54ff53au,0x510e527fu,0x9b05688cu,0x1f83d9abu,0x5be0cd19u};

  size_t new_len = len + 1;
  while ((new_len % 64) != 56) new_len++;
  std::vector<uint8_t> msg(new_len + 8, 0);
  for (size_t i=0;i<len;i++) msg[i]=data[i];
  msg[len]=0x80;
  uint64_t bits = (uint64_t)len * 8;
  for (int i=0;i<8;i++) msg[new_len+i] = (uint8_t)(bits >> (56 - 8*i));

  for (size_t off=0; off<msg.size(); off+=64) {
    uint32_t w[64];
    for (int i=0;i<16;i++) {
      w[i] = (uint32_t)msg[off+4*i]<<24 | (uint32_t)msg[off+4*i+1]<<16 | (uint32_t)msg[off+4*i+2]<<8 | (uint32_t)msg[off+4*i+3];
    }
    for (int i=16;i<64;i++) w[i] = ssig1(w[i-2]) + w[i-7] + ssig0(w[i-15]) + w[i-16];

    uint32_t a=h[0],b=h[1],c=h[2],d=h[3],e=h[4],f=h[5],g=h[6],hh=h[7];
    for (int i=0;i<64;i++) {
      uint32_t t1 = hh + bsig1(e) + ch(e,f,g) + K[i] + w[i];
      uint32_t t2 = bsig0(a) + maj(a,b,c);
      hh=g; g=f; f=e; e=d+t1; d=c; c=b; b=a; a=t1+t2;
    }
    h[0]+=a;h[1]+=b;h[2]+=c;h[3]+=d;h[4]+=e;h[5]+=f;h[6]+=g;h[7]+=hh;
  }

  std::array<uint8_t,32> out{};
  for (int i=0;i<8;i++) {
    out[i*4+0]=(uint8_t)(h[i]>>24);
    out[i*4+1]=(uint8_t)(h[i]>>16);
    out[i*4+2]=(uint8_t)(h[i]>>8);
    out[i*4+3]=(uint8_t)(h[i]);
  }
  return out;
}
} // namespace

std::string sha256_hex(const std::string& data) {
  auto d = sha256(reinterpret_cast<const uint8_t*>(data.data()), data.size());
  std::ostringstream o;
  for (auto b : d) o << std::hex << std::setw(2) << std::setfill('0') << (int)b;
  return o.str();
}

} // namespace nexcore
