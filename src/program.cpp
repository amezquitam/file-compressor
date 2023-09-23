#include <compressor.hpp>

int main(int argc, char const* argv[]) {
    program(std::span(argv, argc));
    return 0;
}