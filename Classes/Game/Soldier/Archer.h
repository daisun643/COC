#ifndef __ARCHER_H__
#define __ARCHER_H__

#include "Game/Soldier/BasicSoldier.h"

USING_NS_CC;

/**
 * 弓箭手
 * 远程单位，攻击范围远，移动速度快
 */
class Archer : public BasicSoldier {
 public:
  /**
   * 创建弓箭手
   * @param level 弓箭手等级
   * @return 弓箭手实例
   */
  static Archer* create(int level);

  /**
   * 初始化弓箭手
   * @param level 弓箭手等级
   * @return 是否初始化成功
   */
  bool init(int level);

 protected:
  /**
   * 创建默认外观（重写基类方法）
   */
  virtual void createDefaultAppearance() override;

  Archer();
  virtual ~Archer();
};

#endif  // __ARCHER_H__
