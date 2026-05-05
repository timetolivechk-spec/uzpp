cmake_minimum_required(VERSION 3.20)

project(uz_plus_plus LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

add_library(uzpp_frontend
    src/codegen.cpp
    src/lexer.cpp
    src/parser.cpp
    src/formatter.cpp
    src/lsp_server.cpp
    src/dap_server.cpp
)

target_include_directories(uzpp_frontend
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/stdlib
)

target_compile_features(uzpp_frontend PUBLIC cxx_std_23)

if (WIN32)
    target_link_libraries(uzpp_frontend PUBLIC ws2_32 user32 gdi32 opengl32)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(uzpp_frontend PUBLIC X11 GL)
endif()

add_executable(uzpp src/main.cpp)
target_link_libraries(uzpp PRIVATE uzpp_frontend)
if (WIN32)
    target_link_options(uzpp PRIVATE -static)
endif()

enable_testing()

add_executable(uzpp_frontend_tests tests/frontend_smoke.cpp)
target_link_libraries(uzpp_frontend_tests PRIVATE uzpp_frontend)
if (WIN32)
    target_link_options(uzpp_frontend_tests PRIVATE -static)
endif()

add_test(NAME uzpp_frontend_smoke COMMAND uzpp_frontend_tests)
