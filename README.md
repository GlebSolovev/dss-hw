# Test hash functions
## Build and run project

1. Make sure you have:
    * smhasher https://github.com/aappleby/smhasher
    * SpookyV2.cpp and SpookyV2.h https://burtleburtle.net/bob/hash/spooky.html
    * xxHash https://github.com/Cyan4973/xxHash

    and directory tree is as follows:

      ```
      top-level folder/

      /project folder/
        CMakeLists.txt
        main.cpp
  
      /smhasher/src/...
 
      /SpookyHash/
        SpookyV2.cpp
        SpookyV2.h
      
      /xxHash/...
      ```
2. Change TinySpeedTest implementation in SpeedTest.cpp to:
      ```
      void TinySpeedTest ( pfHash hash, int hashsize, int keysize, uint32_t seed, bool verbose, double &outCycles)
      {
      const int trials = 999999;

      if(verbose) printf("Small key speed test - %4d-byte keys - ",keysize);
  
      outCycles += SpeedTest(hash,seed,trials,keysize,0);
  
      //printf("%8.2f cycles/hash\n",cycles);
      }
      ```
3. Run `mkdir build && cd build && cmake .. && make` from project folder
4. You can launch programm: `./main <from> <to> <step>`
