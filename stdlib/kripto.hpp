#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <cstring>

// SHA256 Helper macros (before namespace)
#define SHA256_ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define SHA256_CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define SHA256_MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define SHA256_EP0(x) (SHA256_ROTR(x, 2) ^ SHA256_ROTR(x, 13) ^ SHA256_ROTR(x, 22))
#define SHA256_EP1(x) (SHA256_ROTR(x, 6) ^ SHA256_ROTR(x, 11) ^ SHA256_ROTR(x, 25))
#define SHA256_SIG0(x) (SHA256_ROTR(x, 7) ^ SHA256_ROTR(x, 18) ^ ((x) >> 3))
#define SHA256_SIG1(x) (SHA256_ROTR(x, 17) ^ SHA256_ROTR(x, 19) ^ ((x) >> 10))

namespace uzpp::Kripto {

// Base64 Kodlash va Dekodlash
namespace detail {
    static const std::string b64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";
             
    inline bool is_base64(unsigned char c) {
        return (isalnum(c) || (c == '+') || (c == '/'));
    }
}

[[nodiscard]] inline std::string base64Kodlash(const std::string& data) {
    std::string ret;
    int i = 0, j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    std::size_t in_len = data.size();
    const unsigned char* bytes_to_encode = reinterpret_cast<const unsigned char*>(data.c_str());

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            for(i = 0; (i <4) ; i++) ret += detail::b64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for(j = i; j < 3; j++) char_array_3[j] = '\0';
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++) ret += detail::b64_chars[char_array_4[j]];
        while((i++ < 3)) ret += '=';
    }
    return ret;
}

[[nodiscard]] inline std::string base64Dekodlash(const std::string& encoded_string) {
    int in_len = static_cast<int>(encoded_string.size());
    int i = 0, j = 0, in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && (encoded_string[in_] != '=') && detail::is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i ==4) {
            for (i = 0; i <4; i++) char_array_4[i] = static_cast<unsigned char>(detail::b64_chars.find(char_array_4[i]));
            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
            for (i = 0; (i < 3); i++) ret += char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j <4; j++) char_array_4[j] = 0;
        for (j = 0; j <4; j++) char_array_4[j] = static_cast<unsigned char>(detail::b64_chars.find(char_array_4[j]));
        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }
    return ret;
}

// Base64Url Kodlash (JWT uchun maxsus format)
[[nodiscard]] inline std::string base64UrlKodlash(const std::string& data) {
    std::string b64 = base64Kodlash(data);
    std::string url;
    for (char c : b64) {
        if (c == '+') url += '-';
        else if (c == '/') url += '_';
        else if (c != '=') url += c;
    }
    return url;
}

// Oddiy heks kodlash (asosan hashlar uchun ishlatiladi)
[[nodiscard]] inline std::string heksKodlash(const unsigned char* hash, std::size_t length) {
    std::string hex_str;
    hex_str.reserve(length * 2);
    constexpr char hex_chars[] = "0123456789abcdef";
    for (std::size_t i = 0; i < length; ++i) {
        hex_str.push_back(hex_chars[hash[i] >> 4]);
        hex_str.push_back(hex_chars[hash[i] & 0x0F]);
    }
    return hex_str;
}

// Oddiy DJB2 hash algoritmi tezkor hashing ehtiyojlari uchun
[[nodiscard]] inline std::string oddiyHash(const std::string& str) {
    unsigned long hash = 5381;
    for (char c : str) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    
    // Hashni byte-array qilib hexga o'tkazish
    unsigned char bytes[4];
    bytes[0] = (hash >> 24) & 0xFF;
    bytes[1] = (hash >> 16) & 0xFF;
    bytes[2] = (hash >> 8) & 0xFF;
    bytes[3] = hash & 0xFF;
    return heksKodlash(bytes, 4);
}

// SHA-256 Xeshlash algoritmi
namespace detail {
    inline uint32_t sha1_rol(uint32_t value, uint32_t bits) { return (value << bits) | (value >> (32 - bits)); }
    static const uint32_t sha256_k[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };
}

[[nodiscard]] inline std::vector<unsigned char> sha1_baytlar(const std::string& data) {
    uint32_t h[5] = { 0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0 };
    std::vector<uint8_t> block(data.begin(), data.end());
    uint64_t ml = block.size() * 8;
    block.push_back(0x80);
    while (block.size() % 64 != 56) block.push_back(0);
    for (int i = 7; i >= 0; --i) block.push_back(static_cast<uint8_t>(ml >> (i * 8)));

    for (size_t i = 0; i < block.size(); i += 64) {
        uint32_t w[80];
        for (int j = 0; j < 16; ++j) w[j] = (block[i+j*4] << 24) | (block[i+j*4+1] << 16) | (block[i+j*4+2] << 8) | block[i+j*4+3];
        for (int j = 16; j < 80; ++j) w[j] = detail::sha1_rol(w[j-3] ^ w[j-8] ^ w[j-14] ^ w[j-16], 1);
        uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4];
        for (int j = 0; j < 80; ++j) {
            uint32_t f, k;
            if (j < 20) { f = (b & c) | ((~b) & d); k = 0x5A827999; }
            else if (j < 40) { f = b ^ c ^ d; k = 0x6ED9EBA1; }
            else if (j < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDC; }
            else { f = b ^ c ^ d; k = 0xCA62C1D6; }
            uint32_t temp = detail::sha1_rol(a, 5) + f + e + k + w[j];
            e = d; d = c; c = detail::sha1_rol(b, 30); b = a; a = temp;
        }
        h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e;
    }
    std::vector<unsigned char> hash(20);
    for (int i = 0; i < 5; ++i) { hash[i*4] = (h[i] >> 24) & 0xFF; hash[i*4+1] = (h[i] >> 16) & 0xFF; hash[i*4+2] = (h[i] >> 8) & 0xFF; hash[i*4+3] = h[i] & 0xFF; }
    return hash;
}

