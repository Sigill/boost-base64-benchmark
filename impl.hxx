#ifndef BASE64_IMPL
#define BASE64_IMPL

#include <vector>
#include <string>
#include <memory>

template<typename T>
std::string encode_base64(const T* in, size_t sz);

template<typename T>
std::string encode_base64(const std::vector<T>& in);

template<typename T>
std::vector<T> decode_base64(const char* in, size_t sz);

template<typename T>
std::vector<T> decode_base64(const std::string& in);


template<typename T>
std::string encode_base64_2(const T* in, size_t sz);

template<typename T>
std::string encode_base64_2(const std::vector<T>& in);

template<typename T>
std::vector<T> decode_base64_2(const char* in, size_t sz);

template<typename T>
std::vector<T> decode_base64_2(const std::string& in);


template<typename T>
struct free_deleter
{
  void operator()(T* p);
};

template<typename T>
std::unique_ptr<char, free_deleter<char>> encode_base64_rfc(const T* in, size_t sz);

template<typename T>
std::unique_ptr<char, free_deleter<char>> encode_base64_rfc(const std::vector<T>& in);

template<typename T>
std::vector<T> decode_base64_rfc(const char* in, size_t sz);

template<typename T>
std::vector<T> decode_base64_rfc(const std::string& in);

#endif
