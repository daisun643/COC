# COC Game Project

同济大学2025秋程序设计范式期末项目——COC

基于 Cocos2d-x 引擎和 CMake 构建工具的游戏项目。

## 项目结构

```
COC/
├── CMakeLists.txt          # CMake 主配置文件
├── build.bat              # Windows 构建脚本
├── .gitignore            # Git 忽略文件配置
├── README.md             # 项目说明文档
├── Classes/              # 源代码目录
│   ├── AppDelegate.h     # 应用程序委托类（头文件）
│   ├── AppDelegate.cpp   # 应用程序委托类（实现）
│   ├── HelloWorldScene.h # 示例场景类（头文件）
│   ├── HelloWorldScene.cpp # 示例场景类（实现）
│   └── main.cpp          # 程序入口
└── Resources/            # 资源文件目录
    ├── fonts/            # 字体文件
    ├── images/           # 图片资源
    └── sounds/           # 音频资源
```

## 云文档

[腾讯云文档](https://docs.qq.com/doc/DTXB4TURWbFVVWmFQ?scene=b90ac372b1055b658db0c651XoOQv1)

包含内容：
* 开发流程
* 分支管理
* 类设计

## 快速开始

### 1. 配置 Cocos2d-x 路径

方式二：设置环境变量

在 Windows 中设置环境变量 `COCOS2DX_ROOT`：

在cmd中：

设置环境变量

```cmd
setx COCOS2DX_ROOT "D:\path\to\cocos2d-x-4.0"
```

通过回显检验`COCOS2DX_ROOT`环境变量：

```cmd
echo %COCOS2DX_ROOT%
```

### 2. 构建项目

使用构建脚本，即可搭建起VS的项目，VS项目的配置文件位于`build`下。

```bash
build.bat
```


### 3. 运行游戏

通过VS编译运行 或者 `COC\build\Release\COC.exe`

## Android 构建

### 前置要求

1. **Android SDK** - 建议使用 Android Studio 安装
2. **Android NDK** - 推荐 r21 或更高版本
3. **设置环境变量**：
   - `COCOS2DX_ROOT` - 指向 Cocos2d-x 4.0 引擎目录
   - `ANDROID_HOME` 或 `ANDROID_SDK_ROOT` - 指向 Android SDK 目录
   - `ANDROID_NDK_HOME` - 指向 Android NDK 目录

### 配置 local.properties

在 `proj.android` 目录下创建 `local.properties` 文件：

```properties
sdk.dir=C:\\Users\\YourName\\AppData\\Local\\Android\\Sdk
```

### 构建 APK

```bash
cd proj.android

# Debug 版本
gradlew assembleDebug

# Release 版本
gradlew assembleRelease
```

### 安装到设备

```bash
gradlew installDebug
```

生成的 APK 位于：`proj.android/app/build/outputs/apk/`

## 参考资料

- [Cocos2d-x 官方文档](https://docs.cocos2d-x.org/)
- [CMake 官方文档](https://cmake.org/documentation/)