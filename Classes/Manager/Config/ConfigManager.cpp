#include "ConfigManager.h"

#include <fstream>
#include <sstream>

USING_NS_CC;

ConfigManager* ConfigManager::_instance = nullptr;

const ConfigManager* ConfigManager::getInstance() {
  if (_instance == nullptr) {
    _instance = new (std::nothrow) ConfigManager();
    if (_instance && _instance->init()) {
      // 初始化成功
    } else {
      CC_SAFE_DELETE(_instance);
      _instance = nullptr;
    }
  }
  return _instance;
}

ConfigManager::ConfigManager() {
  // 初始化默认值
  _backgroundConfig.designWidth = 1280;
  _backgroundConfig.designHeight = 720;
  _backgroundConfig.smallWidth = 480;
  _backgroundConfig.smallHeight = 320;
  _backgroundConfig.mediumWidth = 1024;
  _backgroundConfig.mediumHeight = 768;
  _backgroundConfig.largeWidth = 2048;
  _backgroundConfig.largeHeight = 1536;

  _constantConfig.gridSize = 44;
  _constantConfig.grassImagePath = "images/backgrond/grass.png";
  _constantConfig.grassWidth = 158.0f;
  _constantConfig.grassHeight = 120.0f;
  _constantConfig.deltaX = 76.0f;
  _constantConfig.deltaY = 60.0f;
  _constantConfig.glowDelay = 0.5f;

  _townHallConfig.image = "images/buildings/TownHall.png";
  _townHallConfig.anchorRatioX = 0.0f;  // 默认左侧中点
  _townHallConfig.anchorRatioY = 0.5f;
  _townHallConfig.gridCount = 4;
  _townHallConfig.defaultLevel = 1;
  _townHallConfig.maxLevel = 10;
}

ConfigManager::~ConfigManager() {}

bool ConfigManager::init() {
  // 加载所有配置文件
  if (!loadBackgroundConfig()) {
    CCLOG("Failed to load background config");
    return false;
  }

  if (!loadConstantConfig()) {
    CCLOG("Failed to load constant config");
    return false;
  }

  if (!loadBuildingConfig()) {
    CCLOG("Failed to load building config");
    return false;
  }

  if (!loadSoldierConfig()) {
    CCLOG("Failed to load soldier config");
    return false;
  }

  if (!loadSpellConfig()) {
    CCLOG("Failed to load spell config");
    return false;
  }

  return true;
}

bool ConfigManager::loadJsonFromFile(const std::string& filePath,
                                     rapidjson::Document& doc) {
  // 使用cocos2d-x的FileUtils读取文件
  FileUtils* fileUtils = FileUtils::getInstance();
  std::string fullPath = fileUtils->fullPathForFilename(filePath);

  if (fullPath.empty()) {
    CCLOG("Config file not found: %s", filePath.c_str());
    return false;
  }

  // 读取文件内容
  std::string content = fileUtils->getStringFromFile(fullPath);
  if (content.empty()) {
    CCLOG("Config file is empty: %s", filePath.c_str());
    return false;
  }

  // 解析JSON
  doc.Parse(content.c_str());
  if (doc.HasParseError()) {
    CCLOG("JSON parse error in file %s at offset %u (error code: %d)",
          filePath.c_str(), (unsigned)doc.GetErrorOffset(),
          (int)doc.GetParseError());
    return false;
  }

  return true;
}

