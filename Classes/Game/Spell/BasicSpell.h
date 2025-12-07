#ifndef __BASIC_SPELL_H__
#define __BASIC_SPELL_H__

#include <vector>

#include "cocos2d.h"

USING_NS_CC;

// 前向声明
class BasicSoldier;
class Building;
/**
 * 法术类型枚举
 */
enum class SpellType {
  HEAL,       // 治疗法术
  LIGHTNING,  // 雷电法术
  RAGE        // 狂暴法术
};

/**
 * 法术效果类型枚举
 */
enum class SpellCategory {
  INSTANT,  // 瞬时效果（立即生效）
  DURATION  // 持续效果（持续一段时间）
};

/**
 * 法术基类
 * 所有法术的基类，提供通用的法术功能
 */
class BasicSpell : public Node {
 public:
  /**
   * 初始化法术
   * @param spellType 法术类型
   * @return 是否初始化成功
   */
  bool init(SpellType spellType);

  /**
   * 更新函数（每帧调用）
   * @param delta 时间间隔
   */
  virtual void update(float delta) override;

  /**
   * 施放法术
   * @param position 施法位置（世界坐标）
   * @param soldiers 士兵列表（用于查找范围内的目标）
   * @param buildings 建筑列表（用于查找范围内的目标）
   * @return 是否施法成功
   */
  bool cast(const Vec2& position, const std::vector<BasicSoldier*>& soldiers,
            const std::vector<Building*>& buildings);

  /**
   * 是否正在生效
   * @return 是否正在生效
   */
  bool isActive() const { return _isActive; }

  /**
   * 获取法术类型
   * @return 法术类型
   */
  SpellType getSpellType() const { return _spellType; }

  /**
   * 设置目标查找回调（用于在施法时查找目标）
   */
  using TargetFinderCallback = std::function<std::vector<BasicSoldier*>()>;
  void setSoldierFinderCallback(TargetFinderCallback callback) {
    _soldierFinderCallback = callback;
  }

  using BuildingFinderCallback = std::function<std::vector<Building*>()>;
  void setBuildingFinderCallback(BuildingFinderCallback callback) {
    _buildingFinderCallback = callback;
  }

  // 属性访问器
  CC_SYNTHESIZE(SpellCategory, _category, Category);
  CC_SYNTHESIZE(float, _radius, Radius);
  CC_SYNTHESIZE(float, _amount, Amount);
  CC_SYNTHESIZE(float, _duration, Duration);
  CC_SYNTHESIZE(Vec2, _castPosition, CastPosition);
  CC_SYNTHESIZE(float, _ratio, Ratio);

  virtual ~BasicSpell();

 protected:
  BasicSpell();

  /**
   * 应用法术效果（子类实现）
   * @param soldiers 范围内的士兵列表
   * @param buildings 范围内的建筑列表
   */
  virtual void applyEffect(const std::vector<BasicSoldier*>& soldiers,
                           const std::vector<Building*>& buildings) = 0;

  /**
   * 创建视觉效果（子类实现）
   */
  virtual void createVisualEffect() = 0;

  /**
   * 更新持续效果（子类实现，仅持续效果类型需要）
   * @param delta 时间间隔
   * @param soldiers 范围内的士兵列表
   * @param buildings 范围内的建筑列表
   */
  virtual void updateEffect(float delta,
                            const std::vector<BasicSoldier*>& soldiers,
                            const std::vector<Building*>& buildings) {}

  /**
   * 法术结束时的处理（子类实现）
   */
  virtual void onSpellEnd() {}

  /**
   * 在范围内查找目标
   * @param soldiers 士兵列表
   * @param buildings 建筑列表
   * @param outSoldiers 输出的士兵列表
   * @param outBuildings 输出的建筑列表
   */
  void findTargetsInRange(const std::vector<BasicSoldier*>& soldiers,
                          const std::vector<Building*>& buildings,
                          std::vector<BasicSoldier*>& outSoldiers,
                          std::vector<Building*>& outBuildings);

  /**
   * 计算两点之间的距离
   * @param pos1 位置1
   * @param pos2 位置2
   * @return 距离
   */
  float getDistance(const Vec2& pos1, const Vec2& pos2) const;

  /**
   * 检查点是否在范围内
   * @param position 目标位置
   * @param center 中心位置
   * @param radius 范围半径
   * @return 是否在范围内
   */
  bool isInRange(const Vec2& position, const Vec2& center, float radius) const;

  SpellType _spellType;                            // 法术类型
  bool _isActive;                                  // 是否正在生效
  float _elapsedTime;                              // 已过时间
  TargetFinderCallback _soldierFinderCallback;     // 士兵查找回调
  BuildingFinderCallback _buildingFinderCallback;  // 建筑查找回调
  Node* _visualEffectNode;                         // 视觉效果节点
  std::string _panelImage;                         // 法术面板图片
};

#endif  // __BASIC_SPELL_H__
