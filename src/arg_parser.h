#ifndef ARG_PARSER_H
#define ARG_PARSER_H

#include "common.h"
#include <string>
#include <vector>

class ArgParser {
public:
    // Parse command line arguments
    static bool parse(int argc, char* argv[], Config& config);
    
    // Print usage information
    static void printUsage(const char* programName);
    
private:
    // Helper functions
    static bool parseOperations(const std::string& ops, Config& config);
    static bool parseCompAlg(const std::string& alg, Config& config);
    static bool parseEncAlg(const std::string& alg, Config& config);
};

#endif // ARG_PARSER_H

