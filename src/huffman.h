#ifndef HUFFMAN_H
#define HUFFMAN_H

#include "common.h"
#include <vector>
#include <map>
#include <memory>

// Huffman Tree Node
struct HuffmanNode {
    uint8_t byte;
    uint32_t frequency;
    std::shared_ptr<HuffmanNode> left;
    std::shared_ptr<HuffmanNode> right;
    
    HuffmanNode(uint8_t b, uint32_t freq) 
        : byte(b), frequency(freq), left(nullptr), right(nullptr) {}
    
    HuffmanNode(std::shared_ptr<HuffmanNode> l, std::shared_ptr<HuffmanNode> r)
        : byte(0), frequency(l->frequency + r->frequency), left(l), right(r) {}
    
    bool isLeaf() const { return left == nullptr && right == nullptr; }
};

class HuffmanCompressor {
public:
    // Compress data using Huffman coding
    static bool compress(const std::vector<uint8_t>& input, std::vector<uint8_t>& output);
    
    // Decompress data using Huffman coding
    static bool decompress(const std::vector<uint8_t>& input, std::vector<uint8_t>& output);

private:
    // Build frequency table
    static std::map<uint8_t, uint32_t> buildFrequencyTable(const std::vector<uint8_t>& data);
    
    // Build Huffman tree from frequency table
    static std::shared_ptr<HuffmanNode> buildHuffmanTree(const std::map<uint8_t, uint32_t>& freqTable);
    
    // Generate codes from Huffman tree
    static void generateCodes(const std::shared_ptr<HuffmanNode>& node, 
                             const std::string& code,
                             std::map<uint8_t, std::string>& codes);
    
    // Encode data using Huffman codes
    static std::string encodeData(const std::vector<uint8_t>& data, 
                                  const std::map<uint8_t, std::string>& codes);
    
    // Write compressed header and data
    static void writeCompressedData(const std::map<uint8_t, uint32_t>& freqTable,
                                   const std::string& encodedData,
                                   std::vector<uint8_t>& output);
    
    // Read compressed header and rebuild tree
    static bool readCompressedData(const std::vector<uint8_t>& input,
                                   std::shared_ptr<HuffmanNode>& tree,
                                   size_t& dataOffset,
                                   size_t& bitCount);
    
    // Decode data using Huffman tree
    static bool decodeData(const std::vector<uint8_t>& input,
                          size_t dataOffset,
                          size_t bitCount,
                          const std::shared_ptr<HuffmanNode>& tree,
                          std::vector<uint8_t>& output);
};

#endif // HUFFMAN_H

