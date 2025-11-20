#ifndef WORKER_H
#define WORKER_H

#include "common.h"
#include <vector>
#include <string>

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#else
#include <pthread.h>
#endif

// Worker task structure
struct WorkerTask {
    std::string inputPath;
    std::string outputPath;
    Config config;
    bool success;
    std::string errorMessage;
};

class Worker {
public:
    // Process a single file with given configuration
    static bool processFile(const std::string& inputPath, 
                           const std::string& outputPath,
                           const Config& config);
    
    // Process multiple files concurrently
    static bool processFilesParallel(const std::vector<FileInfo>& files, const Config& config);

private:
    // Thread worker function
#ifdef PLATFORM_WINDOWS
    static DWORD WINAPI threadWorker(LPVOID param);
#else
    static void* threadWorker(void* param);
#endif

    // Process operations on data
    static bool applyOperations(std::vector<uint8_t>& data, const Config& config);
};

#endif // WORKER_H

