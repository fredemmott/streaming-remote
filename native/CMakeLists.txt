set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
  add_definitions("/Zc:__cplusplus -DUNICODE=1 -D_WIN32_WINNT=0x0601 ")
endif()

include(vendor/asio.cmake)
include(vendor/fmt.cmake)
include(vendor/json.cmake)
include(vendor/libsodium.cmake)
include(vendor/websocketpp.cmake)

set(SIGNTOOL_KEY "" CACHE STRING "Key thumbprint for signtool.exe")
find_program(
  SIGNTOOL_EXE
  signtool
  PATHS
  ${CMAKE_MT}/..
  DOC "Path to signtool.exe if SIGNTOOL_KEY is set"
)
function(sign_target TARGET)
  if(SIGNTOOL_KEY)
    add_custom_command(
      TARGET ${TARGET} POST_BUILD
      COMMAND
      "${SIGNTOOL_EXE}"
      ARGS
      sign
      /sha1 "${SIGNTOOL_KEY}"
      /t http://timestamp.digicert.com
      /fd SHA256
      "$<TARGET_FILE:${TARGET}>"
    )
  endif()
endfunction()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
add_subdirectory(Core)
add_subdirectory(Dummy)
option(WITH_OBS "Build the OBS plugin" OFF)
if (WITH_OBS)
  add_subdirectory(obs)
endif()
option(WITH_XSPLIT "Build the XSplit plugin" OFF)
if (WITH_XSPLIT)
  add_subdirectory(xsplit)
endif()
