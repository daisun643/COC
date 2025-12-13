#ifndef __RECORD_MANAGER_H__
#define __RECORD_MANAGER_H__

#include <chrono>
#include <string>
#include <vector>

#include "cocos2d.h"

USING_NS_CC;

/**
 * 记录项：记录何时何地布置了法术/兵种
 */
struct PlacementRecord {
  std::string type;      // 类型：troop（兵种）或 spell（法术）
  std::string category;  // 类别：如 "barbarian", "Heal" 等
  int level;             // 等级（仅兵种有）
  float x;               // 地图坐标X
  float y;               // 地图坐标Y
  int timestamp;         // 时间戳（从开始进攻算起的秒数）
};

/**
 * 记录管理器
 * 用于记录进攻过程中的所有布置操作
 */
class RecordManager {
 public:
  RecordManager();
  ~RecordManager();

  /**
   * 初始化记录管理器
   */
  bool init();

  /**
   * 开始新的进攻记录
   */
  void startAttack();

  /**
   * 记录兵种布置
   */
  void recordTroopPlacement(const std::string& category, int level, float x,
                            float y, int timestamp);

  /**
   * 记录法术布置
   */
  void recordSpellPlacement(const std::string& category, float x, float y,
                            int timestamp);

  /**
   * 结束进攻并保存记录到JSON文件
   * @param filePath JSON文件路径
   * @return 是否保存成功
   */
  bool endAttackAndSave(const std::string& filePath);

  /**
   * 获取当前时间戳（从开始进攻算起的秒数）
   */
  int getCurrentTimestamp() const;

  /**
   * 清空所有记录
   */
  void clear();

 private:
  std::vector<PlacementRecord> _records;                   // 记录列表
  std::chrono::steady_clock::time_point _attackStartTime;  // 进攻开始时间
  bool _isRecording;                                       // 是否正在记录
};

#endif  // __RECORD_MANAGER_H__
