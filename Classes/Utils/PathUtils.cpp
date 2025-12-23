#include "PathUtils.h"

#include <algorithm>

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
#include <direct.h>
#include <io.h>
#include <windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

USING_NS_CC;

std::string PathUtils::getRealFilePath(const std::string& relativePath,
                                       bool forWrite) {
  std::string path;

  // [新增] 规范化传入的相对路径，防止调用者传入反斜杠
  std::string normalizedRelativePath = relativePath;
  std::replace(normalizedRelativePath.begin(), normalizedRelativePath.end(),
               '\\', '/');

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
  // [开发模式优化] 使用可执行文件路径定位源码目录
  char exePath[MAX_PATH];
  GetModuleFileNameA(NULL, exePath, MAX_PATH);
  std::string exeDir = std::string(exePath);

  // 统一将反斜杠替换为正斜杠，避免混合路径分隔符
  std::replace(exeDir.begin(), exeDir.end(), '\\', '/');

  size_t lastSlash = exeDir.find_last_of("/");
  if (lastSlash != std::string::npos) {
    exeDir = exeDir.substr(0, lastSlash);
  }

  // 向上查找项目根目录 (通过查找 Classes 文件夹来识别)
  // 避免误匹配到 build 目录下复制出来的 Resources
  std::string currentDir = exeDir;
  bool foundSourceRoot = false;
  std::string resourceRoot;

  for (int i = 0; i < 5; ++i) {
    // 检查是否存在 Classes 目录，这是源码目录的特征
    std::string testPath = currentDir + "/Classes";
    if (_access(testPath.c_str(), 0) == 0) {
      // 找到了源码根目录，构造 Resources 路径
      std::string sourceResources = currentDir + "/Resources";
      if (_access(sourceResources.c_str(), 0) == 0) {
        resourceRoot = sourceResources;
        foundSourceRoot = true;
        // CCLOG("DevMode: Found Source Root at: %s", currentDir.c_str());
        break;
      }
    }
    // 向上移动一级
    size_t slash = currentDir.find_last_of("/");
    if (slash == std::string::npos) break;
    currentDir = currentDir.substr(0, slash);
  }

  if (foundSourceRoot) {
    path = resourceRoot + "/" + normalizedRelativePath;
    return path;
  }
#endif

  // 如果不是 Windows 开发环境，或者没找到源码目录
  // 对于写入操作，通常应该使用 getWritablePath
  if (forWrite) {
    // 注意：getWritablePath() 返回的路径通常不包含 Resources，而是
    // AppData/Local 等 但为了保持当前逻辑（直接修改 Resources
    // 下的文件），我们先回退到默认 如果是发布版，这里应该改为
    // FileUtils::getInstance()->getWritablePath() + relativePath;
    // 目前保持原样，直接返回相对路径，由 FileUtils 处理（通常是只读的在移动端）
    path = "Resources/" + normalizedRelativePath;
  } else {
    // 读取操作
    path =
        FileUtils::getInstance()->fullPathForFilename(normalizedRelativePath);
    if (path.empty()) {
      path = "Resources/" + normalizedRelativePath;
    }
  }

  return path;
}

bool PathUtils::ensureDirectoryExists(const std::string& filePath) {
  std::string path = filePath;
  std::replace(path.begin(), path.end(), '\\', '/');

  size_t lastSlash = path.find_last_of("/");
  if (lastSlash == std::string::npos) {
    return true;
  }

  std::string dir = path.substr(0, lastSlash);
  if (dir.empty()) return true;

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
  if (_access(dir.c_str(), 0) == 0) {
    return true;
  }
#else
  if (access(dir.c_str(), F_OK) == 0) {
    return true;
  }
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
  return (_mkdir(dir.c_str()) == 0);
#else
  // 简单的非递归创建，对于目前需求足够
  return (mkdir(dir.c_str(), 0777) == 0);
#endif
}
