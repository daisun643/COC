#ifndef __ATTACK_LAYER_H__
#define __ATTACK_LAYER_H__

#include <chrono>
#include <functional>

#include "cocos2d.h"
#include "ui/CocosGUI.h"

class AttackLayer : public cocos2d::LayerColor {
 public:
  static AttackLayer* create();
  virtual bool init();


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

  bool searchOpponent();
  void updateSearchStatus(float dt);  // 更新搜索状态显示

  // 搜索相关 UI
  cocos2d::Label* _searchStatusLabel;  // 搜索状态标签
  cocos2d::Label* _searchTimeLabel;    // 搜索时间标签
  bool _isSearching;                   // 是否正在搜索
  std::chrono::steady_clock::time_point _searchStartTime;  // 搜索开始时间

  // Helper to create a level item
  cocos2d::ui::Widget* createLevelItem(int levelId, const std::string& name,
                                       const std::string& path, int stars);
};

#endif  // __ATTACK_LAYER_H__
