cmake_minimum_required(VERSION 3.15)

# MSVC_RUNTIME_LIBRARY support
cmake_policy(SET CMP0091 NEW)

project(streaming-remote VERSION 3.0.0 LANGUAGES CXX)

option(WITH_NATIVE_PLUGINS "Build C++ code" ON)
if(WITH_NATIVE_PLUGINS)
  add_subdirectory(native)
endif()

configure_file(
  js-client-lib/src/Version.ts.in
  ${CMAKE_CURRENT_SOURCE_DIR}/js-client-lib/src/Version.ts
  NEWLINE_STYLE LF
)
configure_file(
	streamdeck-plugin/manifest.json.in
	${CMAKE_CURRENT_SOURCE_DIR}/streamdeck-plugin/manifest.json
	NEWLINE_STYLE LF
)

set(XSPLIT_PLUGIN_DLL_API_VERSION ${CMAKE_PROJECT_VERSION})
configure_file(
  xsplit-js-plugin/src/Version.ts.in
  ${CMAKE_CURRENT_SOURCE_DIR}/xsplit-js-plugin/src/Version.ts
  NEWLINE_STYLE LF
)
