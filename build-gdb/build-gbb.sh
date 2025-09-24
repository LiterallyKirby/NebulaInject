rm -rf build-gdb
mkdir build-gdb
cd build-gdb
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=OFF -DENABLE_UBSAN=OFF -DENABLE_TSAN=OFF -DENABLE_MSAN=OFF ..
make -j$(nproc)