[[nodiscard]] inline std::vector<unsigned char> sha256_baytlar(const std::string& data) {
    uint32_t state[8] = { 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 };
    std::vector<uint8_t> block(data.begin(), data.end());
    uint64_t bitlen = block.size() * 8;
    block.push_back(0x80);
    while (block.size() % 64 != 56) block.push_back(0x00);
    for (int i = 7; i >= 0; --i) block.push_back(static_cast<uint8_t>(bitlen >> (i * 8)));

    for (size_t i = 0; i < block.size(); i += 64) {
        uint32_t m[64];
        for (int j = 0; j < 16; ++j) {
            m[j] = (block[i + j*4] << 24) | (block[i + j*4 + 1] << 16) | (block[i + j*4 + 2] << 8) | block[i + j*4 + 3];
        }
        for (int j = 16; j < 64; ++j) {
            m[j] = SHA256_SIG1(m[j-2]) + m[j-7] + SHA256_SIG0(m[j-15]) + m[j-16];
        }
        uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
        uint32_t e = state[4], f = state[5], g = state[6], h = state[7];
        for (int j = 0; j < 64; ++j) {
            uint32_t t1 = h + SHA256_EP1(e) + SHA256_CH(e, f, g) + detail::sha256_k[j] + m[j];
            uint32_t t2 = SHA256_EP0(a) + SHA256_MAJ(a, b, c);
            h = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
        }
        state[0] += a; state[1] += b; state[2] += c; state[3] += d;
        state[4] += e; state[5] += f; state[6] += g; state[7] += h;
    }

    std::vector<unsigned char> hash(32);
    for (int i = 0; i < 8; ++i) {
        hash[i*4] = (state[i] >> 24) & 0xFF;
        hash[i*4+1] = (state[i] >> 16) & 0xFF;
        hash[i*4+2] = (state[i] >> 8) & 0xFF;
        hash[i*4+3] = state[i] & 0xFF;
    }
    return hash;
}

[[nodiscard]] inline std::string sha256(const std::string& data) {
    auto hash = sha256_baytlar(data);
    return heksKodlash(hash.data(), 32);
}

// HMAC-SHA256
[[nodiscard]] inline std::vector<unsigned char> hmac_sha256(const std::string& kalit, const std::string& xabar) {
    std::vector<unsigned char> key_bytes(kalit.begin(), kalit.end());
    if (key_bytes.size() > 64) key_bytes = sha256_baytlar(kalit);
    key_bytes.resize(64, 0x00);
    std::string o_pad, i_pad;
    for (size_t i = 0; i < 64; ++i) {
        o_pad += static_cast<char>(key_bytes[i] ^ 0x5c);
        i_pad += static_cast<char>(key_bytes[i] ^ 0x36);
    }
    auto inner = sha256_baytlar(i_pad + xabar);
    std::string inner_str(inner.begin(), inner.end());
    return sha256_baytlar(o_pad + inner_str);
}

// JSON Web Token (JWT) yaratish
[[nodiscard]] inline std::string jwtYaratish(const std::string& payloadJson, const std::string& maxfiyKalit) {
    std::string header = R"({"alg":"HS256","typ":"JWT"})";
    std::string b64Header = base64UrlKodlash(header);
    std::string b64Payload = base64UrlKodlash(payloadJson);
    std::string imzolashBazas = b64Header + "." + b64Payload;
    auto imzo = hmac_sha256(maxfiyKalit, imzolashBazas);
    return imzolashBazas + "." + base64UrlKodlash(std::string(imzo.begin(), imzo.end()));
}

// SHA-256 va XOR asosidagi xavfsiz oqimli shifrlash (Stream Cipher CTR-mode o'rnida)
[[nodiscard]] inline std::string xavfsizShifrlash(const std::string& matn, const std::string& kalit) {
    if (matn.empty() || kalit.empty()) return "";
    std::string natija = matn;
    std::vector<unsigned char> oqimHash = sha256_baytlar(kalit);
    
    for (size_t i = 0; i < matn.size(); ++i) {
        if (i > 0 && i % 32 == 0) {
            std::string yangiKalit(oqimHash.begin(), oqimHash.end());
            oqimHash = sha256_baytlar(yangiKalit + std::to_string(i));
        }
        natija[i] ^= oqimHash[i % 32];
    }
    return base64Kodlash(natija);
}

[[nodiscard]] inline std::string xavfsizOchish(const std::string& shifr, const std::string& kalit) {
    if (shifr.empty() || kalit.empty()) return "";
    std::string matn = base64Dekodlash(shifr);
    std::string natija = matn;
    std::vector<unsigned char> oqimHash = sha256_baytlar(kalit);
    
    for (size_t i = 0; i < matn.size(); ++i) {
        if (i > 0 && i % 32 == 0) {
            std::string yangiKalit(oqimHash.begin(), oqimHash.end());
            oqimHash = sha256_baytlar(yangiKalit + std::to_string(i));
        }
        natija[i] ^= oqimHash[i % 32];
    }
    return natija;
}

} // namespace uzpp::Kripto
