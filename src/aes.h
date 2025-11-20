#ifndef AES_H
#define AES_H

#include "common.h"
#include <vector>
#include <string>

// AES-128 implementation (128-bit key, 10 rounds)
class AES {
public:
    // Encrypt data using AES-128 in CBC mode
    static bool encrypt(const std::vector<uint8_t>& input, 
                       const std::string& key,
                       std::vector<uint8_t>& output);
    
    // Decrypt data using AES-128 in CBC mode
    static bool decrypt(const std::vector<uint8_t>& input,
                       const std::string& key,
                       std::vector<uint8_t>& output);

private:
    static constexpr int KEY_SIZE = 16;      // 128 bits
    static constexpr int BLOCK_SIZE = 16;    // 128 bits
    static constexpr int NUM_ROUNDS = 10;    // AES-128 uses 10 rounds
    static constexpr int EXPANDED_KEY_SIZE = 176; // 44 words * 4 bytes

    // Key expansion
    static void keyExpansion(const uint8_t* key, uint8_t* expandedKey);
    
    // Core AES operations
    static void addRoundKey(uint8_t* state, const uint8_t* roundKey);
    static void subBytes(uint8_t* state);
    static void invSubBytes(uint8_t* state);
    static void shiftRows(uint8_t* state);
    static void invShiftRows(uint8_t* state);
    static void mixColumns(uint8_t* state);
    static void invMixColumns(uint8_t* state);
    
    // Encrypt/decrypt single block
    static void encryptBlock(const uint8_t* input, uint8_t* output, const uint8_t* expandedKey);
    static void decryptBlock(const uint8_t* input, uint8_t* output, const uint8_t* expandedKey);
    
    // Helper functions
    static uint8_t gMul(uint8_t a, uint8_t b);
    static void deriveKey(const std::string& password, uint8_t* key);
    static void xorBlocks(uint8_t* dest, const uint8_t* src, size_t len);
    
    // S-box and inverse S-box
    static const uint8_t sbox[256];
    static const uint8_t inv_sbox[256];
    static const uint8_t rcon[11];
};

#endif // AES_H

