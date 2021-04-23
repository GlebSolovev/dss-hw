#include <chrono>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

#include "lz4.h"
#include "zstd.h"

using micros = std::chrono::microseconds::rep;

enum CompressionAlgo { LZ4 = 0, ZSTD_LVL1 = 1, ZSTD_LVL7 = 2 };
constexpr CompressionAlgo COMPRESSION_ALGOS[] = {LZ4, ZSTD_LVL1, ZSTD_LVL7};
constexpr size_t COMPRESSION_ALGOS_SIZE =
    sizeof(COMPRESSION_ALGOS) / sizeof(CompressionAlgo);
constexpr const char* TITLES[] = {"lz4", "zstd_lvl1", "zstd_lvl7"};

constexpr const char* OUTPUT_SPEED_RESULTS_FILENAME = "speed.csv";
constexpr const char* OUTPUT_COMPRESSION_RATIO_RESULTS_FILENAME =
    "compression.csv";
constexpr size_t ITERS_PER_ONE_FILE = 10;

namespace utils {

struct CStringBuff {
  size_t size;
  std::shared_ptr<char[]> ptr;

  CStringBuff(size_t size_) : size{size_}, ptr{new char[size + 1]} {}

  char* getBuff() { return ptr.get(); }
};

CStringBuff readFileOrExit(const char* filename) {
  std::filesystem::path filePath{filename};
  std::uintmax_t fileSize = std::filesystem::file_size(filePath);

  std::FILE* fileDescriptor = std::fopen(filename, "rb");
  if (!fileDescriptor) {
    std::cerr << "Failed to open file: " << filename << "\n";
    std::exit(2);
  }
  CStringBuff fileInput(fileSize);

  size_t readSize =
      std::fread(fileInput.getBuff(), sizeof(char), fileSize, fileDescriptor);
  if (readSize != fileSize) {
    std::cerr << "Failed to read file: " << filename << "\n";
    std::exit(2);
  }
  std::fclose(fileDescriptor);

  return fileInput;
}

constexpr const char* getName(CompressionAlgo algo) {
  return TITLES[static_cast<int>(algo)];
}

} // namespace utils

bool testZSTDCompressDecompressCycle(utils::CStringBuff& src,
                                     utils::CStringBuff& dst,
                                     utils::CStringBuff& decomp,
                                     const char* filename,
                                     int zstdCompressionLevel) {
  size_t writtenBytesRes = ZSTD_compress(dst.getBuff(), dst.size, src.getBuff(),
                                         src.size, zstdCompressionLevel);
  if (ZSTD_isError(writtenBytesRes)) {
    std::cerr << "ZSTD_LVL" << zstdCompressionLevel
              << " correctness test failed: filename = " << filename << "\n";
    return false;
  }

  size_t compressedFramesSize =
      ZSTD_getFrameContentSize(dst.getBuff(), writtenBytesRes);
  if (compressedFramesSize == ZSTD_CONTENTSIZE_ERROR ||
      compressedFramesSize == ZSTD_CONTENTSIZE_UNKNOWN) {
    std::cerr << "ZSTD_LVL" << zstdCompressionLevel
              << " correctness test failed: filename = " << filename << "\n";
    return false;
  }
  size_t decompBytesRes = ZSTD_decompress(
      decomp.getBuff(), compressedFramesSize, dst.getBuff(), writtenBytesRes);
  if (ZSTD_isError(decompBytesRes) ||
      std::memcmp(decomp.getBuff(), src.getBuff(), decompBytesRes) != 0) {
    std::cerr << "ZSTD_LVL" << zstdCompressionLevel
              << " correctness test failed: filename = " << filename << "\n";
    return false;
  }

  return true;
}

bool testAllAlgosCompressDecompressCycle(utils::CStringBuff src,
                                         const char* filename) {
  utils::CStringBuff dst(
      std::max(static_cast<size_t>(LZ4_compressBound(src.size)),
               ZSTD_compressBound(src.size)) +
      10);
  utils::CStringBuff decomp(src.size + 100);

  // test LZ4
  size_t writtenBytesRes =
      LZ4_compress_default(src.getBuff(), dst.getBuff(), src.size, dst.size);
  if (writtenBytesRes == 0) {
    std::cerr << "LZ4 correctness test failed: filename " << filename << "\n";
    return false;
  }
  int decompBytesRes = LZ4_decompress_safe(dst.getBuff(), decomp.getBuff(),
                                           writtenBytesRes, decomp.size);
  if (decompBytesRes < 0 ||
      std::memcmp(decomp.getBuff(), src.getBuff(), decompBytesRes) != 0) {
    std::cerr << "LZ4 correctness test failed: filename = " << filename << "\n";
    return false;
  }

  // test ZSTD
  return testZSTDCompressDecompressCycle(src, dst, decomp, filename, 1) &&
         testZSTDCompressDecompressCycle(src, dst, decomp, filename, 7);
}

