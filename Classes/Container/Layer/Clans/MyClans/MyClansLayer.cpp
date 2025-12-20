#include "MyClansLayer.h"

#include "Member/MemberLayer.h"
#include "Chat/ChatLayer.h"
#include "Utils/API/Clans/Clans.h"
#include "Utils/Profile/Profile.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace cocos2d::ui;

namespace {
const Color3B COLOR_BUTTON_BG(128, 128, 128);  // 灰色背景
const std::string FONT_NAME = "Arial";

Label* createLabel(const std::string& text, int fontSize,
                   const Color4B& color = Color4B::WHITE) {
  auto label = Label::createWithSystemFont(text, FONT_NAME, fontSize);
  label->setTextColor(color);
  return label;
}

// 创建圆角背景的 DrawNode
DrawNode* createRoundedBackground(const Size& size, float radius = 8.0f, bool isSelected = false) {
  auto drawNode = DrawNode::create();
  Color4F grayColor;
  if (isSelected) {
    grayColor = Color4F(100.0f / 255.0f, 100.0f / 255.0f, 100.0f / 255.0f, 1.0f);
  } else {
    grayColor = Color4F(128.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f, 1.0f);
  }

  float width = size.width;
  float height = size.height;

  // 绘制中心矩形
  drawNode->drawSolidRect(Vec2(radius, radius),
                          Vec2(width - radius, height - radius), grayColor);

  // 绘制四个圆角
  drawNode->drawSolidCircle(Vec2(radius, radius), radius, 0, 20, grayColor);
  drawNode->drawSolidCircle(Vec2(width - radius, radius), radius, 0, 20,
                            grayColor);
  drawNode->drawSolidCircle(Vec2(radius, height - radius), radius, 0, 20,
                            grayColor);
  drawNode->drawSolidCircle(Vec2(width - radius, height - radius), radius, 0,
                            20, grayColor);

  // 绘制连接圆角的矩形
  drawNode->drawSolidRect(Vec2(radius, 0), Vec2(width - radius, height),
                          grayColor);
  drawNode->drawSolidRect(Vec2(0, radius), Vec2(width, height - radius),
                          grayColor);

  return drawNode;
}
}  // namespace

MyClansLayer* MyClansLayer::create() {
  MyClansLayer* layer = new (std::nothrow) MyClansLayer();
  if (layer && layer->init()) {
    layer->autorelease();
    return layer;
  }
  CC_SAFE_DELETE(layer);
  return nullptr;
}

bool MyClansLayer::init() {
  if (!Layer::init()) {
    return false;
  }

  _contentArea = nullptr;
  _memberLayer = nullptr;
  _chatLayer = nullptr;
  _currentSubLayer = nullptr;
  _memberBg = nullptr;
  _warBg = nullptr;
  _chatBg = nullptr;
  _memberLabel = nullptr;
  _warLabel = nullptr;
  _chatLabel = nullptr;
  _selectedTabIndex = -1;  // 初始没有选中
  _actionButtonLabel = nullptr;
  _actionButtonBg = nullptr;
  _actionButtonTouchLayer = nullptr;
  _isOwner = false;

  buildUI();
  return true;
}

