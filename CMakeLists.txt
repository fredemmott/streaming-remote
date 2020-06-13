cmake_minimum_required(VERSION 3.10.2)

project(streaming-remote VERSION 2.0)
set(XSPLIT_PLUGIN_DLL_API_VERSION ${CMAKE_PROJECT_VERSION})

option(WITHOUT_NATIVE_PLUGINS "Only build version information for JS, no C++ code" OFF)
if(NOT WITHOUT_NATIVE_PLUGINS)
  add_subdirectory(native)
endif()

configure_file(xsplit-js-plugin/src/Version.ts.in ${CMAKE_CURRENT_SOURCE_DIR}/xsplit-js-plugin/src/Version.ts)
