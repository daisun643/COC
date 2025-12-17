#ifndef __PATH_UTILS_H__
#define __PATH_UTILS_H__

#include <string>

#include "cocos2d.h"

class PathUtils {
 public:
  /**
   * 获取用于读写配置文件的真实路径
   * 在 Windows 开发模式下，会尝试向上查找源代码中的 Resources 目录
   * 在发布模式或移动端，返回标准的可写路径或资源路径
   *
   * @param relativePath 相对于 Resources 的路径，例如 "develop/map.json"
   * @param forWrite 是否是为了写入（写入时必须确保目录存在）
   * @return 文件的绝对路径
   */
  static std::string getRealFilePath(const std::string& relativePath,
                                     bool forWrite = false);
};

#endif  // __PATH_UTILS_H__
