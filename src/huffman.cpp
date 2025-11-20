#include "huffman.h"
#include <queue>
#include <iostream>
#include <algorithm>
#include <cstring>

// Comparator for priority queue (min-heap)
struct CompareNode {
    bool operator()(const std::shared_ptr<HuffmanNode>& a, const std::shared_ptr<HuffmanNode>& b) {
        return a->frequency > b->frequency;
    }
};

bool HuffmanCompressor::compress(const std::vector<uint8_t>& input, std::vector<uint8_t>& output) {
    if (input.empty()) {
        std::cerr << "Error: Input data is empty" << std::endl;
        return false;
    }

    // Build frequency table
    auto freqTable = buildFrequencyTable(input);
    
    if (freqTable.empty()) {
        return false;
    }

    // Special case: only one unique byte
    if (freqTable.size() == 1) {
        // Write a simple header for single-byte files
        output.clear();
        output.push_back('H'); // Magic byte
        output.push_back('U'); // Magic byte
        output.push_back('F'); // Magic byte
        output.push_back('1'); // Version: single byte
        
        uint8_t theByte = freqTable.begin()->first;
        uint32_t count = freqTable.begin()->second;
        
        output.push_back(theByte);
        output.push_back((count >> 24) & 0xFF);
        output.push_back((count >> 16) & 0xFF);
        output.push_back((count >> 8) & 0xFF);
        output.push_back(count & 0xFF);
        
        return true;
    }

    // Build Huffman tree
    auto root = buildHuffmanTree(freqTable);
    
    // Generate codes
    std::map<uint8_t, std::string> codes;
    generateCodes(root, "", codes);
    
    // Encode data
    std::string encodedData = encodeData(input, codes);
    
    // Write compressed data
    writeCompressedData(freqTable, encodedData, output);
    
    return true;
}

bool HuffmanCompressor::decompress(const std::vector<uint8_t>& input, std::vector<uint8_t>& output) {
    if (input.size() < 4) {
        std::cerr << "Error: Invalid compressed data" << std::endl;
        return false;
    }

    // Check magic bytes
    if (input[0] != 'H' || input[1] != 'U' || input[2] != 'F') {
        std::cerr << "Error: Invalid Huffman compressed file format" << std::endl;
        return false;
    }

    // Check version
    if (input[3] == '1') {
        // Single byte file
        if (input.size() < 9) {
            std::cerr << "Error: Invalid single-byte compressed file" << std::endl;
            return false;
        }
        
        uint8_t theByte = input[4];
        uint32_t count = (static_cast<uint32_t>(input[5]) << 24) |
                        (static_cast<uint32_t>(input[6]) << 16) |
                        (static_cast<uint32_t>(input[7]) << 8) |
                        static_cast<uint32_t>(input[8]);
        
        output.assign(count, theByte);
        return true;
    }

    // Read compressed data header and rebuild tree
    std::shared_ptr<HuffmanNode> tree;
    size_t dataOffset;
    size_t bitCount;
    
    if (!readCompressedData(input, tree, dataOffset, bitCount)) {
        return false;
    }

    // Decode data
    return decodeData(input, dataOffset, bitCount, tree, output);
}

std::map<uint8_t, uint32_t> HuffmanCompressor::buildFrequencyTable(const std::vector<uint8_t>& data) {
    std::map<uint8_t, uint32_t> freqTable;
    
    for (uint8_t byte : data) {
        freqTable[byte]++;
    }
    
    return freqTable;
}

std::shared_ptr<HuffmanNode> HuffmanCompressor::buildHuffmanTree(const std::map<uint8_t, uint32_t>& freqTable) {
    std::priority_queue<std::shared_ptr<HuffmanNode>, 
                       std::vector<std::shared_ptr<HuffmanNode>>, 
                       CompareNode> pq;
    
    // Create leaf nodes
    for (const auto& pair : freqTable) {
        pq.push(std::make_shared<HuffmanNode>(pair.first, pair.second));
    }
    
    // Build tree
    while (pq.size() > 1) {
        auto left = pq.top(); pq.pop();
        auto right = pq.top(); pq.pop();
        
        auto parent = std::make_shared<HuffmanNode>(left, right);
        pq.push(parent);
    }
    
    return pq.top();
}

void HuffmanCompressor::generateCodes(const std::shared_ptr<HuffmanNode>& node,
                                     const std::string& code,
                                     std::map<uint8_t, std::string>& codes) {
    if (!node) return;
    
    if (node->isLeaf()) {
        codes[node->byte] = code.empty() ? "0" : code;
        return;
    }
    
    generateCodes(node->left, code + "0", codes);
    generateCodes(node->right, code + "1", codes);
}

std::string HuffmanCompressor::encodeData(const std::vector<uint8_t>& data,
                                         const std::map<uint8_t, std::string>& codes) {
    std::string encoded;
    encoded.reserve(data.size() * 8); // Reserve approximate space
    
    for (uint8_t byte : data) {
        encoded += codes.at(byte);
    }
    
    return encoded;
}

