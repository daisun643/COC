#include "RecordScene.h"

#include <algorithm>
#include <fstream>
#include <sstream>

#include "Game/Spell/HealSpell.h"
#include "Game/Spell/LightningSpell.h"
#include "Game/Spell/RageSpell.h"
#include "Game/Building/DefenseBuilding.h"
#include "json/document.h"
#include "platform/CCFileUtils.h"

Scene* RecordScene::createScene() { return RecordScene::create(); }

bool RecordScene::init() {
  // 先调用父类的初始化
  if (!BasicScene::init()) {
    return false;
  }

  // 初始化变量
  _records.clear();
  _placedSoldiers.clear();
  _activeSpells.clear();
  _isPlaying = false;
  _isPaused = false;
  _currentTime = 0.0f;
  _currentRecordIndex = 0;
  _playbackSpeed = 1.0f;
  _playButton = nullptr;
  _pauseButton = nullptr;
  _stopButton = nullptr;
  _timeLabel = nullptr;

  // 加载记录文件
  if (!loadRecordFile("Resources/record/dev.json")) {
    showPopupDialog("错误", "无法加载回放记录文件");
    return false;
  }

  // 创建回放控制按钮
  createPlaybackButtons();

  return true;
}

bool RecordScene::loadRecordFile(const std::string& filePath) {
  FileUtils* fileUtils = FileUtils::getInstance();
  std::string fullPath = filePath;

  // 将路径中的反斜杠转换为正斜杠
  std::replace(fullPath.begin(), fullPath.end(), '\\', '/');

  // 尝试读取文件
  std::string content = fileUtils->getStringFromFile(fullPath);
  if (content.empty()) {
    // 如果FileUtils读取失败，尝试直接读取文件
    std::ifstream file(fullPath);
    if (!file.is_open()) {
      CCLOG("RecordScene: Failed to open record file: %s", fullPath.c_str());
      return false;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    content = buffer.str();
    file.close();
  }

  if (content.empty()) {
    CCLOG("RecordScene: Record file is empty: %s", fullPath.c_str());
    return false;
  }

  // 解析JSON
  rapidjson::Document doc;
  doc.Parse(content.c_str());
  if (doc.HasParseError()) {
    CCLOG("RecordScene: JSON parse error in record file");
    return false;
  }

  _records.clear();

  // 读取记录数组
  if (doc.HasMember("records") && doc["records"].IsArray()) {
    const rapidjson::Value& recordsArray = doc["records"];
    for (rapidjson::SizeType i = 0; i < recordsArray.Size(); i++) {
      const rapidjson::Value& recordObj = recordsArray[i];
      if (recordObj.IsObject()) {
        PlacementRecord record;

        // 读取类型
        if (recordObj.HasMember("type") && recordObj["type"].IsString()) {
          record.type = recordObj["type"].GetString();
        }

        // 读取类别
        if (recordObj.HasMember("category") &&
            recordObj["category"].IsString()) {
          record.category = recordObj["category"].GetString();
        }

        // 读取等级
        if (recordObj.HasMember("level") && recordObj["level"].IsInt()) {
          record.level = recordObj["level"].GetInt();
        }

        // 读取坐标
        if (recordObj.HasMember("x") && recordObj["x"].IsNumber()) {
          record.x = recordObj["x"].GetFloat();
        }
        if (recordObj.HasMember("y") && recordObj["y"].IsNumber()) {
          record.y = recordObj["y"].GetFloat();
        }

        // 读取时间戳
        if (recordObj.HasMember("timestamp") &&
            recordObj["timestamp"].IsInt()) {
          record.timestamp = recordObj["timestamp"].GetInt();
        }

        _records.push_back(record);
      }
    }
  }

  // 按时间戳排序
  std::sort(_records.begin(), _records.end(),
            [](const PlacementRecord& a, const PlacementRecord& b) {
              return a.timestamp < b.timestamp;
            });

  CCLOG("RecordScene: Loaded %zu records from %s", _records.size(),
        fullPath.c_str());

  return true;
}

void RecordScene::createPlaybackButtons() {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 创建播放按钮（左上角）
  _playButton = ui::Button::create();
  _playButton->setTitleText("播放");
  _playButton->setTitleFontSize(20);
  _playButton->setContentSize(Size(100, 40));
  _playButton->setPosition(
      Vec2(origin.x + 60, origin.y + visibleSize.height - 50));
  _playButton->addClickEventListener([this](Ref* sender) {
    if (!_isPlaying) {
      this->startPlayback();
    }
  });
  this->addChild(_playButton, 200);

  // 创建暂停按钮
  _pauseButton = ui::Button::create();
  _pauseButton->setTitleText("暂停");
  _pauseButton->setTitleFontSize(20);
  _pauseButton->setContentSize(Size(100, 40));
  _pauseButton->setPosition(
      Vec2(origin.x + 170, origin.y + visibleSize.height - 50));
  _pauseButton->setEnabled(false);
  _pauseButton->setBright(false);
  _pauseButton->addClickEventListener([this](Ref* sender) {
    if (_isPlaying && !_isPaused) {
      _isPaused = true;
      this->unschedule("updatePlayback");
      // 暂停时停止防御建筑攻击更新
      this->unschedule("updateDefenseBuildings");
    } else if (_isPaused) {
      _isPaused = false;
      this->schedule([this](float dt) { this->updatePlayback(dt); }, 0.1f,
                     "updatePlayback");
      // 恢复时重新启动防御建筑攻击更新
      this->schedule([this](float dt) { this->updateDefenseBuildings(dt); }, 0.0f,
                     "updateDefenseBuildings");
    }
  });
  this->addChild(_pauseButton, 200);

  // 创建停止按钮
  _stopButton = ui::Button::create();
  _stopButton->setTitleText("停止");
  _stopButton->setTitleFontSize(20);
  _stopButton->setContentSize(Size(100, 40));
  _stopButton->setPosition(
      Vec2(origin.x + 280, origin.y + visibleSize.height - 50));
  _stopButton->setEnabled(false);
  _stopButton->setBright(false);
  _stopButton->addClickEventListener([this](Ref* sender) {
    this->stopPlayback();
  });
  this->addChild(_stopButton, 200);

  // 创建时间标签
  _timeLabel = Label::createWithSystemFont("00:00 / 00:00", "Arial", 20);
  _timeLabel->setColor(Color3B::WHITE);
  _timeLabel->setPosition(
      Vec2(origin.x + 400, origin.y + visibleSize.height - 50));
  this->addChild(_timeLabel, 200);

  // 更新时间显示
  int maxTime = 0;
  if (!_records.empty()) {
    maxTime = _records.back().timestamp;
  }
  char buffer[32];
  snprintf(buffer, sizeof(buffer), "00:00 / %02d:%02d", maxTime / 60,
           maxTime % 60);
  _timeLabel->setString(buffer);
}

void RecordScene::startPlayback() {
  if (_isPlaying) {
    return;
  }

  _isPlaying = true;
  _isPaused = false;
  _currentTime = 0.0f;
  _currentRecordIndex = 0;

  // 清空之前的回放
  for (auto soldier : _placedSoldiers) {
    if (soldier) {
      soldier->removeFromParent();
    }
  }
  _placedSoldiers.clear();

  for (auto spell : _activeSpells) {
    if (spell) {
      spell->removeFromParent();
    }
  }
  _activeSpells.clear();

  // 更新按钮状态
  if (_playButton) {
    _playButton->setEnabled(false);
    _playButton->setBright(false);
  }
  if (_pauseButton) {
    _pauseButton->setEnabled(true);
    _pauseButton->setBright(true);
  }
  if (_stopButton) {
    _stopButton->setEnabled(true);
    _stopButton->setBright(true);
  }

  // 启动回放更新
  this->schedule([this](float dt) { this->updatePlayback(dt); }, 0.1f,
                 "updatePlayback");
  this->schedule([this](float dt) { this->updateDefenseBuildings(dt); }, 0.0f, 
                 "updateDefenseBuildings");

  CCLOG("RecordScene: Playback started");
}

void RecordScene::stopPlayback() {
  if (!_isPlaying) {
    return;
  }

  // 停止回放更新
  this->unschedule("updatePlayback");
  
  // 停止防御建筑攻击更新
  this->unschedule("updateDefenseBuildings");

  // 重置状态
  _isPlaying = false;
  _isPaused = false;
  _currentTime = 0.0f;
  _currentRecordIndex = 0;

  // 清空回放内容
  for (auto soldier : _placedSoldiers) {
    if (soldier) {
      soldier->removeFromParent();
    }
  }
  _placedSoldiers.clear();

  for (auto spell : _activeSpells) {
    if (spell) {
      spell->removeFromParent();
    }
  }
  _activeSpells.clear();

  // 更新按钮状态
  if (_playButton) {
    _playButton->setEnabled(true);
    _playButton->setBright(true);
  }
  if (_pauseButton) {
    _pauseButton->setEnabled(false);
    _pauseButton->setBright(false);
  }
  if (_stopButton) {
    _stopButton->setEnabled(false);
    _stopButton->setBright(false);
  }

  // 更新时间显示
  if (_timeLabel) {
    int maxTime = 0;
    if (!_records.empty()) {
      maxTime = _records.back().timestamp;
    }
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "00:00 / %02d:%02d", maxTime / 60,
             maxTime % 60);
    _timeLabel->setString(buffer);
  }

  CCLOG("RecordScene: Playback stopped");
}

