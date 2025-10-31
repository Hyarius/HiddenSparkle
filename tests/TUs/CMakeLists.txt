	########################################
	#            GoogleTest setup          #
	########################################
# --- 1) Locate GoogleTest and its imported targets
find_package(GTest REQUIRED)

# --- 2) Import gtest helper macros (gtest_discover_tests, etc.)
include(GoogleTest)



	########################################
	#           Listing section            #
	########################################
# --- 1) Base folders for tests
set(SPARKLE_TEST_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/srcs)

# --- 2) Collect all test sources
#        Uses GLOB_RECURSE for convenience while prototyping.
#        For strict builds, list sources explicitly.
file(GLOB_RECURSE SPARKLE_TEST_SOURCES ${SPARKLE_TEST_SRC_DIR}/*.cpp)

# --- 3) Include directories visible to tests
#        Add the local test headers first; library headers are provided by Sparkle's target includes.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/includes)



	########################################
	#            Build section             #
	########################################
# --- 1) Define the test executable
add_executable(SparkleTest ${SPARKLE_TEST_SOURCES})

# --- 2) Link against the library under test and GoogleTest
#        GTest::GTest provides the framework; GTest::Main supplies main().
target_link_libraries(SparkleTest PRIVATE
    Sparkle
    GTest::GTest
    GTest::Main
)



	########################################
	#          CTest integration           #
	########################################
# --- 1) Auto-register each TEST() with CTest
#        Scans SparkleTest at build time and adds one ctest per test case.
gtest_discover_tests(SparkleTest)
