#include "impl.hxx"

#include <string.h>

#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>

#define RESTRICT
#include "base64.h"

  template<typename T>
std::string encode_base64(const T* in, size_t sz)
{
  return encode_base64(reinterpret_cast<const char*>(in), sz * sizeof(T));
}

  template<>
std::string encode_base64<char>(const char* in, size_t sz)
{
  namespace bai = boost::archive::iterators;
  using b64_encoder = bai::base64_from_binary<bai::transform_width<const char*, 6, 8> >;
  std::string out;
  out.reserve(1 + (((sz) + 2) / 3) * 4);
  out.assign(b64_encoder(in), b64_encoder(in + sz));

  unsigned int writePaddChars = (3 - sz % 3) % 3;
  out.append(writePaddChars, '=');

  return out;
}

  template<typename T>
std::string encode_base64(const std::vector<T>& in)
{
  return encode_base64(reinterpret_cast<const char*>(in.data()), in.size() * sizeof(T));
}

  template<typename T>
std::vector<T> decode_base64(const char* in, size_t sz)
{
  namespace bai = boost::archive::iterators;
  using b64_decoder = bai::transform_width< bai::binary_from_base64<const char*>, 8, 6 >;

  size_t padding = 0;

  if (sz > 0 && in[sz - 1] == '=') {
    --sz;
    ++padding;
  }
  if (sz > 0 && in[sz - 1] == '=') {
    --sz;
    ++padding;
  }

  std::vector<char> out;
  out.reserve(3 * (sz / 4) + 2);
  out.assign(b64_decoder(in), b64_decoder(in + sz));

  if (out.size() % sizeof(T) != 0)
    throw std::runtime_error("Invalid amount of bytes to build an array of T");

  std::vector<T> out2(out.size() / sizeof(T));
  memcpy(out2.data(), out.data(), out.size());

  return out2;
}

  template<typename T>
std::vector<T> decode_base64(const std::string& in)
{
  return decode_base64<T>(in.data(), in.size());
}


  template<typename T>
std::string encode_base64_2(const T* in, size_t sz)
{
  namespace bai = boost::archive::iterators;
  using b64_encoder = bai::base64_from_binary<bai::transform_width<const T*, 6, sizeof(T) * 8>, char>;
  std::string out;
  out.reserve(1 + (((sz * sizeof(T)) + 2) / 3) * 4);
  out.assign(b64_encoder(in), b64_encoder(in + sz));

  unsigned int writePaddChars = (3 - (sz * sizeof(T)) % 3) % 3;
  out.append(writePaddChars, '=');

  return out;
}

  template<typename T>
std::string encode_base64_2(const std::vector<T>& in)
{
  return encode_base64_2(in.data(), in.size());
}

  template<typename T>
std::vector<T> decode_base64_2(const char* in, size_t sz)
{
  namespace bai = boost::archive::iterators;
  using b64_decoder = bai::transform_width< bai::binary_from_base64<const char*>, 8 * sizeof(T), 6 , T>;

  size_t padding = 0;

  if (sz > 0 && in[sz - 1] == '=') {
    --sz;
    ++padding;
  }
  if (sz > 0 && in[sz - 1] == '=') {
    --sz;
    ++padding;
  }

  std::vector<T> out;
  out.reserve(3 * (sz / sizeof(T) / 4) + 2);
  out.assign(b64_decoder(in), b64_decoder(in + sz));

  return out;
}

  template<typename T>
std::vector<T> decode_base64_2(const std::string& in)
{
  return decode_base64_2<T>(in.data(), in.size());
}

template<typename T>
void free_deleter<T>::operator()(T* p) { free(p); }


  template<typename T>
std::unique_ptr<char, free_deleter<char>> encode_base64_rfc(const T* in, size_t sz)
{
  return encode_base64_rfc(reinterpret_cast<const char*>(in), sz * sizeof(T));
}

  template<>
std::unique_ptr<char, free_deleter<char>> encode_base64_rfc(const char* in, size_t sz)
{
  char* encoded = nullptr;
  size_t encoded_size = base64_encode_alloc(in, sz, &encoded);

  if (encoded == nullptr && encoded_size == 0 && sz != 0)
    throw std::runtime_error("Input too long");

  if (encoded == nullptr)
    throw std::runtime_error("Memory allocation failed");

  return std::unique_ptr<char, free_deleter<char>>(encoded);
}

  template<typename T>
std::unique_ptr<char, free_deleter<char>> encode_base64_rfc(const std::vector<T>& in)
{
  return encode_base64_rfc(in.data(), in.size());
}

  template<typename T>
std::vector<T> decode_base64_rfc(const char* in, size_t sz)
{
  size_t decoded_size = 0;
  char* decoded = nullptr;
  bool ok = base64_decode_alloc(in, sz, &decoded, &decoded_size);

  if (!ok)
    throw std::runtime_error("Input was not base64 encoded");

  if (decoded == nullptr)
    throw std::runtime_error("Memory allocation failed");

  if (decoded_size % sizeof(T) != 0)
    throw std::runtime_error("Invalid amount of data to build an array of T");

  std::vector<T> _decoded(decoded_size / sizeof(T), 0);
  memcpy(_decoded.data(), decoded, decoded_size);
  free(decoded);

  return _decoded;
}

  template<typename T>
std::vector<T> decode_base64_rfc(const std::string& in)
{
  return decode_base64_rfc<T>(in.data(), in.size());
}

#define IMPL(type) \
template std::string encode_base64<type>(const type* in, size_t sz); \
template std::string encode_base64<type>(const std::vector<type>& in); \
template std::vector<type> decode_base64<type>(const char* in, size_t sz); \
template std::vector<type> decode_base64<type>(const std::string& in);

IMPL(char)
IMPL(unsigned short)
IMPL(int)
IMPL(long)
IMPL(float)
IMPL(double)

#define IMPL2(type) \
template std::string encode_base64_2<type>(const type* in, size_t sz); \
template std::string encode_base64_2<type>(const std::vector<type>& in); \
template std::vector<type> decode_base64_2<type>(const char* in, size_t sz); \
template std::vector<type> decode_base64_2<type>(const std::string& in);

IMPL2(char)
IMPL2(unsigned short)
IMPL2(int)
IMPL2(long)
// Does not work on floating types

template struct free_deleter<char>;

#define IMPL_RFC(type) \
template std::unique_ptr<char, free_deleter<char>> encode_base64_rfc<type>(const type* in, size_t sz); \
template std::unique_ptr<char, free_deleter<char>> encode_base64_rfc<type>(const std::vector<type>& in); \
template std::vector<type> decode_base64_rfc<type>(const char* in, size_t sz); \
template std::vector<type> decode_base64_rfc<type>(const std::string& in);

IMPL_RFC(char)
IMPL_RFC(unsigned short)
IMPL_RFC(int)
IMPL_RFC(long)
IMPL_RFC(float)
IMPL_RFC(double)

