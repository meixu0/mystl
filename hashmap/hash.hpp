#ifndef HASH_HPP
#define HASH_HPP
#include <cstddef>
#include <cstdlib>
#include <cmath>
#include <string>
#include <stdexcept>
namespace my_std {
const double num = (sqrt(5) - 1) / 2;
template <typename T> struct hash{
    size_t operator() (T) const{
        std::invalid_argument("invalid argument");

    }
};
template<> struct hash<int>{
    size_t operator() (int key) const {
        return (size_t)(floor(key * num));
    }
};
template<> struct hash<std::string>{
    const int B = 131, P = 9982443;
    size_t operator() (std::string key) const {
        size_t h = 0;
        for(auto ch : key){
            h = (h * B + ch) % P;
        }
        return size_t(h);
    }
};
}
#endif