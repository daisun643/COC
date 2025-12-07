#ifndef __TROOP_MANAGER_H__
#define __TROOP_MANAGER_H__

#include <string>
#include <vector>

#include "cocos2d.h"

USING_NS_CC;

/**
 * 军队配置项
 */
struct TroopItem {
  std::string soldierType;  // 士兵类型（"barbarian", "archer"等）
  int level;                // 等级
  int count;                // 数量
  std::string panelImage;   // 图标路径
};

/**
 * 法术配置项
 */
struct SpellItem {
  std::string spellType;   // 法术类型（"Heal", "Lightning", "Rage"）
  int count;               // 数量
  std::string panelImage;  // 图标路径
};

/**
 * 军队管理器
 * 负责加载和管理玩家携带的军队和法术配置
 */
class TroopManager {
 public:
  /**
   * 构造函数
   */
  TroopManager();

  /**
   * 析构函数
   */
  ~TroopManager();

  /**
   * 初始化管理器，加载配置文件
   * @return 是否初始化成功
   */
  bool init(const std::string& configPath = "config/troop.json");

  /**
   * 加载军队和法术配置
   * @param configPath 配置文件路径
   * @return 是否加载成功
   */
  bool loadTroopConfig(const std::string& configPath);

  /**
   * 获取军队配置列表
   * @return 军队配置列表
   */
  const std::vector<TroopItem>& getTroopItems() const { return _troopItems; }

  /**
   * 获取法术配置列表
   * @return 法术配置列表
   */
  const std::vector<SpellItem>& getSpellItems() const { return _spellItems; }

  /**
   * 减少指定军队的数量
   * @param soldierType 士兵类型
   * @param level 等级
   * @return 是否成功减少（如果数量为0则返回false）
   */
  bool consumeTroop(const std::string& soldierType, int level);

  /**
   * 减少指定法术的数量
   * @param spellType 法术类型
   * @return 是否成功减少（如果数量为0则返回false）
   */
  bool consumeSpell(const std::string& spellType);

 private:
  std::vector<TroopItem> _troopItems;  // 军队配置列表
  std::vector<SpellItem> _spellItems;  // 法术配置列表
};

#endif  // __TROOP_MANAGER_H__
