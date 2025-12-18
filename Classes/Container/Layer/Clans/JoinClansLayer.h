#ifndef __JOIN_CLANS_LAYER_H__
#define __JOIN_CLANS_LAYER_H__

#include "cocos2d.h"

class JoinClansLayer : public cocos2d::Layer {
 public:
  static JoinClansLayer* create();
  virtual bool init();

 private:
  void buildUI();
};

#endif  // __JOIN_CLANS_LAYER_H__

