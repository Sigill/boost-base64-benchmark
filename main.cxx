#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/vector.hpp>

#define RESTRICT
#include "base64.h"

#include "impl.hxx"

#include <sstream>
#include <iostream>
#include <chrono>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <iterator>
#include <limits>
#include <memory>
#include <random>
#include <type_traits>

#include <gmock/gmock.h>

using namespace std::chrono;

template<typename TypeT = milliseconds>
auto elapsed(const steady_clock::time_point& begin,
             const steady_clock::time_point& end) {
  return duration_cast<TypeT>(end - begin).count();
}

  template<typename T, typename std::enable_if<std::is_integral<T>::value, T>::type* = nullptr>
std::vector<T> random_vector(size_t size)
{
  std::uniform_int_distribution<T> distribution(std::numeric_limits<T>::lowest(), std::numeric_limits<T>::max());
  std::default_random_engine generator;

  std::vector<T> data(size);
  std::generate(data.begin(), data.end(), [&distribution, &generator]() { return distribution(generator); });
  return data;
}

  template<typename T, typename std::enable_if<std::is_floating_point<T>::value, T>::type* = nullptr>
std::vector<T> random_vector(size_t size)
{
  std::uniform_real_distribution<T> distribution(-1, 1);
  std::default_random_engine generator;

  std::vector<T> data(size);
  std::generate(data.begin(), data.end(), [&distribution, &generator]() { return distribution(generator); });
  return data;
}

template<typename T>
class VectorSerializationTest : public ::testing::TestWithParam<std::pair<std::vector<T>, std::string>> {
public:
  using super = typename VectorSerializationTest::TestWithParam;
  VectorSerializationTest() : plain(std::get<0>(super::GetParam())), encoded(std::get<1>(super::GetParam())) {}
  const std::vector<T> plain;
  const std::string encoded;
};


class s8VectorBase64Serialization : public VectorSerializationTest<char> {};

TEST_P(s8VectorBase64Serialization, Boost) {
  EXPECT_EQ(encode_base64(plain), encoded);
  EXPECT_EQ(decode_base64<char>(encoded), plain);
}

TEST_P(s8VectorBase64Serialization, TypedBoost) {
  EXPECT_EQ(encode_base64_2(plain), encoded);
  EXPECT_EQ(decode_base64_2<char>(encoded), plain);
}

TEST_P(s8VectorBase64Serialization, RFC) {
  ASSERT_EQ(encode_base64_rfc(plain).get(), encoded);
  ASSERT_EQ(decode_base64_rfc<char>(encoded), plain);
}

std::vector<std::pair<std::vector<char>, std::string>> base64_s8_cases = {
  {{}, ""}, 
  {{'f'}, "Zg=="},
  {{'f', 'o'}, "Zm8="},
  {{'f', 'o', 'o'}, "Zm9v"},
  {{'f', 'o', 'o', 'b'}, "Zm9vYg=="},
  {{'f', 'o', 'o', 'b', 'a'}, "Zm9vYmE="},
  {{'f', 'o', 'o', 'b', 'a', 'r'}, "Zm9vYmFy"}};

INSTANTIATE_TEST_CASE_P(_, s8VectorBase64Serialization, ::testing::ValuesIn(base64_s8_cases));



class u16VectorBase64RawSerialization : public VectorSerializationTest<unsigned short> {};

TEST_P(u16VectorBase64RawSerialization, Boost) {
  EXPECT_EQ(encode_base64(plain), encoded);
  EXPECT_THAT(decode_base64<unsigned short>(encoded), plain);
}

TEST_P(u16VectorBase64RawSerialization, RFC) {
  ASSERT_EQ(encoded, encode_base64_rfc(plain).get());
  ASSERT_EQ(plain, decode_base64_rfc<unsigned short>(encoded));
}

