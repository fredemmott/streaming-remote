cmake_minimum_required(VERSION 3.10.2)

project(streaming-remote VERSION 2.0)
set(XSPLIT_PLUGIN_DLL_API_VERSION ${CMAKE_PROJECT_VERSION})

option(WITH_NATIVE_PLUGINS "Build C++ code" ON)
if(WITH_NATIVE_PLUGINS)
  add_subdirectory(native)
endif()

configure_file(
  xsplit-js-plugin/src/Version.ts.in
  ${CMAKE_CURRENT_SOURCE_DIR}/xsplit-js-plugin/src/Version.ts
  NEWLINE_STYLE LF
)
configure_file(
	streamdeck-plugin/manifest.json.in
	${CMAKE_CURRENT_SOURCE_DIR}/sdplugin/manifest.json
	NEWLINE_STYLE LF
)
