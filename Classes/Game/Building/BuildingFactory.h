#ifndef __BUILDING_FACTORY_H__
#define __BUILDING_FACTORY_H__

#include <functional>
#include <map>

#include "Container/Layer/ShopLayer.h"
#include "Game/Building/Building.h"

/**
 * 建筑工厂类
 * 负责根据 ShopItem 创建对应的 Building 实例
 * 使用单例模式和注册机制，消除手动 switch-case 分支
 */
class BuildingFactory {
 public:
  using Creator = std::function<Building*(const ShopItem&)>;

  static BuildingFactory* getInstance();

  /**
   * 注册一种建筑类型的创建函数
   */
  void registerType(BuildingType type, Creator creator);

  /**
   * 根据商品信息创建建筑
   */
  Building* createBuilding(const ShopItem& item);

 private:
  BuildingFactory();
  ~BuildingFactory() = default;

  std::map<BuildingType, Creator> _creators;
};

#endif  // __BUILDING_FACTORY_H__
