#include "arg_parser.h"
#include <iostream>
#include <algorithm>

void ArgParser::printUsage(const char* programName) {
    std::cout << "GSEA - Gestión Segura y Eficiente de Archivos" << std::endl;
    std::cout << "Usage: " << programName << " [OPTIONS]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -c              Compress" << std::endl;
    std::cout << "  -d              Decompress" << std::endl;
    std::cout << "  -e              Encrypt" << std::endl;
    std::cout << "  -u              Decrypt (unencrypt)" << std::endl;
    std::cout << "  -ce             Compress and encrypt (operations can be combined)" << std::endl;
    std::cout << "  -ud             Decrypt and decompress" << std::endl;
    std::cout << "  --comp-alg ALG  Compression algorithm (huffman)" << std::endl;
    std::cout << "  --enc-alg ALG   Encryption algorithm (aes128)" << std::endl;
    std::cout << "  -i PATH         Input file or directory" << std::endl;
    std::cout << "  -o PATH         Output file or directory" << std::endl;
    std::cout << "  -k KEY          Encryption/Decryption key (required for -e/-u)" << std::endl;
    std::cout << "  -h, --help      Show this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << programName << " -c -i input.txt -o output.huf" << std::endl;
    std::cout << "  " << programName << " -d -i output.huf -o restored.txt" << std::endl;
    std::cout << "  " << programName << " -e -i data.txt -o data.enc -k mypassword" << std::endl;
    std::cout << "  " << programName << " -ce -i folder/ -o archive/ -k secret123" << std::endl;
    std::cout << "  " << programName << " -ud -i archive/ -o restored/ -k secret123" << std::endl;
}

bool ArgParser::parse(int argc, char* argv[], Config& config) {
    if (argc < 2) {
        printUsage(argv[0]);
        return false;
    }
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return false;
        }
        else if (arg == "--comp-alg") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --comp-alg requires an argument" << std::endl;
                return false;
            }
            if (!parseCompAlg(argv[++i], config)) {
                return false;
            }
        }
        else if (arg == "--enc-alg") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --enc-alg requires an argument" << std::endl;
                return false;
            }
            if (!parseEncAlg(argv[++i], config)) {
                return false;
            }
        }
        else if (arg == "-i") {
            if (i + 1 >= argc) {
                std::cerr << "Error: -i requires an argument" << std::endl;
                return false;
            }
            config.inputPath = argv[++i];
        }
        else if (arg == "-o") {
            if (i + 1 >= argc) {
                std::cerr << "Error: -o requires an argument" << std::endl;
                return false;
            }
            config.outputPath = argv[++i];
        }
        else if (arg == "-k") {
            if (i + 1 >= argc) {
                std::cerr << "Error: -k requires an argument" << std::endl;
                return false;
            }
            config.key = argv[++i];
        }
        else if (arg[0] == '-' && arg[1] != '-' && arg.length() > 1) {
            // Parse operation flags (can be combined like -ce)
            if (!parseOperations(arg.substr(1), config)) {
                return false;
            }
        }
        else {
            std::cerr << "Error: Unknown argument: " << arg << std::endl;
            return false;
        }
    }
    
    // Validate configuration
    if (config.operations == Operation::NONE) {
        std::cerr << "Error: No operation specified. Use -c, -d, -e, or -u" << std::endl;
        return false;
    }
    
    if (config.inputPath.empty()) {
        std::cerr << "Error: Input path not specified. Use -i" << std::endl;
        return false;
    }
    
    if (config.outputPath.empty()) {
        std::cerr << "Error: Output path not specified. Use -o" << std::endl;
        return false;
    }
    
    // Check if encryption/decryption requires key
    if ((config.operations & Operation::ENCRYPT) || (config.operations & Operation::DECRYPT)) {
        if (config.key.empty()) {
            std::cerr << "Error: Encryption/Decryption requires a key. Use -k" << std::endl;
            return false;
        }
    }
    
    return true;
}

bool ArgParser::parseOperations(const std::string& ops, Config& config) {
    for (char c : ops) {
        switch (c) {
            case 'c':
                config.operations = config.operations | Operation::COMPRESS;
                break;
            case 'd':
                config.operations = config.operations | Operation::DECOMPRESS;
                break;
            case 'e':
                config.operations = config.operations | Operation::ENCRYPT;
                break;
            case 'u':
                config.operations = config.operations | Operation::DECRYPT;
                break;
            default:
                std::cerr << "Error: Unknown operation flag: -" << c << std::endl;
                return false;
        }
    }
    
    // Validate operation combinations
    if ((config.operations & Operation::COMPRESS) && (config.operations & Operation::DECOMPRESS)) {
        std::cerr << "Error: Cannot compress and decompress simultaneously" << std::endl;
        return false;
    }
    
    if ((config.operations & Operation::ENCRYPT) && (config.operations & Operation::DECRYPT)) {
        std::cerr << "Error: Cannot encrypt and decrypt simultaneously" << std::endl;
        return false;
    }
    
    return true;
}

bool ArgParser::parseCompAlg(const std::string& alg, Config& config) {
    std::string algLower = alg;
    std::transform(algLower.begin(), algLower.end(), algLower.begin(), ::tolower);
    
    if (algLower == "huffman") {
        config.compAlg = CompressionAlgorithm::HUFFMAN;
        return true;
    }
    
    std::cerr << "Error: Unknown compression algorithm: " << alg << std::endl;
    std::cerr << "Available: huffman" << std::endl;
    return false;
}

bool ArgParser::parseEncAlg(const std::string& alg, Config& config) {
    std::string algLower = alg;
    std::transform(algLower.begin(), algLower.end(), algLower.begin(), ::tolower);
    
    if (algLower == "aes128" || algLower == "aes") {
        config.encAlg = EncryptionAlgorithm::AES128;
        return true;
    }
    
    std::cerr << "Error: Unknown encryption algorithm: " << alg << std::endl;
    std::cerr << "Available: aes128" << std::endl;
    return false;
}

