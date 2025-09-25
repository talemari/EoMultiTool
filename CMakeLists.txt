set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)
cmake_minimum_required(VERSION 3.5)
project(EoMultiTool LANGUAGES CXX)

list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_GENERATOR MATCHES "Visual Studio")
  set(CMAKE_PREFIX_PATH "C:/Qt/6.9.2/msvc2022_64" CACHE PATH "" FORCE)
  add_compile_options(/Zc:__cplusplus /utf-8 /fsanitize=address)
  add_link_options(/fsanitize=address)
  add_definitions(-DUNICODE -D_UNICODE)  # optional but typical on Windows
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT EoMultiTool)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
elseif (MINGW)
  set(CMAKE_PREFIX_PATH "C:/Qt/6.9.2/mingw_64" CACHE PATH "" FORCE)
else()
  add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)

file(GLOB MYAPP_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ressources/*.qrc"
)

add_executable(EoMultiTool
                ${MYAPP_SOURCES}
  )

include(GNUInstallDirs)
include(FetchContent)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libs" FORCE)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

FetchContent_Declare(
  zlib
  URL https://zlib.net/zlib-1.3.1.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(zlib)

if (NOT TARGET ZLIB::ZLIB)
  add_library(ZLIB::ZLIB ALIAS zlibstatic)
endif()

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG        0.8.0
)
FetchContent_MakeAvailable(yaml-cpp)
if (MSVC AND NOT BUILD_SHARED_LIBS)
  target_compile_definitions(EoMultiTool PRIVATE YAML_CPP_STATIC_DEFINE)
endif()

# Teach find_package(ZLIB) consumers (like libzip) to use THIS zlib
set(ZLIB_USE_STATIC_LIBS ON CACHE BOOL "" FORCE)
set(ZLIB_INCLUDE_DIR "${zlib_SOURCE_DIR}" CACHE PATH "" FORCE)
set(ZLIB_LIBRARY "${zlib_BINARY_DIR}/zlibstatic$<$<CONFIG:Debug>:d>.lib" CACHE FILEPATH "" FORCE)

set(LIBZIP_DO_INSTALL OFF CACHE BOOL "" FORCE)     # <- primary switch in recent libzip
set(BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(BUILD_REGRESS OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ENABLE_COMMONCRYPTO OFF CACHE BOOL "" FORCE)
set(ENABLE_GNUTLS OFF CACHE BOOL "" FORCE)
set(ENABLE_MBEDTLS OFF CACHE BOOL "" FORCE)
set(ENABLE_OPENSSL OFF CACHE BOOL "" FORCE)
set(ENABLE_BZIP2 OFF CACHE BOOL "" FORCE)
set(ENABLE_LZMA OFF CACHE BOOL "" FORCE)
set(ENABLE_ZSTD OFF CACHE BOOL "" FORCE)

add_subdirectory( "${CMAKE_CURRENT_SOURCE_DIR}/libraries/libzip/" )

target_include_directories(EoMultiTool PRIVATE
                           ${CMAKE_SOURCE_DIR}/include
                           ${CMAKE_CURRENT_SOURCE_DIR}/libraries/libzip/lib
                           ${CMAKE_CURRENT_BINARY_DIR}/libraries/libzip/lib
                           ${zlib_SOURCE_DIR}
                           ${zlib_BINARY_DIR}
                           )
set_target_properties(EoMultiTool PROPERTIES
  AUTOGEN_SOURCE_GROUP "Generated Files"     # moc/uic/rcc files into one filter
  AUTOGEN_TARGETS_FOLDER "CMake Files"       # moves the helper *_autogen target
)

source_group("Generated Files" REGULAR_EXPRESSION "mocs_.*|qrc_.*")

if (WIN32 AND TARGET Qt6::qmake AND NOT CMAKE_CONFIGURATION_TYPES) # single-config
    add_custom_command(TARGET EoMultiTool POST_BUILD
        COMMAND Qt6::windeployqt --no-compiler-runtime \"$<TARGET_FILE:EoMultiTool>\"
        COMMENT "Deploying Qt dependencies (single-config)..."
    )
elseif (WIN32 AND TARGET Qt6::qmake) # multi-config (Visual Studio)
    foreach(config ${CMAKE_CONFIGURATION_TYPES})
        add_custom_command(TARGET EoMultiTool POST_BUILD
            COMMAND Qt6::windeployqt
                --no-compiler-runtime
                \"$<TARGET_FILE:EoMultiTool>\"
            COMMENT "Deploying Qt dependencies for $<CONFIG>..."
        )
    endforeach()
endif()

target_link_libraries(EoMultiTool PRIVATE Qt6::Widgets Qt6::Network zip yaml-cpp)
qt_generate_deploy_app_script(
  TARGET EoMultiTool
  OUTPUT_SCRIPT deploy_script
  NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})