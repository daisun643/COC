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

  Bomber();
  virtual ~Bomber();
};

#endif  // __BOMBER_H__

