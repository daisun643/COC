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
  // 商店按钮 (右下角)
  // 暂时没有图片，使用文本按钮代替，或者创建一个简单的颜色块
  _shopButton = Button::create();
  _shopButton->setTitleText("SHOP");
  _shopButton->setTitleFontSize(24);
  _shopButton->setTitleColor(Color3B::WHITE);
  // 暂时用一个简单的背景色，实际开发请换成 "images/ui/ShopButton.png"
  // 这里为了看清按钮，我们不设置纹理，直接用文字，或者你可以自己加一个 DrawNode
  _shopButton->setPosition(
      Vec2(origin.x + visibleSize.width - 60, origin.y + 60));
  _shopButton->addClickEventListener([this](Ref* sender) {
    if (_onShopClick) {
      _onShopClick();
    }
  });
  this->addChild(_shopButton);

  // 进攻按钮 (左下角)
  _attackButton = Button::create();
  _attackButton->setTitleText("ATTACK");
  _attackButton->setTitleFontSize(24);
  _attackButton->setTitleColor(Color3B::RED);
  _attackButton->setPosition(Vec2(origin.x + 60, origin.y + 60));
  _attackButton->addClickEventListener([this](Ref* sender) {
    if (_onAttackClick) {
      _onAttackClick();
    }
  });
  this->addChild(_attackButton);

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
