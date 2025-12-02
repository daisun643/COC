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

  // 商店按钮 (右下角)
  _shopButton = Button::create("images/ui/Shop.png");
  _shopButton->setScale(0.3f);  // 调整大小，根据实际图片大小修改
  _shopButton->setPosition(Vec2(origin.x + visibleSize.width - 80,
                                origin.y + 80));  // 调整位置，留出更多边距
  _shopButton->addClickEventListener([this](Ref* sender) {
    if (_onShopClick) {
      _onShopClick();
    }
  });
  this->addChild(_shopButton);

  auto shopLabel = createLabel("商店");
  shopLabel->setPosition(
      Vec2(origin.x + visibleSize.width - 80, origin.y + 30));
  this->addChild(shopLabel);

  // 进攻按钮 (左下角)
  _attackButton = Button::create("images/ui/Attack.png");
  _attackButton->setScale(0.8f);                                   // 调整大小
  _attackButton->setPosition(Vec2(origin.x + 80, origin.y + 80));  // 调整位置
  _attackButton->addClickEventListener([this](Ref* sender) {
    if (_onAttackClick) {
      _onAttackClick();
    }
  });
  this->addChild(_attackButton);

  auto attackLabel = createLabel("进攻");
  attackLabel->setPosition(Vec2(origin.x + 80, origin.y + 30));
  this->addChild(attackLabel);

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
