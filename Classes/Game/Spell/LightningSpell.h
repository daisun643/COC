#ifndef __LIGHTNING_SPELL_H__
#define __LIGHTNING_SPELL_H__

#include "Game/Spell/BasicSpell.h"

USING_NS_CC;

/**
 * 雷电法术
 * 对范围内目标造成伤害
 */
class LightningSpell : public BasicSpell {
 public:
  /**
   * 创建雷电法术
   * @return 法术实例
   */
  static LightningSpell* create();

  /**
   * 初始化雷电法术
   * @return 是否初始化成功
   */
  bool init() override;

 protected:
  /**
   * 应用雷电伤害效果
   * @param soldiers 范围内的士兵列表
   * @param buildings 范围内的建筑列表
   */
  void applyEffect(const std::vector<BasicSoldier*>& soldiers,
                   const std::vector<Building*>& buildings) override;

  /**
   * 创建视觉效果（闪电效果）
   */
  void createVisualEffect() override;

  LightningSpell();
  virtual ~LightningSpell();
};

#endif  // __LIGHTNING_SPELL_H__
