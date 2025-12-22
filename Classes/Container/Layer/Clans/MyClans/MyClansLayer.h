#ifndef __MY_CLANS_LAYER_H__
#define __MY_CLANS_LAYER_H__

#include <functional>
#include <string>

#include "cocos2d.h"

// 前向声明
class MemberLayer;
class ChatLayer;
class ClansWarLayer;

class MyClansLayer : public cocos2d::Layer {
 public:
  static MyClansLayer* create();
  virtual bool init();

 private:
  void buildUI();
  void showMemberLayer();
  void showWarLayer();
  void showChatLayer();
  void hideCurrentSubLayer();
  void updateTabSelection(int selectedIndex);  // 0=成员, 1=部落战, 2=聊天室
  void checkOwnerAndSetupActionButton();  // 检查是否是所有者并设置操作按钮
  void setupActionButton();  // 设置操作按钮（解散/离开）
  void showConfirmDialog(const std::string& title, const std::string& message, 
                         std::function<void()> onConfirm);  // 显示确认对话框
  void performLeaveOrDisband(bool isDisband);  // 执行离开或解散操作

  cocos2d::Layer* _contentArea;
  MemberLayer* _memberLayer;
  ChatLayer* _chatLayer;
  ClansWarLayer* _clansWarLayer;
  cocos2d::Layer* _currentSubLayer;  // 当前显示的子Layer
  
  // 按钮背景和标签
  cocos2d::DrawNode* _memberBg;
  cocos2d::DrawNode* _warBg;
  cocos2d::DrawNode* _chatBg;
  cocos2d::Label* _memberLabel;
  cocos2d::Label* _warLabel;
  cocos2d::Label* _chatLabel;
  
  // 解散/离开按钮
  cocos2d::Label* _actionButtonLabel;
  cocos2d::DrawNode* _actionButtonBg;
  cocos2d::Layer* _actionButtonTouchLayer;  // 按钮触摸层
  bool _isOwner;  // 是否是部落所有者
  
  int _selectedTabIndex;  // 当前选中的标签索引：0=成员, 1=部落战, 2=聊天室
};

#endif  // __MY_CLANS_LAYER_H__

