#ifndef __DRAGON_H__
#define __DRAGON_H__

#include "Game/Soldier/BasicSoldier.h"

/**
 * 飞龙兵种
 * 特性：空军单位，无视城墙，直线移动，远程攻击
 */
class Dragon : public BasicSoldier {
 public:
  static Dragon* create(int level);
  virtual bool init(int level);

 protected:
  Dragon();
  virtual ~Dragon();
};

#endif  