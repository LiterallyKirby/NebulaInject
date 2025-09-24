#!/bin/bash
set -euo pipefail # -u: undefined vars, -o pipefail: detect errors in pipes

# Use Java 8 instead of Java 21
export JAVA_HOME=/usr/lib/jvm/java-8-openjdk
export PATH=$JAVA_HOME/bin:$PATH

# Add AWT/XAWT to runtime path so dlopen can find libawt_xawt.so
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

# Verify we're using the correct Java version
echo "Using Java version:"
$JAVA_HOME/bin/java -version

# Enable verbose output
set -x

# Clean up
echo "Cleaning up old build files..."
rm -rf build
rm -fr CMakeCache.txt CMakeFiles/ cmake_install.cmake Makefile || true

# Create build directory
mkdir -p build
cd build

# Configure with sanitizers
echo "Configuring with sanitizers..."
cmake -DJAVA_HOME="$JAVA_HOME" \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      -DENABLE_ASAN="$ENABLE_ASAN" \
      -DENABLE_UBSAN="$ENABLE_UBSAN" \
      -DENABLE_TSAN="$ENABLE_TSAN" \
      -DENABLE_MSAN="$ENABLE_MSAN" \
      -G Ninja \
      .. || { echo "CMake configuration failed!"; exit 1; }

# Build using cmake --build (works with any generator)
echo "Building..."
cmake --build . -j"$(nproc)" || { echo "Build failed! Check above output."; exit 1; }

echo "Build completed successfully!"
echo "Verifying library dependencies:"
ldd libphantom.so | grep java

# Also build a clean version for injection if sanitizers are enabled
if [[ "$ENABLE_ASAN" == "ON" || "$ENABLE_UBSAN" == "ON" ]]; then
    echo ""
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
          .. || { echo "Clean build configuration failed!"; exit 1; }
    
    cmake --build . -j"$(nproc)" || { echo "Clean build failed!"; exit 1; }
    
    echo "Clean build completed! Use build-inject/libphantom.so for injection."
    cd ../build
fi

# Set up sanitizer environment for runtime
# Run sanitizer test if sanitizers are enabled
if [[ "$ENABLE_ASAN" == "ON" || "$ENABLE_UBSAN" == "ON" ]]; then
    echo ""
    echo "=== Running Sanitizer Test ==="
    
    # Create a quick test program
    cat > ../sanitizer_test.cpp << 'EOF'
#include <dlfcn.h>
#include <iostream>
#include <jni.h>

// Mock JNI environment for basic testing
class MockJNIEnv {
public:
    void* FindClass(const char* name) { 
        std::cout << "FindClass: " << name << std::endl; 
        return (void*)0xdeadbeef; 
    }
    void* GetObjectClass(void* obj) { 
        std::cout << "GetObjectClass called" << std::endl;
        return (void*)0xdeadbeef; 
    }
    void* GetFieldID(void* clazz, const char* name, const char* sig) {
        std::cout << "GetFieldID: " << name << " " << sig << std::endl;
        return (void*)0xcafebabe;
    }
    void DeleteLocalRef(void* ref) {
        std::cout << "DeleteLocalRef called" << std::endl;
    }
};

