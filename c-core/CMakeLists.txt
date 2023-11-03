cmake_minimum_required(VERSION 3.20.2 FATAL_ERROR)
project(deciphon VERSION 0.13.7 LANGUAGES C)

include(cmake/warnings.cmake)
include(cmake/sanitizers.cmake)
include(cmake/CPM.cmake)

cpmaddpackage(
  NAME hmmer_reader
  GITHUB_REPOSITORY EBI-Metagenomics/hmmer3
  GIT_TAG hmmer-reader-v0.7.4
  VERSION 0.7.4
  SOURCE_SUBDIR hmmer-reader)
cpmaddpackage(
  NAME h3client
  GITHUB_REPOSITORY EBI-Metagenomics/hmmer3
  GIT_TAG h3client-v0.12.6
  VERSION 0.12.6
  SOURCE_SUBDIR h3client)
cpmaddpackage("gh:EBI-Metagenomics/imm@5.3.7")
cpmaddpackage("gh:EBI-Metagenomics/lip@0.5.4")
find_package(OpenMP)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

message(STATUS "CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
  set(X86 TRUE)
else()
  set(X86 FALSE)
endif()
message(STATUS "X86: ${X86}")

if(X86)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mavx")
endif()

file(GLOB DECIPHON_SOURCES "*.c")
file(GLOB DECIPHON_HEADERS "*.h")
list(FILTER DECIPHON_SOURCES EXCLUDE REGEX "test_.*")
list(FILTER DECIPHON_HEADERS EXCLUDE REGEX "test_.*")

add_library(deciphon ${DECIPHON_SOURCES})
add_library(DECIPHON::deciphon ALIAS deciphon)
target_link_libraries(deciphon PRIVATE IMM::imm)
target_link_libraries(deciphon PRIVATE LIP::lip)
target_link_libraries(deciphon PRIVATE H3CLIENT::h3client)
target_link_libraries(deciphon INTERFACE $<$<BOOL:${UNIX}>:m>)
target_link_libraries(deciphon PRIVATE HMMER_READER::hmmer_reader)
target_link_libraries(deciphon PRIVATE $<$<BOOL:${OpenMP_C_FOUND}>:OpenMP::OpenMP_C>)

file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/deciphon)
file(COPY ${DECIPHON_HEADERS} DESTINATION ${PROJECT_BINARY_DIR}/deciphon)

if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(deciphon PUBLIC -DDECIPHON_STATIC_DEFINE)
endif()

target_include_directories(
  deciphon
  PUBLIC $<INSTALL_INTERFACE:include>
         $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
  PRIVATE ${PROJECT_SOURCE_DIR}
)

set_target_properties(deciphon PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(deciphon PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

install(
  TARGETS deciphon
  EXPORT deciphon-targets
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
install(DIRECTORY ${PROJECT_BINARY_DIR}/deciphon DESTINATION include)

export(
  TARGETS deciphon
  NAMESPACE DECIPHON::
  FILE deciphon-targets.cmake)
install(
  EXPORT deciphon-targets
  NAMESPACE DECIPHON::
  DESTINATION lib/cmake/deciphon)

install(
  FILES ${PROJECT_SOURCE_DIR}/README.md ${PROJECT_SOURCE_DIR}/LICENSE
  DESTINATION share/docs/deciphon)

include(CMakePackageConfigHelpers)
set(PROJECT_CONFIG ${PROJECT_BINARY_DIR}/deciphon-config.cmake)
set(VERSION_CONFIG ${PROJECT_BINARY_DIR}/deciphon-config-version.cmake)
configure_package_config_file(deciphon-config.cmake.in ${PROJECT_CONFIG}
                              INSTALL_DESTINATION lib/cmake/deciphon)
write_basic_package_version_file(${VERSION_CONFIG}
                                 COMPATIBILITY SameMajorVersion)
install(FILES ${PROJECT_CONFIG} ${VERSION_CONFIG} DESTINATION lib/cmake/deciphon)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  enable_testing()

  add_test(NAME start_hmmer COMMAND start_hmmer.sh minifam.hmm WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
  add_test(NAME stop_hmmer COMMAND stop_hmmer.sh minifam.hmm WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
  set_tests_properties(start_hmmer PROPERTIES FIXTURES_SETUP hmmer)
  set_tests_properties(stop_hmmer PROPERTIES FIXTURES_CLEANUP hmmer)

  file(GLOB DECIPHON_TEST_SOURCES "test_*.c")
  foreach(source ${DECIPHON_TEST_SOURCES})
    get_filename_component(x ${source} NAME_WE)
    add_executable(${x} ${source})
    target_link_libraries(${x} PRIVATE deciphon)
    target_link_libraries(${x} PRIVATE IMM::imm)
    target_link_libraries(${x} PRIVATE LIP::lip)
    add_test(NAME ${x} COMMAND ${x} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
  endforeach()

  set_tests_properties(test_scan PROPERTIES FIXTURES_REQUIRED hmmer)
endif()
