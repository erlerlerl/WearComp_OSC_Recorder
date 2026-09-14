cmake_minimum_required(VERSION 3.20)
project(WearComp_OSC_Recorder LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- OSC library: liblo ---

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBLO REQUIRED liblo)



# --- Executable ---
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})

# --- Include-Ordner ---
target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${LIBLO_INCLUDE_DIRS}
)

# --- Libraries ---

target_link_directories(${PROJECT_NAME} PRIVATE ${LIBLO_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBLO_LIBRARIES})
