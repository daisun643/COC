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
      _name("") {
}

Profile::~Profile() {
}

bool Profile::init() {
    // 尝试从文件加载配置
    std::string path = FileUtils::getInstance()->fullPathForFilename("profile/me.json");
    
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
            CCLOG("Profile: Loaded from %s", path.c_str());
            return true;
        }
    }
    
    // 如果加载失败，使用默认值
    CCLOG("Profile: No profile file found or load failed, using default values.");
    _isLogin = false;
    _id = 0;
    _name = "";
    return true;
}

void Profile::setIsLogin(bool isLogin) {
    _isLogin = isLogin;
}

void Profile::setId(int id) {
    _id = id;
}

void Profile::setName(const std::string& name) {
    _name = name;
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

