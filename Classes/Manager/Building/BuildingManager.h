#ifndef __BUILDING_MANAGER_H__
#define __BUILDING_MANAGER_H__

#include <string>
#include <vector>

#include "Game/Building/Building.h"
#include "Manager/Config/ConfigManager.h"
#include "Utils/GridUtils.h"
#include "cocos2d.h"

USING_NS_CC;

/**
 * 建筑管理器
 * 管理地图中所有建筑的创建、显示和交互
 */
class BuildingManager {
 public:
  /**
   * 构造函数
   * @param jsonFilePath JSON配置文件路径
   * @param p00 地图原点p[0][0]的位置
   */
  BuildingManager(const std::string& jsonFilePath, const Vec2& p00);

  /**
   * 初始化建筑管理器，从配置文件加载所有建筑
   * @return 是否初始化成功
   */
  bool init();

  /**
   * 获取所有建筑
   */
  const std::vector<Building*>& getAllBuildings() const { return _buildings; }

  /**
   * 检查指定位置是否有建筑被点击
   * @param pos Layer坐标
   * @return 被点击的建筑指针，如果没有则返回nullptr
   */
  Building* getBuildingAtPosition(const Vec2& pos) const;

  /**
   * 添加建筑到场景
   * @param layer 要添加到的图层
   */
  void addBuildingsToLayer(Layer* layer);

  /**
   * 清除所有建筑
   */
  void clearAllBuildings();

  /**
   * 注册新建筑到管理器
   */
  void registerBuilding(Building* building);

  /**
   * 析构函数
   */
  ~BuildingManager();

 private:
  /**
   * 从配置文件加载建筑地图
   */
  bool loadBuildingMap();

  /**
   * 创建建筑实例
   * @param buildingName 建筑的具体名称（如 "Cannon", "TownHall"），对应配置文件中的Key
   * @param row 行坐标
   * @param col 列坐标
   * @param level 等级
   * @param hp 当前生命值，如果 < 0 则使用 MaxHP 作为默认值
   */
  Building* createBuilding(const std::string& buildingName, float row, float col,
                           int level, float hp = -1.0f);

  std::vector<Building*> _buildings;  // 所有建筑的列表
  Vec2 _p00;                          // 地图原点
  float _deltaX;                      // X方向间距
  float _deltaY;                      // Y方向间距
  int _gridSize;                      // 网格大小
  std::string _jsonFilePath;          // JSON配置文件路径
};

#endif  // __BUILDING_MANAGER_H__