int main() {
    std::cout << "Testing library load..." << std::endl;
    
    void* handle = dlopen("./libphantom.so", RTLD_LAZY);
    if (!handle) {
        std::cerr << "Cannot load library: " << dlerror() << std::endl;
        return 1;
    }
    
    std::cout << "Library loaded successfully!" << std::endl;
    
    // Look for common entry points
    void* init_func = dlsym(handle, "JNI_OnLoad");
    if (init_func) {
        std::cout << "Found JNI_OnLoad function" << std::endl;
    }
    
    // Test basic functionality without crashing
    std::cout << "Basic library test completed" << std::endl;
    
    dlclose(handle);
    return 0;
}
EOF

    # Set up sanitizer environment
    export ASAN_OPTIONS="abort_on_error=1:print_stacktrace=1:symbolize=1:detect_leaks=1:check_initialization_order=1:print_module_map=1:fast_unwind_on_malloc=0"
    export UBSAN_OPTIONS="print_stacktrace=1:halt_on_error=1"
    
    if [[ "$ENABLE_TSAN" == "ON" ]]; then
        export TSAN_OPTIONS="abort_on_error=1:halt_on_error=1:symbolize=1"
    fi
    
    if [[ "$ENABLE_MSAN" == "ON" ]]; then
        export MSAN_OPTIONS="abort_on_error=1:print_stats=1"
    fi
    
    # Compile and run the test
    echo "Compiling sanitizer test..."
    g++ -g -I"$JAVA_HOME/include" -I"$JAVA_HOME/include/linux" -o ../sanitizer_test ../sanitizer_test.cpp -ldl || {
        echo "Failed to compile test, but build succeeded"
        rm -f ../sanitizer_test.cpp
    }
    
    if [[ -f "../sanitizer_test" ]]; then
        echo "Running sanitizer test..."
        
        # Find ASan library for preloading
        ASAN_LIB=$(gcc -print-file-name=libasan.so 2>/dev/null || echo "")
        if [[ -z "$ASAN_LIB" || "$ASAN_LIB" == "libasan.so" ]]; then
            # Try common locations
            for lib in /usr/lib/gcc/x86_64-linux-gnu/*/libasan.so*; do
                if [[ -f "$lib" ]]; then
                    ASAN_LIB="$lib"
                    break
                fi
            done
        fi
        
        # Find UBSan library
        UBSAN_LIB=$(gcc -print-file-name=libubsan.so 2>/dev/null || echo "")
        if [[ -z "$UBSAN_LIB" || "$UBSAN_LIB" == "libubsan.so" ]]; then
            for lib in /usr/lib/gcc/x86_64-linux-gnu/*/libubsan.so*; do
                if [[ -f "$lib" ]]; then
                    UBSAN_LIB="$lib"
                    break
                fi
            done
        fi
        
        # Set up LD_PRELOAD
        PRELOAD_LIBS=""
        if [[ -f "$ASAN_LIB" ]]; then
            PRELOAD_LIBS="$ASAN_LIB"
            echo "Using ASan library: $ASAN_LIB"
        fi
        if [[ -f "$UBSAN_LIB" ]]; then
            if [[ -n "$PRELOAD_LIBS" ]]; then
                PRELOAD_LIBS="$PRELOAD_LIBS:$UBSAN_LIB"
            else
                PRELOAD_LIBS="$UBSAN_LIB"
            fi
            echo "Using UBSan library: $UBSAN_LIB"
        fi
        
        # Run test with preloaded sanitizer libraries
        if [[ -n "$PRELOAD_LIBS" ]]; then
            if LD_PRELOAD="$PRELOAD_LIBS" ../sanitizer_test; then
                echo "✅ Sanitizer test passed!"
            else
                echo "❌ Sanitizer test failed - check for memory errors above"
                echo "Fix these issues before injecting"
            fi
        else
            echo "Warning: Could not find sanitizer libraries for preloading"
            if ../sanitizer_test; then
                echo "✅ Basic test passed (sanitizers may not be active)"
            else
                echo "❌ Test failed - check for errors above"
            fi
        fi
        rm -f ../sanitizer_test ../sanitizer_test.cpp
    fi
    
    echo ""
    echo "=== Manual Testing Setup ==="
    echo "For further debugging, set these environment variables:"
    echo "export ASAN_OPTIONS=\"abort_on_error=1:print_stacktrace=1:symbolize=1:detect_leaks=1:check_initialization_order=1:print_module_map=1:fast_unwind_on_malloc=0\""
    echo "export UBSAN_OPTIONS=\"print_stacktrace=1:halt_on_error=1\""
    echo ""
    echo "Use build/libphantom.so for debugging"
    echo "Use build-inject/libphantom.so for actual injection"
else
    echo ""
    echo "Sanitizers disabled. Use build/libphantom.so for injection."
fi

make run
