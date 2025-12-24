#include "TroopManager.h"

#include <string>

#include "Manager/Config/ConfigManager.h"
#include "json/document.h"

TroopManager::TroopManager() {}

TroopManager::~TroopManager() {}

bool TroopManager::init(const std::string& configPath) {
  // 加载配置文件
  return loadTroopConfig(configPath);
}

bool TroopManager::loadTroopConfig(const std::string& configPath) {
  FileUtils* fileUtils = FileUtils::getInstance();
  std::string fullPath = fileUtils->fullPathForFilename(configPath);

  if (fullPath.empty()) {
    return false;
  }

  std::string content = fileUtils->getStringFromFile(fullPath);
  if (content.empty()) {
    return false;
  }

  rapidjson::Document doc;
  doc.Parse(content.c_str());
  if (doc.HasParseError()) {
    return false;
  }

  _troopItems.clear();
  _spellItems.clear();

  // 读取军队配置
  if (doc.HasMember("troop") && doc["troop"].IsArray()) {
    const rapidjson::Value& troopArray = doc["troop"];
    for (rapidjson::SizeType i = 0; i < troopArray.Size(); i++) {
      const rapidjson::Value& item = troopArray[i];
      if (item.IsObject()) {
        TroopItem troopItem;

        if (item.HasMember("Category") && item["Category"].IsString()) {
          troopItem.soldierType = item["Category"].GetString();
        }
        if (item.HasMember("Level") && item["Level"].IsInt()) {
          troopItem.level = item["Level"].GetInt();
        }
        if (item.HasMember("Count") && item["Count"].IsInt()) {
          troopItem.count = item["Count"].GetInt();
        }

        // 从士兵配置中获取图片路径
        auto configManager = ConfigManager::getInstance();
        if (configManager) {
          auto soldierConfig = configManager->getSoldierConfig(
              troopItem.soldierType, troopItem.level);
          troopItem.panelImage = soldierConfig.panelImage;
        }

        _troopItems.push_back(troopItem);
      }
    }
  }

  // 读取法术配置
  if (doc.HasMember("spell") && doc["spell"].IsArray()) {
    const rapidjson::Value& spellArray = doc["spell"];
    for (rapidjson::SizeType i = 0; i < spellArray.Size(); i++) {
      const rapidjson::Value& item = spellArray[i];
      if (item.IsObject()) {
        SpellItem spellItem;

        if (item.HasMember("Category") && item["Category"].IsString()) {
          spellItem.spellType = item["Category"].GetString();
        }
        if (item.HasMember("Count") && item["Count"].IsInt()) {
          spellItem.count = item["Count"].GetInt();
        }

        // 从法术配置中获取图片路径
        auto configManager = ConfigManager::getInstance();
        if (configManager && !spellItem.spellType.empty()) {
          auto spellConfig = configManager->getSpellConfig(spellItem.spellType);
          spellItem.panelImage = spellConfig.panelImage;
        } else {
          spellItem.panelImage = "";  // 如果获取失败，使用空字符串
        }

        _spellItems.push_back(spellItem);
      }
    }
  }

  return true;
}

bool TroopManager::consumeTroop(const std::string& soldierType, int level) {
  for (auto& troopItem : _troopItems) {
    if (troopItem.soldierType == soldierType && troopItem.level == level) {
      if (troopItem.count > 0) {
        troopItem.count--;
        return true;
      }
      return false;
    }
  }
  return false;
}

bool TroopManager::consumeSpell(const std::string& spellType) {
  for (auto& spellItem : _spellItems) {
    if (spellItem.spellType == spellType) {
      if (spellItem.count > 0) {
        spellItem.count--;
        return true;
      }
      return false;
    }
  }
  return false;
}
