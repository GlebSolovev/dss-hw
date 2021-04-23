# Test compression algorithms
## Build and run project

1. Make sure you have:
    * liblz4.a and lz4.h from https://github.com/lz4/lz4
    * libzstd.a and zstd.h from https://github.com/facebook/zstd

    and directory tree is as follows:

      ```
      top-level folder/

      /project folder/
        CMakeLists.txt
        main.cpp
        lz4.h
        zstd.h
 
      /lib/
        libzstd.a
        liblz4.a
      ```
2. Run `mkdir build && cd build && cmake .. && make` from project folder
3. You can launch programm: `./main <file to compress>...`
