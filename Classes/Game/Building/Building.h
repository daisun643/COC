#ifndef __BUILDING_H__
#define __BUILDING_H__

#include "cocos2d.h"

USING_NS_CC;

/**
 * 建筑类型枚举
 */
enum class BuildingType {
  TOWN_HALL,  // 大本营
  DEFENSE,    // 防御建筑
  RESOURCE,   // 资源建筑
  STORAGE,    // 储存建筑
  BARRACKS,   // 兵营
  WALL,        // 城墙
  TRAP        // 陷阱
};

/**
 * 建筑基础类
 * 所有建筑的基类，提供通用的建筑功能
 */
class Building : public Sprite {
 public:
  // 定义建筑状态枚举
  enum class State {
      NORMAL,     // 正常状态
      UPGRADING,  // 升级施工中
      MOVING      // 移动中（预留）
  };
  // 拖动相关属性
  bool _isDragging;  // 是否正在拖动
  Vec2 _dragOffset;  // 拖动时的偏移量

  /**
   * 初始化建筑
   * 注意：参数略有简化，因为更多属性将从ConfigManager动态获取
   */
  bool init(const std::string& imagePath, BuildingType type, const int& level,
            const int& gridCount, const float& anchorRatioX,
            const float& anchorRatioY, const float& imageScale);

  /**
   * 升级建筑
   * 增加等级，刷新外观和通用属性，子类应重写此方法以更新特有属性
   */
  virtual void upgrade();

  // 状态查询与控制方法
  bool isUpgrading() const { return _state == State::UPGRADING; }
  void cancelUpgrade();             // 取消升级
  void finishUpgradeImmediately();  // 立即完成（消耗宝石）
  
  // 重写 update 方法以处理倒计时
  virtual void update(float dt) override;

  // 建筑属性
  CC_SYNTHESIZE(BuildingType, _buildingType, BuildingType);
  CC_SYNTHESIZE(std::string, _buildingName,
                BuildingName);  // 需要getter/setter用于升级时查询配置
  CC_SYNTHESIZE(int, _level, Level);
  CC_SYNTHESIZE(int, _maxLevel, MaxLevel);
  CC_SYNTHESIZE(float, _centerX, CenterX);
  CC_SYNTHESIZE(float, _centerY, CenterY);
  CC_SYNTHESIZE(int, _gridCount, GridCount);  // 建筑占用的网格数量
  // 这里 _row 和 _col 的类型由 int -> float 进行适配
  CC_SYNTHESIZE(float, _row, Row);                    // 坐标编码：行
  CC_SYNTHESIZE(float, _col, Col);                    // 坐标编码：列
  CC_SYNTHESIZE(float, _anchorRatioX, AnchorRatioX);  // 建筑宽度比例
  CC_SYNTHESIZE(float, _anchorRatioY, AnchorRatioY);  // 建筑高度比例

  CC_SYNTHESIZE(float, _maxHP, MaxHP);          // 最大生命值
  CC_SYNTHESIZE(float, _currentHP, CurrentHP);  // 当前生命值
  /**
   * 检查建筑是否越界
   */
  bool isOutOfBounds(int gridCount) const;
  void showGlow();
  void hideGlow();
  bool inDiamond(const Vec2& pos) const;
  void setPlacementValid(bool isValid);

  /**
   * 设置当前生命值并更新血条显示
   * @param hp 新的生命值
   */
  void setCurrentHPAndUpdate(float hp);

  /**
   * 设置血条是否可见
   * @param visible 是否可见
   */
  void setHealthBarVisible(bool visible);

  /**
   * 受到伤害
   * @param damage 伤害值
   */
  void takeDamage(float damage);

  /**
   * 是否存活（HP > 0）
   * @return 是否存活
   */
  bool isAlive() const;

 protected:
  Building();
  virtual ~Building();

  Label* _infoLabel;
  DrawNode* _glowNode;
  DrawNode* _anchorNode;
  Action* _glowAction;
  Color4F _glowColor;

  // 升级施工相关成员变量
  State _state;                 // 当前状态
  float _upgradeTotalTime;      // 升级总时间
  float _upgradeTimer;          // 当前剩余时间

  // UI 组件
  ProgressTimer* _progressBar;  // 进度条
  Sprite* _progressBarBg;       // 进度条背景
  Label* _timeLabel;            // 倒计时文字

  // 内部辅助方法
  void createUpgradeUI();       // 创建升级进度条UI
  void removeUpgradeUI();       // 移除升级进度条UI
  virtual void completeUpgrade();       // 实际执行升级完成逻辑（改变外观、属性）

  virtual void createDefaultAppearance();
  void updateGlowDrawing();

  /**
   * 更新生命值条显示（内部方法）
   */
  void updateHPBar();

  DrawNode* _hpBarBackground;  // 生命值条背景
  DrawNode* _hpBarForeground;  // 生命值条前景
};

#endif  // __BUILDING_H__
