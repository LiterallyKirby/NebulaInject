cmake_minimum_required(VERSION 3.18)
project(phantom LANGUAGES C CXX)

# === Sanitizer Options ===
option(ENABLE_ASAN "Enable AddressSanitizer" ON)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" ON)
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)  # Don't use with ASAN
option(ENABLE_MSAN "Enable MemorySanitizer" OFF)   # Don't use with ASAN

# === Java Configuration ===
if(JAVA_HOME)
    message(STATUS "Using JAVA_HOME: ${JAVA_HOME}")
    set(JNI_INCLUDE_DIRS ${JAVA_HOME}/include ${JAVA_HOME}/include/linux)
    set(JNI_LIBRARIES ${JAVA_HOME}/jre/lib/amd64/libjawt.so ${JAVA_HOME}/jre/lib/amd64/server/libjvm.so)
endif()

if (NOT DEFINED PHANTOM_SOURCE_FILES)
    file(GLOB_RECURSE PHANTOM_SOURCE_FILES src/*.*)
endif()

set(IMGUI_SOURCE_FILES
    vendor/imgui/imgui.cpp
    vendor/imgui/imgui_impl_opengl3.cpp
    vendor/imgui/imgui_impl_sdl.cpp
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_tables.cpp
    vendor/imgui/imgui_widgets.cpp
    vendor/imgui/imgui_demo.cpp
)

# --- find deps ---
find_package(GLEW REQUIRED)
find_package(SDL2 REQUIRED)
find_package(JNI REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)

# optional: prefer imported targets
if (TARGET GLEW::GLEW)
    set(GLEW_TARGET GLEW::GLEW)
else()
    set(GLEW_TARGET ${GLEW_LIBRARIES})
endif()

if (TARGET SDL2::SDL2)
    set(SDL2_TARGET SDL2::SDL2)
else()
    set(SDL2_TARGET ${SDL2_LIBRARIES})
endif()

# === Create target ===
add_library(phantom SHARED ${PHANTOM_SOURCE_FILES} ${IMGUI_SOURCE_FILES})

# === Sanitizer Configuration ===
set(SANITIZER_FLAGS "")
set(SANITIZER_LINK_FLAGS "")

if(ENABLE_ASAN AND NOT ENABLE_TSAN AND NOT ENABLE_MSAN)
    message(STATUS "Enabling AddressSanitizer")
    list(APPEND SANITIZER_FLAGS 
        "-fsanitize=address"
        "-fno-omit-frame-pointer"
        "-fno-optimize-sibling-calls"
        "-O1"  # Better stack traces with O1
    )
    list(APPEND SANITIZER_LINK_FLAGS "-fsanitize=address")
    
    # ASAN environment variables for better debugging
    set_target_properties(phantom PROPERTIES
        ENVIRONMENT "ASAN_OPTIONS=abort_on_error=1:halt_on_error=1:detect_leaks=1:check_initialization_order=1:strict_init_order=1:symbolize=1"
    )
endif()

if(ENABLE_UBSAN)
    message(STATUS "Enabling UndefinedBehaviorSanitizer")
    list(APPEND SANITIZER_FLAGS 
        "-fsanitize=undefined"
        "-fsanitize=float-divide-by-zero"
        "-fsanitize=float-cast-overflow"
        "-fno-sanitize-recover=all"  # Abort on first error
    )
    list(APPEND SANITIZER_LINK_FLAGS "-fsanitize=undefined")
endif()

if(ENABLE_TSAN AND NOT ENABLE_ASAN)
    message(STATUS "Enabling ThreadSanitizer")
    list(APPEND SANITIZER_FLAGS "-fsanitize=thread")
    list(APPEND SANITIZER_LINK_FLAGS "-fsanitize=thread")
    
    set_target_properties(phantom PROPERTIES
        ENVIRONMENT "TSAN_OPTIONS=abort_on_error=1:halt_on_error=1:symbolize=1"
    )
endif()

if(ENABLE_MSAN AND NOT ENABLE_ASAN)
    message(STATUS "Enabling MemorySanitizer")
    list(APPEND SANITIZER_FLAGS 
        "-fsanitize=memory"
        "-fsanitize-memory-track-origins=2"
        "-fno-omit-frame-pointer"
    )
    list(APPEND SANITIZER_LINK_FLAGS "-fsanitize=memory")
endif()

# Additional debugging flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR SANITIZER_FLAGS)
    list(APPEND SANITIZER_FLAGS
        "-g3"                    # Maximum debug info
        "-ggdb"                  # GDB-specific debug info
        "-fno-omit-frame-pointer" # Better stack traces
        "-fstack-protector-strong" # Stack overflow detection
        "-Wall"
        "-Wextra"
        "-Wpedantic"
        "-Wformat-security"
        "-Wstack-protector"
        "-fstack-clash-protection"
    )
endif()

# Apply sanitizer flags
if(SANITIZER_FLAGS)
    target_compile_options(phantom PRIVATE ${SANITIZER_FLAGS})
    target_link_options(phantom PRIVATE ${SANITIZER_LINK_FLAGS})
endif()

# === Includes ===
target_include_directories(phantom PRIVATE
    ${JNI_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
    ${X11_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/vendor/imgui
    ${CMAKE_SOURCE_DIR}/src/mapping/impl
    ${GLEW_INCLUDE_DIRS}
)

# === Link libs ===
target_link_libraries(phantom PRIVATE
    ${GLEW_TARGET}
    ${SDL2_TARGET}
    OpenGL::GL
    Threads::Threads
    ${CMAKE_DL_LIBS}
    ${X11_LIBRARIES}
    ${JNI_LIBRARIES}
)

# Jawt/jvm if found
if (JAWT_LIBRARY)
    target_link_libraries(phantom PRIVATE ${JAWT_LIBRARY})
endif()

if (JVM_LIBRARY)
    target_link_libraries(phantom PRIVATE ${JVM_LIBRARY})
endif()

# C++ standard etc
set_target_properties(phantom PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED YES
)

# === Debug Configuration Summary ===
if(SANITIZER_FLAGS)
    message(STATUS "=== Sanitizer Configuration ===")
    message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
    message(STATUS "Sanitizer Flags: ${SANITIZER_FLAGS}")
    message(STATUS "AddressSanitizer: ${ENABLE_ASAN}")
    message(STATUS "UndefinedBehaviorSanitizer: ${ENABLE_UBSAN}")
    message(STATUS "ThreadSanitizer: ${ENABLE_TSAN}")
    message(STATUS "MemorySanitizer: ${ENABLE_MSAN}")
    message(STATUS "===============================")
endif()
