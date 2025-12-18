#include "AppDelegate.h"
#include "cocos2d.h"

#ifdef WIN32
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#endif

USING_NS_CC;  // 使用cocos2d命名空间

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPWSTR lpCmdLine, int nCmdShow) {
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

#if defined(WIN32) || defined(_WIN32)
  // 开启控制台窗口以便查看日志
  // AllocConsole();
  // freopen("CONIN$", "r", stdin);
  // freopen("CONOUT$", "w", stdout);
  // freopen("CONOUT$", "w", stderr);
#endif

  // 创建应用程序实例
  AppDelegate app;
  return Application::getInstance()->run();
}
