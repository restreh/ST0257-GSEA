#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "common.h"
#include <vector>
#include <string>

class FileManager {
public:
    // Read entire file using system calls
    static bool readFile(const std::string& path, std::vector<uint8_t>& data);
    
    // Write data to file using system calls
    static bool writeFile(const std::string& path, const std::vector<uint8_t>& data);
    
    // Check if path is a directory
    static bool isDirectory(const std::string& path);
    
    // List all files in a directory (non-recursive)
    static std::vector<std::string> listFiles(const std::string& dirPath);
    
    // Create directory if it doesn't exist
    static bool createDirectory(const std::string& path);
    
    // Get file size
    static size_t getFileSize(const std::string& path);
    
    // Check if file exists
    static bool fileExists(const std::string& path);
    
    // Extract filename from path
    static std::string getFileName(const std::string& path);
    
    // Get directory from path
    static std::string getDirectory(const std::string& path);
    
    // Combine paths
    static std::string combinePath(const std::string& dir, const std::string& file);

private:
#ifdef PLATFORM_WINDOWS
    static HANDLE openFileForReading(const std::string& path);
    static HANDLE openFileForWriting(const std::string& path);
#else
    static int openFileForReading(const std::string& path);
    static int openFileForWriting(const std::string& path);
#endif
};

#endif // FILE_MANAGER_H

