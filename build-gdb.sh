
#!/bin/bash
set -euo pipefail

# === Java Setup ===
export JAVA_HOME=/usr/lib/jvm/java-8-openjdk
export PATH=$JAVA_HOME/bin:$PATH
export LD_LIBRARY_PATH="$JAVA_HOME/jre/lib/amd64/xawt:$JAVA_HOME/jre/lib/amd64:${LD_LIBRARY_PATH:-}"

# === Sanitizer Configuration ===
ENABLE_ASAN=${ENABLE_ASAN:-ON}
ENABLE_UBSAN=${ENABLE_UBSAN:-ON}
ENABLE_TSAN=${ENABLE_TSAN:-OFF}
ENABLE_MSAN=${ENABLE_MSAN:-OFF}

echo "=== Sanitizer Configuration ==="
echo "AddressSanitizer: $ENABLE_ASAN"
echo "UndefinedBehaviorSanitizer: $ENABLE_UBSAN" 
echo "ThreadSanitizer: $ENABLE_TSAN"
echo "MemorySanitizer: $ENABLE_MSAN"
echo "==============================="

echo "Using Java version:"
$JAVA_HOME/bin/java -version

set -x

# === Clean Build ===
rm -rf build build-inject CMakeCache.txt CMakeFiles/ cmake_install.cmake Makefile || true
mkdir -p build
cd build

# === Configure Normal Build ===
cmake -DJAVA_HOME="$JAVA_HOME" \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      -DENABLE_ASAN="$ENABLE_ASAN" \
      -DENABLE_UBSAN="$ENABLE_UBSAN" \
      -DENABLE_TSAN="$ENABLE_TSAN" \
      -DENABLE_MSAN="$ENABLE_MSAN" \
      -G Ninja \
      .. || { echo "CMake configuration failed!"; exit 1; }

cmake --build . -j"$(nproc)"

echo "Build completed. Checking dependencies:"
ldd libphantom.so | grep java

# === Build clean version if sanitizers enabled ===
if [[ "$ENABLE_ASAN" == "ON" || "$ENABLE_UBSAN" == "ON" ]]; then
    echo "Building clean version for injection..."
    cd ..
    mkdir -p build-inject
    cd build-inject
    cmake -DJAVA_HOME="$JAVA_HOME" \
          -DCMAKE_BUILD_TYPE=Debug \
          -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
          -DENABLE_ASAN=OFF \
          -DENABLE_UBSAN=OFF \
          -DENABLE_TSAN=OFF \
          -DENABLE_MSAN=OFF \
          -G Ninja \
          .. || exit 1
    cmake --build . -j"$(nproc)"
    echo "Clean build complete: build-inject/libphantom.so"
    cd ../build
fi

# === Debug Helper Script ===
DEBUG_SCRIPT="$PWD/debug.sh"
echo "#!/bin/bash" > "$DEBUG_SCRIPT"
echo "unset ASAN_OPTIONS LSAN_OPTIONS UBSAN_OPTIONS TSAN_OPTIONS MSAN_OPTIONS" >> "$DEBUG_SCRIPT"
echo "gdb --args $PWD/../nebula_injector \"\$@\"" >> "$DEBUG_SCRIPT"
chmod +x "$DEBUG_SCRIPT"
echo "Debug script created: $DEBUG_SCRIPT"

# === Sanitizer Test ===
if [[ "$ENABLE_ASAN" == "ON" || "$ENABLE_UBSAN" == "ON" ]]; then
    echo "Running sanitizer test..."
    cat > ../sanitizer_test.cpp << 'EOF'
#include <dlfcn.h>
#include <iostream>
int main() {
    std::cout << "Testing libphantom.so load..." << std::endl;
    void* handle = dlopen("./libphantom.so", RTLD_LAZY);
    if (!handle) {
        std::cerr << "Cannot load library: " << dlerror() << std::endl;
        return 1;
    }
    std::cout << "Library loaded successfully!" << std::endl;
    dlclose(handle);
    return 0;
}
EOF
    g++ -g -I"$JAVA_HOME/include" -I"$JAVA_HOME/include/linux" -o ../sanitizer_test ../sanitizer_test.cpp -ldl
    ASAN_OPTIONS="abort_on_error=1:print_stacktrace=1:symbolize=1:detect_leaks=1:check_initialization_order=1"
    UBSAN_OPTIONS="print_stacktrace=1:halt_on_error=1"
    export ASAN_OPTIONS UBSAN_OPTIONS
    ../sanitizer_test
    rm -f ../sanitizer_test ../sanitizer_test.cpp
fi

echo "✅ Build done."
echo "Run with sanitizers disabled: $DEBUG_SCRIPT build-gdb/libphantom.so <PID>"
