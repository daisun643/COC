#ifndef __BUILDING_H__
#define __BUILDING_H__

#include "cocos2d.h"

USING_NS_CC;

/**
 * 建筑类型枚举
 */
enum class BuildingType {
  TOWN_HALL, // 大本营
  DEFENSE,   // 防御建筑
  RESOURCE,  // 资源建筑
  STORAGE,   // 储存建筑
  BARRACKS   // 兵营
};

/**
 * 建筑基础类
 * 所有建筑的基类，提供通用的建筑功能
 */
class Building : public Sprite {
public:
  /**
   * 创建建筑
   * @param imagePath 建筑图片路径
   * @param type 建筑类型
   * @param level 建筑等级（默认为1）
   */
  static Building *create(const std::string &imagePath, BuildingType type,
                          int level = 1);

  /**
   * 初始化建筑
   */
  virtual bool init(const std::string &imagePath, BuildingType type, int level);

  // 建筑属性
  CC_SYNTHESIZE(BuildingType, _buildingType, BuildingType);
  CC_SYNTHESIZE(int, _level, Level);
  CC_SYNTHESIZE(int, _maxLevel, MaxLevel);
  
  // 新增属性：中心坐标、宽度、坐标编码
  CC_SYNTHESIZE(float, _centerX, CenterX);
  CC_SYNTHESIZE(float, _centerY, CenterY);
  CC_SYNTHESIZE(int, _width, Width);  // 建筑宽度（菱形边长）
  CC_SYNTHESIZE(int, _row, Row);      // 坐标编码：行
  CC_SYNTHESIZE(int, _col, Col);      // 坐标编码：列

  // 建筑名称（避免与 Node::getName() 冲突）
  inline void setBuildingName(const std::string &name) { _buildingName = name; }
  inline const std::string &getBuildingName() const { return _buildingName; }
  
  /**
   * 设置建筑的中心坐标和坐标编码
   */
  void setCenterPosition(float x, float y, int row, int col);
  
  /**
   * 检查建筑是否越界
   * @return true表示越界，false表示未越界
   */
  bool isOutOfBounds(int gridSize) const;
  
  /**
   * 获取建筑的四个角的坐标编码
   */
  void getCornerCoordinates(int &topRow, int &topCol, 
                            int &rightRow, int &rightCol,
                            int &bottomRow, int &bottomCol,
                            int &leftRow, int &leftCol) const;

  /**
   * 升级建筑
   * @return 是否升级成功
   */
  virtual bool upgrade();

  /**
   * 获取升级所需资源
   * @return 升级所需资源（可以根据等级计算）
   */
  virtual int getUpgradeCost() const;

  /**
   * 获取建筑信息
   */
  virtual std::string getBuildingInfo() const;

  /**
   * 显示建筑信息标签
   */
  virtual void showInfo();

  /**
   * 隐藏建筑信息标签
   */
  virtual void hideInfo();
  
  /**
   * 显示选中光晕效果
   */
  void showGlow();
  
  /**
   * 隐藏选中光晕效果
   */
  void hideGlow();

  // 拖动相关属性（需要被GameScene访问）
  bool _isDragging;          // 是否正在拖动
  Vec2 _dragOffset;          // 拖动时的偏移量

protected:
  Building();
  virtual ~Building();

  Label *_infoLabel;         // 信息显示标签
  std::string _buildingName; // 建筑名称
  DrawNode *_glowNode;       // 光晕效果节点
  DrawNode *_anchorNode;     // 锚点标记节点（红点）

  /**
   * 创建默认建筑外观（如果图片不存在）
   */
  virtual void createDefaultAppearance();
};

#endif // __BUILDING_H__
