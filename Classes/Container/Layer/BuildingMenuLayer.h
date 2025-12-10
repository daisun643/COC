#ifndef __BUILDING_MENU_LAYER_H__
#define __BUILDING_MENU_LAYER_H__

#include "Game/Building/AllBuildings.h"
#include "cocos2d.h"
#include "ui/CocosGUI.h"

class BuildingMenuLayer : public cocos2d::Layer {
 public:
  CREATE_FUNC(BuildingMenuLayer);
  virtual bool init();

  void showBuildingOptions(Building* building);
  void hideOptions();

  void setOnInfoCallback(std::function<void(Building*)> callback);
  void setOnUpgradeCallback(std::function<void(Building*)> callback);
  void setOnCollectCallback(std::function<void(Building*)> callback);

  bool isPointInMenu(const cocos2d::Vec2& worldPos);

 private:
  void createButton(const std::string& imagePath, const std::string& title,
                    const std::function<void()>& callback, int index,
                    int total);

  cocos2d::Node* _menuContainer;
  Building* _currentBuilding;

  std::function<void(Building*)> _onInfoCallback;
  std::function<void(Building*)> _onUpgradeCallback;
  std::function<void(Building*)> _onCollectCallback;
};

#endif  // __BUILDING_MENU_LAYER_H__
