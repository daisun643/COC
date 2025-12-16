#ifndef __AUTH_SCENE_H__
#define __AUTH_SCENE_H__

#include "cocos2d.h"

class AuthScene : public cocos2d::Scene {
 public:
  static cocos2d::Scene* createScene();
  virtual bool init();

  CREATE_FUNC(AuthScene);

 private:
  // 显示登录界面
  void showLoginLayer();

  // 显示注册界面
  void showRegisterLayer();
};

#endif  // __AUTH_SCENE_H__
