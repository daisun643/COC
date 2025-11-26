#ifndef _APP_DELEGATE_H_
#define _APP_DELEGATE_H_

#include "cocos2d.h"

/**
@brief    应用程序委托类
*/
class AppDelegate : private cocos2d::Application {
public:
  AppDelegate();
  virtual ~AppDelegate();

  /**
  @brief    初始化应用程序
  @return    true    初始化成功
  @return    false   初始化失败
  */
  virtual bool applicationDidFinishLaunching();

  /**
  @brief    当应用程序进入后台时调用
  */
  virtual void applicationDidEnterBackground();

  /**
  @brief    当应用程序进入前台时调用
  */
  virtual void applicationWillEnterForeground();
};

#endif // _APP_DELEGATE_H_
