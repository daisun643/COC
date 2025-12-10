#ifndef __ATTACK_LAYER_H__
#define __ATTACK_LAYER_H__

#include <functional>

#include "cocos2d.h"
#include "ui/CocosGUI.h"

class AttackLayer : public cocos2d::LayerColor {
 public:
  static AttackLayer* create();
  virtual bool init();

  // 设置回调
  void setOnSearchOpponentCallback(std::function<void()> callback);
  void setOnLevelSelectedCallback(std::function<void(int levelId)> callback);

 private:
  void buildUI();
  void initTabs();
  void showSinglePlayerTab();
  void showMultiplayerTab();
  void updateTabButtons(bool isSinglePlayer);

  // UI Elements
  cocos2d::ui::Layout* _panel;
  cocos2d::ui::Button* _btnSinglePlayer;
  cocos2d::ui::Button* _btnMultiplayer;
  cocos2d::ui::Layout* _contentArea;
  cocos2d::ui::ScrollView* _scrollView;

  // Callbacks
  std::function<void()> _onSearchOpponent;
  std::function<void(int)> _onLevelSelected;

  // Helper to create a level item
  cocos2d::ui::Widget* createLevelItem(int levelId, const std::string& name,
                                       int stars);
};

#endif  // __ATTACK_LAYER_H__
