cmake_minimum_required(VERSION 3.5)
project(libmacaroons)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)

add_library(macaroons SHARED
            src/base64.c
            src/explicit_bzero.c
            src/macaroons.c
            src/packet.c
            src/port.c
            src/sha256.c
            src/shim.c
            src/slice.c
            src/timingsafe_bcmp.c
            src/tweetnacl.c
            src/v1.c
            src/v2.c
            src/varint.c
            )

target_include_directories(macaroons PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

ament_export_interfaces(export_macaroons HAS_LIBRARY_TARGET)

install(
  DIRECTORY include/
  DESTINATION include
)

install(
  TARGETS macaroons
  EXPORT export_macaroons
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

# build a simple test application
# add_executable(macaroons_test
#               test_cpp/test.cpp)
# target_link_libraries(macaroons_test macaroons)
# ament_target_dependencies(macaroons_test macaroons)

# install(TARGETS
#   macaroons_test
#   DESTINATION lib/${PROJECT_NAME})

# build a second test application based on kzemek's repo
add_executable(macaroons_test_1
              test_cpp/testExamples.cpp)
target_link_libraries(macaroons_test_1 macaroons)
ament_target_dependencies(macaroons_test_1 macaroons)

install(TARGETS
  macaroons_test_1
  DESTINATION lib/${PROJECT_NAME})

ament_export_include_directories(include)
ament_export_libraries(macaroons)

ament_package()