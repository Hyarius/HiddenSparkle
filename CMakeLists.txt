cmake_minimum_required(VERSION 3.16)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)



	########################################
	#           Project creation           #
	########################################
project(Sparkle VERSION 0.0.1)



	########################################
	#        CPP standard definition       #
	########################################
# --- 1) Define the version of the language to Cpp20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 2) Remove the NOMINMAX from Windows compilation pipeline
add_compile_definitions(NOMINMAX)



	########################################
	#            CMake Includes            #
	########################################
# --- 1) Add resources custom function
#        Used to construct and save a sparkle header containing raw data for each resources inside the resources folder
include(cmake/add_resources.cmake)
include(cmake/add_coverage.cmake)

# --- 2) Utils for the installation process
#        Helpers to make the generated files relocatable and version-aware
include(CMakePackageConfigHelpers)

# --- 3) Utils to make the installation process more portable
#        Avoids hardcoding paths and fixes packaging on systems where lib != lib64
include(GNUInstallDirs)


	########################################
	#           Listing section            #
	########################################
# --- 1) Folder listing
set(SPARKLE_SRC_DIR ${CMAKE_SOURCE_DIR}/srcs)
set(SPARKLE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/includes)
set(SPARKLE_RESOURCE_DIR ${CMAKE_SOURCE_DIR}/resources)

