#include <iostream>

int main() {
    std::cout << "__cplusplus value: " << __cplusplus << std::endl;

    #if __cplusplus == 201103L
        std::cout << "C++11 is being used" << std::endl;
    #elif __cplusplus == 201402L
        std::cout << "C++14 is being used" << std::endl;
    #elif __cplusplus == 201703L
        std::cout << "C++17 is being used" << std::endl;
    #elif __cplusplus == 202002L
        std::cout << "C++20 is being used" << std::endl;
    #else
        std::cout << "Unknown or pre-C++11 standard is being used" << std::endl;
    #endif
    return 0;
}
