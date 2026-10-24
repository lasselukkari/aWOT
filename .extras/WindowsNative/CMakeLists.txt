cmake_minimum_required(VERSION 3.18)
project(aWOT-WindowsNative LANGUAGES CXX)

if(NOT WIN32 OR NOT MINGW)
  message(FATAL_ERROR "Use a Windows MinGW toolchain, such as MSYS2 UCRT64 GCC")
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# All native examples share the bundled ArduinoCore-API.
set(ARDUINO_API "${CMAKE_CURRENT_SOURCE_DIR}/../ArduinoCore-API/api")
set(AWOT_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../../src")
find_package(Threads REQUIRED)

add_library(WindowsNative STATIC
  Arduino.cpp
  WindowsSocket.cpp
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
target_include_directories(WindowsNative PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${AWOT_SRC}")
target_include_directories(WindowsNative SYSTEM PUBLIC "${ARDUINO_API}" "${ARDUINO_API}/deprecated-avr-comp")
target_compile_definitions(WindowsNative PUBLIC
  HOST _WIN32_WINNT=0x0601 WIN32_LEAN_AND_MEAN NOUSER NOMINMAX _CRT_NONSTDC_NO_WARNINGS
)
target_compile_options(WindowsNative PUBLIC -Wall -Wextra)
target_link_libraries(WindowsNative PUBLIC ws2_32 Threads::Threads)
# Bundle the GCC/C++/thread runtimes so the executables run outside MSYS2 too.
target_link_options(WindowsNative INTERFACE -static)

add_executable(HelloWorld HelloWorld.cpp)
target_link_libraries(HelloWorld PRIVATE WindowsNative)

include(CTest)
if(BUILD_TESTING)
  find_package(Python3 REQUIRED COMPONENTS Interpreter)

  add_executable(SocketTests tests/sockets.cpp)
  target_link_libraries(SocketTests PRIVATE WindowsNative)
  add_test(NAME windows-sockets COMMAND SocketTests)

  # Timing and numeric conversions have the same contract on both platforms.
  add_executable(ShimTests ../MacOSNative/tests/shims.cpp)
  target_link_libraries(ShimTests PRIVATE WindowsNative)
  add_test(NAME windows-shims COMMAND ShimTests)

  set(SMOKE_RUNNER)
  if(CMAKE_CROSSCOMPILING_EMULATOR)
    list(APPEND SMOKE_RUNNER --runner "${CMAKE_CROSSCOMPILING_EMULATOR}")
  endif()
  add_test(NAME windows-http
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/http_smoke.py"
      "$<TARGET_FILE:HelloWorld>" ${SMOKE_RUNNER}
  )
  set_tests_properties(windows-sockets windows-shims PROPERTIES TIMEOUT 60)
  set_tests_properties(windows-http PROPERTIES TIMEOUT 90)
endif()