bool ConfigManager::loadBackgroundConfig() {
  rapidjson::Document doc;
  if (!loadJsonFromFile("config/background.json", doc)) {
    return false;
  }

  // 解析设计分辨率
  if (doc.HasMember("ResolutionSize") && doc["ResolutionSize"].IsObject()) {
    const rapidjson::Value& resSize = doc["ResolutionSize"];
    if (resSize.HasMember("width") && resSize["width"].IsInt()) {
      _backgroundConfig.designWidth = resSize["width"].GetInt();
    }
    if (resSize.HasMember("height") && resSize["height"].IsInt()) {
      _backgroundConfig.designHeight = resSize["height"].GetInt();
    }
  }

  // 解析分辨率策略
  if (doc.HasMember("ResolutionPolicies") &&
      doc["ResolutionPolicies"].IsObject()) {
    const rapidjson::Value& policies = doc["ResolutionPolicies"];

    // Small resolution
    if (policies.HasMember("small") && policies["small"].IsObject()) {
      const rapidjson::Value& smallRes = policies["small"];
      if (smallRes.HasMember("width") && smallRes["width"].IsInt()) {
        _backgroundConfig.smallWidth = smallRes["width"].GetInt();
      }
      if (smallRes.HasMember("height") && smallRes["height"].IsInt()) {
        _backgroundConfig.smallHeight = smallRes["height"].GetInt();
      }
    }

    // Medium resolution
    if (policies.HasMember("medium") && policies["medium"].IsObject()) {
      const rapidjson::Value& mediumRes = policies["medium"];
      if (mediumRes.HasMember("width") && mediumRes["width"].IsInt()) {
        _backgroundConfig.mediumWidth = mediumRes["width"].GetInt();
      }
      if (mediumRes.HasMember("height") && mediumRes["height"].IsInt()) {
        _backgroundConfig.mediumHeight = mediumRes["height"].GetInt();
      }
    }

    // Large resolution
    if (policies.HasMember("large") && policies["large"].IsObject()) {
      const rapidjson::Value& largeRes = policies["large"];
      if (largeRes.HasMember("width") && largeRes["width"].IsInt()) {
        _backgroundConfig.largeWidth = largeRes["width"].GetInt();
      }
      if (largeRes.HasMember("height") && largeRes["height"].IsInt()) {
        _backgroundConfig.largeHeight = largeRes["height"].GetInt();
      }
    }
  }

  return true;
}

bool ConfigManager::loadConstantConfig() {
  rapidjson::Document doc;
  if (!loadJsonFromFile("config/constant.json", doc)) {
    return false;
  }

  // 解析基础网格配置
  if (doc.HasMember("GridSize") && doc["GridSize"].IsInt()) {
    _constantConfig.gridSize = doc["GridSize"].GetInt();
  }

  // 解析草地图片配置
  if (doc.HasMember("GrassImage") && doc["GrassImage"].IsObject()) {
    const rapidjson::Value& grass = doc["GrassImage"];
    if (grass.HasMember("path") && grass["path"].IsString()) {
      _constantConfig.grassImagePath = grass["path"].GetString();
    }
    if (grass.HasMember("width") && grass["width"].IsNumber()) {
      _constantConfig.grassWidth = grass["width"].GetFloat();
    }
    if (grass.HasMember("height") && grass["height"].IsNumber()) {
      _constantConfig.grassHeight = grass["height"].GetFloat();
    }
  }

  // 解析网格间距配置
  if (doc.HasMember("GridDelta") && doc["GridDelta"].IsObject()) {
    const rapidjson::Value& delta = doc["GridDelta"];
    if (delta.HasMember("deltaX") && delta["deltaX"].IsNumber()) {
      _constantConfig.deltaX = delta["deltaX"].GetFloat();
    }
    if (delta.HasMember("deltaY") && delta["deltaY"].IsNumber()) {
      _constantConfig.deltaY = delta["deltaY"].GetFloat();
    }
  }

  // 解析GlowDelay
  if (doc.HasMember("GlowDelay") && doc["GlowDelay"].IsNumber()) {
    _constantConfig.glowDelay = doc["GlowDelay"].GetFloat();
  }

  return true;
}

