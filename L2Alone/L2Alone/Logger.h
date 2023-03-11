#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

class Logger {
public:
    
    ~Logger();

    template<typename... Args>
    void log(const std::string& message, Args... args);

    template<typename... Args>
    void warn(const std::string& message, Args... args);
    
    template<typename... Args>
    void error(const std::string& message, Args... args);

    void open(const std::string& filename);

private:
    std::ofstream file_;

    template<typename T>
    void expand(std::stringstream& ss, const T& t) {
        ss << t;
    }

    template<typename T, typename... Args>
    void expand(std::stringstream& ss, const T& t, Args... args) {
        ss << t;
        expand(ss, args...);
    }
};

Logger logger;

template<typename... Args>
void Logger::log(const std::string& message, Args... args) {
    std::stringstream ss;
    expand(ss, message, args...);
    auto s = ss.str();

    if (file_.is_open()) {
        file_ << s << std::endl;
    }
    std::cout << s << std::endl;
}

template<typename... Args>
void Logger::warn(const std::string& message, Args... args) {
    std::stringstream ss;
    expand(ss, message, args...);
    auto s = ss.str();

    if (file_.is_open()) {
        file_ << "WARN:" << s << std::endl;
    }
    std::cout << "WARN:" << s << std::endl;
}

template<typename... Args>
void Logger::error(const std::string& message, Args... args) {
    std::stringstream ss;
    expand(ss, message, args...);
    auto s = ss.str();

    if (file_.is_open()) {
        file_ << "ERROR:" << s << std::endl;
    }
    std::cout << "ERROR:" << s << std::endl;
}

void Logger::open(const std::string& filename) {
    file_.open(filename, std::ios::out);
    if (!file_.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
    }
}

Logger::~Logger() {
    if (file_.is_open()) {
        file_.close();
    }
}