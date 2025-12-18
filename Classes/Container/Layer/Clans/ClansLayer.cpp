#include "ClansLayer.h"
#include "MyClansLayer.h"
#include "JoinClansLayer.h"

USING_NS_CC;
using namespace cocos2d::ui;

namespace {
const Color3B COLOR_PANEL_BG(40, 42, 54);
const Color3B COLOR_TAB_BG(128, 128, 128);      // 灰色背景
const Color3B COLOR_TAB_BG_SELECTED(100, 100, 100);  // 选中时的深灰色背景
const std::string FONT_NAME = "Arial";

Label* createLabel(const std::string& text, int fontSize,
                   const Color4B& color = Color4B::WHITE) {
  auto label = Label::createWithSystemFont(text, FONT_NAME, fontSize);
  label->setTextColor(color);
  return label;
}
}  // namespace

ClansLayer* ClansLayer::create() {
  ClansLayer* layer = new (std::nothrow) ClansLayer();
  if (layer && layer->init()) {
    layer->autorelease();
    return layer;
  }
  CC_SAFE_DELETE(layer);
  return nullptr;
}

bool ClansLayer::init() {
  if (!LayerColor::initWithColor(Color4B(0, 0, 0, 180))) {
    return false;
  }

  buildUI();

  // Swallow touches
  auto listener = EventListenerTouchOneByOne::create();
  listener->setSwallowTouches(true);
  listener->onTouchBegan = [this](Touch* touch, Event* event) {
    // Close if clicked outside panel
    if (!_panel) return true;
    Vec2 location = touch->getLocation();
    Vec2 panelPos = _panel->convertToNodeSpace(location);
    Rect panelRect(0, 0, _panel->getContentSize().width,
                   _panel->getContentSize().height);
    if (!panelRect.containsPoint(panelPos)) {
      this->removeFromParent();
    }
    return true;
  };
  _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

  return true;
}

