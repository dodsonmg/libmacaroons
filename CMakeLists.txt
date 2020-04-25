cmake_minimum_required(VERSION 3.5)
project(macaroons)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(HAVE_CONFIG_H 1)

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
target_compile_definitions(macaroons
  PRIVATE "MACAROONS_DLL")
ament_target_dependencies(macaroons)

target_include_directories(macaroons PRIVATE include)
target_include_directories(macaroons PRIVATE src)

# This package installs libraries without exporting them.
# Export the library path to ensure that the installed libraries are available.
if(NOT WIN32)
  ament_environment_hooks(
    "${ament_cmake_package_templates_ENVIRONMENT_HOOK_LIBRARY_PATH}"
    )
endif()

install(TARGETS
  macaroons
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

ament_package()
