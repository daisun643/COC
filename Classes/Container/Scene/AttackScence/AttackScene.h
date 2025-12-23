#ifndef __ATTACK_SCENE_H__
#define __ATTACK_SCENE_H__

#include <string>
#include <vector>

#include "Container/Scene/Basic/BasicScene.h"
#include "Game/Building/DefenseBuilding.h"
#include "Game/Soldier/BasicSoldier.h"
#include "Game/Spell/BasicSpell.h"
#include "Manager/Record/RecordManager.h"
#include "Manager/Troop/TroopManager.h"
#include "ui/CocosGUI.h"
#include "Game/Building/TrapBuilding.h"

USING_NS_CC;

/**
 * 进攻场景
 * 玩家可以布置军队和法术进行攻击
 */
class AttackScene : public BasicScene {
 public:
  static Scene* createScene(const std::string& levelFilePath = "",
                            const std::string& levelName = "");

  bool init(const std::string& jsonFilePath);
  void setClansWarInfo(const std::string& clans_id, const std::string& map_id);
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

  /**
   * 创建开始进攻和结束进攻按钮
   */
  void createAttackButtons();

  /**
   * 开始进攻（开始倒计时）
   */
  void startAttack();

  /**
   * 结束进攻（保存记录）
   */
  void endAttack();

  /**
   * 展示进攻结果
   * @note 调用_buildingManager.getBuildingResult(int& stars, float& ratio)
   */
  void showResult();
  /**
   * 倒计时更新函数
   */
  void updateCountdown(float dt);

  /**
   * 格式化倒计时时间显示
   */
  std::string formatTime(int seconds) const;

  /**
   * 更新防御建筑攻击（每帧调用）
   * @param delta 时间间隔
   */
  void updateDefenseBuildings(float delta);

  /**
     * 更新陷阱检测（每帧调用）
     * 检测是否有士兵触发陷阱
     */
    void updateTraps(float delta);

  /**
   * 更新记录摘要文件 record/summary.json
   * @param recordName 记录名称
   * @param recordPath 记录文件路径
   * @param timeStr 时间字符串
   */
  void updateRecordSummary(const std::string& recordName,
                           const std::string& recordPath,
                           const std::string& timeStr);

  TroopManager* _troopManager;         // 军队管理器实例
  RecordManager* _recordManager;       // 记录管理器实例
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

  // 进攻控制相关
  cocos2d::ui::Button* _startAttackButton;  // 开始进攻按钮
  cocos2d::ui::Button* _endAttackButton;    // 结束进攻按钮
  cocos2d::ui::Button* _exitButton;         // 退出按钮
  Label* _countdownLabel;                   // 倒计时标签
  bool _isAttackStarted;                    // 是否已开始进攻
  int _countdownSeconds;  // 倒计时剩余秒数（默认180秒，即3分钟）
  static const int ATTACK_DURATION = 180;  // 进攻持续时间（秒）
  bool _isEnd;                             // 是否结束
  bool _isClansWar;                        // 是否是部落战
  /**
   * 退出场景，返回到主场景
   */
  void exitScene();

  std::string _levelFilePath;  // 关卡文件路径
  std::string _levelName;      // 关卡名称，用于保存记录文件
  std::string _clans_id;
  std::string _map_id;
  bool _attackFinished = false;
};

#endif  // __ATTACK_SCENE_H__