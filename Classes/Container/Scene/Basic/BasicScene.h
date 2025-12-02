#ifndef __BASIC_SCENE_H__
#define __BASIC_SCENE_H__

#include "Game/Building/TownHall.h"
#include "Manager/Building/BuildingManager.h"
#include "Utils/GridUtils.h"
#include "cocos2d.h"

USING_NS_CC;

/**
 * 游戏主场景
 * 展示村庄和所有建筑
 */
class BasicScene : public Scene {
 public:
  static Scene* createScene();

  virtual bool init() override;

  CREATE_FUNC(BasicScene);

 protected:
  Layer* _mapLayer;             // 地图容器层，用于整体移动和缩放
  float _currentScale;          // 当前缩放比例
  Vec2 _lastMousePos;           // 上次鼠标位置，用于拖拽
  Vec2 _mouseDownPos;           // 鼠标按下时的位置，用于判断是否开始拖动
  bool _isDragging;             // 是否正在拖拽地图
  bool _isMouseDown;            // 鼠标左键是否被按住
  Building* _selectedBuilding;  // 当前选中的建筑（点击但未拖动）
  Building* _draggingBuilding;  // 正在拖动的建筑
  Vec2 _buildingStartPos;       // 建筑开始拖动时的位置

  // 地图参数（从initGrassBackground中提取）
  Vec2 _p00;      // 地图原点p[0][0]
  float _deltaX;  // X方向间距
  float _deltaY;  // Y方向间距
  int _gridSize;  // 网格大小（44）

  BuildingManager* _buildingManager;  // 建筑管理器
  /**
   * 初始化大本营
   */
  void initTownHall();

  /**
   * 初始化草地背景，创建44x44网格的菱形密铺
   */
  void initGrassBackground();

  /**
   * 初始化鼠标事件监听器（滚轮缩放和拖拽移动）
   */
  void initMouseEventListeners();

  /**
   * 处理鼠标滚轮事件
   */
  virtual void onMouseScroll(Event* event);

  /**
   * 处理鼠标拖拽事件
   */
  virtual void onMouseMove(Event* event);
  virtual void onMouseDown(Event* event);
  virtual void onMouseUp(Event* event);

  /**
   * 显示弹窗对话框
   */
  void showPopupDialog(const std::string& title, const std::string& message);

  /**
   * 计算地图原点p00
   */
  void calculateP00();

  /**
   * 检查建筑放置是否有效（可被子类重写以添加额外规则，如重叠检测）
   */
  virtual bool isPlacementValid(Building* building) const;

  // 析构函数需要清理BuildingManager
  virtual ~BasicScene();
};

#endif  // __BASIC_SCENE_H__