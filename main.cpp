#include <cstdlib>
#include <fstream>
#include <iostream>

#include "../smhasher/src/MurmurHash3.h"
#include "../smhasher/src/SpeedTest.h"

#define XXH_INLINE_ALL
#include "xxhash.h"

#include "../SpookyHash/SpookyV2.h"

constexpr const char* OUTPUT_FILENAME = "out.csv";
constexpr const char* TITLES[] = {"mmh3_x86_128", "xxh3_64", "spooky"};
constexpr size_t TITLES_SIZE = sizeof TITLES / sizeof(char*);
constexpr size_t ITERS = 5;

constexpr int IGNORED_PARAM = 0;
constexpr int SPEED_TEST_RANDOM_SEED = 0;

inline void test(pfHash hashFunc, uint32_t bytes, double& outCycles) {
  TinySpeedTest(hashFunc, IGNORED_PARAM, bytes, SPEED_TEST_RANDOM_SEED, false,
                outCycles);
}

void XXH3_64bitsAdapter(const void* key, int len, uint32_t seed, void* out) {
  // XXH64_hash_t = uint64_t fits in uint32_t temp[16] used by SpeedTest for saving hash function results
  *static_cast<XXH64_hash_t*>(out) = XXH3_64bits_withSeed(key, len, seed);
}

void SpookyHashAdapter(const void* key, int len, uint32_t seed, void* out) {
  *static_cast<uint64_t*>(out) = SpookyHash::Hash64(key, len, seed);
}

int main(int argc, char* argv[]) {
  if (argc != 4) {
    std::cerr
        << "bad arguments, main requires: <range start> <range upper bound> <step>\n";
    return 1;
  }
  uint32_t rangeStart = std::strtoll(argv[1], nullptr, 10);
  uint32_t rangeUpperBound = std::strtoll(argv[2], nullptr, 10);
  uint32_t step = std::strtoll(argv[3], nullptr, 10);
  if (!(rangeStart >= 0 && rangeUpperBound >= rangeStart && step > 0)) {
    std::cerr
        << "bad input values, main requires: 0 <= <range start> <= <range upper bound> && <step> > 0\n";
    return 2;
  }

  std::vector<std::vector<double>> results(
      (rangeUpperBound - rangeStart) / step + 1,
      std::vector<double>(TITLES_SIZE, 0));

  for (std::size_t iter = 0; iter < ITERS; ++iter) {
    for (uint32_t bytes = rangeStart, i = 0; bytes <= rangeUpperBound;
         bytes += step, i++) {
      test(MurmurHash3_x86_32, bytes, results[i][0]);
      test(XXH3_64bitsAdapter, bytes, results[i][1]);
      test(SpookyHashAdapter, bytes, results[i][2]);
    }
  }

  std::ofstream outs(OUTPUT_FILENAME);
  outs << "bytes";
  for (const auto& title : TITLES) {
    outs << "," << title;
  }
  for (uint32_t bytes = rangeStart, i = 0; bytes <= rangeUpperBound;
       bytes += step, ++i) {
    outs << "\n" << bytes;
    for (const auto& res : results[i]) {
      outs << "," << res / ITERS;
    }
  }
  outs << "\n";
  outs.close();
  return 0;
}
