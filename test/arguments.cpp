#include <compressor.hpp>

int main(int argc, char const *argv[])
{
    const char* args[] = {
        "-f", 
        "C:\\Migue\\GitHub\\file-compressor\\res\\hellow.txt",
        "-o",
        "C:\\Migue\\GitHub\\file-compressor\\res\\hellow.cmp",
    };

    program(args);

    const char* other_args[] = {
        "-f", 
        "C:\\Migue\\GitHub\\file-compressor\\res\\hellow.cmp",
        "-o",
        "C:\\Migue\\GitHub\\file-compressor\\res\\hellow-cmp.txt",
        "-u",
        "please"
    };

    program(other_args);
    
    return 0;
}