void RecordScene::updatePlayback(float dt) {
  if (!_isPlaying || _isPaused) {
    return;
  }

  // 更新当前时间
  _currentTime += dt * _playbackSpeed;

  // 检查并执行需要播放的记录
  while (_currentRecordIndex < _records.size()) {
    const PlacementRecord& record = _records[_currentRecordIndex];
    if (record.timestamp <= static_cast<int>(_currentTime)) {
      // 执行记录
      if (record.type == "troop") {
        createSoldierFromRecord(record);
      } else if (record.type == "spell") {
        createSpellFromRecord(record);
      }
      _currentRecordIndex++;
    } else {
      break;
    }
  }

  // 更新时间显示
  if (_timeLabel) {
    int maxTime = 0;
    if (!_records.empty()) {
      maxTime = _records.back().timestamp;
    }
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%02d:%02d / %02d:%02d",
             static_cast<int>(_currentTime) / 60,
             static_cast<int>(_currentTime) % 60, maxTime / 60, maxTime % 60);
    _timeLabel->setString(buffer);
  }

  // 检查是否播放完成
  if (_currentRecordIndex >= _records.size()) {
    int maxTime = 0;
    if (!_records.empty()) {
      maxTime = _records.back().timestamp;
    }
    if (_currentTime >= maxTime) {
      stopPlayback();
    }
  }
}

