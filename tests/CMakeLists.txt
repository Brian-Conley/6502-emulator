# ------------------------------
# Catch2
# ------------------------------
Include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.8.1 # or a later release
)
FetchContent_MakeAvailable(Catch2)

file(GLOB TEST_SOURCES "test_*.cpp")

add_executable(unit_tests ${TEST_SOURCES})
target_link_libraries(unit_tests PRIVATE
    Catch2::Catch2WithMain
    mos6502_lib
)

include(Catch)
catch_discover_tests(unit_tests)
