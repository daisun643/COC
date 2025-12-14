#include "PlayerManager.h"

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
    : _gold(0),
      _elixir(0),
      _maxGold(10000),
      _maxElixir(10000),
      _goldProduction(0),
      _elixirProduction(0) {}

PlayerManager::~PlayerManager() {
  // 析构时取消定时器
  Director::getInstance()->getScheduler()->unschedule("AutoSaveKey", this);
}

bool PlayerManager::init() {
  // 尝试加载存档，如果加载失败则使用默认值
  bool loadSuccess = loadUserData();
  if (!loadSuccess) {
      CCLOG("PlayerManager: No save data found or load failed, using default values.");
      _gold = 5000;
      _elixir = 500;
  } else {
      CCLOG("PlayerManager: Initialized with saved data - Gold: %d, Elixir: %d", _gold, _elixir);
  }

  // [注意] _maxGold 和 _maxElixir 这里是初始值。
  // 实际游戏中，这些值应该由 BuildingManager 根据已加载的建筑（如储金罐等级）来重新计算并设置。
  // 如果 BuildingManager 没有保存/加载功能，重启后最大容量可能会变回 10000。
  _maxGold = 10000;
  _maxElixir = 10000;
  _goldProduction = 100;
  _elixirProduction = 100;

  // 开启自动保存定时器
  // 每 5 秒自动保存一次，防止 PC 端直接关闭窗口时不触发 applicationDidEnterBackground
  Director::getInstance()->getScheduler()->schedule([this](float dt){
      this->saveUserData();
  }, this, 5.0f, false, "AutoSaveKey");
  
  return true;
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
}

void PlayerManager::addElixir(int amount) {
  _elixir += amount;
  if (_elixir > _maxElixir) {
    _elixir = _maxElixir;
  }
}

bool PlayerManager::consumeGold(int amount) {
  if (_gold >= amount) {
    _gold -= amount;
    return true;
  }
  return false;
}

bool PlayerManager::consumeElixir(int amount) {
  if (_elixir >= amount) {
    _elixir -= amount;
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

    std::string path = FileUtils::getInstance()->getWritablePath() + "user_data.json";
    
    // 使用 FileUtils::writeStringToFile 替代 ofstream
    // 这能更好地处理跨平台路径问题，且代码更简洁
    bool success = FileUtils::getInstance()->writeStringToFile(buffer.GetString(), path);
    
    if (success) {
        // 为了避免日志刷屏，仅在调试时偶尔查看，或者注释掉
        // CCLOG("PlayerManager: User data saved to %s", path.c_str());
    } else {
        CCLOG("PlayerManager: Failed to save user data to %s", path.c_str());
    }
}

// 实现加载功能
bool PlayerManager::loadUserData() {
    std::string path = FileUtils::getInstance()->getWritablePath() + "user_data.json";

    // [修改] 增加日志以便调试路径
    CCLOG("PlayerManager: Loading data from %s", path.c_str());
    
    if (!FileUtils::getInstance()->isFileExist(path)) {
        return false;
    }

    std::string content = FileUtils::getInstance()->getStringFromFile(path);
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

    CCLOG("PlayerManager: User data loaded.");
    return true;
}