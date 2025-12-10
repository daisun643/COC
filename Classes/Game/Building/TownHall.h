#ifndef __TOWN_HALL_H__
#define __TOWN_HALL_H__

#include "Building.h"

/**
 * 大本营类
 * 村庄的核心建筑，决定其他建筑的等级上限
 */
class TownHall : public Building {
 public:
  /**
   * 创建TownHall
   */
  static TownHall* create(int level);

  /**
   * 初始化TownHall
   * 内部会从ConfigManager读取 "TownHall" 的配置
   */
  bool init(int level);
  
  // 重写升级方法
  virtual void upgrade() override;

 protected:
  TownHall();
  virtual ~TownHall();
};

#endif  // __TOWN_HALL_H__
