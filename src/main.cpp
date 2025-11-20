#include "common.h"
#include "arg_parser.h"
#include "file_manager.h"
#include "worker.h"
#include <iostream>
#include <vector>

int main(int argc, char* argv[]) {
    // Parse command line arguments
    Config config;
    if (!ArgParser::parse(argc, argv, config)) {
        return 1;
    }
    
    std::cout << "=== GSEA - Gestión Segura y Eficiente de Archivos ===" << std::endl;
    std::cout << std::endl;
    
    // Print configuration
    std::cout << "Configuration:" << std::endl;
    std::cout << "  Input:  " << config.inputPath << std::endl;
    std::cout << "  Output: " << config.outputPath << std::endl;
    
    std::cout << "  Operations: ";
    if (config.operations & Operation::COMPRESS) std::cout << "Compress ";
    if (config.operations & Operation::DECOMPRESS) std::cout << "Decompress ";
    if (config.operations & Operation::ENCRYPT) std::cout << "Encrypt ";
    if (config.operations & Operation::DECRYPT) std::cout << "Decrypt ";
    std::cout << std::endl;
    
    if (config.operations & Operation::COMPRESS) {
        std::cout << "  Compression: Huffman" << std::endl;
    }
    if ((config.operations & Operation::ENCRYPT) || (config.operations & Operation::DECRYPT)) {
        std::cout << "  Encryption: AES-128" << std::endl;
    }
    std::cout << std::endl;
    
    // Check if input exists
    if (!FileManager::fileExists(config.inputPath)) {
        std::cerr << "Error: Input path does not exist: " << config.inputPath << std::endl;
        return 1;
    }
    
    // Determine if input is a file or directory
    bool isInputDir = FileManager::isDirectory(config.inputPath);
    
    std::vector<FileInfo> filesToProcess;
    
    if (isInputDir) {
        // Process directory
        std::cout << "Input is a directory. Listing files..." << std::endl;
        
        auto files = FileManager::listFiles(config.inputPath);
        if (files.empty()) {
            std::cout << "No files found in directory." << std::endl;
            return 0;
        }
        
        std::cout << "Found " << files.size() << " file(s)" << std::endl;
        
        // Create output directory if it doesn't exist
        if (!FileManager::fileExists(config.outputPath)) {
            if (!FileManager::createDirectory(config.outputPath)) {
                std::cerr << "Error: Failed to create output directory: " << config.outputPath << std::endl;
                return 1;
            }
        } else if (!FileManager::isDirectory(config.outputPath)) {
            std::cerr << "Error: Output path exists but is not a directory: " << config.outputPath << std::endl;
            return 1;
        }
        
        // Prepare file list
        for (const auto& inputFile : files) {
            FileInfo info;
            info.inputPath = inputFile;
            
            std::string fileName = FileManager::getFileName(inputFile);
            // Normalize/remove extensions according to the operations
            auto removeSuffix = [](std::string& name, const std::string& suffix) {
                if (name.size() >= suffix.size() &&
                    name.compare(name.size() - suffix.size(), suffix.size(), suffix) == 0) {
                    name.erase(name.size() - suffix.size());
                }
            };

            if (config.operations & Operation::DECRYPT) {
                removeSuffix(fileName, ".aes");
            }
            if (config.operations & Operation::DECOMPRESS) {
                removeSuffix(fileName, ".huf");
            }
            
            // Add appropriate extension(s) for forward operations
            if (config.operations & Operation::COMPRESS) {
                fileName += ".huf";
            }
            if (config.operations & Operation::ENCRYPT) {
                fileName += ".aes";
            }
            
            info.outputPath = FileManager::combinePath(config.outputPath, fileName);
            info.isDirectory = false;
            
            filesToProcess.push_back(info);
        }
        
        std::cout << std::endl;
        
        // Process files in parallel
        if (!Worker::processFilesParallel(filesToProcess, config)) {
            std::cerr << "Error: Some files failed to process" << std::endl;
            return 1;
        }
    } else {
        // Process single file
        std::cout << "Input is a file." << std::endl;
        std::cout << std::endl;
        
        if (!Worker::processFile(config.inputPath, config.outputPath, config)) {
            std::cerr << "Error: Failed to process file" << std::endl;
            return 1;
        }
    }
    
    std::cout << std::endl;
    std::cout << "=== Processing Complete ===" << std::endl;
    
    return 0;
}