std::vector<std::pair<std::vector<unsigned short>, std::string>> base64_u16_cases = {
  {{},                  ""},
  {{0},                 "AAA="},
  {{127},               "fwA="},     // 011111|11 0000|0000 on little endian
  {{16255},             "fz8="},     // 011111|11 0011|1111
  {{16255, 127},        "fz9/AA=="}, // 011111|11 0011|1111 01|111111 000000|00
  {{16255, 7999},       "fz8/Hw=="}, // 011111|11 0011|1111 00|111111 000111|11
  {{16255, 7999, 127},  "fz8/H38A"}, // 011111|11 0011|1111 00|111111 000111|11 0111|1111 00|000000
  {{16255, 7999, 3871}, "fz8/Hx8P"}  // 011111|11 0011|1111 00|111111 000111|11 0001|1111 00|001111
};

INSTANTIATE_TEST_CASE_P(Raw, u16VectorBase64RawSerialization, ::testing::ValuesIn(base64_u16_cases));



class u16VectorBase64TypedSerialization : public VectorSerializationTest<unsigned short> {};

TEST_P(u16VectorBase64TypedSerialization, TypedBoost) {
  EXPECT_EQ(encode_base64_2(plain), encoded);

  EXPECT_THAT(decode_base64_2<unsigned short>(encoded), plain);
}

std::vector<std::pair<std::vector<unsigned short>, std::string>> endian_safe_base64_u16_cases = {
  {{},                  ""},
  {{0},                 "AAA="},
  {{127},               "AH8="},     // 000000|00 0111|1111
  {{16255},             "P38="},     // 001111|11 0111|1111
  {{16255, 127},        "P38Afw=="}, // 001111|11 0111|1111 00|000000 011111|11
  {{16255, 7999},       "P38fPw=="}, // 001111|11 0111|1111 00|011111 001111|11
  {{16255, 7999, 127},  "P38fPwB/"}, // 001111|11 0111|1111 00|011111 001111|11 0000|0000 01|111111
  {{16255, 7999, 3871}, "P38fPw8f"}  // 001111|11 0111|1111 00|011111 001111|11 0000|1111 00|011111
};

INSTANTIATE_TEST_CASE_P(_, u16VectorBase64TypedSerialization, ::testing::ValuesIn(endian_safe_base64_u16_cases));



template <typename T>
class Benchmark : public ::testing::Test {
public:
  using value_type = T;
  Benchmark() : in(random_vector<value_type>(1000000)), out() {}
  const std::vector<value_type> in;
  std::vector<value_type> out;
};

using PrimitiveValueTypes = ::testing::Types<char, unsigned short, int, float, double>;
using IntegralValueTypes = ::testing::Types<char, unsigned short, int>;


template <typename T>
class BoostRawMemoryBase64Benchmark : public Benchmark<T> {};

TYPED_TEST_CASE(BoostRawMemoryBase64Benchmark, PrimitiveValueTypes);

TYPED_TEST(BoostRawMemoryBase64Benchmark, _)
{
  using value_type = typename TestFixture::value_type;

  auto t0 = steady_clock::now();

  auto encoded = encode_base64(this->in);

  auto t1 = steady_clock::now();

  auto decoded = decode_base64<value_type>(encoded);

  auto t2 = steady_clock::now();

  EXPECT_EQ(this->in, decoded);

  std::cout << "Content size: " << encoded.size() << std::endl;

  std::cout << "Write time: " << elapsed(t0, t1) << std::endl;
  std::cout << "Read time : " << elapsed(t1, t2) << std::endl;
}


template <typename T>
class BoostSafeBase64Benchmark : public Benchmark<T> {};

TYPED_TEST_CASE(BoostSafeBase64Benchmark, IntegralValueTypes);

TYPED_TEST(BoostSafeBase64Benchmark, _)
{
  using value_type = typename TestFixture::value_type;

  auto t0 = steady_clock::now();

  auto encoded = encode_base64_2(this->in);

  auto t1 = steady_clock::now();

  auto decoded = decode_base64_2<value_type>(encoded);

  auto t2 = steady_clock::now();

  EXPECT_EQ(this->in, decoded);

  std::cout << "Content size: " << encoded.size() << std::endl;

  std::cout << "Write time: " << elapsed(t0, t1) << std::endl;
  std::cout << "Read time : " << elapsed(t1, t2) << std::endl;
}