# --- 2) File listing
file(GLOB_RECURSE SPARKLE_SOURCES ${SPARKLE_SRC_DIR}/*.cpp)
file(GLOB_RECURSE SPARKLE_HEADERS ${SPARKLE_INCLUDE_DIR}/*.hpp ${SPARKLE_INCLUDE_DIR}/*.h)
# Add extensionless convenience header for <sparkle> include.
list(APPEND SPARKLE_HEADERS ${SPARKLE_INCLUDE_DIR}/sparkle)
file(GLOB_RECURSE SPARKLE_RESOURCE_FILES CONFIGURE_DEPENDS RELATIVE ${CMAKE_SOURCE_DIR} ${SPARKLE_RESOURCE_DIR}*)

# --- 3) find package for libraries to link against
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)




	########################################
	#          Static analysis             #
	########################################
option(SPARKLE_ENABLE_CLANG_TIDY "Enable clang-tidy analysis during builds" OFF)

if (SPARKLE_ENABLE_CLANG_TIDY)
    find_program(SPARKLE_CLANG_TIDY_EXECUTABLE NAMES clang-tidy)

    if (SPARKLE_CLANG_TIDY_EXECUTABLE)
        set(SPARKLE_CLANG_TIDY_COMMON_ARGS --warnings-as-errors=*)
        set(SPARKLE_CLANG_TIDY_FIX_ARGS ${SPARKLE_CLANG_TIDY_COMMON_ARGS} --fix --format-style=file)
        set(CMAKE_CXX_CLANG_TIDY "${SPARKLE_CLANG_TIDY_EXECUTABLE};${SPARKLE_CLANG_TIDY_COMMON_ARGS}")
    else()
        message(FATAL_ERROR "SPARKLE_ENABLE_CLANG_TIDY is ON but clang-tidy was not found. Install LLVM or disable the option.")
    endif()

	if (SPARKLE_CLANG_TIDY_EXECUTABLE)
		add_custom_target(clang-tidy-fix
			COMMAND ${SPARKLE_CLANG_TIDY_EXECUTABLE} ${SPARKLE_CLANG_TIDY_FIX_ARGS} -p ${CMAKE_BINARY_DIR} ${SPARKLE_SOURCES}
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			COMMENT "Applying clang-tidy fixes across Sparkle sources"
			VERBATIM
		)
	endif()
endif()



	########################################
	#          Coverage instrumentation     #
	########################################
option(SPARKLE_ENABLE_COVERAGE "Enable code coverage instrumentation for supported toolchains" OFF)
set(SPARKLE_COVERAGE_MIN_LINE_RATE "75" CACHE STRING "Minimum line coverage percentage required by the coverage target.")


	########################################
	#    Output directory composition      #
	########################################
# --- 1) Build and output file folder definition 
# Set the max directory path to 4096 chars
set(CMAKE_OBJECT_PATH_MAX 4096)

# Architecture-based structure
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCH_FOLDER x64)
else()
    set(ARCH_FOLDER x86)
endif()

# Definition of the configuration mode key
set(CONFIGURATION_KEY "$<IF:$<CONFIG:Debug>,Debug,Release>")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/out/bin/${CONFIGURATION_KEY}/${ARCH_FOLDER}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/out/lib/${CONFIGURATION_KEY}/${ARCH_FOLDER}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/out/lib/${CONFIGURATION_KEY}/${ARCH_FOLDER}")


	########################################
	#            Build section             #
	########################################
# --- 1) Base library project creation
add_library(Sparkle STATIC ${SPARKLE_SOURCES} ${SPARKLE_HEADERS})

# --- 2) Construct the sparkle resource file using the tools
add_resources(${SPARKLE_RESOURCE_FILES})

# --- 3) Add a dependency between Sparkle and the construction of the sparkle resources header produce by add_resources
add_dependencies(Sparkle SparkleResourcesHeaderTarget)

# --- 4) Indicate the include directories, and set up the installation interface
# Add build-tree include for generated resources header
get_filename_component(SPARKLE_RES_HDR_DIR "${SPARKLE_RESOURCES_HEADER}" DIRECTORY)

# Declare the includes from sparkle and the include due to generate source header
target_include_directories(Sparkle
	PUBLIC
		$<BUILD_INTERFACE:${SPARKLE_INCLUDE_DIR}>
		$<BUILD_INTERFACE:${SPARKLE_RES_HDR_DIR}>
		$<INSTALL_INTERFACE:include>
	PRIVATE
		"${CMAKE_BINARY_DIR}"
)

# --- 5) Linking against the external libraries
target_link_libraries(Sparkle PUBLIC GLEW::GLEW)
target_link_libraries(Sparkle PUBLIC OpenGL::GL)
target_link_libraries(Sparkle PUBLIC dinput8 dxguid ws2_32)

	########################################
	#         Installation process         #
	########################################
# --- 1) Where to place the package config files
set(INSTALL_CONFIGDIR "${CMAKE_INSTALL_LIBDIR}/cmake/Sparkle")

# --- 2) Install the targets and headers
install(
    TARGETS Sparkle
    EXPORT  SparkleTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}   # for DLLs / tools, if any
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}   # for shared libs
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}   # for static libs
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# --- 3) Public headers from your source tree
# Install the includes folder
install(
    DIRECTORY ${SPARKLE_INCLUDE_DIR}/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install the generated resources header
install(
    FILES "${SPARKLE_RESOURCES_HEADER}"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

# --- 4) Export targets for find_package(Sparkle)
install(
    EXPORT SparkleTargets
    FILE   SparkleTargets.cmake
    NAMESPACE Sparkle::
    DESTINATION ${INSTALL_CONFIGDIR}
)

# --- 5) Generate Version files
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/SparkleConfigVersion.cmake"
    VERSION       ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# --- 6) Generate Config files
# Configure a relocatable Config file from a .in template
configure_package_config_file(
    "${CMAKE_SOURCE_DIR}/cmake/SparkleConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/SparkleConfig.cmake"
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/SparkleConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/SparkleConfigVersion.cmake"
    DESTINATION ${INSTALL_CONFIGDIR}
)

# --- 7) Register this build-tree with CMake's user package registry
export(
    EXPORT SparkleTargets
    FILE   "${CMAKE_CURRENT_BINARY_DIR}/SparkleTargets.cmake"
    NAMESPACE Sparkle::
)

	########################################
	#            Nested project            #
	########################################
# --- 1) Definition of keyword to build nested project
option(ENABLE_TESTS "Enable building of test units" OFF)
option(ENABLE_PLAYGROUND "Enable building of playground" OFF)

if (SPARKLE_ENABLE_COVERAGE AND NOT ENABLE_TESTS)
    message(FATAL_ERROR "SPARKLE_ENABLE_COVERAGE=ON requires ENABLE_TESTS=ON so that coverage can run the unit suite.")
endif()

# --- 2) Playground project
if (ENABLE_PLAYGROUND)
    add_subdirectory(playground)
endif()

# --- 3) Various test folder : TUs, User Tests, Integration, etc etc

if (ENABLE_TESTS)
	enable_testing()
    
	add_subdirectory(tests)

    sparkle_enable_coverage(
        TEST_TARGET SparkleTest
        LIBRARIES Sparkle
    )

endif()
