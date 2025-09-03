#pragma once
#include <cstdio>
#include <string>

namespace MF::Print::Internal {
    struct RawIO {
        // handle printing anything that can convert to string / primitive types
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
        void print(const std::string& s) { std::printf("%s", s.c_str()); }
        void print(const char* s) { std::printf("%s", s); }
        void print(char c) { std::printf("%c", c); }
        void print(bool b) { std::printf("%s", b ? "true" : "false"); }
        void print(int i) { std::printf("%d", i); }
        void print(unsigned int u) { std::printf("%u", u); }
        void print(long l) { std::printf("%ld", l); }
        void print(unsigned long ul) { std::printf("%lu", ul); }
        void print(double d) { std::printf("%f", d); }
    };

    // global instance for output
    inline RawIO cout;

    // endl manipulator
    inline RawIO& endl(RawIO& out) {
        std::printf("\n");
        return out;
    }

    // --------- cin-like input ---------
    struct RawIOIn {
        template <typename T>
        RawIOIn& operator>>(T& value) {
            read(value);
            return *this;
        }

    private:
        void read(int& i) { std::scanf("%d", &i); }
        void read(unsigned int& u) { std::scanf("%u", &u); }
        void read(long& l) { std::scanf("%ld", &l); }
        void read(unsigned long& ul) { std::scanf("%lu", &ul); }
        void read(double& d) { std::scanf("%lf", &d); }
        void read(char& c) { std::scanf(" %c", &c); } // note leading space to skip whitespace
        void read(bool& b) { int tmp; std::scanf("%d", &tmp); b = (tmp != 0); }
        void read(std::string& s) {
            char buf[1024];
            std::scanf("%1023s", buf);
            s = buf;
        }
    };

    // global instance for input
    inline RawIOIn cin;
}
