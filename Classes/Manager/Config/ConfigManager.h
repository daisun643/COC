#ifndef __CONFIG_MANAGER_H__
#define __CONFIG_MANAGER_H__

#include <map>
#include <string>

#include "Game/Soldier/BasicSoldier.h"
#include "Game/Spell/BasicSpell.h"
#include "cocos2d.h"
#include "json/document.h"
USING_NS_CC;

/**
 * 配置管理器
 * 负责读取和解析JSON配置文件
 */
class ConfigManager {
 public:
  static const ConfigManager* getInstance();

  /**
   * 初始化配置管理器，加载所有配置文件
   * @return 是否初始化成功
   */
  bool init();

  /**
   * 获取背景配置
   */
  struct BackgroundConfig {
    int designWidth;
    int designHeight;
    int smallWidth;
    int smallHeight;
    int mediumWidth;
    int mediumHeight;
    int largeWidth;
    int largeHeight;
  };

  /**
   * 获取常量配置
   */
  struct ConstantConfig {
    int gridSize;                // 网格边长
    std::string grassImagePath;  // 草地图片路径
    float grassWidth;            // 草地图片宽度
    float grassHeight;           // 草地图片高度
    float deltaX;                // 网格间距X
    float deltaY;                // 网格间距Y
    float glowDelay;             // 光晕明暗交替间隔
  };

  /**
   * 获取建筑配置(扩展后的全能配置结构体)
   */
  struct BuildingConfig {
    // 基础属性
    std::string type;
    std::string image;
    float anchorRatioX = 0.5f;
    float anchorRatioY = 0.5f;
    int gridCount = 1;
    int maxLevel = 10;

    // --- 修改点：添加 defaultLevel ---
    int defaultLevel = 1;

    float imageScale = 1.0f;

    // --- 修改点：确保有 health ---
    int health = 0;

    // 防御属性
    float attackRange = 0.0f;
    int damage = 0;
    float attackSpeed = 0.0f;

    // 资源与储存属性
    int productionRate = 0;
    int capacity = 0;
    std::string resourceType;

    // 兵营属性
    int queueSize = 0;
  };

  /**
   * 获取士兵配置
   */
  struct SoldierConfig {
    std::string panelImage;
    std::string moveImage;
    float attack;           // 攻击力
    float health;           // 生命值
    float moveSpeed;        // 移动速度
    float attackSpeed;      // 攻击速度
    float attackRange;      // 攻击范围
    AttackType attackType;  // 攻击类型（字符串："Any", "Defense", "Resource",
                            // "TownHall"）
    SoldierCategory soldierCategory;  // 士兵类型（字符串："LAND", "AIR"）
  };

  /**
   * 获取法术配置
   */
  struct SpellConfig {
    SpellCategory category;  // 效果类型（"INSTANT" 或 "DURATION"）
    float duration;          // 持续时间（秒）
    float radius;            // 作用范围（像素）
    float amount;            // 作用总量
    float ratio;             // 倍率
    std::string panelImage;  // 法术图片路径
  };

  /**
   * 获取背景配置
   */
  BackgroundConfig getBackgroundConfig() const { return _backgroundConfig; }

  /**
   * 获取常量配置
   */
  ConstantConfig getConstantConfig() const { return _constantConfig; }

  /**
   * 获取指定名称建筑的配置
   */
  BuildingConfig getBuildingConfig(const std::string& name,
                                   int level = 1) const;

  /**
   * 获取士兵配置
   * @param soldierType 士兵类型（字符串："barbarian", "archer", "giant",
   * "goblin"）
   * @param level 士兵等级
   * @return 士兵配置，如果不存在则返回默认配置
   */
  SoldierConfig getSoldierConfig(const std::string& soldierType,
                                 int level) const;

  /**
   * 获取法术配置
   * @param spellType 法术类型（字符串："Heal", "Lightning", "Rage"）
   * @return 法术配置，如果不存在则返回默认配置
   */
  SpellConfig getSpellConfig(const std::string& spellType) const;

 private:
  ConfigManager();
  ~ConfigManager();

  /**
   * 加载背景配置文件
   */
  bool loadBackgroundConfig();

  /**
   * 加载常量配置文件
   */
  bool loadConstantConfig();

  /**
   * 加载建筑配置文件
   */
  bool loadBuildingConfig();

  /**
   * 加载士兵配置文件
   */
  bool loadSoldierConfig();

  /**
   * 加载法术配置文件
   */
  bool loadSpellConfig();

  /**
   * 从文件读取JSON文档
   */
  bool loadJsonFromFile(const std::string& filePath, rapidjson::Document& doc);

  BackgroundConfig _backgroundConfig;
  ConstantConfig _constantConfig;
  BuildingConfig _townHallConfig;
  rapidjson::Document _soldierConfigDoc;  // 士兵配置JSON文档
  rapidjson::Document _spellConfigDoc;    // 法术配置JSON文档

  static ConfigManager* _instance;

  // 存储所有建筑配置的字典
  std::map<std::string, std::map<int, BuildingConfig>> _buildingConfigs;
};

#endif  // __CONFIG_MANAGER_H__