template <typename T>
class RFCBenchmark : public Benchmark<T> {};

TYPED_TEST_CASE(RFCBenchmark, PrimitiveValueTypes);

TYPED_TEST(RFCBenchmark, _)
{
  using value_type = typename TestFixture::value_type;

  auto t0 = steady_clock::now();

  auto encoded = encode_base64_rfc(this->in);

  auto t1 = steady_clock::now();

  auto decoded = decode_base64_rfc<value_type>(encoded.get(), strlen(encoded.get()));

  auto t2 = steady_clock::now();

  EXPECT_EQ(this->in, decoded);

  std::cout << "Content size: " << strlen(encoded.get()) << std::endl;

  std::cout << "Write time: " << elapsed(t0, t1) << std::endl;
  std::cout << "Read time : " << elapsed(t1, t2) << std::endl;
}



TEST(BoostTextArchive, short)
{
  std::vector<unsigned short> in = {65535, 65535};

  std::stringstream os;
  boost::archive::text_oarchive oa(os);
  oa << BOOST_SERIALIZATION_NVP(in);

  std::cout << os.str() << std::endl;

  std::vector<unsigned short> out;
  boost::archive::text_iarchive ia(os);
  ia >> BOOST_SERIALIZATION_NVP(out);

  for(unsigned short s : out) std::cout << s << "; ";
  std::cout << std::endl;
}

TEST(BoostXmlArchive, short)
{
  std::vector<unsigned short> in = {65535, 65535};

  std::stringstream os;
  boost::archive::xml_oarchive oa(os);
  oa << BOOST_SERIALIZATION_NVP(in);

  std::cout << os.str() << std::endl;

  std::vector<unsigned short> out;
  boost::archive::xml_iarchive ia(os);
  ia >> BOOST_SERIALIZATION_NVP(out);

  for(unsigned short s : out) std::cout << s << "; ";
  std::cout << std::endl;
}



template <typename T>
class BoostArchiveBenchmark : public ::testing::Test {
public:
  using oarchive_t = typename T::first_type;
  using iarchive_t = typename T::second_type;

  BoostArchiveBenchmark() : in(random_vector<unsigned short>(1000000)), ss(), oarchive(ss), out() {}
  const std::vector<unsigned short> in;
  std::vector<unsigned short> out;
  std::stringstream ss;
  oarchive_t oarchive;
};

using BoostBinaryArchive = std::pair<boost::archive::binary_oarchive, boost::archive::binary_iarchive>;
using BoostTextArchive   = std::pair<boost::archive::text_oarchive, boost::archive::text_iarchive>;
using BoostXMLArchive    = std::pair<boost::archive::xml_oarchive, boost::archive::xml_iarchive>;

using BoostArchiveTypes = ::testing::Types<BoostBinaryArchive,
      BoostTextArchive,
      BoostXMLArchive>;
TYPED_TEST_CASE(BoostArchiveBenchmark, BoostArchiveTypes);

TYPED_TEST(BoostArchiveBenchmark, benchmark)
{
  auto t0 = steady_clock::now();

  this->oarchive << boost::serialization::make_nvp("data", this->in);

  auto t1 = steady_clock::now();

  typename TestFixture::iarchive_t iarchive(this->ss);
  iarchive >> boost::serialization::make_nvp("data", this->out);

  auto t2 = steady_clock::now();

  std::cout << "Content size: " << this->ss.tellp() << std::endl;

  std::cout << "Write time: " << elapsed(t0, t1) << std::endl;
  std::cout << "Read time : " << elapsed(t1, t2) << std::endl;

  EXPECT_EQ(this->in, this->out);
}
