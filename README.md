# Test hash functions
## Build and run project

1. Make sure you have:
    * smhasher https://github.com/aappleby/smhasher
    * SpookyV2.cpp and SpookyV2.h https://burtleburtle.net/bob/hash/spooky.html
    * xxHash https://github.com/Cyan4973/xxHash

    And directory tree is as follows:

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

2. Run `mkdir build && cd build && cmake .. && make` from project folder
3. You can launch programm: `./main <from> <to> <step>`
