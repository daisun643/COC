#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"

class HelloWorld : public cocos2d::Scene {
public:
  static cocos2d::Scene *createScene();

  virtual bool init();

  // 实现 "static create()" 方法
  CREATE_FUNC(HelloWorld);

private:
  void menuCloseCallback(cocos2d::Ref *pSender);
};

#endif // __HELLOWORLD_SCENE_H__
