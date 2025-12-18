#ifndef __JOIN_CLANS_LAYER_H__
#define __JOIN_CLANS_LAYER_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

// 前向声明
struct ClanInfo;

class JoinClansLayer : public cocos2d::Layer {
 public:
  static JoinClansLayer* create();
  virtual bool init();

 private:
  void buildUI();
  void onOverviewClick();
  void onSearchClick();
  void showOverviewList();
  void showSearchUI();
  void onSearchConfirm();
  void displayClansList(const std::vector<struct ClanInfo>& clans);
  cocos2d::ui::Widget* createClanItem(const struct ClanInfo& clan);
  void updateTabSelection(bool isOverview);
  void onJoinClanClick(const struct ClanInfo& clan);

  // UI Elements
  cocos2d::Layer* _contentArea;
  cocos2d::Label* _overviewLabel;
  cocos2d::DrawNode* _overviewBg;
  cocos2d::Label* _searchLabel;
  cocos2d::DrawNode* _searchBg;
  cocos2d::ui::TextField* _searchTextField;
  cocos2d::ui::Button* _searchConfirmButton;
  cocos2d::ui::ScrollView* _scrollView;
  cocos2d::Layer* _listContainer;
  
  // State
  bool _isOverviewSelected;
};

#endif  // __JOIN_CLANS_LAYER_H__

// 需要在"概览"按钮点击后，显示所有部落的简要信息，以列表的形式显示
// 列表中需要显示部落的名称、成员数量
// 需要在"搜索"按钮点击后，生成搜索框，和确认搜索的Button
// 搜索的结果同样以列表的形式展示