#ifndef __GIANT_H__
#define __GIANT_H__

#include "Game/Soldier/BasicSoldier.h"

USING_NS_CC;

/**
 * 巨人
 * 近战单位，生命值高，优先攻击防御建筑
 */
class Gaint : public BasicSoldier {
 public:
  /**
   * 创建巨人
   * @param level 巨人等级
   * @return 巨人实例
   */
  static Gaint* create(int level);

  /**
   * 初始化巨人
   * @param level 巨人等级
   * @return 是否初始化成功
   */
  bool init(int level);

 protected:
  /**
   * 创建默认外观（重写基类方法）
   */
  virtual void createDefaultAppearance() override;

  Gaint();
  virtual ~Gaint();
};

#endif  // __GIANT_H__
