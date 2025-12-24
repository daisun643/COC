cmake_minimum_required(VERSION 3.10)

# ================= 0. Android NDK 配置 (必须在 project() 之前) =================
if(ANDROID)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

project(COC)

# ================= 1. 基础设置 =================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    cmake_policy(SET CMP0091 NEW)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL" CACHE STRING "" FORCE)
    set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MD")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MDd")
endif()

# 输出目录设置 (仅非 Android 平台)
if(NOT ANDROID)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/debug)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/release)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif()

# ================= 2. 路径配置 =================
# Android 平台从 local.properties 或环境变量读取
if(ANDROID)
    # Android 构建时，COCOS2DX_ROOT 需要通过 CMake 参数传入
    if(NOT DEFINED COCOS2DX_ROOT)
        if(DEFINED ENV{COCOS2DX_ROOT})
            set(COCOS2DX_ROOT "$ENV{COCOS2DX_ROOT}")
        else()
            message(FATAL_ERROR "Android 构建需要设置 COCOS2DX_ROOT 变量！")
        endif()
    endif()
    message(STATUS "Android: Using Cocos2d-x root: ${COCOS2DX_ROOT}")
elseif(DEFINED ENV{COCOS2DX_ROOT})
    set(COCOS2DX_ROOT "$ENV{COCOS2DX_ROOT}")
    message(STATUS "Using Cocos2d-x root: ${COCOS2DX_ROOT}")
else()
    message(FATAL_ERROR "未找到引擎！请设置环境变量 COCOS2DX_ROOT。")
endif()