bool ConfigManager::loadBuildingConfig() {
  rapidjson::Document doc;
  if (!loadJsonFromFile("config/building.json", doc)) return false;

  _buildingConfigs.clear();

  for (auto& m : doc.GetObject()) {
    std::string name = m.name.GetString();
    const rapidjson::Value& val = m.value;

    if (val.IsObject()) {
      BuildingConfig baseConfig;
      // 读取通用属性
      if (val.HasMember("type")) baseConfig.type = val["type"].GetString();
      if (val.HasMember("GridSize"))
        baseConfig.gridCount = val["GridSize"].GetInt();
      if (val.HasMember("imageScale"))
        baseConfig.imageScale = val["imageScale"].GetFloat();
      if (val.HasMember("maxLevel"))
        baseConfig.maxLevel = val["maxLevel"].GetInt();
      if (val.HasMember("resourceType"))
        baseConfig.resourceType = val["resourceType"].GetString();

      if (val.HasMember("AnchorRatio") && val["AnchorRatio"].IsArray()) {
        baseConfig.anchorRatioX = val["AnchorRatio"][0].GetFloat();
        baseConfig.anchorRatioY = val["AnchorRatio"][1].GetFloat();
      }

      // 遍历解析 1 到 maxLevel
      for (int lvl = 1; lvl <= baseConfig.maxLevel; lvl++) {
        std::string lvlKey = std::to_string(lvl);
        BuildingConfig levelConfig = baseConfig;  // 复制基础配置

        if (val.HasMember(lvlKey.c_str())) {
          const rapidjson::Value& lvlVal = val[lvlKey.c_str()];

          if (lvlVal.HasMember("image"))
            levelConfig.image = lvlVal["image"].GetString();
          if (lvlVal.HasMember("health"))
            levelConfig.maxHP = lvlVal["health"].GetFloat();
          if (lvlVal.HasMember("maxHP"))
            levelConfig.maxHP = lvlVal["maxHP"].GetFloat();

          // Defense
          if (lvlVal.HasMember("damage"))
            levelConfig.damage = lvlVal["damage"].GetInt();
          if (lvlVal.HasMember("attackRange"))
            levelConfig.attackRange = lvlVal["attackRange"].GetFloat();
          if (lvlVal.HasMember("attackSpeed"))
            levelConfig.attackSpeed = lvlVal["attackSpeed"].GetFloat();

          // Resource & Storage
          if (lvlVal.HasMember("productionRate"))
            levelConfig.productionRate = lvlVal["productionRate"].GetInt();
          if (lvlVal.HasMember("capacity"))
            levelConfig.capacity = lvlVal["capacity"].GetInt();

          // Barracks
          if (lvlVal.HasMember("queueSize"))
            levelConfig.queueSize = lvlVal["queueSize"].GetInt();
        }
        _buildingConfigs[name][lvl] = levelConfig;
      }
    }
  }
  return true;
}

// 实现获取接口
ConfigManager::BuildingConfig ConfigManager::getBuildingConfig(
    const std::string& name, int level) const {
  auto nameIt = _buildingConfigs.find(name);
  if (nameIt != _buildingConfigs.end()) {
    auto levelIt = nameIt->second.find(level);
    if (levelIt != nameIt->second.end()) {
      return levelIt->second;
    }
    // 降级处理：如果没有该等级配置，尝试返回等级1
    if (!nameIt->second.empty()) {
      return nameIt->second.begin()->second;
    }
  }
  CCLOG("Warning: Config for building '%s' not found.", name.c_str());
  return BuildingConfig();
}

bool ConfigManager::loadSoldierConfig() {
  // 直接加载到成员变量
  if (!loadJsonFromFile("config/soilder.json", _soldierConfigDoc)) {
    return false;
  }

  return true;
}

ConfigManager::SoldierConfig ConfigManager::getSoldierConfig(
    const std::string& soldierType, int level) const {
  SoldierConfig config;

  // 检查文档是否有效
  if (_soldierConfigDoc.IsNull() || !_soldierConfigDoc.IsObject()) {
    CCLOG("Soldier config document is not loaded");
    return config;
  }

  // 获取士兵类型配置
  if (!_soldierConfigDoc.HasMember(soldierType.c_str()) ||
      !_soldierConfigDoc[soldierType.c_str()].IsObject()) {
    CCLOG("Soldier type '%s' not found in config", soldierType.c_str());
    return config;
  }

  const rapidjson::Value& soldierTypeConfig =
      _soldierConfigDoc[soldierType.c_str()];
  // 攻击偏好
  if (soldierTypeConfig.HasMember("AttackType") &&
      soldierTypeConfig["AttackType"].IsString()) {
    std::string attackTypeStr = soldierTypeConfig["AttackType"].GetString();
    if (attackTypeStr == "Any") {
      config.attackType = AttackType::ANY;
    } else if (attackTypeStr == "Defense") {
      config.attackType = AttackType::DEFENSE;
    } else if (attackTypeStr == "Resource") {
      config.attackType = AttackType::RESOURCE;
    } else if (attackTypeStr == "TownHall") {
      config.attackType = AttackType::TOWN_HALL;
    } else if (attackTypeStr == "WALL") {
      config.attackType = AttackType::WALL;
    } else {
      CCLOG("Invalid attack type: %s", attackTypeStr.c_str());
      return config;
    }
  }
  // 士兵类型
  if (soldierTypeConfig.HasMember("SoldierType") &&
      soldierTypeConfig["SoldierType"].IsString()) {
    std::string soldierTypeStr = soldierTypeConfig["SoldierType"].GetString();
    if (soldierTypeStr == "LAND") {
      config.soldierCategory = SoldierCategory::LAND;
    } else if (soldierTypeStr == "AIR") {
      config.soldierCategory = SoldierCategory::AIR;
    } else {
      CCLOG("Invalid soldier type: %s", soldierTypeStr.c_str());
      return config;
    }
  }
  // 获取等级配置
  std::string levelKey = std::to_string(level);
  if (!soldierTypeConfig.HasMember(levelKey.c_str()) ||
      !soldierTypeConfig[levelKey.c_str()].IsObject()) {
    CCLOG("Level %d not found for soldier type '%s'", level,
          soldierType.c_str());
    return config;
  }

  const rapidjson::Value& levelConfig = soldierTypeConfig[levelKey.c_str()];

  // 读取配置值
  if (levelConfig.HasMember("PanelImage") &&
      levelConfig["PanelImage"].IsString()) {
    config.panelImage = levelConfig["PanelImage"].GetString();
  }
  if (levelConfig.HasMember("MoveImage") &&
      levelConfig["MoveImage"].IsString()) {
    config.moveImage = levelConfig["MoveImage"].GetString();
  }

  if (levelConfig.HasMember("Attack") && levelConfig["Attack"].IsNumber()) {
    config.attack = levelConfig["Attack"].GetFloat();
  }

  if (levelConfig.HasMember("Health") && levelConfig["Health"].IsNumber()) {
    config.health = levelConfig["Health"].GetFloat();
  }

  if (levelConfig.HasMember("MoveSpeed") &&
      levelConfig["MoveSpeed"].IsNumber()) {
    config.moveSpeed = levelConfig["MoveSpeed"].GetFloat();
  }

  if (levelConfig.HasMember("AttackSpeed") &&
      levelConfig["AttackSpeed"].IsNumber()) {
    config.attackSpeed = levelConfig["AttackSpeed"].GetFloat();
  }

  if (levelConfig.HasMember("AttackRange") &&
      levelConfig["AttackRange"].IsNumber()) {
    config.attackRange = levelConfig["AttackRange"].GetFloat();
  }
  return config;
}