void MyClansLayer::buildUI() {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 创建内容区域
  _contentArea = Layer::create();
  _contentArea->setContentSize(Size(760.0f, 450.0f));
  this->addChild(_contentArea);

  // 四个按钮：成员、部落战、聊天室、离开/解散
  float buttonHeight = 50.0f;
  float buttonWidth = 140.0f;  // 缩小按钮宽度
  float spacing = 15.0f;
  float topY = _contentArea->getContentSize().height - 30.0f;
  float totalWidth = 4 * buttonWidth + 3 * spacing;
  float startX = (_contentArea->getContentSize().width - totalWidth) / 2.0f;
  float radius = 8.0f;

  // "成员" 按钮
  float memberCenterX = startX + buttonWidth / 2.0f;
  float memberCenterY = topY - buttonHeight / 2.0f;
  _memberBg = createRoundedBackground(Size(buttonWidth, buttonHeight), radius, false);
  _memberBg->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
  _memberBg->setPosition(Vec2(memberCenterX - buttonWidth / 2.0f, memberCenterY - buttonHeight / 2.0f));
  _contentArea->addChild(_memberBg, 0);

  _memberLabel = createLabel("成员", 28);
  _memberLabel->setPosition(Vec2(memberCenterX, memberCenterY));
  _contentArea->addChild(_memberLabel, 10);

  // 为"成员"按钮添加触摸事件
  auto memberListener = EventListenerTouchOneByOne::create();
  memberListener->setSwallowTouches(true);
  memberListener->onTouchBegan = [this, memberCenterX, memberCenterY, buttonWidth, buttonHeight](Touch* touch, Event* event) {
    Vec2 location = touch->getLocation();
    Vec2 contentPos = _contentArea->convertToNodeSpace(location);
    Rect memberRect(memberCenterX - buttonWidth / 2.0f, 
                    memberCenterY - buttonHeight / 2.0f, 
                    buttonWidth, buttonHeight);
    if (memberRect.containsPoint(contentPos)) {
      return true;
    }
    return false;
  };
  memberListener->onTouchEnded = [this](Touch* touch, Event* event) {
    this->showMemberLayer();
  };
  _eventDispatcher->addEventListenerWithSceneGraphPriority(memberListener, _contentArea);

  // "部落战" 按钮
  float warCenterX = startX + buttonWidth + spacing + buttonWidth / 2.0f;
  float warCenterY = topY - buttonHeight / 2.0f;
  _warBg = createRoundedBackground(Size(buttonWidth, buttonHeight), radius, false);
  _warBg->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
  _warBg->setPosition(Vec2(warCenterX - buttonWidth / 2.0f, warCenterY - buttonHeight / 2.0f));
  _contentArea->addChild(_warBg, 0);

  _warLabel = createLabel("部落战", 28);
  _warLabel->setPosition(Vec2(warCenterX, warCenterY));
  _contentArea->addChild(_warLabel, 10);

  // "聊天室" 按钮
  float chatCenterX = startX + 2 * (buttonWidth + spacing) + buttonWidth / 2.0f;
  float chatCenterY = topY - buttonHeight / 2.0f;
  _chatBg = createRoundedBackground(Size(buttonWidth, buttonHeight), radius, false);
  _chatBg->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
  _chatBg->setPosition(Vec2(chatCenterX - buttonWidth / 2.0f, chatCenterY - buttonHeight / 2.0f));
  _contentArea->addChild(_chatBg, 0);

  _chatLabel = createLabel("聊天室", 28);
  _chatLabel->setPosition(Vec2(chatCenterX, chatCenterY));
  _contentArea->addChild(_chatLabel, 10);

  // 为"聊天室"按钮添加触摸事件
  auto chatListener = EventListenerTouchOneByOne::create();
  chatListener->setSwallowTouches(true);
  chatListener->onTouchBegan = [this, chatCenterX, chatCenterY, buttonWidth, buttonHeight](Touch* touch, Event* event) {
    Vec2 location = touch->getLocation();
    Vec2 contentPos = _contentArea->convertToNodeSpace(location);
    Rect chatRect(chatCenterX - buttonWidth / 2.0f, 
                  chatCenterY - buttonHeight / 2.0f, 
                  buttonWidth, buttonHeight);
    if (chatRect.containsPoint(contentPos)) {
      return true;
    }
    return false;
  };
  chatListener->onTouchEnded = [this](Touch* touch, Event* event) {
    this->showChatLayer();
  };
  _eventDispatcher->addEventListenerWithSceneGraphPriority(chatListener, _contentArea);
  showMemberLayer();
  
  // 检查是否是所有者并设置操作按钮
  checkOwnerAndSetupActionButton();
}

