#ifndef __BARBARIAN_H__
#define __BARBARIAN_H__

#include "Game/Soldier/BasicSoldier.h"

USING_NS_CC;

/**
 * 野蛮人
 * 近战单位，攻击力强，移动速度快
 */
class Barbarian : public BasicSoldier {
 public:
  /**
   * 创建野蛮人
   * @param level 野蛮人等级
   * @return 野蛮人实例
   */
  static Barbarian* create(int level);

  /**
   * 初始化野蛮人
   * @param level 野蛮人等级
   * @return 是否初始化成功
   */
  bool init(int level);

 protected:
  /**
   * 创建默认外观（重写基类方法）
   */
  virtual void createDefaultAppearance() override;

  Barbarian();
  virtual ~Barbarian();
};

#endif  // __BARBARIAN_H__
