#include "AppDelegate.h"
#include "cocos2d.h"

// 仅在 Windows 平台包含并编译 Win32 主函数
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
#include <fcntl.h>
#include <io.h>
#include <windows.h>

USING_NS_CC;  // 使用cocos2d命名空间

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPWSTR lpCmdLine, int nCmdShow) {
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  // 创建应用程序实例
  AppDelegate app;
  return Application::getInstance()->run();
}
#endif