void RecordScene::createSoldierFromRecord(const PlacementRecord& record) {
  // 转换士兵类型字符串为枚举
  SoldierType soldierType = SoldierType::BARBARIAN;
  if (record.category == "barbarian") {
    soldierType = SoldierType::BARBARIAN;
  } else if (record.category == "archer") {
    soldierType = SoldierType::ARCHER;
  } else if (record.category == "giant") {
    soldierType = SoldierType::GIANT;
  } else if (record.category == "bomber") {
    soldierType = SoldierType::BOMBER;
  }

  // 创建士兵
  auto soldier = BasicSoldier::create(soldierType, record.level);
  if (soldier) {
    soldier->setPosition(Vec2(record.x, record.y));
    _mapLayer->addChild(soldier, 5);
    _placedSoldiers.push_back(soldier);

    // 设置建筑查找回调
    soldier->setBuildingFinderCallback([this]() {
      std::vector<Building*> buildings;
      if (_buildingManager) {
        const auto& allBuildings = _buildingManager->getAllBuildings();
        // 只返回存活的建筑
        for (Building* building : allBuildings) {
          if (building && building->isVisible() && building->isAlive()) {
            buildings.push_back(building);
          }
        }
      }
      return buildings;
    });

    CCLOG("RecordScene: Created soldier %s Lv%d at (%.1f, %.1f) @ %ds",
          record.category.c_str(), record.level, record.x, record.y,
          record.timestamp);
  }
}