void HuffmanCompressor::writeCompressedData(const std::map<uint8_t, uint32_t>& freqTable,
                                           const std::string& encodedData,
                                           std::vector<uint8_t>& output) {
    output.clear();
    
    // Write magic bytes and version
    output.push_back('H');
    output.push_back('U');
    output.push_back('F');
    output.push_back('2'); // Version 2: normal Huffman
    
    // Write frequency table size
    uint16_t tableSize = static_cast<uint16_t>(freqTable.size());
    output.push_back((tableSize >> 8) & 0xFF);
    output.push_back(tableSize & 0xFF);
    
    // Write frequency table
    for (const auto& pair : freqTable) {
        output.push_back(pair.first); // byte value
        output.push_back((pair.second >> 24) & 0xFF);
        output.push_back((pair.second >> 16) & 0xFF);
        output.push_back((pair.second >> 8) & 0xFF);
        output.push_back(pair.second & 0xFF);
    }
    
    // Write bit count
    uint64_t bitCount = encodedData.size();
    output.push_back((bitCount >> 56) & 0xFF);
    output.push_back((bitCount >> 48) & 0xFF);
    output.push_back((bitCount >> 40) & 0xFF);
    output.push_back((bitCount >> 32) & 0xFF);
    output.push_back((bitCount >> 24) & 0xFF);
    output.push_back((bitCount >> 16) & 0xFF);
    output.push_back((bitCount >> 8) & 0xFF);
    output.push_back(bitCount & 0xFF);
    
    // Convert bit string to bytes
    for (size_t i = 0; i < encodedData.size(); i += 8) {
        uint8_t byte = 0;
        for (size_t j = 0; j < 8 && i + j < encodedData.size(); ++j) {
            if (encodedData[i + j] == '1') {
                byte |= (1 << (7 - j));
            }
        }
        output.push_back(byte);
    }
}

bool HuffmanCompressor::readCompressedData(const std::vector<uint8_t>& input,
                                          std::shared_ptr<HuffmanNode>& tree,
                                          size_t& dataOffset,
                                          size_t& bitCount) {
    size_t offset = 4; // Skip magic bytes and version
    
    if (input.size() < offset + 2) {
        std::cerr << "Error: Corrupted compressed data (missing table size)" << std::endl;
        return false;
    }
    
    // Read frequency table size
    uint16_t tableSize = (static_cast<uint16_t>(input[offset]) << 8) | input[offset + 1];
    offset += 2;
    
    if (input.size() < offset + tableSize * 5 + 8) {
        std::cerr << "Error: Corrupted compressed data (truncated)" << std::endl;
        return false;
    }
    
    // Read frequency table
    std::map<uint8_t, uint32_t> freqTable;
    for (uint16_t i = 0; i < tableSize; ++i) {
        uint8_t byte = input[offset];
        uint32_t freq = (static_cast<uint32_t>(input[offset + 1]) << 24) |
                       (static_cast<uint32_t>(input[offset + 2]) << 16) |
                       (static_cast<uint32_t>(input[offset + 3]) << 8) |
                       static_cast<uint32_t>(input[offset + 4]);
        freqTable[byte] = freq;
        offset += 5;
    }
    
    // Read bit count
    bitCount = (static_cast<uint64_t>(input[offset]) << 56) |
               (static_cast<uint64_t>(input[offset + 1]) << 48) |
               (static_cast<uint64_t>(input[offset + 2]) << 40) |
               (static_cast<uint64_t>(input[offset + 3]) << 32) |
               (static_cast<uint64_t>(input[offset + 4]) << 24) |
               (static_cast<uint64_t>(input[offset + 5]) << 16) |
               (static_cast<uint64_t>(input[offset + 6]) << 8) |
               static_cast<uint64_t>(input[offset + 7]);
    offset += 8;
    
    dataOffset = offset;
    
    // Rebuild Huffman tree
    tree = buildHuffmanTree(freqTable);
    
    return true;
}

bool HuffmanCompressor::decodeData(const std::vector<uint8_t>& input,
                                  size_t dataOffset,
                                  size_t bitCount,
                                  const std::shared_ptr<HuffmanNode>& tree,
                                  std::vector<uint8_t>& output) {
    output.clear();

    const size_t neededBytes = (bitCount + 7) / 8;
    if (input.size() < dataOffset + neededBytes) {
        std::cerr << "Error: Compressed data truncated" << std::endl;
        return false;
    }
    
    auto current = tree;
    size_t bitsDecoded = 0;
    
    for (size_t i = dataOffset; i < input.size() && bitsDecoded < bitCount; ++i) {
        uint8_t byte = input[i];
        
        for (int j = 7; j >= 0 && bitsDecoded < bitCount; --j) {
            bool bit = (byte >> j) & 1;
            
            if (bit) {
                current = current->right;
            } else {
                current = current->left;
            }
            
            if (current->isLeaf()) {
                output.push_back(current->byte);
                current = tree;
            }
            
            bitsDecoded++;
        }
    }

    if (bitsDecoded != bitCount) {
        std::cerr << "Error: Bitstream ended prematurely" << std::endl;
        return false;
    }

    return true;
}

