cmake_minimum_required(VERSION 3.10)

project(symfuscate)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-strong -fPIE -D_FORTIFY_SOURCE=2 -O2 -Wl,-z,relro,-z,now,-z,noexecstack -s")

include_directories(${PROJECT_SOURCE_DIR}/src)

set(SOURCES
    src/Main.c
    src/Hashing.c
    src/Elf_Utils.c
)

add_executable(symfuscate ${SOURCES})

target_link_libraries(symfuscate dl)