void MyClansLayer::showMemberLayer() {
  if (_selectedTabIndex == 0) {
    return;  // 已经选中，不需要切换
  }

  _selectedTabIndex = 0;
  updateTabSelection(0);

  // 隐藏当前子Layer
  hideCurrentSubLayer();

  // 创建或显示 MemberLayer
  if (!_memberLayer) {
    _memberLayer = MemberLayer::create();
    if (_memberLayer) {
      _contentArea->addChild(_memberLayer);
      _currentSubLayer = _memberLayer;
    }
  } else {
    _currentSubLayer = _memberLayer;
    _currentSubLayer->setVisible(true);
  }
}

void MyClansLayer::showChatLayer() {
  if (_selectedTabIndex == 2) {
    return;  // 已经选中，不需要切换
  }

  _selectedTabIndex = 2;
  updateTabSelection(2);

  // 隐藏当前子Layer
  hideCurrentSubLayer();

  // 创建或显示 ChatLayer
  if (!_chatLayer) {
    _chatLayer = ChatLayer::create();
    if (_chatLayer) {
      _contentArea->addChild(_chatLayer);
      _currentSubLayer = _chatLayer;
    }
  } else {
    _currentSubLayer = _chatLayer;
    _currentSubLayer->setVisible(true);
  }
}

void MyClansLayer::hideCurrentSubLayer() {
  if (_currentSubLayer) {
    _currentSubLayer->setVisible(false);
    _currentSubLayer=nullptr;
  }
}

void MyClansLayer::updateTabSelection(int selectedIndex) {
  float buttonWidth = 140.0f;  // 缩小按钮宽度，与其他按钮保持一致
  float buttonHeight = 50.0f;
  float radius = 8.0f;
  
  // 更新"成员"按钮
  if (_memberBg) {
    Vec2 oldPos = _memberBg->getPosition();
    int oldZOrder = _memberBg->getLocalZOrder();
    _memberBg->removeFromParent();
    _memberBg = createRoundedBackground(Size(buttonWidth, buttonHeight), radius, selectedIndex == 0);
    _memberBg->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    _memberBg->setPosition(oldPos);
    _contentArea->addChild(_memberBg, oldZOrder);
  }
  
  // 更新"部落战"按钮
  if (_warBg) {
    Vec2 oldPos = _warBg->getPosition();
    int oldZOrder = _warBg->getLocalZOrder();
    _warBg->removeFromParent();
    _warBg = createRoundedBackground(Size(buttonWidth, buttonHeight), radius, selectedIndex == 1);
    _warBg->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    _warBg->setPosition(oldPos);
    _contentArea->addChild(_warBg, oldZOrder);
  }
  
  // 更新"聊天室"按钮
  if (_chatBg) {
    Vec2 oldPos = _chatBg->getPosition();
    int oldZOrder = _chatBg->getLocalZOrder();
    _chatBg->removeFromParent();
    _chatBg = createRoundedBackground(Size(buttonWidth, buttonHeight), radius, selectedIndex == 2);
    _chatBg->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    _chatBg->setPosition(oldPos);
    _contentArea->addChild(_chatBg, oldZOrder);
  }
  
  // 确保 Label 在背景之上（提高 z-order）
  if (_memberLabel) {
    _memberLabel->setLocalZOrder(10);
  }
  if (_warLabel) {
    _warLabel->setLocalZOrder(10);
  }
  if (_chatLabel) {
    _chatLabel->setLocalZOrder(10);
  }
}

void MyClansLayer::checkOwnerAndSetupActionButton() {
  auto profile = Profile::getInstance();
  if (!profile || profile->getClansId() <= 0) {
    return;  // 用户未加入部落
  }

  int clanId = profile->getClansId();
  std::string clanIdStr = std::to_string(clanId);
  int userId = profile->getId();

  // 调用 API 获取部落所有者
  Clans::getClanOwner(clanIdStr, [this, userId](
                                      bool success, const std::string& message,
                                      int owner_id) {
    // 这里要进行的id的比对，而不是名字的比对
    if (success) {
      _isOwner = (owner_id == userId);
      setupActionButton();
    } else {
      CCLOG("Failed to get clan owner: %s", message.c_str());
    }
  });
}

