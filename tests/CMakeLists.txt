cmake_minimum_required(VERSION 3.20)

# This CMakeLists.txt is included from the module root when:
#   -DVIX_VALIDATION_BUILD_TESTS=ON

include(CTest)
enable_testing()

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
  target_compile_features(${name} PRIVATE cxx_std_20)
  target_link_libraries(${name} PRIVATE vix::validation)

  if (TARGET vix_warnings)
    target_link_libraries(${name} PRIVATE vix_warnings)
  endif()

  if (VIX_ENABLE_SANITIZERS AND TARGET vix_sanitizers)
    target_link_libraries(${name} PRIVATE vix_sanitizers)
  endif()

  add_test(NAME ${name} COMMAND ${name})
endfunction()

if (TARGET GTest::gtest AND TARGET GTest::gtest_main)
  message(STATUS "[validation][tests] Using GoogleTest.")

  add_executable(vix_validation_tests ${VIX_VALIDATION_TEST_SOURCES})
  target_compile_features(vix_validation_tests PRIVATE cxx_std_20)
  target_link_libraries(vix_validation_tests
    PRIVATE
      vix::validation
      GTest::gtest
      GTest::gtest_main
  )

  if (TARGET vix_warnings)
    target_link_libraries(vix_validation_tests PRIVATE vix_warnings)
  endif()

  if (VIX_ENABLE_SANITIZERS AND TARGET vix_sanitizers)
    target_link_libraries(vix_validation_tests PRIVATE vix_sanitizers)
  endif()

  include(GoogleTest)
  gtest_discover_tests(vix_validation_tests)

else()
  message(STATUS "[validation][tests] GoogleTest not found. Building plain tests.")

  foreach(src ${VIX_VALIDATION_TEST_SOURCES})
    get_filename_component(fname ${src} NAME_WE)
    set(tname "vix_validation_test_${fname}")
    vix_validation_add_plain_test(${tname} ${src})
  endforeach()
endif()
