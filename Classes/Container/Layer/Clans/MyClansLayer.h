#ifndef __MY_CLANS_LAYER_H__
#define __MY_CLANS_LAYER_H__

#include "cocos2d.h"

class MyClansLayer : public cocos2d::Layer {
 public:
  static MyClansLayer* create();
  virtual bool init();

 private:
  void buildUI();
};

#endif  // __MY_CLANS_LAYER_H__