void MyClansLayer::setupActionButton() {
  // 移除旧的按钮
  if (_actionButtonBg) {
    _actionButtonBg->removeFromParent();
    _actionButtonBg = nullptr;
  }
  if (_actionButtonLabel) {
    _actionButtonLabel->removeFromParent();
    _actionButtonLabel = nullptr;
  }
  if (_actionButtonTouchLayer) {
    _actionButtonTouchLayer->removeFromParent();
    _actionButtonTouchLayer = nullptr;
  }

  // 使用与其他按钮相同的布局参数
  float buttonHeight = 50.0f;
  float buttonWidth = 140.0f;  // 缩小按钮宽度
  float spacing = 15.0f;
  float topY = _contentArea->getContentSize().height - 30.0f;
  float totalWidth = 4 * buttonWidth + 3 * spacing;
  float startX = (_contentArea->getContentSize().width - totalWidth) / 2.0f;
  float radius = 8.0f;

  // 计算第四个按钮的位置（离开/解散）
  float actionCenterX = startX + 3 * (buttonWidth + spacing) + buttonWidth / 2.0f;
  float actionCenterY = topY - buttonHeight / 2.0f;

  _actionButtonBg = createRoundedBackground(Size(buttonWidth, buttonHeight), radius, false);
  _actionButtonBg->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
  _actionButtonBg->setPosition(Vec2(actionCenterX - buttonWidth / 2.0f, actionCenterY - buttonHeight / 2.0f));
  _contentArea->addChild(_actionButtonBg, 0);

  // 根据是否是所有者设置按钮文本
  std::string buttonText = _isOwner ? "解散" : "离开";
  _actionButtonLabel = createLabel(buttonText, 28);  // 使用与其他按钮相同的字体大小
  _actionButtonLabel->setPosition(Vec2(actionCenterX, actionCenterY));
  _contentArea->addChild(_actionButtonLabel, 10);

  // 创建一个透明的可触摸 Layer 覆盖在按钮区域上
  _actionButtonTouchLayer = Layer::create();
  _actionButtonTouchLayer->setContentSize(Size(buttonWidth, buttonHeight));
  _actionButtonTouchLayer->setPosition(Vec2(actionCenterX - buttonWidth / 2.0f, actionCenterY - buttonHeight / 2.0f));
  _actionButtonTouchLayer->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
  _contentArea->addChild(_actionButtonTouchLayer, 11);  // 稍微提高优先级

  // 添加触摸事件到 touchLayer
  auto listener = EventListenerTouchOneByOne::create();
  listener->setSwallowTouches(true);  // 吞噬触摸事件，防止传递给下层
  listener->onTouchBegan = [this, actionCenterX, actionCenterY, buttonWidth, buttonHeight](Touch* touch, Event* event) {
    Vec2 location = touch->getLocation();
    Vec2 contentPos = _contentArea->convertToNodeSpace(location);
    Rect buttonRect(actionCenterX - buttonWidth / 2.0f, 
                    actionCenterY - buttonHeight / 2.0f, 
                    buttonWidth, buttonHeight);
    if (buttonRect.containsPoint(contentPos)) {
      return true;  // 返回 true 表示处理这个触摸事件
    }
    return false;
  };
  listener->onTouchEnded = [this](Touch* touch, Event* event) {
    if (_isOwner) {
      showConfirmDialog("解散部落", "确定要解散部落吗？此操作不可撤销！", [this]() {
        performLeaveOrDisband(true);
      });
    } else {
      showConfirmDialog("离开部落", "确定要离开部落吗？", [this]() {
        performLeaveOrDisband(false);
      });
    }
  };
  // 添加到 touchLayer，这样触摸事件会被优先处理
  _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, _actionButtonTouchLayer);
}

