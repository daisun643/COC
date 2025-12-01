#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include <vector>

#include "Game/Building/TownHall.h"
#include "Manager/Building/BuildingManager.h"
#include "Manager/PlayerManager.h"
#include "UI/MainUILayer.h"
#include "UI/ShopLayer.h"
#include "Utils/GridUtils.h"
#include "cocos2d.h"

USING_NS_CC;

/**
 * 游戏主场景
 * 展示村庄和所有建筑
 */
class GameScene : public Scene {
 public:
  static Scene* createScene();

  virtual bool init() override;

  CREATE_FUNC(GameScene);

 private:
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
  MainUILayer* _uiLayer;              // UI 层
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
  void onMouseScroll(Event* event);

  /**
   * 处理鼠标拖拽事件
   */
  void onMouseMove(Event* event);
  void onMouseDown(Event* event);
  void onMouseUp(Event* event);

  /**
   * 显示弹窗对话框
   */
  void showPopupDialog(const std::string& title, const std::string& message);

  /**
   * 计算地图原点p00
   */
  void calculateP00();

  /**
   * 打开商店界面
   */
  void openShop();

  bool isShopOpen() const;

  /**
   * 构建商店商品列表
   */
  std::vector<ShopItem> buildShopCatalog() const;

  /**
   * 处理购买逻辑
   */
  bool handleShopPurchase(const ShopItem& item);

  /**
   * 进入建筑放置模式
   */
  void enterPlacementMode(const ShopItem& item);

  /**
   * 取消建筑放置
   */
  void cancelPlacementMode(bool refundResources = false);

  /**
   * 根据商品创建建筑实例
   */
  Building* createBuildingForItem(const ShopItem& item);

  /**
   * 放置提示
   */
  void showPlacementHint(const std::string& text);

  /**
   * 更新放置预览位置
   */
  void updatePlacementPreview(const cocos2d::Vec2& worldPos);

  // 析构函数需要清理BuildingManager
  virtual ~GameScene();

  bool _isPlacingBuilding;                // 是否处于放置模式
  Building* _placementBuilding;           // 正在放置的建筑
  ShopItem _placementItem;                // 当前放置商品
  cocos2d::Label* _placementHintLabel;    // 放置提示文本
  bool _isPlacementMouseDown;             // 放置模式左键是否按下
  bool _placementDraggingMap;             // 放置模式是否正在拖动地图
  cocos2d::Vec2 _placementMouseDownPos;   // 放置模式按下位置
  cocos2d::Vec2 _placementLastMousePos;   // 放置模式最后一次鼠标位置
  bool _placementPreviewValid;            // 当前预览是否有效
  int _placementPreviewRow;               // 预览所在行
  int _placementPreviewCol;               // 预览所在列
  cocos2d::Vec2 _placementPreviewAnchor;  // 预览锚点位置
  cocos2d::Vec2 _currentMousePos;         // 当前鼠标位置
  bool _ignoreNextMouseUp;                // 是否忽略下一次鼠标抬起事件
};

#endif  // __GAME_SCENE_H__
