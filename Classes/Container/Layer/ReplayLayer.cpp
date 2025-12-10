#include "ReplayLayer.h"

USING_NS_CC;
using namespace cocos2d::ui;

namespace {
const Color3B COLOR_PANEL_BG(40, 42, 54);
const Color3B COLOR_ITEM_BG(58, 60, 72);
const std::string FONT_NAME = "Arial";

Label* createLabel(const std::string& text, int fontSize,
                   const Color4B& color = Color4B::WHITE) {
  auto label = Label::createWithSystemFont(text, FONT_NAME, fontSize);
  label->setTextColor(color);
  return label;
}
}  // namespace

ReplayLayer* ReplayLayer::create() {
  ReplayLayer* layer = new (std::nothrow) ReplayLayer();
  if (layer && layer->init()) {
    layer->autorelease();
    return layer;
  }
  CC_SAFE_DELETE(layer);
  return nullptr;
}

bool ReplayLayer::init() {
  if (!LayerColor::initWithColor(Color4B(0, 0, 0, 180))) {
    return false;
  }

  buildUI();

  auto listener = EventListenerTouchOneByOne::create();
  listener->setSwallowTouches(true);
  listener->onTouchBegan = [this](Touch* touch, Event* event) {
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

void ReplayLayer::setOnReplaySelectedCallback(
    std::function<void(int)> callback) {
  _onReplaySelected = callback;
}

void ReplayLayer::buildUI() {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

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

  auto title = createLabel("战斗回放", 32);
  title->setPosition(Vec2(_panel->getContentSize().width / 2.0f,
                          _panel->getContentSize().height - 35.0f));
  title->enableBold();
  _panel->addChild(title);

  auto closeBtn = Button::create("images/ui/Bar.png");
  closeBtn->setScale(0.5f);
  closeBtn->setTitleText("关闭");
  closeBtn->setTitleFontSize(20);
  closeBtn->setPosition(Vec2(_panel->getContentSize().width - 60.0f,
                             _panel->getContentSize().height - 35.0f));
  closeBtn->addClickEventListener([this](Ref*) { this->removeFromParent(); });
  _panel->addChild(closeBtn);

  _scrollView = ScrollView::create();
  _scrollView->setContentSize(Size(760.0f, 500.0f));
  _scrollView->setDirection(ScrollView::Direction::VERTICAL);
  _scrollView->setBounceEnabled(true);
  _scrollView->setPosition(Vec2(20.0f, 20.0f));
  _panel->addChild(_scrollView);

  // Mock Data
  int itemCount = 10;
  float itemHeight = 100.0f;
  float spacing = 10.0f;
  float innerHeight = (itemHeight + spacing) * itemCount;
  if (innerHeight < _scrollView->getContentSize().height) {
    innerHeight = _scrollView->getContentSize().height;
  }
  _scrollView->setInnerContainerSize(
      Size(_scrollView->getContentSize().width, innerHeight));

  for (int i = 0; i < itemCount; ++i) {
    bool isWin = (i % 2 == 0);
    auto item = createReplayItem(i, StringUtils::format("玩家 %d", i + 100),
                                 isWin, "2025-12-03 12:00");
    item->setPosition(Vec2(_scrollView->getContentSize().width / 2.0f,
                           innerHeight - (i + 0.5f) * (itemHeight + spacing)));
    _scrollView->addChild(item);
  }
}

Widget* ReplayLayer::createReplayItem(int replayId,
                                      const std::string& opponentName,
                                      bool isVictory,
                                      const std::string& timeStr) {
  float width = 700.0f;
  float height = 100.0f;

  auto widget = Layout::create();
  widget->setContentSize(Size(width, height));
  widget->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
  widget->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
  widget->setBackGroundColor(COLOR_ITEM_BG);

  // Result Icon/Text
  auto resultLabel = createLabel(isVictory ? "胜利" : "失败", 28,
                                 isVictory ? Color4B::GREEN : Color4B::RED);
  resultLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
  resultLabel->setPosition(Vec2(20.0f, height / 2.0f + 15.0f));
  widget->addChild(resultLabel);

  // Opponent Name
  auto nameLabel = createLabel("VS " + opponentName, 20);
  nameLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
  nameLabel->setPosition(Vec2(120.0f, height / 2.0f + 15.0f));
  widget->addChild(nameLabel);

  // Time
  auto timeLabel = createLabel(timeStr, 16, Color4B::GRAY);
  timeLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
  timeLabel->setPosition(Vec2(20.0f, height / 2.0f - 20.0f));
  widget->addChild(timeLabel);

  // Replay Button
  auto btn = Button::create("images/ui/Bar.png");
  btn->setScale(0.4f);
  btn->setTitleText("观看");
  btn->setTitleFontSize(24);
  btn->setPosition(Vec2(width - 80.0f, height / 2.0f));
  btn->addClickEventListener([this, replayId](Ref*) {
    if (_onReplaySelected) {
      _onReplaySelected(replayId);
    }
  });
  widget->addChild(btn);

  return widget;
}