void RecordScene::createSpellFromRecord(const PlacementRecord& record) {
  // 转换法术类型字符串为枚举
  SpellType spellType = SpellType::HEAL;
  if (record.category == "Heal") {
    spellType = SpellType::HEAL;
  } else if (record.category == "Lightning") {
    spellType = SpellType::LIGHTNING;
  } else if (record.category == "Rage") {
    spellType = SpellType::RAGE;
  }

  // 创建法术
  BasicSpell* spell = nullptr;
  switch (spellType) {
    case SpellType::HEAL:
      spell = HealSpell::create();
      break;
    case SpellType::LIGHTNING:
      spell = LightningSpell::create();
      break;
    case SpellType::RAGE:
      spell = RageSpell::create();
      break;
  }

  if (spell) {
    // 设置目标查找回调
    spell->setSoldierFinderCallback([this]() { return _placedSoldiers; });
    spell->setBuildingFinderCallback([this]() {
      // 从 BuildingManager 获取建筑列表
      std::vector<Building*> buildings;
      if (_buildingManager) {
        const auto& allBuildings = _buildingManager->getAllBuildings();
        // 只返回存活的建筑
        for (Building* building : allBuildings) {
          if (building && building->isVisible() && building->isAlive()) {
            buildings.push_back(building);
          }
        }
      }
      return buildings;
    });

    // 获取所有建筑（用于cast方法）
    std::vector<Building*> buildings;
    if (_buildingManager) {
      const auto& allBuildings = _buildingManager->getAllBuildings();
      // 只返回存活的建筑
      for (Building* building : allBuildings) {
        if (building && building->isVisible() && building->isAlive()) {
          buildings.push_back(building);
        }
      }
    }

    // 施放法术
    if (spell->cast(Vec2(record.x, record.y), _placedSoldiers, buildings)) {
      _mapLayer->addChild(spell, 8);
      _activeSpells.push_back(spell);

      CCLOG("RecordScene: Created spell %s at (%.1f, %.1f) @ %ds",
            record.category.c_str(), record.x, record.y, record.timestamp);
    } else {
      CC_SAFE_DELETE(spell);
    }
  }
}

void RecordScene::updateDefenseBuildings(float delta) {
  // 如果回放未开始或已暂停，不更新防御建筑
  if (!_isPlaying || _isPaused) {
    return;
  }

  // 如果没有建筑管理器或没有士兵，直接返回
  if (!_buildingManager || _placedSoldiers.empty()) {
    return;
  }

  // 获取所有建筑
  const auto& allBuildings = _buildingManager->getAllBuildings();
  
  // 遍历所有建筑，找到防御建筑并更新攻击
  for (Building* building : allBuildings) {
    if (!building || !building->isVisible() || !building->isAlive()) {
      continue;
    }

    // 检查是否为防御建筑
    if (building->getBuildingType() != BuildingType::DEFENSE) {
      continue;
    }

    // 转换为防御建筑
    DefenseBuilding* defenseBuilding = dynamic_cast<DefenseBuilding*>(building);
    if (!defenseBuilding) {
      continue;
    }

    // 根据建筑名称决定攻击类别
    // 默认攻击所有类别，可以根据需要扩展
    std::vector<SoldierCategory> targetCategories = {SoldierCategory::LAND, SoldierCategory::AIR};
    
    // 可以根据建筑名称设置不同的攻击类别
    // 例如：防空火箭只攻击空军，加农炮只攻击陆军等
    // 这里先实现攻击所有类别，后续可以根据配置扩展
    // 如果 targetCategories 为空，attackSoldiers 会攻击所有类别
    
    // 调用防御建筑的攻击方法
    defenseBuilding->attackSoldiers(_placedSoldiers, targetCategories, delta);
  }
}

RecordScene::~RecordScene() {
  // 停止回放
  if (_isPlaying) {
    this->unschedule("updatePlayback");
    this->unschedule("updateDefenseBuildings");
  }

  // 清理士兵和法术
  for (auto soldier : _placedSoldiers) {
    if (soldier) {
      soldier->removeFromParent();
    }
  }
  _placedSoldiers.clear();

  for (auto spell : _activeSpells) {
    if (spell) {
      spell->removeFromParent();
    }
  }
  _activeSpells.clear();
}
