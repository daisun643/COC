#ifndef __BUILDING_MANAGER_H__
#define __BUILDING_MANAGER_H__

#include "Building/Building.h"
#include "Config/ConfigManager.h"
#include "util/GridUtils.h"
#include "cocos2d.h"
#include <vector>
#include <string>

USING_NS_CC;

/**
 * 建筑管理器
 * 管理地图中所有建筑的创建、显示和交互
 */
class BuildingManager {
public:
  static BuildingManager* getInstance();
  
  /**
   * 初始化建筑管理器（单例初始化）
   * @param p00 地图原点p[0][0]的位置
   */
  static void initialize(const Vec2 &p00);
  
  /**
   * 初始化建筑管理器，从配置文件加载所有建筑
   * @param p00 地图原点p[0][0]的位置
   * @return 是否初始化成功
   */
  bool init(const Vec2 &p00);
  
  /**
   * 获取所有建筑
   */
  const std::vector<Building*>& getAllBuildings() const { return _buildings; }
  
  /**
   * 检查指定位置是否有建筑被点击
   * @param pos 屏幕坐标
   * @return 被点击的建筑指针，如果没有则返回nullptr
   */
  Building* getBuildingAtPosition(const Vec2 &pos) const;
  
  /**
   * 添加建筑到场景
   * @param layer 要添加到的图层
   */
  void addBuildingsToLayer(Layer* layer);
  
  /**
   * 清除所有建筑
   */
  void clearAllBuildings();

private:
  BuildingManager();
  ~BuildingManager();
  
  /**
   * 从配置文件加载建筑地图
   */
  bool loadBuildingMap();
  
  /**
   * 创建建筑实例
   */
  Building* createBuilding(const std::string &buildingType, int row, int col, int level);
  
  std::vector<Building*> _buildings;  // 所有建筑的列表
  Vec2 _p00;           // 地图原点
  float _deltaX;       // X方向间距
  float _deltaY;       // Y方向间距
  int _gridSize;       // 网格大小
  
  static BuildingManager* _instance;
};

#endif // __BUILDING_MANAGER_H__

