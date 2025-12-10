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
  BARRACKS    // 兵营
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

  // 建筑属性
  CC_SYNTHESIZE(BuildingType, _buildingType, BuildingType);
  CC_SYNTHESIZE(std::string, _buildingName, BuildingName); // 需要getter/setter用于升级时查询配置
  CC_SYNTHESIZE(int, _level, Level);
  CC_SYNTHESIZE(int, _maxLevel, MaxLevel);
  CC_SYNTHESIZE(float, _centerX, CenterX);
  CC_SYNTHESIZE(float, _centerY, CenterY);
  CC_SYNTHESIZE(int, _gridCount, GridCount);
  CC_SYNTHESIZE(int, _row, Row);
  CC_SYNTHESIZE(int, _col, Col);
  CC_SYNTHESIZE(float, _anchorRatioX, AnchorRatioX);
  CC_SYNTHESIZE(float, _anchorRatioY, AnchorRatioY);
  
  // 生命值相关
  CC_SYNTHESIZE(float, _maxHealth, MaxHealth);
  CC_SYNTHESIZE(float, _currentHealth, CurrentHealth);

  bool isOutOfBounds(int gridCount) const;
  void showGlow();
  void hideGlow();
  bool inDiamond(const Vec2& pos) const;
  void setPlacementValid(bool isValid);
  
  // 受到伤害
  void takeDamage(float damage);

 protected:
  Building();
  virtual ~Building();

  Label* _infoLabel;
  DrawNode* _glowNode;
  DrawNode* _anchorNode;
  Action* _glowAction;
  Color4F _glowColor;
  LayerColor* _errorLayer;
  DrawNode* _hpBarNode; // 血条节点

  virtual void createDefaultAppearance();
  void updateGlowDrawing();
  void updateHPBar(); // 更新血条显示
};

#endif  // __BUILDING_H__
