#ifndef __BASIC_SOILDER_H__
#define __BASIC_SOILDER_H__

#include <functional>
#include <vector>

#include "Game/Building/Building.h"
#include "cocos2d.h"

USING_NS_CC;

/**
 * 士兵状态枚举
 */
enum class SoldierState {
  IDLE,       // 待机
  MOVING,     // 移动中
  ATTACKING,  // 攻击中
  DEAD        // 死亡
};
/**
 * 士兵分类枚举
 */
enum class SoldierCategory {
  LAND,  // 陆军
  AIR,   // 空军
};
/**
 * 士兵类型枚举
 */
enum class SoldierType {
  BARBARIAN,  // 野蛮人
  ARCHER,     // 弓箭手
  GIANT,      // 巨人
  BOMBER      // 炸弹人
};
// TODO 大本营删除
/**
 * 攻击类型枚举
 */
enum class AttackType {
  ANY,       // 任意目标
  DEFENSE,   // 优先防御建筑
  RESOURCE,  // 优先资源建筑
  TOWN_HALL, // 优先大本营
  WALL       // 优先墙
};

/**
 * 士兵基础类
 * 所有士兵的基类，提供通用的士兵功能
 */
class BasicSoldier : public Sprite {
 public:
  /**
   * 创建士兵
   * @param soldierType 士兵类型
   * @param level 士兵等级
   * @return 士兵实例
   */
  static BasicSoldier* create(SoldierType soldierType, int level);

  /**
   * 初始化士兵
   * @param soldierType 士兵类型
   * @param level 士兵等级
   * @return 是否初始化成功
   */
  bool init(SoldierType soldierType, int level);

  /**
   * 更新函数（每帧调用）
   * @param delta 时间间隔
   */
  virtual void update(float delta) override;

  // 属性访问器
  CC_SYNTHESIZE(SoldierType, _soldierType, SoldierType);
  CC_SYNTHESIZE(int, _level, Level);
  CC_SYNTHESIZE(float, _maxHP, MaxHP);
  CC_SYNTHESIZE(float, _currentHP, CurrentHP);
  CC_SYNTHESIZE(float, _attackDamage, AttackDamage);
  CC_SYNTHESIZE(float, _attackSpeed, AttackSpeed);  // 每秒攻击次数
  CC_SYNTHESIZE(float, _moveSpeed, MoveSpeed);      // 像素/秒
  CC_SYNTHESIZE(float, _attackRange, AttackRange);  // 攻击范围（像素）
  CC_SYNTHESIZE(AttackType, _attackType, AttackType);
  CC_SYNTHESIZE(SoldierState, _state, State);
  CC_SYNTHESIZE(float, _centerX, CenterX);
  CC_SYNTHESIZE(float, _centerY, CenterY);
  CC_SYNTHESIZE(Building*, _target, Target);

  /**
   * 受到伤害
   * @param damage 伤害值
   */
  void takeDamage(float damage);

  /**
   * 是否存活
   * @return 是否存活
   */
  bool isAlive() const;

  /**
   * 设置目标位置（用于移动）
   * @param position 目标位置
   */
  void setTargetPosition(const Vec2& position);

  /**
   * 寻找目标
   * @param buildings 建筑列表
   * @return 是否找到目标
   */
  bool findTarget(const std::vector<Building*>& buildings);

  /**
   * 设置建筑查找回调函数
   * @param callback 返回建筑列表的回调函数
   */
  void setBuildingFinderCallback(
      std::function<std::vector<Building*>()> callback);

  /**
   * 检查目标是否在攻击范围内
   * @param targetPos 目标位置
   * @return 是否在范围内
   */
  bool isInRange(const Vec2& targetPos) const;

  /**
   * 计算到目标位置的距离
   * @param pos 目标位置
   * @return 距离
   */
  float getDistanceTo(const Vec2& pos) const;

  /**
   * 更新生命值条显示
   */
  void updateHPBar();

 protected:
  BasicSoldier();
  virtual ~BasicSoldier();

  /**
   * 创建默认外观（如果图片不存在）
   */
  virtual void createDefaultAppearance();

  /**
   * 更新状态机
   * @param delta 时间间隔
   */
  void updateState(float delta);

  /**
   * 移动到目标位置
   * @param delta 时间间隔
   */
  void moveToTarget(float delta);

  /**
   * 攻击目标
   * @param delta 时间间隔
   */
  void attackTarget(float delta);

  /**
   * 死亡处理
   */
  void die();

  Vec2 _targetPosition;        // 目标位置（用于移动）
  float _attackCooldown;       // 攻击冷却时间
  DrawNode* _hpBarBackground;  // 生命值条背景
  DrawNode* _hpBarForeground;  // 生命值条前景
  Label* _infoLabel;           // 信息显示标签
  std::function<std::vector<Building*>()>
      _buildingFinderCallback;  // 建筑查找回调函数
};

#endif  // __BASIC_SOILDER_H__