# ================= 3. 引擎配置与警告屏蔽 =================
if(EXISTS "${COCOS2DX_ROOT}/CMakeLists.txt")
    set(BUILD_LUA_LIBS OFF CACHE BOOL "Build lua libraries" FORCE)
    
    if(WIN32)
        add_definitions(-DUNICODE -D_UNICODE)
        add_definitions(-DGWL_WNDPROC=-4)
        add_definitions(-DGWL_USERDATA=-21)
    endif()
    
    # [新增] 解决 strncpy 等不安全警告
    if(MSVC)
        add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    endif()
    
    # [关键] 设置引擎路径变量，供 cocos 子目录使用
    set(COCOS2DX_ROOT_PATH ${COCOS2DX_ROOT})
    set(CMAKE_MODULE_PATH ${COCOS2DX_ROOT_PATH}/cmake/Modules/)
    
    # 引入引擎构建辅助模块
    include(CocosBuildSet)
    
    # 只添加引擎核心，不添加整个引擎项目（避免测试和模板项目）
    set(ENGINE_BINARY_PATH ${CMAKE_BINARY_DIR}/cocos2d/engine)
    add_subdirectory(${COCOS2DX_ROOT}/cocos ${ENGINE_BINARY_PATH}/cocos/core)
    
    # 标记引擎已构建，防止其他地方重复构建
    set(BUILD_ENGINE_DONE ON)
    
    # [新增] 暴力屏蔽引擎所有子模块的警告 (/W0)
    if(MSVC)
        set(ENGINE_TARGETS cocos2d external ext_unzip ext_poly2tri ext_tinyxml2 ext_xxhash ext_clipper ext_edtaa3func ext_convertUTF ext_recast)
        foreach(t ${ENGINE_TARGETS})
            if(TARGET ${t})
                target_compile_options(${t} PRIVATE "/W0")
            endif()
        endforeach()
    endif()

    if(MSVC)
        if(TARGET cocos2d)
            set_property(TARGET cocos2d PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
        endif()
        if(TARGET external)
            set_property(TARGET external PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
        endif()
    endif()
else()
    message(FATAL_ERROR "Cocos2d-x path invalid.")
endif()

# ================= 4. 项目文件配置 =================
file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS "Classes/*.cpp" "Classes/*.h")

# 排除 class/test 目录下的文件
list(FILTER GAME_SOURCES EXCLUDE REGEX "Classes/test/.*")

# Android 平台需要添加 jni 入口文件
if(ANDROID)
    list(APPEND GAME_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/proj.android/app/jni/hellocpp/main.cpp
    )
    # ==============================================================================
    # 修复安卓运行时崩溃的终极补丁 (Cocos2d-x 4.0 专用)
    # ==============================================================================
    
    # 1. 核心 JNI 入口 (必须!)
    if(EXISTS "${COCOS2DX_ROOT_PATH}/cocos/platform/android/javaactivity-android.cpp")
        list(APPEND GAME_SOURCES
            ${COCOS2DX_ROOT_PATH}/cocos/platform/android/javaactivity-android.cpp
        )
    endif()
    if(EXISTS "${COCOS2DX_ROOT_PATH}/cocos/platform/android/jni/JniHelper.cpp")
        list(APPEND GAME_SOURCES
            ${COCOS2DX_ROOT_PATH}/cocos/platform/android/jni/JniHelper.cpp
        )
    endif()
    
    # 2. 渲染器 JNI 绑定 (解决 nativeRender 找不到的问题)
    if(EXISTS "${COCOS2DX_ROOT_PATH}/cocos/platform/android/jni/Java_org_cocos2dx_lib_Cocos2dxRenderer.cpp")
        list(APPEND GAME_SOURCES
            ${COCOS2DX_ROOT_PATH}/cocos/platform/android/jni/Java_org_cocos2dx_lib_Cocos2dxRenderer.cpp
        )
    endif()
    
    # 2.5 触摸事件 JNI 绑定 (解决 nativeTouchesBegin 找不到的问题)
    if(EXISTS "${COCOS2DX_ROOT_PATH}/cocos/platform/android/jni/TouchesJni.cpp")
        list(APPEND GAME_SOURCES
            ${COCOS2DX_ROOT_PATH}/cocos/platform/android/jni/TouchesJni.cpp
        )
    endif()
    
    # 3. 辅助功能 JNI (解决其他潜在崩溃)
    if(EXISTS "${COCOS2DX_ROOT_PATH}/cocos/platform/android/jni/Java_org_cocos2dx_lib_Cocos2dxHelper.cpp")
        list(APPEND GAME_SOURCES
            ${COCOS2DX_ROOT_PATH}/cocos/platform/android/jni/Java_org_cocos2dx_lib_Cocos2dxHelper.cpp
        )
    endif()
    if(EXISTS "${COCOS2DX_ROOT_PATH}/cocos/platform/android/jni/Java_org_cocos2dx_lib_Cocos2dxBitmap.cpp")
        list(APPEND GAME_SOURCES
            ${COCOS2DX_ROOT_PATH}/cocos/platform/android/jni/Java_org_cocos2dx_lib_Cocos2dxBitmap.cpp
        )
    endif()
    if(EXISTS "${COCOS2DX_ROOT_PATH}/cocos/platform/android/jni/Java_org_cocos2dx_lib_Cocos2dxAccelerometer.cpp")
        list(APPEND GAME_SOURCES
            ${COCOS2DX_ROOT_PATH}/cocos/platform/android/jni/Java_org_cocos2dx_lib_Cocos2dxAccelerometer.cpp
        )
    endif()
    
    # 4. 硬件与系统交互实现
    if(EXISTS "${COCOS2DX_ROOT_PATH}/cocos/platform/android/CCDevice-android.cpp")
        list(APPEND GAME_SOURCES
            ${COCOS2DX_ROOT_PATH}/cocos/platform/android/CCDevice-android.cpp
        )
    endif()
    if(EXISTS "${COCOS2DX_ROOT_PATH}/cocos/platform/android/CCCommon-android.cpp")
        list(APPEND GAME_SOURCES
            ${COCOS2DX_ROOT_PATH}/cocos/platform/android/CCCommon-android.cpp
        )
    endif()
    if(EXISTS "${COCOS2DX_ROOT_PATH}/cocos/platform/android/CCFileUtils-android.cpp")
        list(APPEND GAME_SOURCES
            ${COCOS2DX_ROOT_PATH}/cocos/platform/android/CCFileUtils-android.cpp
        )
    endif()
    if(EXISTS "${COCOS2DX_ROOT_PATH}/cocos/platform/android/CCBitmap-android.cpp")
        list(APPEND GAME_SOURCES
            ${COCOS2DX_ROOT_PATH}/cocos/platform/android/CCBitmap-android.cpp
        )
    endif()
    
    # 5. 音频引擎实现
    if(EXISTS "${COCOS2DX_ROOT_PATH}/cocos/audio/android/AudioEngine-android.cpp")
        list(APPEND GAME_SOURCES
            ${COCOS2DX_ROOT_PATH}/cocos/audio/android/AudioEngine-android.cpp
        )
    endif()

    # 强制包含 JNI 目录，否则编译上面的文件会报头文件找不到
    include_directories(
        ${COCOS2DX_ROOT_PATH}/cocos/platform/android
        ${COCOS2DX_ROOT_PATH}/cocos/platform/android/jni
        ${COCOS2DX_ROOT_PATH}/cocos/audio/android
    )
    # ==============================================================================
endif()

file(GLOB_RECURSE GAME_RESOURCES CONFIGURE_DEPENDS "Resources/*")

# Android 构建共享库，其他平台构建可执行文件
if(ANDROID)
    add_library(${PROJECT_NAME} SHARED ${GAME_SOURCES})
    # Android 需要链接 log 库
    target_link_libraries(${PROJECT_NAME} log)
else()
    add_executable(${PROJECT_NAME} WIN32 ${GAME_SOURCES} ${GAME_RESOURCES})
endif()

# [新增] 只针对你的代码开启 UTF-8 (解决 C4819 中文乱码)
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE "/utf-8")
endif()

