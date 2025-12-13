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
  WALL        // 城墙
};

/**
 * 建筑基础类
 * 所有建筑的基类，提供通用的建筑功能
 */
class Building : public Sprite {
 public:
  // 拖动相关属性
  bool _isDragging;  // 是否正在拖动
  Vec2 _dragOffset;  // 拖动时的偏移量
  /**
   * 创建建筑
   * @param imagePath 建筑图片路径
   * @param type 建筑类型
   * @param level 建筑等级
   * @param gridCount 建筑占用的网格大小（菱形边长）
   * @param anchorRatioX 锚点X比例
   * @param anchorRatioY 锚点Y比例
   * @param imageScale 图片缩放比例
   */
  bool init(const std::string& imagePath, BuildingType type, const int& level,
            const int& gridCount, const float& anchorRatioX,
            const float& anchorRatioY, const float& imageScale);

  // 建筑属性
  CC_SYNTHESIZE(BuildingType, _buildingType, BuildingType);
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

  /**
   * 获取建筑名称
   */
  std::string getBuildingName() const { return _buildingName; }

  CC_SYNTHESIZE(float, _maxHP, MaxHP);                // 最大生命值
  CC_SYNTHESIZE(float, _currentHP, CurrentHP);        // 当前生命值
  /**
   * 检查建筑是否越界
   */
  bool isOutOfBounds(int gridCount) const;

  /**
   * 显示选中光晕效果
   */
  void showGlow();

  /**
   * 隐藏选中光晕效果
   */
  void hideGlow();

  /**
   * 判断点是否在建筑的菱形区域内
   * @param pos Layer坐标点
   * @return 是否在菱形区域内
   */
  bool inDiamond(const Vec2& pos) const;

  /**
   * 设置放置状态是否有效
   * @param isValid 是否有效
   */
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

  Label* _infoLabel;          // 信息显示标签
  std::string _buildingName;  // 建筑名称
  DrawNode* _glowNode;        // 光晕效果节点
  DrawNode* _anchorNode;      // 锚点标记节点（红点）
  Action* _glowAction;        // 光晕动画动作
  Color4F _glowColor;         // 光晕颜色
  LayerColor* _errorLayer;    // 错误状态遮罩层（红色半透明）

  /**
   * 创建默认建筑外观（如果图片不存在）
   */
  virtual void createDefaultAppearance();

  /**
   * 更新光晕绘制（内部方法）
   */
  void updateGlowDrawing();

  /**
   * 更新生命值条显示（内部方法）
   */
  void updateHPBar();

  DrawNode* _hpBarBackground;  // 生命值条背景
  DrawNode* _hpBarForeground;  // 生命值条前景
};

#endif  // __BUILDING_H__
