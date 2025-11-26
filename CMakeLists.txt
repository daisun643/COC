cmake_minimum_required(VERSION 3.10)
project(COC)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置运行时库（修复 LNK4098 警告）
# 使用 MultiThreadedDLL (/MD) 以匹配大多数 Windows 库
if(MSVC)
    # 启用 CMP0091 策略，允许设置运行时库
    cmake_policy(SET CMP0091 NEW)
    
    # 设置全局运行时库为 MultiThreadedDLL (/MD)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL" CACHE STRING "MSVC runtime library" FORCE)
    
    # 为所有新创建的目标设置默认运行时库
    set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)
    
    # 设置编译器标志（确保使用 /MD）
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MD")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MDd")
endif()

# 设置输出目录（根据构建类型分别输出到 release 和 debug 目录）
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/release)
# 默认输出目录（如果未指定构建类型）
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Cocos2d-x 路径配置
if(DEFINED ENV{COCOS2DX_ROOT})
    set(COCOS2DX_ROOT "$ENV{COCOS2DX_ROOT}")
    message(STATUS "Found environment Cocos2d-x: ${COCOS2DX_ROOT}")
else()
    message(FATAL_ERROR "未找到 cocos2d 引擎！请正确设置环境变量 COCOS2DX_ROOT。")
endif()

# 包含 Cocos2d-x CMake 配置
if(EXISTS "${COCOS2DX_ROOT}/CMakeLists.txt")
    # 禁用测试项目构建（只构建引擎库）
    # 注意：Cocos2d-x 主 CMakeLists.txt 会强制构建测试，但我们可以通过运行时库设置来避免冲突
    set(BUILD_LUA_LIBS OFF CACHE BOOL "Build lua libraries" FORCE)
    
    # 添加 Windows API 兼容性定义（修复 GWL_WNDPROC 和 GWL_USERDATA 未定义问题）
    # 这些常量在某些 Windows SDK 版本中可能未定义，需要手动定义
    if(WIN32)
        # 定义 Windows API 常量（兼容旧代码）
        # GWL_WNDPROC = -4, GWL_USERDATA = -21
        add_definitions(-DUNICODE -D_UNICODE)
        add_definitions(-DGWL_WNDPROC=-4)
        add_definitions(-DGWL_USERDATA=-21)
    endif()
    
    # 在包含 Cocos2d-x 之前，确保运行时库设置会应用到所有子项目
    if(MSVC)
        # 设置环境变量，确保子项目也使用相同的运行时库
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL" CACHE STRING "" FORCE)
    endif()
    
    add_subdirectory(${COCOS2DX_ROOT} cocos2d)
    
    # 包含 Cocos2d-x 后，为所有已创建的目标设置运行时库
    if(MSVC)
        # 为 Cocos2d-x 的目标设置运行时库
        if(TARGET cocos2d)
            set_property(TARGET cocos2d PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
        endif()
        if(TARGET external)
            set_property(TARGET external PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
        endif()
    endif()
else()
    message(FATAL_ERROR "Cocos2d-x not found at ${COCOS2DX_ROOT}. Please set COCOS2DX_ROOT to your Cocos2d-x installation path.")
endif()

# 源文件
file(GLOB_RECURSE GAME_SOURCES
    "Classes/*.cpp"
    "Classes/*.h"
)

# 资源文件
file(GLOB_RECURSE GAME_RESOURCES
    "Resources/*"
)

# 创建可执行文件
add_executable(${PROJECT_NAME} WIN32
    ${GAME_SOURCES}
    ${GAME_RESOURCES}
)

# 设置运行时库（确保与 Cocos2d-x 一致）
if(MSVC)
    set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    
    # 添加链接器标志，忽略冲突的默认库（抑制 LNK4098 警告）
    # 这告诉链接器忽略 LIBCMT 和 MSVCRT，因为我们使用的是 MultiThreadedDLL (/MD)
    get_target_property(CURRENT_LINK_FLAGS ${PROJECT_NAME} LINK_FLAGS)
    if(CURRENT_LINK_FLAGS)
        set_target_properties(${PROJECT_NAME} PROPERTIES
            LINK_FLAGS "${CURRENT_LINK_FLAGS} /NODEFAULTLIB:LIBCMT /NODEFAULTLIB:LIBCMTD"
        )
    else()
        set_target_properties(${PROJECT_NAME} PROPERTIES
            LINK_FLAGS "/NODEFAULTLIB:LIBCMT /NODEFAULTLIB:LIBCMTD"
        )
    endif()
endif()

# 包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Classes
    ${COCOS2DX_ROOT}
    ${COCOS2DX_ROOT}/cocos
    ${COCOS2DX_ROOT}/external
)

# 链接 Cocos2d-x 库（确保依赖关系正确）
add_dependencies(${PROJECT_NAME} cocos2d external)

# 获取 Cocos2d-x 链接的库
get_target_property(COCOS2D_LINK_LIBS cocos2d INTERFACE_LINK_LIBRARIES)
if(COCOS2D_LINK_LIBS)
    target_link_libraries(${PROJECT_NAME} ${COCOS2D_LINK_LIBS})
endif()

target_link_libraries(${PROJECT_NAME}
    cocos2d
    external
)

# Windows 特定库
if(WIN32)
    # 链接 Windows 系统库
    target_link_libraries(${PROJECT_NAME}
        winmm
        ws2_32
        wsock32
    )
endif()

# Windows 特定设置
if(WIN32)
    # 复制 Cocos2d-x DLL 文件到输出目录
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:cocos2d>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
    
    # 复制必需的第三方 DLL 文件到输出目录
    # GLEW (OpenGL Extension Wrangler Library)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${COCOS2DX_ROOT}/external/win32-specific/gles/prebuilt/glew32.dll
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
    
    # ZLib (压缩库)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${COCOS2DX_ROOT}/external/win32-specific/zlib/prebuilt/zlib1.dll
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
    
    # OpenAL (音频库) - 如果使用音频功能需要
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${COCOS2DX_ROOT}/external/win32-specific/OpenalSoft/prebuilt/OpenAL32.dll
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
    
    # iconv (字符编码转换库)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${COCOS2DX_ROOT}/external/win32-specific/icon/prebuilt/iconv.dll
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
    
    # 复制资源文件到输出目录
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Resources
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/Resources
    )
    
    # 设置资源文件的工作目录
    set_target_properties(${PROJECT_NAME} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )
endif()
