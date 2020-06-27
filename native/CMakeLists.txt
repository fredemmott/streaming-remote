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

if(WIN32)
  get_filename_component(
    WINDOWS_10_KITS_ROOT
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]"
    ABSOLUTE CACHE
  )
  set(WINDOWS_10_KIT_DIR "${WINDOWS_10_KITS_ROOT}/bin/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}" CACHE PATH "Current Windows 10 kit directory")
  set(SIGNTOOL_KEY_ARGS "" CACHE STRING "Key arguments for signtool.exe - separate with ';'")
  find_program(
    SIGNTOOL_EXE
    signtool
    PATHS
    "${WINDOWS_10_KIT_DIR}/x64"
    "${WINDOWS_10_KIT_DIR}/x86"
    DOC "Path to signtool.exe if SIGNTOOL_KEY_ARGS is set"
  )
endif()
function(sign_target TARGET)
  if(SIGNTOOL_KEY_ARGS AND WIN32)
    add_custom_command(
      TARGET ${TARGET} POST_BUILD
      COMMAND
      "${SIGNTOOL_EXE}"
      ARGS
      sign
      ${SIGNTOOL_KEY_ARGS}
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
