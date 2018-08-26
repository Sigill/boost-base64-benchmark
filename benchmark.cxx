#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/vector.hpp>

#include <iostream>
#include <chrono>
#include <random>
#include <type_traits>

#include "impl.hxx"

using namespace std::chrono;

template<typename T = milliseconds>
unsigned long elapsed(const steady_clock::time_point& begin,
                      const steady_clock::time_point& end) {
  return duration_cast<T>(end - begin).count();
}

template<typename T, typename std::enable_if<std::is_integral<T>::value, T>::type* = nullptr>
std::vector<T> random_vector(size_t size) {
  std::uniform_int_distribution<T> distribution(std::numeric_limits<T>::lowest(), std::numeric_limits<T>::max());
  std::default_random_engine generator;

  std::vector<T> data(size);
  std::generate(data.begin(), data.end(), [&distribution, &generator]() { return distribution(generator); });
  return data;
}

template<typename T, typename std::enable_if<std::is_floating_point<T>::value, T>::type* = nullptr>
std::vector<T> random_vector(size_t size) {
  std::uniform_real_distribution<T> distribution(-1, 1);
  std::default_random_engine generator;

  std::vector<T> data(size);
  std::generate(data.begin(), data.end(), [&distribution, &generator]() { return distribution(generator); });
  return data;
}

template<typename OArchive, typename IArchive, typename T>
void benchmark_boost_archive(const std::vector<T>& in) {
  std::stringstream ss;

  auto t0 = steady_clock::now();

  OArchive oa(ss);
  oa << boost::serialization::make_nvp("data", in);

  auto t1 = steady_clock::now();

  std::vector<T> out;
  IArchive ia(ss);
  ia >> boost::serialization::make_nvp("data", out);

  auto t2 = steady_clock::now();

  if (in != out) throw std::runtime_error("Mismatch");

  std::cout << __PRETTY_FUNCTION__ << std::endl;
  std::cout << "Content size: " << ss.tellp() << std::endl;
  std::cout << "Write time: " << elapsed(t0, t1) << std::endl;
  std::cout << "Read time : " << elapsed(t1, t2) << std::endl;
}

template<typename T>
void benchmark_boost_archive() {
  const std::vector<T> in = random_vector<T>(1000000);
  benchmark_boost_archive<boost::archive::binary_oarchive, boost::archive::binary_iarchive, T>(in);
  benchmark_boost_archive<boost::archive::text_oarchive, boost::archive::text_iarchive, T>(in);
  benchmark_boost_archive<boost::archive::xml_oarchive, boost::archive::xml_iarchive, T>(in);
}

void benchmark_boost_archive() {
  benchmark_boost_archive<char>();
  benchmark_boost_archive<unsigned short>();
  benchmark_boost_archive<int>();
  benchmark_boost_archive<long>();
  benchmark_boost_archive<float>();
  benchmark_boost_archive<double>();
}

template<typename T>
void benchmark_base64_boost_raw(const std::vector<T> &in) {
  auto t0 = steady_clock::now();

  auto encoded = encode_base64(in);

  auto t1 = steady_clock::now();

  auto decoded = decode_base64<T>(encoded);

  auto t2 = steady_clock::now();

  if (in != decoded) throw std::runtime_error("Mismatch");

  std::cout << __PRETTY_FUNCTION__ << std::endl;
  std::cout << "Content size: " << encoded.size() << std::endl;
  std::cout << "Write time: " << elapsed(t0, t1) << std::endl;
  std::cout << "Read time : " << elapsed(t1, t2) << std::endl;
}

template<typename T>
void benchmark_base64_boost_typed(const std::vector<T> &in) {
  auto t0 = steady_clock::now();

  auto encoded = encode_base64_2(in);

  auto t1 = steady_clock::now();

  auto decoded = decode_base64_2<T>(encoded);

  auto t2 = steady_clock::now();

  if (in != decoded) throw std::runtime_error("Mismatch");

  std::cout << __PRETTY_FUNCTION__ << std::endl;
  std::cout << "Content size: " << encoded.size() << std::endl;
  std::cout << "Write time: " << elapsed(t0, t1) << std::endl;
  std::cout << "Read time : " << elapsed(t1, t2) << std::endl;
}

template<typename T>
void benchmark_base64_rfc(const std::vector<T> &in) {
  auto t0 = steady_clock::now();

  auto encoded = encode_base64_rfc(in);

  auto t1 = steady_clock::now();

  auto decoded = decode_base64_rfc<T>(encoded.get(), strlen(encoded.get()));

  auto t2 = steady_clock::now();

  if (in != decoded) throw std::runtime_error("Mismatch");

  std::cout << __PRETTY_FUNCTION__ << std::endl;
  std::cout << "Content size: " << strlen(encoded.get()) << std::endl;
  std::cout << "Write time: " << elapsed(t0, t1) << std::endl;
  std::cout << "Read time : " << elapsed(t1, t2) << std::endl;
}

template<typename T, typename std::enable_if<std::is_integral<T>::value, T>::type* = nullptr>
void benchmark_base64() {
  const std::vector<T> in = random_vector<T>(1000000);
  benchmark_base64_boost_raw(in);
  benchmark_base64_boost_typed(in);
  benchmark_base64_rfc(in);
}

template<typename T, typename std::enable_if<std::is_floating_point<T>::value, T>::type* = nullptr>
void benchmark_base64() {
  const std::vector<T> in = random_vector<T>(1000000);
  benchmark_base64_boost_raw(in);
  benchmark_base64_rfc(in);
}

void benchmark_base64() {
  benchmark_base64<char>();
  benchmark_base64<unsigned short>();
  benchmark_base64<int>();
  benchmark_base64<long>();
  benchmark_base64<float>();
  benchmark_base64<double>();
}

int main(int argc, char** argv)
{
  benchmark_boost_archive();

  benchmark_base64();

  return 0;
}
