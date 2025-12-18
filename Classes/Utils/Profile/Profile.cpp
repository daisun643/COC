#include "Utils/Profile/Profile.h"

#include "cocos2d.h"
#include "json/document.h"
#include "json/stringbuffer.h"
#include "json/writer.h"

USING_NS_CC;

Profile* Profile::_instance = nullptr;

Profile* Profile::getInstance() {
    if (_instance == nullptr) {
        _instance = new (std::nothrow) Profile();
        if (_instance && _instance->init()) {
            // 成功初始化
        } else {
            CC_SAFE_DELETE(_instance);
        }
    }
    return _instance;
}

void Profile::destroyInstance() {
    CC_SAFE_DELETE(_instance);
}

Profile::Profile()
    : _isLogin(false),
      _id(0),
      _name(""),
      _clansId(0),
      _clansName("") {
}

Profile::~Profile() {
}

bool Profile::init() {
    // 先从FileUtils::getInstance()->getWritablePath() + "profile/me.json";尝试加载配置
    // 如果失败，尝试从FileUtils::getInstance()->fullPathForFilename("profile/me.json");加载配置
    
    // 辅助函数：尝试从指定路径加载配置
    auto loadFromPath = [this](const std::string& path) -> bool {
        if (FileUtils::getInstance()->isFileExist(path)) {
            std::string content = FileUtils::getInstance()->getStringFromFile(path);
            rapidjson::Document doc;
            doc.Parse(content.c_str());
            
            if (!doc.HasParseError()) {
                if (doc.HasMember("isLogin") && doc["isLogin"].IsBool()) {
                    _isLogin = doc["isLogin"].GetBool();
                }
                if (doc.HasMember("id") && doc["id"].IsInt()) {
                    _id = doc["id"].GetInt();
                }
                if (doc.HasMember("name") && doc["name"].IsString()) {
                    _name = doc["name"].GetString();
                }
                if (doc.HasMember("clans-id") && doc["clans-id"].IsInt()) {
                    _clansId = doc["clans-id"].GetInt();
                }
                if (doc.HasMember("clans-name") && doc["clans-name"].IsString()) {
                    _clansName = doc["clans-name"].GetString();
                }
                CCLOG("Profile: Loaded from %s", path.c_str());
                return true;
            }
        }
        return false;
    };
    
    // 首先尝试从可写路径加载
    std::string writablePath = FileUtils::getInstance()->getWritablePath() + "me.json";
    if (loadFromPath(writablePath)) {
        return true;
    }
    
    // 如果可写路径加载失败，尝试从 Resources 路径加载
    std::string resourcesPath = FileUtils::getInstance()->fullPathForFilename("profile/me.json");
    if (loadFromPath(resourcesPath)) {
        return true;
    }
    
    // 如果都加载失败，使用默认值
    CCLOG("Profile: No profile file found or load failed, using default values.");
    _isLogin = false;
    _id = 0;
    _name = "";
    _clansId = 0;
    _clansName = "";
    return true;
}

void Profile::setIsLogin(bool isLogin) {
    _isLogin = isLogin;
    _instance->save();
}

void Profile::setId(int id) {
    _id = id;
    _instance->save();
}

void Profile::setName(const std::string& name) {
    _name = name;
    _instance->save();
}

bool Profile::getIsLogin() const {
    return _isLogin;
}

int Profile::getId() const {
    return _id;
}

const std::string& Profile::getName() const {
    return _name;
}

void Profile::setClansId(int clansId) {
    _clansId = clansId;
    _instance->save();
}

void Profile::setClansName(const std::string& clansName) {
    _clansName = clansName;
    _instance->save();
}

int Profile::getClansId() const {
    return _clansId;
}

const std::string& Profile::getClansName() const {
    return _clansName;
}

bool Profile::save() {
    // 创建 JSON 文档
    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
    
    // 设置所有字段
    doc.AddMember("isLogin", _isLogin, allocator);
    doc.AddMember("id", _id, allocator);
    doc.AddMember("name", rapidjson::Value(_name.c_str(), allocator), allocator);
    doc.AddMember("clans-id", _clansId, allocator);
    doc.AddMember("clans-name", rapidjson::Value(_clansName.c_str(), allocator), allocator);
    
    // 序列化为字符串
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    
    std::string jsonString = buffer.GetString();
    
    // 保存到可写路径的profile/me.json（与init()加载路径对应）
    std::string path = FileUtils::getInstance()->getWritablePath() + "me.json";
    
    // 写入文件（writeStringToFile会自动创建必要的目录）
    bool result = FileUtils::getInstance()->writeStringToFile(jsonString, path);
    
    if (result) {
        CCLOG("Profile: Saved to %s", path.c_str());
    } else {
        CCLOG("Profile: Failed to save to %s", path.c_str());
    }
    
    return result;
}

