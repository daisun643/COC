#ifndef __TOWN_HALL_H__
#define __TOWN_HALL_H__

#include "Building.h"

/**
 * 大本营类
 * 村庄的核心建筑，决定其他建筑的等级上限
 */
class TownHall : public Building {
 public:
  // static TownHall* create(int level = 1);

  /**
   * 创建TownHall（新版本，包含gridSize和anchorRatio）
   */
  static TownHall* create(int level);

  // bool init(int level);

  /**
   * 初始化TownHall（新版本，包含gridSize和anchorRatio）
   */
  bool init(int level, std::string imagePath, int gridCount, float anchorRatioX,
            float anchorRatioY, float imageScale);

 protected:
  TownHall();
  virtual ~TownHall();
};

#endif  // __TOWN_HALL_H__