bool ConfigManager::loadSpellConfig() {
  // 直接加载到成员变量
  if (!loadJsonFromFile("config/spell.json", _spellConfigDoc)) {
    return false;
  }

  return true;
}

ConfigManager::SpellConfig ConfigManager::getSpellConfig(
    const std::string& spellType) const {
  SpellConfig config;

  // 设置默认值
  config.category = SpellCategory::INSTANT;
  config.duration = 0.0f;
  config.radius = 100.0f;
  config.amount = 100.0f;
  config.ratio = 1.0f;  // 默认倍率为1.0（不提升）
  config.panelImage = "";

  // 检查文档是否有效
  if (_spellConfigDoc.IsNull() || !_spellConfigDoc.IsObject()) {
    CCLOG("Spell config document is not loaded");
    return config;
  }

  // 获取法术类型配置
  if (!_spellConfigDoc.HasMember(spellType.c_str()) ||
      !_spellConfigDoc[spellType.c_str()].IsObject()) {
    CCLOG("Spell type '%s' not found in config", spellType.c_str());
    return config;
  }

  const rapidjson::Value& spellTypeConfig = _spellConfigDoc[spellType.c_str()];

  // 读取配置值
  if (spellTypeConfig.HasMember("Category") &&
      spellTypeConfig["Category"].IsString()) {
    std::string categoryStr = spellTypeConfig["Category"].GetString();
    if (categoryStr == "INSTANT") {
      config.category = SpellCategory::INSTANT;
    } else if (categoryStr == "DURATION") {
      config.category = SpellCategory::DURATION;
    } else {
      CCLOG("Invalid spell category: %s", categoryStr.c_str());
      return config;
    }
  }

  if (spellTypeConfig.HasMember("Duration") &&
      spellTypeConfig["Duration"].IsNumber()) {
    config.duration = spellTypeConfig["Duration"].GetFloat();
  }

  if (spellTypeConfig.HasMember("Radius") &&
      spellTypeConfig["Radius"].IsNumber()) {
    config.radius = spellTypeConfig["Radius"].GetFloat();
  }

  if (spellTypeConfig.HasMember("Amount") &&
      spellTypeConfig["Amount"].IsNumber()) {
    config.amount = spellTypeConfig["Amount"].GetFloat();
  }
  if (spellTypeConfig.HasMember("PanelImage") &&
      spellTypeConfig["PanelImage"].IsString()) {
    config.panelImage = spellTypeConfig["PanelImage"].GetString();
  }
  if (spellTypeConfig.HasMember("Ratio") &&
      spellTypeConfig["Ratio"].IsNumber()) {
    config.ratio = spellTypeConfig["Ratio"].GetFloat();
  }
  return config;
}
