#include "worker.h"
#include "file_manager.h"
#include "huffman.h"
#include "aes.h"
#include <iostream>
#include <vector>
#include <cstring>

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#else
#include <pthread.h>
#endif

bool Worker::processFile(const std::string& inputPath, 
                        const std::string& outputPath,
                        const Config& config) {
    std::cout << "Processing: " << inputPath << " -> " << outputPath << std::endl;
    
    // Read input file
    std::vector<uint8_t> data;
    if (!FileManager::readFile(inputPath, data)) {
        std::cerr << "Error: Failed to read file: " << inputPath << std::endl;
        return false;
    }
    
    if (data.empty()) {
        std::cerr << "Warning: Input file is empty: " << inputPath << std::endl;
        // Create empty output file
        return FileManager::writeFile(outputPath, data);
    }
    
    // Apply operations
    if (!applyOperations(data, config)) {
        std::cerr << "Error: Failed to process file: " << inputPath << std::endl;
        return false;
    }
    
    // Write output file
    if (!FileManager::writeFile(outputPath, data)) {
        std::cerr << "Error: Failed to write file: " << outputPath << std::endl;
        return false;
    }
    
    std::cout << "Successfully processed: " << inputPath << std::endl;
    return true;
}

bool Worker::applyOperations(std::vector<uint8_t>& data, const Config& config) {
    std::vector<uint8_t> tempData;
    
    // Check for compression operation
    if (config.operations & Operation::COMPRESS) {
        std::cout << "  Compressing with Huffman..." << std::endl;
        if (!HuffmanCompressor::compress(data, tempData)) {
            std::cerr << "Error: Compression failed" << std::endl;
            return false;
        }
        data = std::move(tempData);
    }
    
    // Check for encryption operation
    if (config.operations & Operation::ENCRYPT) {
        std::cout << "  Encrypting with AES-128..." << std::endl;
        if (!AES::encrypt(data, config.key, tempData)) {
            std::cerr << "Error: Encryption failed" << std::endl;
            return false;
        }
        data = std::move(tempData);
    }
    
    // Check for decryption operation
    if (config.operations & Operation::DECRYPT) {
        std::cout << "  Decrypting with AES-128..." << std::endl;
        if (!AES::decrypt(data, config.key, tempData)) {
            std::cerr << "Error: Decryption failed" << std::endl;
            return false;
        }
        data = std::move(tempData);
    }
    
    // Check for decompression operation
    if (config.operations & Operation::DECOMPRESS) {
        std::cout << "  Decompressing with Huffman..." << std::endl;
        if (!HuffmanCompressor::decompress(data, tempData)) {
            std::cerr << "Error: Decompression failed" << std::endl;
            return false;
        }
        data = std::move(tempData);
    }
    
    return true;
}

#ifdef PLATFORM_WINDOWS
DWORD WINAPI Worker::threadWorker(LPVOID param) {
    WorkerTask* task = static_cast<WorkerTask*>(param);
    task->success = processFile(task->inputPath, task->outputPath, task->config);
    if (!task->success) {
        task->errorMessage = "Failed to process file: " + task->inputPath;
    }
    return 0;
}
#else
void* Worker::threadWorker(void* param) {
    WorkerTask* task = static_cast<WorkerTask*>(param);
    task->success = processFile(task->inputPath, task->outputPath, task->config);
    if (!task->success) {
        task->errorMessage = "Failed to process file: " + task->inputPath;
    }
    return nullptr;
}
#endif

bool Worker::processFilesParallel(const std::vector<FileInfo>& files, const Config& config) {
    if (files.empty()) {
        std::cout << "No files to process." << std::endl;
        return true;
    }
    
    std::cout << "Processing " << files.size() << " file(s) in parallel..." << std::endl;
    
    // Create tasks
    std::vector<WorkerTask> tasks(files.size());
    for (size_t i = 0; i < files.size(); ++i) {
        tasks[i].inputPath = files[i].inputPath;
        tasks[i].outputPath = files[i].outputPath;
        tasks[i].config = config;
        tasks[i].success = false;
    }
    
#ifdef PLATFORM_WINDOWS
    // Windows threads
    std::vector<HANDLE> threads(files.size());
    
    // Create threads
    for (size_t i = 0; i < files.size(); ++i) {
        threads[i] = CreateThread(
            NULL,
            0,
            threadWorker,
            &tasks[i],
            0,
            NULL
        );
        
        if (threads[i] == NULL) {
            std::cerr << "Error: Failed to create thread for " << files[i].inputPath << std::endl;
            
            // Wait for already created threads
            for (size_t j = 0; j < i; ++j) {
                WaitForSingleObject(threads[j], INFINITE);
                CloseHandle(threads[j]);
            }
            return false;
        }
    }
    
    // Wait for all threads
    WaitForMultipleObjects(static_cast<DWORD>(threads.size()), threads.data(), TRUE, INFINITE);
    
    // Clean up
    for (auto& handle : threads) {
        CloseHandle(handle);
    }
#else
    // POSIX threads
    std::vector<pthread_t> threads(files.size());
    
    // Create threads
    for (size_t i = 0; i < files.size(); ++i) {
        int result = pthread_create(&threads[i], NULL, threadWorker, &tasks[i]);
        
        if (result != 0) {
            std::cerr << "Error: Failed to create thread for " << files[i].inputPath << std::endl;
            
            // Wait for already created threads
            for (size_t j = 0; j < i; ++j) {
                pthread_join(threads[j], NULL);
            }
            return false;
        }
    }
    
    // Wait for all threads
    for (auto& thread : threads) {
        pthread_join(thread, NULL);
    }
#endif
    
    // Check results
    bool allSuccess = true;
    for (const auto& task : tasks) {
        if (!task.success) {
            std::cerr << "Error: " << task.errorMessage << std::endl;
            allSuccess = false;
        }
    }
    
    if (allSuccess) {
        std::cout << "All files processed successfully!" << std::endl;
    } else {
        std::cerr << "Some files failed to process." << std::endl;
    }
    
    return allSuccess;
}

