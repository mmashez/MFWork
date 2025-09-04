#pragma once
#include <cstdio>
#include <string>

namespace MF::Print::Internal {

    struct RawIO {
        template <typename T>
        RawIO& operator<<(const T& value) {
            print(value);
            return *this;
        }

        // endline manipulator
        RawIO& operator<<(RawIO&(*manip)(RawIO&)) {
            return manip(*this);
        }

    private:
        // strings
        void print(const std::string& s) { std::printf("%s", s.c_str()); }
        void print(const char* s)        { std::printf("%s", s); }
        void print(char c)               { std::printf("%c", c); }
        void print(bool b)               { std::printf("%s", b ? "true" : "false"); }

        // integers
        void print(short v)              { std::printf("%hd", v); }
        void print(unsigned short v)     { std::printf("%hu", v); }
        void print(int v)                { std::printf("%d", v); }
        void print(unsigned int v)       { std::printf("%u", v); }
        void print(long v)               { std::printf("%ld", v); }
        void print(unsigned long v)      { std::printf("%lu", v); }
        void print(long long v)          { std::printf("%lld", v); }
        void print(unsigned long long v) { std::printf("%llu", v); }

        // floating point
        void print(float f)              { std::printf("%g", f); }
        void print(double d)             { std::printf("%g", d); }
        void print(long double ld)       { std::printf("%Lg", ld); }
    };

    inline RawIO mf_cout; // avoids conflict with std::cout

    inline RawIO& endl(RawIO& out) {
        std::printf("\n");
        return out;
    }

    // -------- safer input --------
    struct RawIOIn {
        template <typename T>
        RawIOIn& operator>>(T& value) {
            if (!read(value)) {
                // reset value on failure to avoid stale data
                value = T{};
            }
            return *this;
        }

    private:
        // integers
        bool read(int& i)                { return std::scanf("%d", &i) == 1; }
        bool read(unsigned int& u)       { return std::scanf("%u", &u) == 1; }
        bool read(long& l)               { return std::scanf("%ld", &l) == 1; }
        bool read(unsigned long& ul)     { return std::scanf("%lu", &ul) == 1; }
        bool read(long long& ll)         { return std::scanf("%lld", &ll) == 1; }
        bool read(unsigned long long& ull){ return std::scanf("%llu", &ull) == 1; }

        // floating point
        bool read(float& f)              { return std::scanf("%f", &f) == 1; }
        bool read(double& d)             { return std::scanf("%lf", &d) == 1; }
        bool read(long double& ld)       { return std::scanf("%Lf", &ld) == 1; }

        // char & bool
        bool read(char& c)               { return std::scanf(" %c", &c) == 1; } // skip whitespace
        bool read(bool& b) {
            int tmp;
            if (std::scanf("%d", &tmp) == 1) { b = (tmp != 0); return true; }
            return false;
        }

        // strings (getline-like)
        bool read(std::string& s) {
            char buf[4096];
            if (std::scanf(" %4095[^\n]", buf) == 1) {
                s = buf;
                return true;
            }
            s.clear();
            return false;
        }
    };

    inline RawIOIn mf_cin; // avoids conflict with std::cin

} // namespace MF::Print::Internal
