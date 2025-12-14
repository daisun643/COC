#include "MainUILayer.h"

#include "Manager/PlayerManager.h"

USING_NS_CC;
using namespace cocos2d::ui;

bool MainUILayer::init() {
  if (!Layer::init()) {
    return false;
  }

  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // ================= 资源条 =================
  // 右上角显示
  float margin = 20.0f;

  // 圣水条 (最上面)
  _elixirWidget = ResourceWidget::create(ResourceWidget::Type::ELIXIR);
  _elixirWidget->setPosition(Vec2(origin.x + visibleSize.width - 160,
                                  origin.y + visibleSize.height - margin - 20));
  this->addChild(_elixirWidget);

  // 金币条 (在圣水条下面)
  _goldWidget = ResourceWidget::create(ResourceWidget::Type::GOLD);
  _goldWidget->setPosition(Vec2(origin.x + visibleSize.width - 160,
                                origin.y + visibleSize.height - margin - 70));
  this->addChild(_goldWidget);

  // ================= 按钮 =================
  // 字体配置
  std::string fontFile = "fonts/NotoSansSC-VariableFont_wght.ttf";
  bool useTTF = FileUtils::getInstance()->isFileExist(fontFile);
  auto createLabel = [&](const std::string& text) {
    Label* label;
    if (useTTF) {
      TTFConfig ttfConfig(fontFile, 24);
      label = Label::createWithTTF(ttfConfig, text);
    } else {
      label = Label::createWithSystemFont(text, "Arial", 24);
    }
    label->enableOutline(Color4B::BLACK, 2);
    return label;
  };

  auto createButton = [&](const std::string& img, float scale, const Vec2& pos,
                          const std::string& labelText, const Vec2& labelPos,
                          const std::function<void()>& callback) -> Button* {
    auto btn = Button::create(img);
    btn->setScale(scale);
    btn->setPosition(pos);
    btn->addClickEventListener([callback](Ref*) {
      if (callback) callback();
    });
    this->addChild(btn);

    auto label = createLabel(labelText);
    label->setPosition(labelPos);
    this->addChild(label);

    return btn;
  };

  // 商店按钮 (右下角)
  _shopButton = createButton(
      "images/ui/Shop.png", 0.3f,
      Vec2(origin.x + visibleSize.width - 80, origin.y + 80), "商店",
      Vec2(origin.x + visibleSize.width - 80, origin.y + 30), [this]() {
        if (_onShopClick) _onShopClick();
      });

  // 进攻按钮 (左下角)
  _attackButton = createButton("images/ui/Attack.png", 0.8f,
                               Vec2(origin.x + 80, origin.y + 80), "进攻",
                               Vec2(origin.x + 80, origin.y + 30), [this]() {
                                 if (_onAttackClick) _onAttackClick();
                               });

  // 回放按钮 (进攻按钮上方)
  _replayButton = createButton("images/ui/record.png", 0.8f,
                               Vec2(origin.x + 80, origin.y + 210), "回放",
                               Vec2(origin.x + 80, origin.y + 180), [this]() {
                                 if (_onReplayClick) _onReplayClick();
                               });

  // 地图编辑按钮 (商店按钮上方)
  _mapEditButton = createButton(
      "images/ui/map.png", 0.8f,
      Vec2(origin.x + visibleSize.width - 80, origin.y + 210), "地图编辑",
      Vec2(origin.x + visibleSize.width - 80, origin.y + 180), [this]() {
        if (_onMapEditClick) _onMapEditClick();
      });

  // 开启 update 用于实时刷新资源
  this->scheduleUpdate();

  return true;
}

void MainUILayer::update(float dt) {
  // 每一帧从 PlayerManager 获取最新资源数并更新 UI
  auto playerManager = PlayerManager::getInstance();
  if (playerManager) {
    if (_goldWidget) {
      _goldWidget->updateAmount(playerManager->getGold(),
                                playerManager->getMaxGold());
      _goldWidget->setProduction(playerManager->getGoldProduction());
    }
    if (_elixirWidget) {
      _elixirWidget->updateAmount(playerManager->getElixir(),
                                  playerManager->getMaxElixir());
      _elixirWidget->setProduction(playerManager->getElixirProduction());
    }
  }
}

void MainUILayer::setOnShopClickCallback(std::function<void()> callback) {
  _onShopClick = callback;
}

void MainUILayer::setOnAttackClickCallback(std::function<void()> callback) {
  _onAttackClick = callback;
}

void MainUILayer::setOnReplayClickCallback(std::function<void()> callback) {
  _onReplayClick = callback;
}

void MainUILayer::setOnMapEditClickCallback(std::function<void()> callback) {
  _onMapEditClick = callback;
}
