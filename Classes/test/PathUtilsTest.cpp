// Lightweight unit test for PathUtils::ensureDirectoryExists
#include <cstdio>
#include <iostream>
#include <string>

#include "Utils/PathUtils.h"

TEST(PathUtilsTest, EnsureDirectoryExists) {
  // 使用相对 Resources 路径，保持与项目运行时路径一致
  std::string testFile = "Resources/test_unit_tmp/subdir/test.txt";

  bool ok = PathUtils::ensureDirectoryExists(testFile);
  if (!ok) {
    FAIL() << "PathUtils::ensureDirectoryExists returned false";
  }

  FILE* f = std::fopen(testFile.c_str(), "w");
  if (!f) {
    FAIL() << "fopen failed to create file: " << testFile;
  }
  std::fputs("unit test", f);
  std::fclose(f);

  // 清理测试产物（忽略错误）
  std::remove(testFile.c_str());
#if defined(_WIN32)
  _rmdir("Resources/test_unit_tmp/subdir");
  _rmdir("Resources/test_unit_tmp");
#else
  rmdir("Resources/test_unit_tmp/subdir");
  rmdir("Resources/test_unit_tmp");
#endif
}

TEST(PathUtilsTest, GetRealFilePath_NormalizesSlashes) {
  std::string result = PathUtils::getRealFilePath("test\\path/file.txt");
  EXPECT_NE(result.find("test/path/file.txt"), std::string::npos);
}

TEST(PathUtilsTest, GetRealFilePath_ForWriteUsesWritablePath) {
  std::string result = PathUtils::getRealFilePath("save/data.json", true);
  EXPECT_NE(result.find("Resources/save/data.json"), std::string::npos);
}
