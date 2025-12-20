#ifndef __CHAT_LAYER_H__
#define __CHAT_LAYER_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Utils/API/Clans/Clans.h"

/**
 * 部落聊天室层
 * 先判断我是否在一个部落中
 * 如果不在，Layer内容提示你尚未加入一个部落
 * 如果在，根据Profile中的clansId获取部落聊天室消息
 * api: /clans/chat/messages 参考server\app\api\clans.py
 * 使用ScrollView展示消息列表 参考Classes\Container\Layer\Clans\JoinClans\JoinClansLayer.cpp
 * 使用TextField输入消息，点击发送按钮发送消息，api:/clans/chat/send
 * 发送消息后，刷新消息列表
 * 消息列表使用时间降序排序
 * 消息列表使用消息发送者的用户名作为消息发送者
 */

//  TODO 需要定时更新消息列表
class ChatLayer : public cocos2d::Layer {
 public:
  static ChatLayer* create();
  virtual bool init();

 private:
  void buildUI();
  void loadMessages();
  void displayMessagesList(const std::vector<struct Clans::ChatMessage>& messages);
  cocos2d::ui::Widget* createMessageItem(const struct Clans::ChatMessage& msg);
  void onSendButtonClick();
  void refreshMessages();

  cocos2d::Layer* _contentArea;
  cocos2d::ui::ScrollView* _scrollView;
  cocos2d::ui::TextField* _inputTextField;
  cocos2d::ui::Button* _sendButton;
};

#endif  // __CHAT_LAYER_H__
