cmake_minimum_required(VERSION 3.20)

# Versiya — YAGONA manba. Bu yerdan main.cpp ga (UZPP_VERSION) uzatiladi.
# CHANGELOG.md, vscode-uzpp/package.json va installer/windows/installer.iss
# shu raqam bilan mos bo'lishi kerak.
project(uz_plus_plus VERSION 2.3.1 LANGUAGES CXX)

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
target_compile_definitions(uzpp PRIVATE UZPP_VERSION="${PROJECT_VERSION}")
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

# ─── Stdlib stress testlari ─────────────────────────────────────────────────
# Ular ko'p oqimli `Kesh` va `Xotira` modullarini millionlab amal bilan
# sinaydi — bir necha daqiqa oladi, shuning uchun `add_test` ga qo'shilmagan
# (CI da avtomatik yugurmaydi). Lekin QURILADI: shu tariqa ular stdlib
# o'zgarganda jimgina chirimaydi.
#
# Qo'lda yugurtirish:  ./build/uzpp_stress_kesh   va   ./build/uzpp_stress_xotira
add_executable(uzpp_stress_kesh   tests/stress_test_kesh.cpp)
add_executable(uzpp_stress_xotira tests/stress_test_memory.cpp)
foreach(stress_target uzpp_stress_kesh uzpp_stress_xotira)
    target_include_directories(${stress_target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdlib)
    target_compile_features(${stress_target} PRIVATE cxx_std_23)
    if (NOT WIN32)
        find_package(Threads REQUIRED)
        target_link_libraries(${stress_target} PRIVATE Threads::Threads)
    endif()
endforeach()
