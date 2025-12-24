#include "PlayerManager.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
#include <direct.h>  // for _getcwd
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <fstream>  // Added for std::ofstream

#include "Utils/PathUtils.h"
#include "cocos2d.h"
#include "json/document.h"
#include "json/stringbuffer.h"
#include "json/writer.h"

USING_NS_CC;

PlayerManager* PlayerManager::_instance = nullptr;

PlayerManager* PlayerManager::getInstance() {
  if (_instance == nullptr) {
    _instance = new (std::nothrow) PlayerManager();
    if (_instance && _instance->init()) {
      // 成功初始化
    } else {
      CC_SAFE_DELETE(_instance);
    }
  }
  return _instance;
}

void PlayerManager::destroyInstance() { CC_SAFE_DELETE(_instance); }

PlayerManager::PlayerManager()
    : _isNewGame(false),
      _gold(0),
      _elixir(0),
      _gems(20),
      _maxGold(0),
      _maxElixir(0),
      _goldProduction(0),
      _elixirProduction(0),
      _allowUpdateMaxLimit(true),
      _autoSaveCallback(nullptr) {}

PlayerManager::~PlayerManager() {
  // 析构时取消定时器
  Director::getInstance()->getScheduler()->unschedule("AutoSaveKey", this);
}

#include "Manager/Config/ConfigManager.h"

bool PlayerManager::init() {
  // 尝试加载存档，如果加载失败则使用默认值
  bool loadSuccess = loadUserData();
  if (!loadSuccess) {
    _isNewGame = true;

    // 初始资源设为0，等待BuildingManager计算上限后填满
    _gold = 0;
    _elixir = 0;
  } else {
    _isNewGame = false;
  }

  // [注意] _maxGold 和 _maxElixir 这里是初始值。
  // 实际游戏中，这些值应该由 BuildingManager
  // 根据已加载的建筑（如储金罐等级）来重新计算并设置。
  if (_isNewGame) {
    _maxGold = 0;
    _maxElixir = 0;
  }
  _goldProduction = 100;
  _elixirProduction = 100;

  // 开启自动保存定时器
  // 每 60 秒自动保存一次，作为资源数据的备份机制
  // 主要的保存时机应为：程序退出/后台、关键操作（如消费）
  Director::getInstance()->getScheduler()->schedule(
      [this](float dt) {
        this->saveUserData();
        // 如果设置了回调（如保存建筑），则执行回调
        if (_autoSaveCallback) {
          _autoSaveCallback();
        }
      },
      this, 60.0f, false, "AutoSaveKey");

  return true;
}

// 实现设置回调的方法
void PlayerManager::setAutoSaveCallback(const std::function<void()>& callback) {
  _autoSaveCallback = callback;
}

void PlayerManager::addGems(int amount) {
  _gems += amount;
  // saveUserData(); // 建议保存
}
bool PlayerManager::consumeGems(int amount) {
  if (_gems >= amount) {
    _gems -= amount;
    // saveUserData(); // 建议保存
    return true;
  }
  return false;
}

void PlayerManager::setGold(int amount) {
  _gold = amount;
  // 这里可以发送一个事件通知UI更新，或者让UI在update里轮询
  // 为了简单起见，我们暂时让UI主动获取，或者在MainUILayer里监听变化
}

void PlayerManager::setElixir(int amount) { _elixir = amount; }

void PlayerManager::addGold(int amount) {
  _gold += amount;
  if (_gold > _maxGold) {
    _gold = _maxGold;
  }
  // [新增] 资源增加（收集）后立即保存
  saveUserData();
}

void PlayerManager::addElixir(int amount) {
  _elixir += amount;
  if (_elixir > _maxElixir) {
    _elixir = _maxElixir;
  }
  // [新增] 资源增加（收集）后立即保存
  saveUserData();
}

bool PlayerManager::consumeGold(int amount) {
  if (_gold >= amount) {
    _gold -= amount;
    // [新增] 消费后立即保存，防止意外退出导致回档
    saveUserData();
    return true;
  }
  return false;
}

bool PlayerManager::consumeElixir(int amount) {
  if (_elixir >= amount) {
    _elixir -= amount;
    // [新增] 消费后立即保存，防止意外退出导致回档
    saveUserData();
    return true;
  }
  return false;
}

// 实现保存功能
void PlayerManager::saveUserData() {
  rapidjson::Document doc;
  doc.SetObject();
  rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

  doc.AddMember("gold", _gold, allocator);
  doc.AddMember("elixir", _elixir, allocator);

  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  doc.Accept(writer);

  // [修改] 保存到 Resources/develop/user_data.json 以便开发调试
  std::string relativePath = "develop/user_data.json";
  std::string path = PathUtils::getRealFilePath(relativePath, true);

  // [新增] 确保目录存在
  PathUtils::ensureDirectoryExists(path);

  // 优先使用 std::ofstream 以确保 Windows DevMode 下能写入源码目录
  std::ofstream outFile(path.c_str());
  if (outFile.is_open()) {
    outFile << buffer.GetString();
    outFile.close();
  } else {
    // 尝试使用 FileUtils 作为备选 (主要针对非 Windows 平台)
    if (FileUtils::getInstance()->writeStringToFile(buffer.GetString(), path)) {
    }
  }
}

// 实现加载功能
bool PlayerManager::loadUserData() {
  std::string relativePath = "develop/user_data.json";
  std::string path = PathUtils::getRealFilePath(relativePath, false);

  std::string content;

  // 策略1: 尝试使用 std::ifstream 读取绝对路径 (最优先)
  std::ifstream inFile(path.c_str());
  if (inFile.is_open()) {
    std::stringstream buffer;
    buffer << inFile.rdbuf();
    content = buffer.str();
    inFile.close();
  }

  // 策略2: 如果失败，尝试使用 FileUtils 读取 (兼容相对路径/SearchPaths)
  if (content.empty()) {
    if (FileUtils::getInstance()->isFileExist(path)) {
      content = FileUtils::getInstance()->getStringFromFile(path);
    } else if (FileUtils::getInstance()->isFileExist(relativePath)) {
      content = FileUtils::getInstance()->getStringFromFile(relativePath);
    }
  }

  if (content.empty()) {
    return false;
  }

  rapidjson::Document doc;
  doc.Parse(content.c_str());

  if (doc.HasParseError()) {
    return false;
  }

  if (doc.HasMember("gold") && doc["gold"].IsInt()) {
    _gold = doc["gold"].GetInt();
  }
  if (doc.HasMember("elixir") && doc["elixir"].IsInt()) {
    _elixir = doc["elixir"].GetInt();
  }

  return true;
}
