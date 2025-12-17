#include "PathUtils.h"

#include <algorithm>

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
#include <direct.h>
#include <io.h>
#include <windows.h>
#endif

USING_NS_CC;

std::string PathUtils::getRealFilePath(const std::string& relativePath,
                                       bool forWrite) {
  std::string path;

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

  // 向上查找 Resources 目录 (最多向上5层)
  std::string currentDir = exeDir;
  bool foundResources = false;
  std::string resourceRoot;

  for (int i = 0; i < 5; ++i) {
    std::string testPath = currentDir + "/Resources";
    if (_access(testPath.c_str(), 0) == 0) {
      resourceRoot = testPath;
      foundResources = true;
      // CCLOG("DevMode: Found Resources at: %s", testPath.c_str());
      break;
    }
    // 向上移动一级
    size_t slash = currentDir.find_last_of("/");
    if (slash == std::string::npos) break;
    currentDir = currentDir.substr(0, slash);
  }

  if (foundResources) {
    path = resourceRoot + "/" + relativePath;
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
    path = "Resources/" + relativePath;
  } else {
    // 读取操作
    path = FileUtils::getInstance()->fullPathForFilename(relativePath);
    if (path.empty()) {
      path = "Resources/" + relativePath;
    }
  }

  return path;
}
