cmake_minimum_required(VERSION 2.8.8)
project(thundercat)

message(STATUS ${llvm_libs})
set(LLVM_DIR "~/Programs/llvm/build/share/llvm/cmake/")
find_package(LLVM 3.5.0 REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS}
                    ${LLVM_BUILD_MAIN_SRC_DIR}
                    ${LLVM_BUILD_BINARY_DIR}/lib/Target/X86/)
add_definitions(${LLVM_DEFINITIONS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -O3 -fno-rtti -std=c++11" )

set(dir ${CMAKE_CURRENT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${dir}")

set(SOURCE_FILES csrByNZ.cpp
                 csrByNZAnalyzer.cpp
                 genOski.cpp
                 genOskiAnalyzer.cpp
                 main.cpp
                 matrix.cpp
                 method.cpp
                 mkl.cpp
                 plaincsr.cpp
                 profiler.cpp
                 spMVgen.cpp
                 stencil.cpp
                 stencilAnalyzer.cpp
                 svmAnalyzer.cpp
                 unfolding.cpp
                 unfoldingAnalyzer.cpp
                 unrollingWithGOTO.cpp
                 unrollingWithGOTOAnalyzer.cpp)

add_executable(spMVgen ${SOURCE_FILES})

llvm_map_components_to_libnames(llvm_libs all)

target_link_libraries(spMVgen ${llvm_libs})