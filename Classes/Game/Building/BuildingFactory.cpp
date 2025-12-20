#include "BuildingFactory.h"

#include "Game/Building/AllBuildings.h"
#include "Game/Building/Wall.h"

BuildingFactory* BuildingFactory::getInstance() {
  static BuildingFactory instance;
  return &instance;
}

BuildingFactory::BuildingFactory() {
  // 注册所有已知的建筑类型
  // 这样当需要添加新类型时，只需在这里注册，而不需要修改 GameScene

  // 大本营
  registerType(BuildingType::TOWN_HALL, [](const ShopItem& item) {
    return TownHall::create(item.defaultLevel);
  });

  // 资源建筑
  registerType(BuildingType::RESOURCE, [](const ShopItem& item) {
    return ResourceBuilding::create(item.defaultLevel, item.id);
  });

  // 防御建筑
  registerType(BuildingType::DEFENSE, [](const ShopItem& item) {
    return DefenseBuilding::create(item.defaultLevel, item.id);
  });

  // 储存建筑
  registerType(BuildingType::STORAGE, [](const ShopItem& item) {
    return StorageBuilding::create(item.defaultLevel, item.id);
  });

  // 兵营
  registerType(BuildingType::BARRACKS, [](const ShopItem& item) {
    return BarracksBuilding::create(item.defaultLevel, item.id);
  });

  // 城墙
  registerType(BuildingType::WALL, [](const ShopItem& item) {
    return Wall::create(item.defaultLevel, item.id);
  });

  // 炸弹
  registerType(BuildingType::TRAP, [](const ShopItem& item) {
    return TrapBuilding::create(item.defaultLevel, item.id);
  });
}

void BuildingFactory::registerType(BuildingType type, Creator creator) {
  _creators[type] = creator;
}

Building* BuildingFactory::createBuilding(const ShopItem& item) {
  auto it = _creators.find(item.category);
  if (it != _creators.end()) {
    return it->second(item);
  }

  // 默认回退到占位符建筑
  return PlaceholderBuilding::create(
      item.displayName, item.category, item.placeholderColor, item.defaultLevel,
      item.gridCount, item.anchorRatioX, item.anchorRatioY, item.imageScale);
}
