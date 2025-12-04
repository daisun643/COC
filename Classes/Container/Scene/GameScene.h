#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include <vector>

#include "Container/Layer/MainUILayer.h"
#include "Container/Layer/ShopLayer.h"
#include "Container/Scene/Basic/BasicScene.h"
#include "Manager/PlayerManager.h"

USING_NS_CC;

/**
 * 游戏主场景
 * 展示村庄和所有建筑
 */
class GameScene : public BasicScene {
 public:
  static Scene* createScene();

  virtual bool init() override;

  CREATE_FUNC(GameScene);

 private:
  MainUILayer* _uiLayer;  // UI 层

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

  /**
   * 检查建筑是否与其他建筑重叠
   */
  bool checkBuildingOverlap(const Building* building) const;

  /**
   * 重写基类的放置有效性检查
   */
  virtual bool isPlacementValid(Building* building) const override;

  /**
   * 尝试吸附到网格（辅助函数）
   * @param building 目标建筑
   * @param mapPos 地图层坐标
   * @return 是否成功吸附
   */
  bool snapToGrid(Building* building, const Vec2& mapPos);

  /**
   * 重写父类的鼠标事件处理方法，添加放置模式和商店相关逻辑
   */
  void onMouseScroll(Event* event) override;
  void onMouseDown(Event* event) override;
  void onMouseMove(Event* event) override;
  void onMouseUp(Event* event) override;

  // 析构函数（清理放置模式相关资源，BuildingManager由父类清理）
  virtual ~GameScene();

  // --- 放置模式变量 ---
  bool _isPlacingBuilding;                // 是否处于放置模式（新建）
  Building* _placementBuilding;           // 正在放置的新建筑
  ShopItem _placementItem;                // 当前放置商品
  cocos2d::Label* _placementHintLabel;    // 放置提示文本
  bool _isPlacementMouseDown;             // 放置模式左键是否按下
  bool _placementPreviewValid;            // 当前预览是否有效
  int _placementPreviewRow;               // 预览所在行
  int _placementPreviewCol;               // 预览所在列
  cocos2d::Vec2 _placementPreviewAnchor;  // 预览锚点位置
  
  // --- 拖拽已有建筑变量 ---
  Building* _draggingBuilding;            // 当前正在拖动的已有建筑
  cocos2d::Vec2 _dragOffset;              // 鼠标点击位置相对于建筑锚点的偏移
  bool _isDraggingExisting;               // 标记是否正在拖动已有建筑
  cocos2d::Vec2 _originalPos;             // 拖动前的原始位置（用于取消或无效时回滚）
  int _originalRow;                       // 拖动前的原始行
  int _originalCol;                       // 拖动前的原始列

  // --- 通用鼠标状态 ---
  cocos2d::Vec2 _currentMousePos;         // 当前鼠标位置
  bool _ignoreNextMouseUp;                // 是否忽略下一次鼠标抬起事件
};

#endif  // __GAME_SCENE_H__
