#include "AppDelegate.h"
#include "cocos2d.h"

USING_NS_CC;  // 使用cocos2d命名空间

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPWSTR lpCmdLine, int nCmdShow) {
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  // 创建应用程序实例
  AppDelegate app;
  return Application::getInstance()->run();
}
