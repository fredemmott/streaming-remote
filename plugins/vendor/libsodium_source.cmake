include(ExternalProject)

ExternalProject_Add(
  libsodium_source
  URL https://github.com/jedisct1/libsodium/releases/download/1.0.18-RELEASE/libsodium-1.0.18.tar.gz
  URL_HASH SHA512=17e8638e46d8f6f7d024fe5559eccf2b8baf23e143fadd472a7d29d228b186d86686a5e6920385fe2020729119a5f12f989c3a782afbd05a8db4819bb18666ef
  CONFIGURE_COMMAND
  <SOURCE_DIR>/configure
    --prefix=<INSTALL_DIR>
    --disable-debug
    --disable-dependency-tracking
    --disable-shared
    --enable-static
)
add_dependencies(libsodium libsodium_source)
ExternalProject_Get_Property(libsodium_source INSTALL_DIR)
target_include_directories(libsodium INTERFACE ${INSTALL_DIR}/include)
target_link_libraries(
  libsodium
  INTERFACE
  ${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}sodium${CMAKE_STATIC_LIBRARY_SUFFIX}
)
