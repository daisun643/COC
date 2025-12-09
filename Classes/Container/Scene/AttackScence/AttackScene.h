#ifndef __ATTACK_SCENE_H__
#define __ATTACK_SCENE_H__

#include <string>
#include <vector>

#include "Container/Scene/Basic/BasicScene.h"
#include "Game/Soldier/BasicSoldier.h"
#include "Game/Spell/BasicSpell.h"
#include "Manager/Troop/TroopManager.h"

USING_NS_CC;

/**
 * 进攻场景
 * 玩家可以布置军队和法术进行攻击
 */
class AttackScene : public BasicScene {
 public:
  static Scene* createScene();

  virtual bool init() override;

  CREATE_FUNC(AttackScene);

  /**
   * 重写鼠标事件处理，添加军队和法术布置逻辑
   */
  virtual void onMouseDown(Event* event) override;
  virtual void onMouseMove(Event* event) override;
  virtual void onMouseUp(Event* event) override;

  virtual ~AttackScene();

 private:
  /**
   * 创建底部状态栏
   */
  void createStatusBar();

  /**
   * 创建军队图标
   */
  void createTroopIcons();

  /**
   * 创建法术图标
   */
  void createSpellIcons();

  /**
   * 进入军队布置模式
   */
  void enterTroopPlacementMode(const TroopItem& item);

  /**
   * 进入法术布置模式
   */
  void enterSpellPlacementMode(const SpellItem& item);

  /**
   * 取消布置模式
   */
  void cancelPlacementMode();

  /**
   * 在指定位置放置士兵
   */
  void placeSoldier(const Vec2& worldPos, const TroopItem& item);

  /**
   * 在指定位置施放法术
   */
  void castSpell(const Vec2& worldPos, const SpellItem& item);

  /**
   * 将屏幕坐标转换为地图坐标
   */
  Vec2 screenToMapPosition(const Vec2& screenPos) const;

  /**
   * 检查位置是否有效（在地图范围内）
   */
  bool isValidPlacementPosition(const Vec2& mapPos) const;

  TroopManager* _troopManager;         // 军队管理器实例
  std::vector<TroopItem> _troopItems;  // 军队配置列表
  std::vector<SpellItem> _spellItems;  // 法术配置列表
  Layer* _statusBarLayer;              // 状态栏层
  bool _isPlacingTroop;                // 是否正在布置军队
  bool _isPlacingSpell;                // 是否正在布置法术
  bool _isTroopSelected;             // 是否选中了士兵panel（等待在地图上点击）
  bool _isSpellSelected;             // 是否选中了法术panel（等待在地图上点击）
  TroopItem _currentTroopItem;       // 当前布置的军队项
  SpellItem _currentSpellItem;       // 当前布置的法术项
  Sprite* _placementPreview;         // 布置预览精灵
  LayerColor* _selectedTroopIconBg;  // 当前选中的士兵图标背景（用于高亮）
  LayerColor* _selectedSpellIconBg;  // 当前选中的法术图标背景（用于高亮）
  std::vector<std::pair<LayerColor*, TroopItem>>
      _troopIconBgs;  // 士兵图标背景列表（用于鼠标点击检测）
  std::vector<std::pair<LayerColor*, SpellItem>>
      _spellIconBgs;  // 法术图标背景列表（用于鼠标点击检测）
  std::vector<BasicSoldier*> _placedSoldiers;  // 已布置的士兵列表
  std::vector<BasicSpell*> _activeSpells;      // 活跃的法术列表
};

#endif  // __ATTACK_SCENE_H__