void MyClansLayer::showConfirmDialog(const std::string& title, const std::string& message,
                                     std::function<void()> onConfirm) {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 创建半透明背景层
  auto bgLayer = LayerColor::create(Color4B(0, 0, 0, 0), visibleSize.width, visibleSize.height);
  bgLayer->setPosition(origin);
  bgLayer->setName("ConfirmDialogBackground");
  this->addChild(bgLayer, 1000);

  // 创建对话框背景
  float dialogWidth = 400.0f;
  float dialogHeight = 200.0f;
  auto dialogBg = LayerColor::create(Color4B(50, 50, 50, 255), dialogWidth, dialogHeight);
  dialogBg->setAnchorPoint(Vec2::ANCHOR_MIDDLE);  // 设置锚点为中心
  dialogBg->setPosition(Vec2(origin.x + dialogWidth / 2, origin.y + 50) );  // 屏幕中心
  dialogBg->setName("ConfirmDialog");
  this->addChild(dialogBg, 1001);

  // 创建标题标签
  auto titleLabel = createLabel(title, 24);
  titleLabel->setColor(Color3B::WHITE);
  titleLabel->setPosition(Vec2(dialogWidth / 2.0f, dialogHeight - 50.0f));
  dialogBg->addChild(titleLabel);

  // 创建消息标签
  auto messageLabel = createLabel(message, 18);
  messageLabel->setColor(Color3B::WHITE);
  messageLabel->setDimensions(dialogWidth - 40, 0);
  messageLabel->setAlignment(TextHAlignment::CENTER);
  messageLabel->setPosition(Vec2(dialogWidth / 2.0f, dialogHeight / 2.0f));
  dialogBg->addChild(messageLabel);

  // 创建确定按钮
  float btnWidth = 80.0f;
  float btnHeight = 35.0f;
  float confirmBtnX = dialogWidth / 2.0f - btnWidth - 10.0f;
  float confirmBtnY = 30.0f;
  auto confirmBtnBg = LayerColor::create(Color4B(100, 150, 100, 255), btnWidth, btnHeight);
  confirmBtnBg->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);  // 设置锚点为左下角
  confirmBtnBg->setPosition(Vec2(confirmBtnX, confirmBtnY));
  auto confirmLabel = createLabel("确定", 18);
  confirmLabel->setPosition(Vec2(btnWidth / 2.0f, btnHeight / 2.0f));
  confirmBtnBg->addChild(confirmLabel);
  dialogBg->addChild(confirmBtnBg);

  // 创建取消按钮
  float cancelBtnX = dialogWidth / 2.0f + 10.0f;
  float cancelBtnY = 30.0f;
  auto cancelBtnBg = LayerColor::create(Color4B(150, 100, 100, 255), btnWidth, btnHeight);
  cancelBtnBg->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);  // 设置锚点为左下角
  cancelBtnBg->setPosition(Vec2(cancelBtnX, cancelBtnY));
  auto cancelLabel = createLabel("取消", 18);
  cancelLabel->setPosition(Vec2(btnWidth / 2.0f, btnHeight / 2.0f));
  cancelBtnBg->addChild(cancelLabel);
  dialogBg->addChild(cancelBtnBg);

  // 添加按钮点击事件
  auto listener = EventListenerTouchOneByOne::create();
  listener->setSwallowTouches(true);
  listener->onTouchBegan = [this, bgLayer, dialogBg, confirmBtnBg, cancelBtnBg, 
                             confirmBtnX, confirmBtnY, cancelBtnX, cancelBtnY,
                             btnWidth, btnHeight, onConfirm](
                               Touch* touch, Event* event) -> bool {
    Vec2 location = touch->getLocation();
    // 将触摸位置转换为相对于 dialogBg 的坐标
    Vec2 dialogLocalPos = dialogBg->convertToNodeSpace(location);

    // 检查是否点击了确定按钮
    Rect confirmRect(confirmBtnX, confirmBtnY, btnWidth, btnHeight);
    if (confirmRect.containsPoint(dialogLocalPos)) {
      this->removeChild(bgLayer);
      this->removeChild(dialogBg);
      if (onConfirm) {
        onConfirm();
      }
      return true;
    }

    // 检查是否点击了取消按钮
    Rect cancelRect(cancelBtnX, cancelBtnY, btnWidth, btnHeight);
    if (cancelRect.containsPoint(dialogLocalPos)) {
      this->removeChild(bgLayer);
      this->removeChild(dialogBg);
      return true;
    }

    // 点击背景也关闭对话框
    Vec2 bgLocalPos = bgLayer->convertToNodeSpace(location);
    if (bgLocalPos.x >= 0 && bgLocalPos.x <= bgLayer->getContentSize().width &&
        bgLocalPos.y >= 0 && bgLocalPos.y <= bgLayer->getContentSize().height) {
      this->removeChild(bgLayer);
      this->removeChild(dialogBg);
      return true;
    }
    return false;
  };

  _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, dialogBg);
}

