#ifndef COMMON_H
#define COMMON_H

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

// Platform detection
#ifdef _WIN32
    #define PLATFORM_WINDOWS
    #include <windows.h>
#else
    #define PLATFORM_POSIX
    #include <fcntl.h>
    #include <unistd.h>
    #include <dirent.h>
    #include <sys/stat.h>
    #include <pthread.h>
#endif

// Operation modes
enum class Operation {
    NONE = 0,
    COMPRESS = 1,
    DECOMPRESS = 2,
    ENCRYPT = 4,
    DECRYPT = 8
};

inline Operation operator|(Operation a, Operation b) {
    return static_cast<Operation>(static_cast<int>(a) | static_cast<int>(b));
}

inline int operator&(Operation a, Operation b) {
    return static_cast<int>(a) & static_cast<int>(b);
}

// Compression algorithms
enum class CompressionAlgorithm {
    HUFFMAN,
    NONE
};

// Encryption algorithms
enum class EncryptionAlgorithm {
    AES128,
    NONE
};

// Configuration structure
struct Config {
    Operation operations = Operation::NONE;
    CompressionAlgorithm compAlg = CompressionAlgorithm::HUFFMAN;
    EncryptionAlgorithm encAlg = EncryptionAlgorithm::AES128;
    std::string inputPath;
    std::string outputPath;
    std::string key;
};

// File information structure
struct FileInfo {
    std::string inputPath;
    std::string outputPath;
    bool isDirectory;
};

#endif // COMMON_H

