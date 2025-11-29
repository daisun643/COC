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
  _constantConfig.gridResolutionWidth = 79;
  _constantConfig.gridResolutionHeight = 60;
  _constantConfig.grassImagePath = "images/backgrond/grass.png";
  _constantConfig.grassWidth = 158.0f;
  _constantConfig.grassHeight = 120.0f;
  _constantConfig.deltaX = 76.0f;
  _constantConfig.deltaY = 60.0f;
  _constantConfig.glowDelay = 0.5f;

  _townHallConfig.image = "images/buildings/TownHall.png";
  _townHallConfig.anchorRatioX = 0.0f;  // 默认左侧中点
  _townHallConfig.anchorRatioY = 0.5f;
  _townHallConfig.gridSize = 4;
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
  if (doc.HasMember("GridResolutionWidth") &&
      doc["GridResolutionWidth"].IsInt()) {
    _constantConfig.gridResolutionWidth = doc["GridResolutionWidth"].GetInt();
  }
  if (doc.HasMember("GridResolutionHeight") &&
      doc["GridResolutionHeight"].IsInt()) {
    _constantConfig.gridResolutionHeight = doc["GridResolutionHeight"].GetInt();
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
  if (!loadJsonFromFile("config/building.json", doc)) {
    return false;
  }

  // 解析TownHall配置
  if (doc.HasMember("TownHall") && doc["TownHall"].IsObject()) {
    const rapidjson::Value& townHall = doc["TownHall"];

    if (townHall.HasMember("image") && townHall["image"].IsString()) {
      _townHallConfig.image = townHall["image"].GetString();
    }

    // 解析AnchorRatio
    if (townHall.HasMember("AnchorRatio") &&
        townHall["AnchorRatio"].IsArray() &&
        townHall["AnchorRatio"].Size() >= 2) {
      _townHallConfig.anchorRatioX = townHall["AnchorRatio"][0].GetFloat();
      _townHallConfig.anchorRatioY = townHall["AnchorRatio"][1].GetFloat();
    } else {
      // 默认值：左侧中点
      _townHallConfig.anchorRatioX = 0.0f;
      _townHallConfig.anchorRatioY = 0.5f;
    }

    if (townHall.HasMember("GridSize") && townHall["GridSize"].IsInt()) {
      _townHallConfig.gridSize = townHall["GridSize"].GetInt();
    }

    if (townHall.HasMember("defaultLevel") &&
        townHall["defaultLevel"].IsInt()) {
      _townHallConfig.defaultLevel = townHall["defaultLevel"].GetInt();
    }

    if (townHall.HasMember("maxLevel") && townHall["maxLevel"].IsInt()) {
      _townHallConfig.maxLevel = townHall["maxLevel"].GetInt();
    }
    if (townHall.HasMember("imageScale") && townHall["imageScale"].IsNumber()) {
      _townHallConfig.imageScale = townHall["imageScale"].GetFloat();
    }
  }

  return true;
}
