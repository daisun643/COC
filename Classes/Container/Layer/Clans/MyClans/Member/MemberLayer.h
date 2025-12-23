#ifndef __MEMBER_LAYER_H__
#define __MEMBER_LAYER_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

/**
 * 用于展示部落成员
 * 先判断我是否在一个部落中
 * 如果不在，Layer内容提示你尚未加入一个部落
 * 如果在，根据Profile中的clansId获取部落成员
 * api: /clans/members 参考server\app\api\clans.py
 * 使用ScrollView展示成员列表 参考Classes\Container\Layer\Clans\JoinClans\JoinClansLayer.cpp
 */
class MemberLayer : public cocos2d::Layer {
 public:
  static MemberLayer* create();
  virtual bool init();

 private:
  void buildUI();
  void loadMembers();
  void displayMembersList(const std::vector<std::string>& members);
  cocos2d::ui::Widget* createMemberItem(const std::string& memberName);

  cocos2d::Layer* _contentArea;
  cocos2d::ui::ScrollView* _scrollView;
};

#endif  // __MEMBER_LAYER_H__