# 运行时库修正
if(MSVC)
    set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/NODEFAULTLIB:LIBCMT /NODEFAULTLIB:LIBCMTD")
endif()

# ================= 5. 包含目录 (SYSTEM 屏蔽头文件警告) =================
# 自己的代码 (正常检查)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Classes)

# [修改] 引擎代码设为 SYSTEM (屏蔽 include 进来的警告)
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE
    ${COCOS2DX_ROOT}
    ${COCOS2DX_ROOT}/cocos
    ${COCOS2DX_ROOT}/external
)

# 链接库
add_dependencies(${PROJECT_NAME} cocos2d external)
get_target_property(COCOS2D_LINK_LIBS cocos2d INTERFACE_LINK_LIBRARIES)
if(COCOS2D_LINK_LIBS)
    target_link_libraries(${PROJECT_NAME} ${COCOS2D_LINK_LIBS})
endif()
target_link_libraries(${PROJECT_NAME} cocos2d external)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} winmm ws2_32 wsock32)
endif()

# ================= 6. 资源复制 (保持原样) =================
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:cocos2d> $<TARGET_FILE_DIR:${PROJECT_NAME}>)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${COCOS2DX_ROOT}/external/win32-specific/gles/prebuilt/glew32.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${COCOS2DX_ROOT}/external/win32-specific/zlib/prebuilt/zlib1.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${COCOS2DX_ROOT}/external/win32-specific/OpenalSoft/prebuilt/OpenAL32.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${COCOS2DX_ROOT}/external/win32-specific/icon/prebuilt/iconv.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>)
    # 复制 libcurl.dll (HTTP 请求需要，x86 版本)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${COCOS2DX_ROOT}/external/curl/prebuilt/win32/libcurl.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>)
    # 复制 OpenSSL DLL (libcurl 依赖，x86 版本)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${COCOS2DX_ROOT}/external/openssl/prebuilt/win32/libssl-1_1.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${COCOS2DX_ROOT}/external/openssl/prebuilt/win32/libcrypto-1_1.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/Resources $<TARGET_FILE_DIR:${PROJECT_NAME}>/Resources)
    set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif()