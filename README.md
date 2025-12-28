# COC Game Project

同济大学 2025 秋程序设计范式期末项目 —— COC

基于 Cocos2d-x 引擎和 CMake 构建的游戏项目（Windows + Android，带简单 Python 后端）。

## 项目结构（简要）

```
COC/
├── CMakeLists.txt       # CMake 主配置
├── build.bat            # Windows 构建脚本
├── README.md            # 本文件
├── Classes/             # C++ 源代码
├── Resources/           # 资源
├── proj.android/        # Android 工程
└── server/              # 简单后端（Flask）
```

## 云文档

[腾讯云文档](https://docs.qq.com/doc/DTXB4TURWbFVVWmFQ?scene=b90ac372b1055b658db0c651XoOQv1)

包含：开发流程、分支管理、类设计等设计文档。

## 先决条件

- Windows 开发：
    - Visual Studio 2022（推荐），安装 "Desktop development with C++" 工作负载
    - CMake >= 3.10
    - Cocos2d-x 4.0——本项目依赖本地引擎源码
- Android（移动端）：
    - Android Studio（含 SDK/NDK）和合适的 NDK 版本
    - Java JDK
- 后端（可选）：
    - Python 3.8+（用于运行 `server/run.py`）

## Windows：本地构建（推荐）

1. 设置 `COCOS2DX_ROOT`（永久）：

```cmd
setx COCOS2DX_ROOT "D:\path\to\cocos2d-x-4.0"
```

设置后请重新打开终端/VS 才能看到新变量；也可在 CMake 命令中临时传入 `-DCOCOS2DX_ROOT=...`。

2. 使用仓库自带脚本快速生成 VS 解决方案并构建：

```cmd
:: 在仓库根目录
build.bat
```

脚本说明：默认使用生成器 `Visual Studio 17 2022` 且 `-A Win32`（32-bit）。如需 x64：编辑 `build.bat` 中的 `-A` 或手动运行：

```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64 -DCOCOS2DX_ROOT="D:\path\to\cocos2d-x-4.0"
cmake --build . --config Release --target COC
```

3. 可执行文件位置：

- 常见位置：`build/release/`（由 CMakeLists 指定），或直接打开生成的 Visual Studio 解决方案 `build/COC.sln` 后在 IDE 中运行/调试。

故障排查要点：
- 如果 CMake 报错找不到引擎，请确认 `COCOS2DX_ROOT` 指向引擎源码根目录并可访问。
- 若提示找不到生成器或编译器，请确认已安装 Visual Studio 和 C++ 工作负载。

## Android（移动端）

本仓库含 `proj.android/`，使用 Android Studio 或 Gradle 命令行构建。

前置项：Android Studio、SDK、NDK、Java JDK。确保 `proj.android/local.properties` 指定 `sdk.dir`（Android Studio 会自动生成）。

常见构建步骤（命令行）：

```cmd
cd proj.android
gradlew.bat assembleDebug
# 或生成 release： gradlew.bat assembleRelease
```

如果使用 Android Studio：
- 删除 `proj.android/app/.cxx` 和 `proj.android/app/build`（如需清洁构建）
- 打开 Android Studio -> Sync Project with Gradle Files -> Build -> Generate Signed Bundle / APKs

注意：
- 若 C++ 层依赖 Cocos 引擎代码，确保在运行或导出前已通过 CMake 将引擎路径（`COCOS2DX_ROOT`）配置到项目中。

## 后端（Server）说明

项目包含一个轻量 Flask 服务，入口为 `server/run.py`，路由实现位于 `server/app/api/`，用户数据保存在 `server/static/user/users.json`。

快速运行（Windows）：

```cmd
cd server
python -m venv venv
venv\Scripts\activate
pip install Flask
python run.py
```

说明：默认 `run.py` 绑定端口 `80`（需要管理员权限）。开发时建议修改为端口 `5000` 或在命令行以管理员权限运行：

```cmd
:: 修改 run.py 中的 app.run(..., port=5000) 或
set FLASK_RUN_PORT=5000
python run.py
```

如果需要生成 `requirements.txt`：

```cmd
pip freeze > requirements.txt
```

安全与持久化提示：
- 当前后端使用基于文件的存储 `users.json`，仅适合测试；生产请切换到数据库并做好认证/权限控制。

## 常见问题与建议

- CMake 报错 "未找到引擎"：设置 `COCOS2DX_ROOT` 或在 CMake 命令中传入 `-DCOCOS2DX_ROOT`。
- 32/64 位：`build.bat` 默认构建 Win32，可按需改为 `-A x64`。
- 绑定 80 端口失败：非管理员请使用 5000 或其他高端口。

## 参考资料

- Cocos2d-x 官方文档: https://docs.cocos2d-x.org/
- CMake 官方文档: https://cmake.org/documentation/