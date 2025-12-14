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

  // 重写升级方法
  virtual void upgrade() override;

  bool init(int level, std::string imagePath, int gridCount, float anchorRatioX,
            float anchorRatioY, float imageScale);

 protected:
  TownHall();
  virtual ~TownHall();
};

#endif  // __TOWN_HALL_H__
