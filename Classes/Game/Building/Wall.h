#ifndef __WALL_H__
#define __WALL_H__

#include "Building.h"

/**
 * 城墙类
 * 防御性建筑，用于保护村庄
 */
class Wall : public Building {
 public:
  /**
   * 创建城墙
   * @param level 城墙等级
   * @param buildingName 建筑名称（用于从配置读取）
   */
  static Wall* create(int level, const std::string& buildingName);
  
  /**
   * 初始化城墙
   * @param level 城墙等级
   * @param buildingName 建筑名称
   */
  bool init(int level, const std::string& buildingName);

  CC_SYNTHESIZE(float, _defense, Defense);  // 防御值

 protected:
  Wall();
  virtual ~Wall();
};

#endif  // __WALL_H__
