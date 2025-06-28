cmake_minimum_required(VERSION 3.27)
project(
        Joker
        VERSION 0.1.0
        LANGUAGES C
        DESCRIPTION " A C-based joker language build project."
)

# 设置 C 标准版本
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED OFF)
set(CMAKE_C_EXTENSIONS ON)

# 定义项目目录结构
set(PROJECT_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(PROJECT_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)


# 所有源文件
file(GLOB_RECURSE SOURCES
        "${PROJECT_SRC_DIR}/*.c"
        # "${PROJECT_SRC_DIR}/*.h"
)

# 可执行目标
add_executable(${PROJECT_NAME} joker.c
        ${SOURCES}
)

# 头文件目录
target_include_directories(${PROJECT_NAME} PRIVATE
        ${PROJECT_INC_DIR}
        # ${PROJECT_SRC_DIR}
)

find_package(Threads REQUIRED)
target_link_libraries(Joker PRIVATE atomic ${CMAKE_THREAD_LIBS_INIT})

add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
add_link_options(-fsanitize=address)

# 启用 clang-tidy (需 cmake 3.6+)
find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
if(CLANG_TIDY_EXE)
    # 定义检查规则
    set(CLANG_TIDY_CHECKS
            "-checks=*,-modernize-use-trailing-return-type,-readability-magic-numbers"
    )
    # 将 clang-tidy 附加到目标
    set(CMAKE_C_CLANG_TIDY
            "${CLANG_TIDY_EXE}"
            "${CLANG_TIDY_CHECKS}"
            "-header-filter=.*"    # 匹配所有头文件
    )
endif()

set(CLANG_TIDY_CHECKS
        "-config-file=${CMAKE_CURRENT_SOURCE_DIR}/.clang-tidy"
)

# 添加编译选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE
            -g       # 生成调试信息
            -Wall    # 显示所有警告
            -Wextra  # 显示更多警告
            -Werror  # 将警告转换为错误
            -O3
    )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE
            -O2      # 最小化编译
            -DNDEBUG # 禁用调试信息
    )
endif()

if(WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0A00)  # 统一通过 CMake 设置
endif()

# 安装规则
install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin
)
