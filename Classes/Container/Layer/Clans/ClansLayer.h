#ifndef __CLANS_LAYER_H__
#define __CLANS_LAYER_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

class MyClansLayer;
class JoinClansLayer;

class ClansLayer : public cocos2d::LayerColor {
 public:
  static ClansLayer* create();
  virtual bool init();

 private:
  void buildUI();
  void showMyClansLayer();
  void showJoinClansLayer();
  void updateTabSelection(bool isMyClan);

  cocos2d::ui::Layout* _panel;
  cocos2d::ui::Layout* _myClanLayout;
  cocos2d::ui::Layout* _clanLayout;
  cocos2d::Label* _myClanLabel;
  cocos2d::Label* _clanLabel;
  cocos2d::ui::Layout* _contentArea;
  MyClansLayer* _myClansLayer;
  JoinClansLayer* _joinClansLayer;
  bool _isMyClanSelected;
};

#endif  // __CLANS_LAYER_H__

