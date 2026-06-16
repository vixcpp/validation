cmake_minimum_required(VERSION 3.20)

# This CMakeLists.txt is included from the module root when:
#   -DVIX_VALIDATION_BUILD_TESTS=ON

include(CTest)
enable_testing()

# ------------------------------------------------------------
# Validation tests
# ------------------------------------------------------------
# These tests are smoke-style executable tests.
# Each test source owns its own main(), so they must be built as
# one executable per source file.
# Do not aggregate them into a single GoogleTest executable.
# ------------------------------------------------------------

set(VIX_VALIDATION_TESTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB_RECURSE VIX_VALIDATION_TEST_SOURCES
  "${VIX_VALIDATION_TESTS_DIR}/*.cpp"
)

if (NOT VIX_VALIDATION_TEST_SOURCES)
  message(STATUS "[validation][tests] No test sources found.")
  return()
endif()

function(vix_validation_add_plain_test name src)
  add_executable(${name} ${src})

  target_compile_features(${name}
    PRIVATE
      cxx_std_20
  )

  target_link_libraries(${name}
    PRIVATE
      vix::validation
  )

  if (TARGET vix_warnings)
    target_link_libraries(${name}
      PRIVATE
        vix_warnings
    )
  endif()

  if (VIX_ENABLE_SANITIZERS AND TARGET vix_sanitizers)
    target_link_libraries(${name}
      PRIVATE
        vix_sanitizers
    )
  endif()

  add_test(
    NAME ${name}
    COMMAND ${name}
  )
endfunction()

message(STATUS "[validation][tests] Building plain executable tests.")

foreach(src IN LISTS VIX_VALIDATION_TEST_SOURCES)
  get_filename_component(fname "${src}" NAME_WE)
  set(tname "vix_validation_test_${fname}")
  vix_validation_add_plain_test(${tname} "${src}")
endforeach()
