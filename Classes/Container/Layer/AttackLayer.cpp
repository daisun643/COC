#include "AttackLayer.h"

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

AttackLayer* AttackLayer::create() {
  AttackLayer* layer = new (std::nothrow) AttackLayer();
  if (layer && layer->init()) {
    layer->autorelease();
    return layer;
  }
  CC_SAFE_DELETE(layer);
  return nullptr;
}

bool AttackLayer::init() {
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

void AttackLayer::setOnSearchOpponentCallback(std::function<void()> callback) {
  _onSearchOpponent = callback;
}

void AttackLayer::setOnLevelSelectedCallback(
    std::function<void(int)> callback) {
  _onLevelSelected = callback;
}

void AttackLayer::buildUI() {
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

  // Title
  auto title = createLabel("进攻", 32);
  title->setPosition(Vec2(_panel->getContentSize().width / 2.0f,
                          _panel->getContentSize().height - 35.0f));
  title->enableBold();
  _panel->addChild(title);

  // Close Button
  auto closeBtn = Button::create("images/ui/Bar.png");  // Reuse existing asset
  closeBtn->setScale(0.5f);
  closeBtn->setTitleText("关闭");
  closeBtn->setTitleFontSize(20);
  closeBtn->setPosition(Vec2(_panel->getContentSize().width - 60.0f,
                             _panel->getContentSize().height - 35.0f));
  closeBtn->addClickEventListener([this](Ref*) { this->removeFromParent(); });
  _panel->addChild(closeBtn);

  initTabs();
}

void AttackLayer::initTabs() {
  float tabWidth = 200.0f;
  float tabHeight = 50.0f;
  float startY = _panel->getContentSize().height - 80.0f;

  // Single Player Tab Button
  _btnSinglePlayer = Button::create("images/ui/Bar.png");  // Placeholder image
  _btnSinglePlayer->setScale9Enabled(true);
  _btnSinglePlayer->setContentSize(Size(tabWidth, tabHeight));
  _btnSinglePlayer->setTitleText("单人模式");
  _btnSinglePlayer->setTitleFontSize(24);
  _btnSinglePlayer->setPosition(Vec2(
      _panel->getContentSize().width / 2.0f - tabWidth / 2.0f - 10.0f, startY));
  _btnSinglePlayer->addClickEventListener(
      [this](Ref*) { showSinglePlayerTab(); });
  _panel->addChild(_btnSinglePlayer);

  // Multiplayer Tab Button
  _btnMultiplayer = Button::create("images/ui/Bar.png");  // Placeholder image
  _btnMultiplayer->setScale9Enabled(true);
  _btnMultiplayer->setContentSize(Size(tabWidth, tabHeight));
  _btnMultiplayer->setTitleText("联机模式");
  _btnMultiplayer->setTitleFontSize(24);
  _btnMultiplayer->setPosition(Vec2(
      _panel->getContentSize().width / 2.0f + tabWidth / 2.0f + 10.0f, startY));
  _btnMultiplayer->addClickEventListener(
      [this](Ref*) { showMultiplayerTab(); });
  _panel->addChild(_btnMultiplayer);

  // Content Area
  _contentArea = Layout::create();
  _contentArea->setContentSize(Size(760.0f, 450.0f));
  _contentArea->setPosition(Vec2(20.0f, 20.0f));
  _panel->addChild(_contentArea);

  // Default to Single Player
  showSinglePlayerTab();
}

void AttackLayer::updateTabButtons(bool isSinglePlayer) {
  // Simple visual feedback (opacity or color)
  // Since we are using "Bar.png" which might be an image, setColor affects
  // tint.
  if (isSinglePlayer) {
    _btnSinglePlayer->setColor(Color3B::WHITE);
    _btnMultiplayer->setColor(Color3B::GRAY);
  } else {
    _btnSinglePlayer->setColor(Color3B::GRAY);
    _btnMultiplayer->setColor(Color3B::WHITE);
  }
}

void AttackLayer::showSinglePlayerTab() {
  updateTabButtons(true);
  _contentArea->removeAllChildren();

  // Vertical ScrollView
  _scrollView = ScrollView::create();
  _scrollView->setContentSize(_contentArea->getContentSize());
  _scrollView->setDirection(ScrollView::Direction::VERTICAL);
  _scrollView->setBounceEnabled(true);
  _contentArea->addChild(_scrollView);

  // Populate Levels (Mock Data)
  float itemHeight = 100.0f;
  float spacing = 10.0f;
  int levelCount = 10;
  float innerHeight = (itemHeight + spacing) * levelCount;
  if (innerHeight < _contentArea->getContentSize().height) {
    innerHeight = _contentArea->getContentSize().height;
  }

  _scrollView->setInnerContainerSize(
      Size(_contentArea->getContentSize().width, innerHeight));

  for (int i = 0; i < levelCount; ++i) {
    auto item =
        createLevelItem(i + 1, StringUtils::format("哥布林前哨 %d", i + 1),
                        i % 4);  // Mock stars
    item->setPosition(Vec2(_contentArea->getContentSize().width / 2.0f,
                           innerHeight - (i + 0.5f) * (itemHeight + spacing)));
    _scrollView->addChild(item);
  }
}

void AttackLayer::showMultiplayerTab() {
  updateTabButtons(false);
  _contentArea->removeAllChildren();

  auto label = createLabel("搜索其他玩家进行对战！", 24);
  label->setPosition(
      Vec2(_contentArea->getContentSize().width / 2.0f,
           _contentArea->getContentSize().height / 2.0f + 50.0f));
  _contentArea->addChild(label);

  auto searchBtn =
      Button::create("images/ui/Attack.png");  // Reuse Attack icon or Bar
  searchBtn->setScale(0.8f);
  searchBtn->setPosition(
      Vec2(_contentArea->getContentSize().width / 2.0f,
           _contentArea->getContentSize().height / 2.0f - 50.0f));
  searchBtn->addClickEventListener([this](Ref*) {
    if (_onSearchOpponent) {
      _onSearchOpponent();
    }
  });
  _contentArea->addChild(searchBtn);

  auto btnLabel = createLabel("搜索对手", 24);
  btnLabel->setPosition(
      Vec2(searchBtn->getPositionX(), searchBtn->getPositionY() - 60.0f));
  _contentArea->addChild(btnLabel);

  auto costLabel = createLabel("花费: 50 金币", 20, Color4B::YELLOW);
  costLabel->setPosition(
      Vec2(searchBtn->getPositionX(), searchBtn->getPositionY() - 90.0f));
  _contentArea->addChild(costLabel);
}

Widget* AttackLayer::createLevelItem(int levelId, const std::string& name,
                                     int stars) {
  float width = 700.0f;
  float height = 100.0f;

  auto widget = Layout::create();
  widget->setContentSize(Size(width, height));
  widget->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
  widget->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
  widget->setBackGroundColor(COLOR_ITEM_BG);

  // Level Name
  auto nameLabel = createLabel(name, 24);
  nameLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
  nameLabel->setPosition(Vec2(20.0f, height / 2.0f));
  widget->addChild(nameLabel);

  // Stars
  std::string starStr = "星星: ";
  for (int i = 0; i < stars; ++i) starStr += "★";
  for (int i = stars; i < 3; ++i) starStr += "☆";

  auto starLabel = createLabel(starStr, 20, Color4B::YELLOW);
  starLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
  starLabel->setPosition(Vec2(300.0f, height / 2.0f));
  widget->addChild(starLabel);

  // Attack Button
  auto btn = Button::create("images/ui/Bar.png");
  btn->setScale(0.4f);
  btn->setTitleText("进攻");
  btn->setTitleFontSize(24);
  btn->setPosition(Vec2(width - 80.0f, height / 2.0f));
  btn->addClickEventListener([this, levelId](Ref*) {
    if (_onLevelSelected) {
      _onLevelSelected(levelId);
    }
  });
  widget->addChild(btn);

  return widget;
}
