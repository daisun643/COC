#ifndef __BOMBER_H__
#define __BOMBER_H__

#include "Game/Soldier/BasicSoldier.h"

USING_NS_CC;

/**
 * 炸弹人
 * 近战单位，攻击力强，专门攻击城墙
 */
class Bomber : public BasicSoldier {
 public:
  /**
   * 创建炸弹人
   * @param level 炸弹人等级
   * @return 炸弹人实例
   */
  static Bomber* create(int level);

  /**
   * 初始化炸弹人
   * @param level 炸弹人等级
   * @return 是否初始化成功
   */
  bool init(int level);

 protected:
  /**
   * 创建默认外观（重写基类方法）
   */
  virtual void createDefaultAppearance() override;

  /**
   * 攻击目标（重写基类方法）
   * 炸弹人特殊逻辑：对半径100像素内的所有城墙造成伤害，然后自己死亡
   * @param delta 时间间隔
   */
  virtual void attackTarget(float delta) override;

  Bomber();
  virtual ~Bomber();
};

#endif  // __BOMBER_H__