void ClansLayer::buildUI() {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // Main Panel
  _panel = Layout::create();
  _panel->setContentSize(Size(800.0f, 600.0f));
  _panel->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
  _panel->setBackGroundColor(COLOR_PANEL_BG);
  _panel->setBackGroundColorOpacity(255);
  _panel->setPosition(Vec2(
      origin.x + (visibleSize.width - _panel->getContentSize().width) / 2.0f,
      origin.y +
          (visibleSize.height - _panel->getContentSize().height) / 2.0f));
  this->addChild(_panel);

  // Close Button
  auto closeBtn = Button::create("images/ui/Bar.png");
  closeBtn->setScale(0.5f);
  closeBtn->setTitleText("关闭");
  closeBtn->setTitleFontSize(20);
  closeBtn->setPosition(Vec2(_panel->getContentSize().width - 60.0f,
                             _panel->getContentSize().height - 35.0f));
  closeBtn->addClickEventListener([this](Ref*) { this->removeFromParent(); });
  _panel->addChild(closeBtn);

  // 创建标签容器区域（顶部，在关闭按钮下方）
  float tabHeight = 60.0f;
  float tabWidth = _panel->getContentSize().width / 2.0f;
  float topY = _panel->getContentSize().height - 80.0f;  // 降低位置，避免与关闭按钮重叠

  // "我的部落" Label with background
  _myClanLayout = Layout::create();
  _myClanLayout->setContentSize(Size(tabWidth, tabHeight));
  _myClanLayout->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
  _myClanLayout->setBackGroundColor(COLOR_TAB_BG_SELECTED);
  _myClanLayout->setBackGroundColorOpacity(255);
  _myClanLayout->setPosition(Vec2(0, topY - tabHeight / 2.0f));
  
  // 添加触摸监听器使标签可点击
  auto myClanListener = EventListenerTouchOneByOne::create();
  myClanListener->setSwallowTouches(true);
  myClanListener->onTouchBegan = [this](Touch* touch, Event* event) {
    Vec2 location = touch->getLocation();
    Vec2 layoutPos = _myClanLayout->convertToNodeSpace(location);
    Rect layoutRect(0, 0, _myClanLayout->getContentSize().width,
                    _myClanLayout->getContentSize().height);
    if (layoutRect.containsPoint(layoutPos)) {
      showMyClansLayer();
      return true;
    }
    return false;
  };
  _eventDispatcher->addEventListenerWithSceneGraphPriority(myClanListener,
                                                           _myClanLayout);
  _panel->addChild(_myClanLayout);

  _myClanLabel = createLabel("我的部落", 28);
  _myClanLabel->setPosition(Vec2(tabWidth / 2.0f, tabHeight / 2.0f));
  _myClanLayout->addChild(_myClanLabel);

  // "部落" Label with background
  _clanLayout = Layout::create();
  _clanLayout->setContentSize(Size(tabWidth, tabHeight));
  _clanLayout->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
  _clanLayout->setBackGroundColor(COLOR_TAB_BG);
  _clanLayout->setBackGroundColorOpacity(255);
  _clanLayout->setPosition(Vec2(tabWidth, topY - tabHeight / 2.0f));
  
  // 添加触摸监听器使标签可点击
  auto clanListener = EventListenerTouchOneByOne::create();
  clanListener->setSwallowTouches(true);
  clanListener->onTouchBegan = [this](Touch* touch, Event* event) {
    Vec2 location = touch->getLocation();
    Vec2 layoutPos = _clanLayout->convertToNodeSpace(location);
    Rect layoutRect(0, 0, _clanLayout->getContentSize().width,
                    _clanLayout->getContentSize().height);
    if (layoutRect.containsPoint(layoutPos)) {
      showJoinClansLayer();
      return true;
    }
    return false;
  };
  _eventDispatcher->addEventListenerWithSceneGraphPriority(clanListener,
                                                          _clanLayout);
  _panel->addChild(_clanLayout);

  _clanLabel = createLabel("部落", 28);
  _clanLabel->setPosition(Vec2(tabWidth / 2.0f, tabHeight / 2.0f));
  _clanLayout->addChild(_clanLabel);

  // 创建内容区域
  _contentArea = Layout::create();
  _contentArea->setContentSize(Size(760.0f, 450.0f));
  _contentArea->setPosition(Vec2(20.0f, 20.0f));
  _panel->addChild(_contentArea);

  // 初始化状态
  _isMyClanSelected = false;
  _myClansLayer = nullptr;
  _joinClansLayer = nullptr;

  // 默认显示"我的部落"
  showMyClansLayer();
}

void ClansLayer::showMyClansLayer() {
  if (_isMyClanSelected) {
    return;  // 已经选中，不需要切换
  }

  _isMyClanSelected = true;
  updateTabSelection(true);

  // 移除 JoinClansLayer
  if (_joinClansLayer) {
    _joinClansLayer->removeFromParent();
    _joinClansLayer = nullptr;
  }

  // 创建并显示 MyClansLayer
  if (!_myClansLayer) {
    _myClansLayer = MyClansLayer::create();
    if (_myClansLayer) {
      _contentArea->addChild(_myClansLayer);
    }
  } else {
    _contentArea->addChild(_myClansLayer);
  }
}

void ClansLayer::showJoinClansLayer() {
  if (!_isMyClanSelected) {
    return;  // 已经选中，不需要切换
  }

  _isMyClanSelected = false;
  updateTabSelection(false);

  // 移除 MyClansLayer
  if (_myClansLayer) {
    _myClansLayer->removeFromParent();
    _myClansLayer = nullptr;
  }

  // 创建并显示 JoinClansLayer
  if (!_joinClansLayer) {
    _joinClansLayer = JoinClansLayer::create();
    if (_joinClansLayer) {
      _contentArea->addChild(_joinClansLayer);
    }
  } else {
    _contentArea->addChild(_joinClansLayer);
  }
}

void ClansLayer::updateTabSelection(bool isMyClan) {
  if (isMyClan) {
    // "我的部落" 选中
    _myClanLayout->setBackGroundColor(COLOR_TAB_BG_SELECTED);
    _clanLayout->setBackGroundColor(COLOR_TAB_BG);
  } else {
    // "部落" 选中
    _myClanLayout->setBackGroundColor(COLOR_TAB_BG);
    _clanLayout->setBackGroundColor(COLOR_TAB_BG_SELECTED);
  }
}

