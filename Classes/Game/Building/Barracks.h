#ifndef __BARRACKS_H__
#define __BARRACKS_H__

#include "Building.h"

/**
 * 兵种类型
 */
enum class TroopType {
  BARBARIAN, // 野蛮人
  ARCHER,    // 弓箭手
  GIANT,     // 巨人
  GOBLIN     // 哥布林
};

/**
 * 兵营类
 * 用于训练士兵
 */
class Barracks : public Building {
public:
  static Barracks *create(int level = 1);

  virtual bool init(int level);

  // 兵营属性
  CC_SYNTHESIZE(int, _trainingCapacity, TrainingCapacity); // 训练容量
  CC_SYNTHESIZE(int, _currentTroops, CurrentTroops);       // 当前士兵数量

  /**
   * 训练士兵
   * @param troopType 兵种类型
   * @param count 数量
   * @return 是否训练成功
   */
  bool trainTroop(TroopType troopType, int count);

  /**
   * 获取训练时间（秒）
   */
  int getTrainingTime(TroopType troopType) const;

  /**
   * 获取训练成本
   */
  int getTrainingCost(TroopType troopType) const;

  /**
   * 获取兵营信息
   */
  virtual std::string getBuildingInfo() const override;

protected:
  Barracks();
  virtual ~Barracks();

  /**
   * 根据等级初始化属性
   */
  virtual void initBarracksProperties();

  /**
   * 获取兵种名称
   */
  std::string getTroopName(TroopType type) const;
};

#endif // __BARRACKS_H__
