#ifndef __MY_CLANS_LAYER_H__
#define __MY_CLANS_LAYER_H__

#include "cocos2d.h"

// 前向声明
class MemberLayer;
class ChatLayer;

class MyClansLayer : public cocos2d::Layer {
 public:
  static MyClansLayer* create();
  virtual bool init();

 private:
  void buildUI();
  void showMemberLayer();
  void showChatLayer();
  void hideCurrentSubLayer();

  cocos2d::Layer* _contentArea;
  MemberLayer* _memberLayer;
  ChatLayer* _chatLayer;
  cocos2d::Layer* _currentSubLayer;  // 当前显示的子Layer
};

#endif  // __MY_CLANS_LAYER_H__

