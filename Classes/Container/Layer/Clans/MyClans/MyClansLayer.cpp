#include "MyClansLayer.h"

#include "Member/MemberLayer.h"
#include "Chat/ChatLayer.h"
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
DrawNode* createRoundedBackground(const Size& size, float radius = 8.0f) {
  auto drawNode = DrawNode::create();
  Color4F grayColor(128.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f, 1.0f);

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

  // 三个按钮：成员、部落战、聊天室
  float buttonHeight = 50.0f;
  float buttonWidth = 200.0f;
  float spacing = 20.0f;
  float topY = _contentArea->getContentSize().height - 30.0f;
  float totalWidth = 3 * buttonWidth + 2 * spacing;
  float startX = (_contentArea->getContentSize().width - totalWidth) / 2.0f;
  float radius = 8.0f;

  // "成员" 按钮
  float memberCenterX = startX + buttonWidth / 2.0f;
  float memberCenterY = topY - buttonHeight / 2.0f;
  auto memberBg = createRoundedBackground(Size(buttonWidth, buttonHeight), radius);
  memberBg->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
  memberBg->setPosition(Vec2(memberCenterX - buttonWidth / 2.0f, memberCenterY - buttonHeight / 2.0f));
  _contentArea->addChild(memberBg);

  auto memberLabel = createLabel("成员", 28);
  memberLabel->setPosition(Vec2(memberCenterX, memberCenterY));
  _contentArea->addChild(memberLabel);

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
  auto warBg = createRoundedBackground(Size(buttonWidth, buttonHeight), radius);
  warBg->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
  warBg->setPosition(Vec2(warCenterX - buttonWidth / 2.0f, warCenterY - buttonHeight / 2.0f));
  _contentArea->addChild(warBg);

  auto warLabel = createLabel("部落战", 28);
  warLabel->setPosition(Vec2(warCenterX, warCenterY));
  _contentArea->addChild(warLabel);

  // "聊天室" 按钮
  float chatCenterX = startX + 2 * (buttonWidth + spacing) + buttonWidth / 2.0f;
  float chatCenterY = topY - buttonHeight / 2.0f;
  auto chatBg = createRoundedBackground(Size(buttonWidth, buttonHeight), radius);
  chatBg->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
  chatBg->setPosition(Vec2(chatCenterX - buttonWidth / 2.0f, chatCenterY - buttonHeight / 2.0f));
  _contentArea->addChild(chatBg);

  auto chatLabel = createLabel("聊天室", 28);
  chatLabel->setPosition(Vec2(chatCenterX, chatCenterY));
  _contentArea->addChild(chatLabel);

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
}

void MyClansLayer::showMemberLayer() {
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
    _contentArea->addChild(_memberLayer);
    _currentSubLayer = _memberLayer;
  }
}

void MyClansLayer::showChatLayer() {
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
    _contentArea->addChild(_chatLayer);
    _currentSubLayer = _chatLayer;
  }
}

void MyClansLayer::hideCurrentSubLayer() {
  if (_currentSubLayer) {
    _currentSubLayer->removeFromParent();
    _currentSubLayer = nullptr;
  }
}

