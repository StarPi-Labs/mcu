cmake_minimum_required(VERSION 3.14)
project(radio_app LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# --------------------------------------------------------------------------
# RadioLib — pulls in the library itself AND its Raspberry Pi HAL
# (src/hal/RPi/PiHal.h + PiHal.cpp ship inside RadioLib's own repo now,
# so there is nothing separate to fetch for PiHal).
# --------------------------------------------------------------------------
FetchContent_Declare(
  radiolib
  GIT_REPOSITORY https://github.com/jgromes/RadioLib.git
  GIT_TAG        master        # pin to a release tag (e.g. 7.1.2) for reproducible builds
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(radiolib)

# RadioLib is distributed as an Arduino library, so it has no CMakeLists.txt
# of its own — collect its sources and build them as a static library.
file(GLOB_RECURSE RADIOLIB_SOURCES ${radiolib_SOURCE_DIR}/src/*.cpp)
list(FILTER RADIOLIB_SOURCES EXCLUDE REGEX ".*/hal/Arduino/.*")
list(FILTER RADIOLIB_SOURCES EXCLUDE REGEX ".*/hal/RPiPico/.*")

add_library(radiolib STATIC ${RADIOLIB_SOURCES})
target_include_directories(radiolib PUBLIC
  ${radiolib_SOURCE_DIR}/src
  ${radiolib_SOURCE_DIR}/src/hal/RPi
)

# --------------------------------------------------------------------------
# lgpio — GPIO/SPI backend used by PiHal. Use it if already installed
# (e.g. `sudo apt install liblgpio-dev`); otherwise fetch + build it.
# --------------------------------------------------------------------------
find_library(LGPIO_LIB NAMES lgpio)
find_path(LGPIO_INCLUDE_DIR NAMES lgpio.h)

if(NOT LGPIO_LIB OR NOT LGPIO_INCLUDE_DIR)
  message(STATUS "liblgpio not found on system — downloading and building it")

  FetchContent_Declare(
    lgpio_src
    GIT_REPOSITORY https://github.com/joan2937/lg.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE
  )
  FetchContent_MakeAvailable(lgpio_src)

  # lg only ships a plain Makefile (no CMake), so build it directly.
  # We link straight against the build-tree .so, so no `sudo make install`
  # is required.
  if(NOT EXISTS ${lgpio_src_SOURCE_DIR}/liblgpio.so)
    message(STATUS "Building lgpio from source...")
    execute_process(
      COMMAND make
      WORKING_DIRECTORY ${lgpio_src_SOURCE_DIR}
      RESULT_VARIABLE LGPIO_BUILD_RESULT
    )
    if(NOT LGPIO_BUILD_RESULT EQUAL 0)
      message(FATAL_ERROR "Failed to build lgpio from source")
    endif()
  endif()

  find_library(LGPIO_LIB NAMES lgpio PATHS ${lgpio_src_SOURCE_DIR} NO_DEFAULT_PATH)
  find_path(LGPIO_INCLUDE_DIR NAMES lgpio.h PATHS ${lgpio_src_SOURCE_DIR} NO_DEFAULT_PATH)

  # Since it isn't installed system-wide, bake the build-tree path into
  # the executable's rpath so it can find libgpio.so at runtime.
  set(CMAKE_BUILD_RPATH ${lgpio_src_SOURCE_DIR})
endif()

target_include_directories(radiolib PUBLIC ${LGPIO_INCLUDE_DIR})
target_link_libraries(radiolib PUBLIC ${LGPIO_LIB} pthread)

# --------------------------------------------------------------------------
# Your application
# --------------------------------------------------------------------------
add_executable(radio_app src/main.cpp)
target_link_libraries(radio_app PRIVATE radiolib)