std::pair<micros, double> testCompression(utils::CStringBuff& fileInput,
                                          utils::CStringBuff& dstBuff,
                                          const char* filename,
                                          CompressionAlgo algo) {
  size_t writtenBytes = 0;
  size_t res;

  auto start = std::chrono::high_resolution_clock::now();
  switch (algo) {
  case LZ4:
    writtenBytes = LZ4_compress_default(fileInput.getBuff(), dstBuff.getBuff(),
                                        fileInput.size, dstBuff.size);
    break;
  case ZSTD_LVL1:
    res = ZSTD_compress(dstBuff.getBuff(), dstBuff.size, fileInput.getBuff(),
                        fileInput.size, 1);
    writtenBytes = ZSTD_isError(res) ? 0 : res;
    break;
  case ZSTD_LVL7:
    res = ZSTD_compress(dstBuff.getBuff(), dstBuff.size, fileInput.getBuff(),
                        fileInput.size, 7);
    writtenBytes = ZSTD_isError(res) ? 0 : res;
    break;
  }
  auto finish = std::chrono::high_resolution_clock::now();

  if (writtenBytes == 0) {
    std::cerr << "Compression of file " << filename << " by "
              << utils::getName(algo) << " failed"
              << "\n";
    std::exit(3);
  }

  return std::make_pair<>(
      std::chrono::duration_cast<std::chrono::microseconds>(finish - start)
          .count(),
      static_cast<double>(fileInput.size) / writtenBytes);
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "bad arguments, main requires: <filename> ...\n";
    return 1;
  }

  // simple test correctness
  if (!testAllAlgosCompressDecompressCycle(utils::readFileOrExit("../main.cpp"),
                                           "../main.cpp")) {
    std::cerr << "correctness tests failed, libs are not working correctly\n";
    return -1;
  }

  // test speed and compression ratio
  size_t filesNumber = argc - 1;
  std::vector<std::vector<micros>> speedResults(
      filesNumber, std::vector<micros>(COMPRESSION_ALGOS_SIZE, 0));
  std::vector<std::vector<double>> compressionRatioResults(
      filesNumber, std::vector<double>(COMPRESSION_ALGOS_SIZE, 0));

  for (size_t fileIndex = 0; fileIndex < filesNumber; ++fileIndex) {
    const char* filename = argv[fileIndex + 1];
    utils::CStringBuff fileInput = utils::readFileOrExit(filename);
    if (fileInput.size > LZ4_MAX_INPUT_SIZE) {
      std::cerr << "File " << filename << " is too large to "
                << utils::getName(CompressionAlgo::LZ4) << "\n";
      return 2;
    }
    // test correctness first
    if (!testAllAlgosCompressDecompressCycle(fileInput, filename)) {
      std::cerr << "correctness tests failed, file " << filename
                << ", libs are not working correctly\n";
      return -1;
    }
    utils::CStringBuff dstBuff(
        std::max(static_cast<size_t>(LZ4_compressBound(fileInput.size)),
                 ZSTD_compressBound(fileInput.size)) +
        10);

    for (size_t iter = 0; iter < ITERS_PER_ONE_FILE; ++iter) {
      for (CompressionAlgo algo : COMPRESSION_ALGOS) {
        auto [compressionTime, compressionRatio] =
            testCompression(fileInput, dstBuff, filename, algo);
        speedResults[fileIndex][static_cast<int>(algo)] += compressionTime;
        compressionRatioResults[fileIndex][static_cast<int>(algo)] +=
            compressionRatio;
      }
    }
  }

  // write results to file
  std::ofstream speedResultsOutStream(OUTPUT_SPEED_RESULTS_FILENAME);
  std::ofstream compressionRatioResultsOutStream(
      OUTPUT_COMPRESSION_RATIO_RESULTS_FILENAME);
  speedResultsOutStream << "filename";
  compressionRatioResultsOutStream << "filename";
  for (const auto& title : TITLES) {
    speedResultsOutStream << "," << title;
    compressionRatioResultsOutStream << "," << title;
  }
  for (size_t fileIndex = 0; fileIndex < filesNumber; ++fileIndex) {
    speedResultsOutStream << "\n\"" << argv[fileIndex + 1] << "\"";
    compressionRatioResultsOutStream << "\n\"" << argv[fileIndex + 1] << "\"";
    for (const auto& speedResSum : speedResults[fileIndex]) {
      speedResultsOutStream
          << ","
          << static_cast<double>(speedResSum) / 1000 /
                 ITERS_PER_ONE_FILE; // print res in milliseconds
    }
    for (const auto& comprRatioResSum : compressionRatioResults[fileIndex]) {
      compressionRatioResultsOutStream << ","
                                       << comprRatioResSum / ITERS_PER_ONE_FILE;
    }
  }
  speedResultsOutStream << "\n";
  compressionRatioResultsOutStream << "\n";
  speedResultsOutStream.close();
  compressionRatioResultsOutStream.close();
  return 0;
}
