#include "utils.hh"

#include <fstream>
#include <sstream>
#include <iostream>

void error(const std::string& msg) {
    std::cout << "error: " << msg << std::endl;
}

void printUsage() {
    std::cout << "usage: adscript <file>" << std::endl;
}

std::string readFile(const std::string& filename) {
    std::ifstream ifstream(filename);
    if (ifstream.bad())
        error("cannot read from file '" + filename + "'");
    std::stringstream sstream;
    sstream << ifstream.rdbuf();
    return sstream.str();
}
