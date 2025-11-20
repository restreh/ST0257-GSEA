#include "file_manager.h"
#include <iostream>
#include <cstring>
#include <limits>

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#endif

bool FileManager::readFile(const std::string& path, std::vector<uint8_t>& data) {
#ifdef PLATFORM_WINDOWS
    HANDLE hFile = CreateFileA(
        path.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Error: Cannot open file for reading: " << path << std::endl;
        return false;
    }

    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE) {
        CloseHandle(hFile);
        return false;
    }

    data.resize(fileSize);
    DWORD bytesRead;
    if (!ReadFile(hFile, data.data(), fileSize, &bytesRead, NULL) || bytesRead != fileSize) {
        std::cerr << "Error: Failed to read file: " << path << std::endl;
        CloseHandle(hFile);
        return false;
    }

    CloseHandle(hFile);
    return true;
#else
    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) {
        std::cerr << "Error: Cannot open file for reading: " << path << std::endl;
        return false;
    }

    // Get file size
    struct stat st;
    if (fstat(fd, &st) < 0) {
        close(fd);
        return false;
    }

    size_t fileSize = st.st_size;
    data.resize(fileSize);

    ssize_t totalRead = 0;
    while (totalRead < static_cast<ssize_t>(fileSize)) {
        ssize_t bytesRead = read(fd, data.data() + totalRead, fileSize - totalRead);
        if (bytesRead < 0) {
            std::cerr << "Error: Failed to read file: " << path << std::endl;
            close(fd);
            return false;
        }
        if (bytesRead == 0) break;
        totalRead += bytesRead;
    }

    close(fd);
    return totalRead == static_cast<ssize_t>(fileSize);
#endif
}

bool FileManager::writeFile(const std::string& path, const std::vector<uint8_t>& data) {
#ifdef PLATFORM_WINDOWS
    HANDLE hFile = CreateFileA(
        path.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Error: Cannot open file for writing: " << path << std::endl;
        return false;
    }

    DWORD bytesWritten;
    if (!WriteFile(hFile, data.data(), static_cast<DWORD>(data.size()), &bytesWritten, NULL) || 
        bytesWritten != data.size()) {
        std::cerr << "Error: Failed to write file: " << path << std::endl;
        CloseHandle(hFile);
        return false;
    }

    CloseHandle(hFile);
    return true;
#else
    int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        std::cerr << "Error: Cannot open file for writing: " << path << std::endl;
        return false;
    }

    ssize_t totalWritten = 0;
    while (totalWritten < static_cast<ssize_t>(data.size())) {
        ssize_t bytesWritten = write(fd, data.data() + totalWritten, data.size() - totalWritten);
        if (bytesWritten < 0) {
            std::cerr << "Error: Failed to write file: " << path << std::endl;
            close(fd);
            return false;
        }
        totalWritten += bytesWritten;
    }

    close(fd);
    return totalWritten == static_cast<ssize_t>(data.size());
#endif
}

bool FileManager::isDirectory(const std::string& path) {
#ifdef PLATFORM_WINDOWS
    DWORD attrs = GetFileAttributesA(path.c_str());
    return (attrs != INVALID_FILE_ATTRIBUTES) && (attrs & FILE_ATTRIBUTE_DIRECTORY);
#else
    struct stat st;
    if (stat(path.c_str(), &st) < 0) {
        return false;
    }
    return S_ISDIR(st.st_mode);
#endif
}

std::vector<std::string> FileManager::listFiles(const std::string& dirPath) {
    std::vector<std::string> files;

#ifdef PLATFORM_WINDOWS
    WIN32_FIND_DATAA findData;
    std::string searchPath = dirPath + "\\*";
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);

    if (hFind == INVALID_HANDLE_VALUE) {
        std::cerr << "Error: Cannot open directory: " << dirPath << std::endl;
        return files;
    }

    do {
        std::string fileName = findData.cFileName;
        if (fileName != "." && fileName != "..") {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                files.push_back(combinePath(dirPath, fileName));
            }
        }
    } while (FindNextFileA(hFind, &findData));

    FindClose(hFind);
#else
    DIR* dir = opendir(dirPath.c_str());
    if (!dir) {
        std::cerr << "Error: Cannot open directory: " << dirPath << std::endl;
        return files;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string fileName = entry->d_name;
        if (fileName != "." && fileName != "..") {
            std::string fullPath = combinePath(dirPath, fileName);
            if (!isDirectory(fullPath)) {
                files.push_back(fullPath);
            }
        }
    }

    closedir(dir);
#endif

    return files;
}

bool FileManager::createDirectory(const std::string& path) {
#ifdef PLATFORM_WINDOWS
    return CreateDirectoryA(path.c_str(), NULL) || GetLastError() == ERROR_ALREADY_EXISTS;
#else
    return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
#endif
}

size_t FileManager::getFileSize(const std::string& path) {
#ifdef PLATFORM_WINDOWS
    HANDLE hFile = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, 
                                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return 0;
    }

    LARGE_INTEGER size;
    if (!GetFileSizeEx(hFile, &size) || size.QuadPart < 0) {
        CloseHandle(hFile);
        return 0;
    }

    CloseHandle(hFile);

    if (static_cast<unsigned long long>(size.QuadPart) >
        static_cast<unsigned long long>(std::numeric_limits<size_t>::max())) {
        return 0;
    }
    return static_cast<size_t>(size.QuadPart);
#else
    struct stat st;
    if (stat(path.c_str(), &st) < 0) return 0;
    return st.st_size;
#endif
}

bool FileManager::fileExists(const std::string& path) {
#ifdef PLATFORM_WINDOWS
    DWORD attrs = GetFileAttributesA(path.c_str());
    return attrs != INVALID_FILE_ATTRIBUTES;
#else
    struct stat st;
    return stat(path.c_str(), &st) == 0;
#endif
}

std::string FileManager::getFileName(const std::string& path) {
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos) return path;
    return path.substr(pos + 1);
}

std::string FileManager::getDirectory(const std::string& path) {
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos) return ".";
    return path.substr(0, pos);
}

std::string FileManager::combinePath(const std::string& dir, const std::string& file) {
#ifdef PLATFORM_WINDOWS
    char sep = '\\';
#else
    char sep = '/';
#endif
    
    if (dir.empty()) return file;
    if (file.empty()) return dir;
    
    if (dir.back() == '/' || dir.back() == '\\') {
        return dir + file;
    }
    return dir + sep + file;
}

