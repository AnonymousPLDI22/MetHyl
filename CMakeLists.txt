cmake_minimum_required(VERSION 3.5.1)
project(DPSynthesisNew)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

ADD_DEFINITIONS(-DDEBUG)
#ADD_DEFINITIONS(-DTESTLEX)

SET(PARSER_DIR ${CMAKE_CURRENT_BINARY_DIR})

include_directories(. ./basic ./parser ${PARSER_DIR} hylodp solver)
INCLUDE_DIRECTORIES(/home/PolyGen/gurobi912/linux64/include)
file(GLOB basic_files "basic/*.cpp" "basic/*.h")
add_library(basic_lib ${basic_files})

find_package(BISON)
find_package(FLEX)
BISON_TARGET(my_parser parser/surface.yy ${PARSER_DIR}/parser.cpp
        DEFINES_FILE ${PARSER_DIR}/parser.h)
FLEX_TARGET(my_lexer parser/surface.ll ${PARSER_DIR}/lexer.cpp)
ADD_FLEX_BISON_DEPENDENCY(my_lexer my_parser)
file(GLOB parser_files "parser/*.cpp" ${PARSER_DIR}/parser.h "parser/*.h")
add_library(parser_lib ${parser_files})
find_library(LEX_LIB l)

file(GLOB solver_files "solver/*.cpp" "solver/*.h")
add_library(solver_lib ${solver_files})

file(GLOB main_files "hylodp/*.cpp" "hylodp/*.h")
add_library(main_lib ${main_files})
link_directories(/home/PolyGen/gurobi912/linux64/lib/)

file(GLOB polygen_files "polygen/*.cpp" "polygen/*.h")
add_library(polygen_lib ${polygen_files})

file(GLOB autolifter_files "autolifter/*.cpp" "autolifter/*.h")
add_library(autolifter_lib ${autolifter_files})

add_executable(main main.cpp ${PARSER_DIR}/parser.cpp ${PARSER_DIR}/lexer.cpp)
target_link_libraries(main main_lib parser_lib autolifter_lib polygen_lib solver_lib basic_lib
        ${LEX_LIB} libgurobi_g++5.2.a libgurobi91.so glog)