void MyClansLayer::performLeaveOrDisband(bool isDisband) {
  auto profile = Profile::getInstance();
  if (!profile || profile->getClansId() <= 0) {
    return;
  }

  int clanId = profile->getClansId();
  std::string clanIdStr = std::to_string(clanId);
  int userId = profile->getId();

  if (isDisband) {
    // 解散部落
    Clans::disbandClan(clanIdStr, userId, [this](bool success, const std::string& message) {
      // 进行提示
      if (success) {
        CCLOG("Clan disbanded successfully");
        // 更新用户信息
        auto profile = Profile::getInstance();
        if (profile) {
          profile->setClansId(-1);
          profile->setClansName("");
          profile->save();
        }
        
        // 显示成功提示
        this->setVisible(false);
        auto successLabel = createLabel("解散成功", 24, Color4B::GREEN);
        successLabel->setPosition(
            Vec2(_contentArea->getContentSize().width / 2.0f,
                 _contentArea->getContentSize().height / 2.0f));
        this->getParent()->addChild(successLabel, 1000);
        // 3秒后移除提示并关闭当前层
        auto delay = DelayTime::create(3.0f);
        auto remove = CallFunc::create([this, successLabel]() {
          successLabel->removeFromParent();
        });
        successLabel->runAction(Sequence::create(delay, remove, nullptr));
      } else {
        CCLOG("Failed to disband clan: %s", message.c_str());
        // 显示错误提示
        auto errorLabel = createLabel("解散失败: " + message, 24, Color4B::RED);
        errorLabel->setPosition(
            Vec2(_contentArea->getContentSize().width / 2.0f,
                 _contentArea->getContentSize().height / 2.0f));
        this->getParent()->addChild(errorLabel, 1000);
        // 3秒后移除提示
        auto delay = DelayTime::create(3.0f);
        auto remove = CallFunc::create([errorLabel]() {
          errorLabel->removeFromParent();
        });
        errorLabel->runAction(Sequence::create(delay, remove, nullptr));
      }
    });
  } else {
    // 离开部落
    Clans::leaveClan(clanIdStr, userId, [this](bool success, const std::string& message) {
      if (success) {
        CCLOG("Left clan successfully");
        // 更新用户信息
        auto profile = Profile::getInstance();
        if (profile) {
          profile->setClansId(-1);
          profile->setClansName("");
          profile->save();
        }
        // 显示成功提示
        this->setVisible(false);
        auto successLabel = createLabel("离开成功", 24, Color4B::GREEN);
        successLabel->setPosition(
            Vec2(_contentArea->getContentSize().width / 2.0f,
                 _contentArea->getContentSize().height / 2.0f));
        this->getParent()->addChild(successLabel, 1000);
        // 3秒后移除提示并关闭当前层
        auto delay = DelayTime::create(3.0f);
        auto remove = CallFunc::create([this, successLabel]() {
          successLabel->removeFromParent();
        });
        successLabel->runAction(Sequence::create(delay, remove, nullptr));
      } else {
        CCLOG("Failed to leave clan: %s", message.c_str());
        // 显示错误提示
        auto errorLabel = createLabel("离开失败: " + message, 24, Color4B::RED);
        errorLabel->setPosition(
            Vec2(_contentArea->getContentSize().width / 2.0f,
                 _contentArea->getContentSize().height / 2.0f));
        this->getParent()->addChild(errorLabel, 1000);
        // 3秒后移除提示
        auto delay = DelayTime::create(3.0f);
        auto remove = CallFunc::create([errorLabel]() {
          errorLabel->removeFromParent();
        });
        errorLabel->runAction(Sequence::create(delay, remove, nullptr));
      }
    });
  }
}
