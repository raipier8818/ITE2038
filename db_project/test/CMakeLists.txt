# GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/3b49be074d5c1340eeb447e6a8e78427051e675a.zip)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(DB_TESTS
  # file_test.cc
  basic_test.cc
  # Add your test files here
  # foo/bar/your_test.cc
  )

add_executable(db_test ${DB_TESTS})

target_link_libraries(
  db_test
  db
  gtest_main
  )

include(GoogleTest)
gtest_discover_tests(db_test)

