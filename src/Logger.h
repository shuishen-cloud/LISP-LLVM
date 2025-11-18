#ifndef LOGGER_H
#define LOGGER_H

#include <cstdlib>
#include <iostream>
#include <sstream>

class ErrorLogMessage : public std::basic_ostringstream<char> {
  public:
    ~ErrorLogMessage() {
        // str().c_str() 是什么意思
        std::cerr << "Fatal error: " << str().c_str();
        exit(EXIT_FAILURE);
    }
};

#define DIE ErrorLogMessage()

#endif
