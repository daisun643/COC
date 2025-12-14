#include "BuildingMenuLayer.h"

USING_NS_CC;
using namespace cocos2d::ui;

bool BuildingMenuLayer::init() {
  if (!Layer::init()) {
    return false;
  }

  _menuContainer = Node::create();
  this->addChild(_menuContainer);
  _currentBuilding = nullptr;

  return true;
}

void BuildingMenuLayer::showBuildingOptions(Building* building) {
  _currentBuilding = building;
  _menuContainer->removeAllChildren();

  if (!building) {
    return;
  }

  std::vector<std::tuple<std::string, std::string, std::function<void()>>>
      buttons;

  // 1. 信息按钮
  buttons.push_back(
      std::make_tuple("images/ui/Information.png", "信息", [this]() {
        if (_onInfoCallback && _currentBuilding) {
          _onInfoCallback(_currentBuilding);
        }
      }));

  // 2. 升级按钮
  buttons.push_back(std::make_tuple("images/ui/Upgrade.png", "升级", [this]() {
    // 添加日志，确认点击事件触发
    CCLOG("BuildingMenuLayer: Upgrade button clicked for %s", 
          _currentBuilding ? _currentBuilding->getBuildingName().c_str() : "null");
          
    if (_onUpgradeCallback && _currentBuilding) {
      // 触发回调，GameScene 应在回调中调用 building->upgrade()
      _onUpgradeCallback(_currentBuilding);
    }
  }));

  // 3. 收集按钮 (仅针对资源建筑)
  if (building->getBuildingType() == BuildingType::RESOURCE) {
    auto resourceBuilding = dynamic_cast<ResourceBuilding*>(building);
    if (resourceBuilding) {
      std::string iconPath;
      std::string title;
      if (resourceBuilding->getResourceType() == "Gold") {
        iconPath = "images/ui/Gold.png";
        title = "收集金币";
      } else if (resourceBuilding->getResourceType() == "Elixir") {
        iconPath = "images/ui/Elixir.png";
        title = "收集圣水";
      }

      if (!iconPath.empty()) {
        buttons.push_back(std::make_tuple(iconPath, title, [this]() {
          if (_onCollectCallback && _currentBuilding) {
            _onCollectCallback(_currentBuilding);
          }
        }));
      }
    }
  }

  // 创建按钮
  int total = buttons.size();
  for (int i = 0; i < total; ++i) {
    auto& btn = buttons[i];
    createButton(std::get<0>(btn), std::get<1>(btn), std::get<2>(btn), i,
                 total);
  }
}

void BuildingMenuLayer::showRemoveOption(Building* building) {
  _currentBuilding = building;
  _menuContainer->removeAllChildren();

  if (!building) {
    return;
  }

  // 仅显示移除按钮
  createButton(
      "images/ui/Remove.png", "移除",
      [this]() {
        if (_onRemoveCallback && _currentBuilding) {
          _onRemoveCallback(_currentBuilding);
        }
        hideOptions();
      },
      0, 1, 140.0f);
}

void BuildingMenuLayer::hideOptions() {
  _menuContainer->removeAllChildren();
  _currentBuilding = nullptr;
}

void BuildingMenuLayer::createButton(const std::string& imagePath,
                                     const std::string& title,
                                     const std::function<void()>& callback,
                                     int index, int total, float yOffset) {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  float buttonSize = 80.0f;
  float spacing = 20.0f;
  float totalWidth = total * buttonSize + (total - 1) * spacing;
  float startX =
      origin.x + (visibleSize.width - totalWidth) / 2.0f + buttonSize / 2.0f;
  float y = origin.y + 80.0f + yOffset;  // 底部上方一点

  float x = startX + index * (buttonSize + spacing);

  auto button = Button::create(imagePath);
  // button->ignoreContentAdaptWithSize(false);
  // button->setContentSize(Size(buttonSize, buttonSize));
  button->setScale(0.8f);  // 适当缩放
  button->setPosition(Vec2(x, y));
  button->addClickEventListener([callback](Ref*) { callback(); });
  _menuContainer->addChild(button);

  // 标题
  auto label = Label::createWithSystemFont(title, "Arial", 16);
  label->setPosition(Vec2(x, y - 50.0f));
  label->enableOutline(Color4B::BLACK, 1);
  _menuContainer->addChild(label);
}

void BuildingMenuLayer::setOnInfoCallback(
    std::function<void(Building*)> callback) {
  _onInfoCallback = callback;
}

void BuildingMenuLayer::setOnUpgradeCallback(
    std::function<void(Building*)> callback) {
  _onUpgradeCallback = callback;
}

void BuildingMenuLayer::setOnCollectCallback(
    std::function<void(Building*)> callback) {
  _onCollectCallback = callback;
}
void BuildingMenuLayer::setOnRemoveCallback(
    std::function<void(Building*)> callback) {
  _onRemoveCallback = callback;
}
bool BuildingMenuLayer::isPointInMenu(const Vec2& worldPos) {
  if (!_menuContainer) {
    return false;
  }

  for (auto child : _menuContainer->getChildren()) {
    auto button = dynamic_cast<Button*>(child);
    if (button) {
      // 检查点击位置是否在按钮的包围盒内
      // 注意：worldPos是世界坐标，需要转换到按钮的父节点空间，或者直接使用hitTest
      // Button::hitTest 接受世界坐标
      Vec2 touchPos = button->getParent()->convertToNodeSpace(worldPos);
      if (button->getBoundingBox().containsPoint(touchPos)) {
        return true;
      }
    }
  }
  return false;
}
