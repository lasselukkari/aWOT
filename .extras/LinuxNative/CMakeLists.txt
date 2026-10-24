cmake_minimum_required(VERSION 3.18)
project(aWOT-LinuxNative LANGUAGES CXX)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
  message(FATAL_ERROR "Build this example on Linux with a native GCC or Clang toolchain")
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Linux and macOS share the POSIX adapter and Arduino shim.
set(POSIX_NATIVE "${CMAKE_CURRENT_SOURCE_DIR}/../MacOSNative")
# All native examples share the bundled ArduinoCore-API.
set(ARDUINO_API "${CMAKE_CURRENT_SOURCE_DIR}/../ArduinoCore-API/api")
set(AWOT_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../../src")
find_package(Threads REQUIRED)

add_library(LinuxNative STATIC
  "${POSIX_NATIVE}/MacOSSocket.cpp"
  "${ARDUINO_API}/Print.cpp"
  "${ARDUINO_API}/Stream.cpp"
  "${ARDUINO_API}/WString.cpp"
  "${ARDUINO_API}/IPAddress.cpp"
  "${ARDUINO_API}/Common.cpp"
  "${AWOT_SRC}/App.cpp"
  "${AWOT_SRC}/Request.cpp"
  "${AWOT_SRC}/Response.cpp"
  "${AWOT_SRC}/Router.cpp"
)
target_include_directories(LinuxNative PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}" "${POSIX_NATIVE}" "${AWOT_SRC}"
)
target_include_directories(LinuxNative SYSTEM PUBLIC
  "${ARDUINO_API}" "${ARDUINO_API}/deprecated-avr-comp"
)
target_compile_definitions(LinuxNative PUBLIC HOST)
target_compile_options(LinuxNative PUBLIC -Wall -Wextra)
target_link_libraries(LinuxNative PUBLIC Threads::Threads)

add_executable(HelloWorld HelloWorld.cpp)
target_link_libraries(HelloWorld PRIVATE LinuxNative)

include(CTest)
if(BUILD_TESTING)
  find_package(Python3 REQUIRED COMPONENTS Interpreter)

  add_executable(SocketTests "${POSIX_NATIVE}/tests/sockets.cpp")
  target_link_libraries(SocketTests PRIVATE LinuxNative)
  add_test(NAME linux-sockets COMMAND SocketTests)

  add_executable(ShimTests "${POSIX_NATIVE}/tests/shims.cpp")
  target_link_libraries(ShimTests PRIVATE LinuxNative)
  add_test(NAME linux-shims COMMAND ShimTests)

  add_test(NAME linux-http
    COMMAND "${Python3_EXECUTABLE}"
      "${CMAKE_CURRENT_SOURCE_DIR}/../WindowsNative/tests/http_smoke.py"
      "$<TARGET_FILE:HelloWorld>"
  )
  set_tests_properties(linux-sockets linux-shims PROPERTIES TIMEOUT 60)
  set_tests_properties(linux-http PROPERTIES TIMEOUT 90)
endif()
