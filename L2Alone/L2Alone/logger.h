#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

class Logger {
public:
    Logger() {
    }

    ~Logger() {
        if (file_.is_open()) {
            file_.close();
        }
    }

    template<typename... Args>
    void log(const std::string& message, Args... args) {
        std::stringstream ss;
        expand(ss, message, args...);
        auto s = ss.str();

        if (file_.is_open()) {
            file_ << s << std::endl;
        }
        std::cout << s << std::endl;
    }

    void open(const std::string& filename) {
        file_.open(filename, std::ios::out);
        if (!file_.is_open()) {
            std::cerr << "Failed to open file: " << filename << std::endl;
        }
    }

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