#include "AttackLayer.h"

#include <chrono>
#include <utility>
#include <vector>

#include "Container/Scene/AttackScence/AttackScene.h"
#include "Utils/API/Battle/Battle.h"
#include "Utils/PathUtils.h"
#include "Utils/Profile/Profile.h"
#include "json/document.h"
#include "platform/CCFileUtils.h"

USING_NS_CC;
using namespace cocos2d::ui;

namespace {
const Color3B COLOR_PANEL_BG(40, 42, 54);
const Color3B COLOR_ITEM_BG(58, 60, 72);
const std::string FONT_NAME = "Arial";

Label* createLabel(const std::string& text, int fontSize,
                   const Color4B& color = Color4B::WHITE) {
  auto label = Label::createWithSystemFont(text, FONT_NAME, fontSize);
  label->setTextColor(color);
  return label;
}
}  // namespace

AttackLayer* AttackLayer::create() {
  AttackLayer* layer = new (std::nothrow) AttackLayer();
  if (layer && layer->init()) {
    layer->autorelease();
    return layer;
  }
  CC_SAFE_DELETE(layer);
  return nullptr;
}

bool AttackLayer::init() {
  if (!LayerColor::initWithColor(Color4B(0, 0, 0, 180))) {
    return false;
  }

  // 初始化搜索相关变量
  _searchStatusLabel = nullptr;
  _searchTimeLabel = nullptr;
  _isSearching = false;

  buildUI();

  // Swallow touches
  auto listener = EventListenerTouchOneByOne::create();
  listener->setSwallowTouches(true);
  listener->onTouchBegan = [this](Touch* touch, Event* event) {
    // Close if clicked outside panel
    if (!_panel) return true;
    Vec2 location = touch->getLocation();
    Vec2 panelPos = _panel->convertToNodeSpace(location);
    Rect panelRect(0, 0, _panel->getContentSize().width,
                   _panel->getContentSize().height);
    if (!panelRect.containsPoint(panelPos)) {
      this->removeFromParent();
    }
    return true;
  };
  _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

  return true;
}

void AttackLayer::buildUI() {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // Main Panel
  _panel = Layout::create();
  _panel->setContentSize(Size(800.0f, 600.0f));
  _panel->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
  _panel->setBackGroundColor(COLOR_PANEL_BG);
  _panel->setBackGroundColorOpacity(255);
  _panel->setPosition(Vec2(
      origin.x + (visibleSize.width - _panel->getContentSize().width) / 2.0f,
      origin.y +
          (visibleSize.height - _panel->getContentSize().height) / 2.0f));
  this->addChild(_panel);

  // Title
  auto title = createLabel("进攻", 32);
  title->setPosition(Vec2(_panel->getContentSize().width / 2.0f,
                          _panel->getContentSize().height - 35.0f));
  title->enableBold();
  _panel->addChild(title);

  // Close Button
  auto closeBtn = Button::create("images/ui/Bar.png");  // Reuse existing asset
  closeBtn->setScale(0.5f);
  closeBtn->setTitleText("关闭");
  closeBtn->setTitleFontSize(20);
  closeBtn->setPosition(Vec2(_panel->getContentSize().width - 60.0f,
                             _panel->getContentSize().height - 35.0f));
  closeBtn->addClickEventListener([this](Ref*) { this->removeFromParent(); });
  _panel->addChild(closeBtn);

  initTabs();
}

void AttackLayer::initTabs() {
  float tabWidth = 200.0f;
  float tabHeight = 50.0f;
  float startY = _panel->getContentSize().height - 80.0f;

  // Single Player Tab Button
  _btnSinglePlayer = Button::create("images/ui/Bar.png");  // Placeholder image
  _btnSinglePlayer->setScale9Enabled(true);
  _btnSinglePlayer->setContentSize(Size(tabWidth, tabHeight));
  _btnSinglePlayer->setTitleText("单人模式");
  _btnSinglePlayer->setTitleFontSize(24);
  _btnSinglePlayer->setPosition(Vec2(
      _panel->getContentSize().width / 2.0f - tabWidth / 2.0f - 10.0f, startY));
  _btnSinglePlayer->addClickEventListener(
      [this](Ref*) { showSinglePlayerTab(); });
  _panel->addChild(_btnSinglePlayer);

  // Multiplayer Tab Button
  _btnMultiplayer = Button::create("images/ui/Bar.png");  // Placeholder image
  _btnMultiplayer->setScale9Enabled(true);
  _btnMultiplayer->setContentSize(Size(tabWidth, tabHeight));
  _btnMultiplayer->setTitleText("联机模式");
  _btnMultiplayer->setTitleFontSize(24);
  _btnMultiplayer->setPosition(Vec2(
      _panel->getContentSize().width / 2.0f + tabWidth / 2.0f + 10.0f, startY));
  _btnMultiplayer->addClickEventListener(
      [this](Ref*) { showMultiplayerTab(); });
  _panel->addChild(_btnMultiplayer);

  // Content Area
  _contentArea = Layout::create();
  _contentArea->setContentSize(Size(760.0f, 450.0f));
  _contentArea->setPosition(Vec2(20.0f, 20.0f));
  _panel->addChild(_contentArea);

  // Default to Single Player
  showSinglePlayerTab();
}

void AttackLayer::updateTabButtons(bool isSinglePlayer) {
  // Simple visual feedback (opacity or color)
  // Since we are using "Bar.png" which might be an image, setColor affects
  // tint.
  if (isSinglePlayer) {
    _btnSinglePlayer->setColor(Color3B::WHITE);
    _btnMultiplayer->setColor(Color3B::GRAY);
  } else {
    _btnSinglePlayer->setColor(Color3B::GRAY);
    _btnMultiplayer->setColor(Color3B::WHITE);
  }
}

void AttackLayer::showSinglePlayerTab() {
  // 如果正在搜索，停止搜索
  if (_isSearching) {
    this->unschedule("updateSearchStatus");
    _isSearching = false;
  }

  updateTabButtons(true);
  _contentArea->removeAllChildren();

  // Vertical ScrollView
  _scrollView = ScrollView::create();
  _scrollView->setContentSize(_contentArea->getContentSize());
  _scrollView->setDirection(ScrollView::Direction::VERTICAL);
  _scrollView->setBounceEnabled(true);
  _contentArea->addChild(_scrollView);

  // 读Resources\level\summary.json中的关卡信息
  FileUtils* fileUtils = FileUtils::getInstance();
  std::vector<std::pair<std::string, std::string>> levels;  // <name, path>

  rapidjson::Document doc;
  std::string summaryPath = "level/summary.json";
  // 使用 PathUtils 获取真实路径
  std::string fullPath = PathUtils::getRealFilePath(summaryPath, true);
  CCLOG("fullPath: %s", fullPath.c_str());
  if (!fullPath.empty() && fileUtils->isFileExist(fullPath)) {
    std::string content = fileUtils->getStringFromFile(fullPath);
    if (!content.empty()) {
      doc.Parse(content.c_str());
      if (!doc.HasParseError() && doc.HasMember("levels") &&
          doc["levels"].IsArray()) {
        const rapidjson::Value& levelsArray = doc["levels"];
        for (rapidjson::SizeType i = 0; i < levelsArray.Size(); i++) {
          const rapidjson::Value& levelObj = levelsArray[i];
          if (levelObj.IsObject() && levelObj.HasMember("name") &&
              levelObj.HasMember("path")) {
            std::string name =
                levelObj["name"].IsString() ? levelObj["name"].GetString() : "";
            std::string path =
                levelObj["path"].IsString() ? levelObj["path"].GetString() : "";
            if (!name.empty() && !path.empty()) {
              levels.push_back(std::make_pair(name, path));
            }
          }
        }
      } else {
        CCLOG("Failed to parse summary.json or levels array not found");
      }
    }
  } else {
    CCLOG("summary.json not found");
  }

  // Populate Levels
  float itemHeight = 100.0f;
  float spacing = 10.0f;
  int levelCount = levels.size();
  float innerHeight = (itemHeight + spacing) * levelCount;
  if (innerHeight < _contentArea->getContentSize().height) {
    innerHeight = _contentArea->getContentSize().height;
  }

  _scrollView->setInnerContainerSize(
      Size(_contentArea->getContentSize().width, innerHeight));

  for (size_t i = 0; i < levels.size(); ++i) {
    int levelId = i + 1;  // 关卡ID从1开始
    const auto& levelInfo = levels[i];
    auto item = createLevelItem(levelId, levelInfo.first, levelInfo.second,
                                0);  // Mock stars
    item->setPosition(Vec2(_contentArea->getContentSize().width / 2.0f,
                           innerHeight - (i + 0.5f) * (itemHeight + spacing)));
    _scrollView->addChild(item);
  }
}

void AttackLayer::showMultiplayerTab() {
  updateTabButtons(false);
  _contentArea->removeAllChildren();

  auto label = createLabel("搜索其他玩家进行对战！", 24);
  label->setPosition(
      Vec2(_contentArea->getContentSize().width / 2.0f,
           _contentArea->getContentSize().height / 2.0f + 50.0f));
  _contentArea->addChild(label);

  auto searchBtn =
      Button::create("images/ui/Attack.png");  // Reuse Attack icon or Bar
  searchBtn->setScale(0.8f);
  searchBtn->setPosition(
      Vec2(_contentArea->getContentSize().width / 2.0f,
           _contentArea->getContentSize().height / 2.0f - 50.0f));
  searchBtn->addClickEventListener([this](Ref*) {
    this->searchOpponent();
  });
  _contentArea->addChild(searchBtn);

  auto btnLabel = createLabel("搜索对手", 24);
  btnLabel->setPosition(
      Vec2(searchBtn->getPositionX(), searchBtn->getPositionY() - 60.0f));
  _contentArea->addChild(btnLabel);

  auto costLabel = createLabel("花费: 50 金币", 20, Color4B::YELLOW);
  costLabel->setPosition(
      Vec2(searchBtn->getPositionX(), searchBtn->getPositionY() - 90.0f));
  _contentArea->addChild(costLabel);

  // 创建搜索状态标签（初始隐藏）
  _searchStatusLabel = createLabel("搜索中...", 24, Color4B::GREEN);
  _searchStatusLabel->setPosition(
      Vec2(_contentArea->getContentSize().width / 2.0f,
           _contentArea->getContentSize().height / 2.0f + 100.0f));
  _searchStatusLabel->setVisible(false);
  _contentArea->addChild(_searchStatusLabel);

  // 创建搜索时间标签（初始隐藏）
  _searchTimeLabel = createLabel("搜索时间: 0秒", 20, Color4B::WHITE);
  _searchTimeLabel->setPosition(
      Vec2(_contentArea->getContentSize().width / 2.0f,
           _contentArea->getContentSize().height / 2.0f + 70.0f));
  _searchTimeLabel->setVisible(false);
  _contentArea->addChild(_searchTimeLabel);
}

bool AttackLayer::searchOpponent() {
  // 如果正在搜索，则忽略新的搜索请求
  if (_isSearching) {
    return false;
  }

  // 获取用户ID
  Profile* profile = Profile::getInstance();
  if (!profile || !profile->getIsLogin()) {
    CCLOG("User not logged in, cannot search opponent");
    return false;
  }

  int userId = profile->getId();
  if (userId <= 0) {
    CCLOG("Invalid user ID: %d", userId);
    return false;
  }

  // 开始搜索
  _isSearching = true;
  _searchStartTime = std::chrono::steady_clock::now();

  // 显示搜索状态标签
  if (_searchStatusLabel) {
    _searchStatusLabel->setVisible(true);
    _searchStatusLabel->setString("搜索中...");
  }
  if (_searchTimeLabel) {
    _searchTimeLabel->setVisible(true);
    _searchTimeLabel->setString("搜索时间: 0秒");
  }

  // 启动定时器更新搜索时间
  this->schedule([this](float dt) { this->updateSearchStatus(dt); }, 0.1f,
                 "updateSearchStatus");

  // 调用 API 搜索对手
  Battle::searchOpponent(
      userId,
      [this](bool success, const std::string& message, int opponentId,
             const std::string& opponentName, const std::string& mapJsonData) {
        // 停止搜索状态更新
        this->unschedule("updateSearchStatus");
        _isSearching = false;

        // 隐藏搜索状态标签
        if (_searchStatusLabel) {
          _searchStatusLabel->setVisible(false);
        }
        if (_searchTimeLabel) {
          _searchTimeLabel->setVisible(false);
        }

        if (success) {
          CCLOG("Found opponent: %s (ID: %d)", opponentName.c_str(),
                opponentId);

          // 使用 mapJsonData 创建 AttackScene
          std::string tmpPath = "level/_tmp.json";
          // 使用 PathUtils 获取真实路径（forWrite=true 表示用于写入）
          std::string fullPath = PathUtils::getRealFilePath(tmpPath, false);
        
          // 确保目录存在
          if (!PathUtils::ensureDirectoryExists(fullPath)) {
            CCLOG("Failed to create directory for temp map file: %s",
                  fullPath.c_str());
            return;
          }

          // 将 mapJsonData 写入到 fullPath 文件
          auto fileUtils = FileUtils::getInstance();
          if (!fileUtils->writeStringToFile(mapJsonData, fullPath)) {
            CCLOG("Failed to write map data to file: %s", fullPath.c_str());
            return;
          }

          CCLOG("Map data written to: %s", fullPath.c_str());

          // 使用文件路径创建 AttackScene（使用 opponentName 作为 levelName）
          auto attackScene = AttackScene::createScene(tmpPath, opponentName);
          if (attackScene) {
            auto director = Director::getInstance();
            director->replaceScene(attackScene);
          } else {
            CCLOG("Failed to create AttackScene from file: %s", fullPath.c_str());
          }
          // 删除临时文件
          // fileUtils->removeFile(fullPath);
        } else {
          CCLOG("Search opponent failed: %s", message.c_str());
          // 可以在这里显示错误提示
        }
      });

  return true;
}

void AttackLayer::updateSearchStatus(float dt) {
  if (!_isSearching || !_searchTimeLabel) {
    return;
  }

  // 计算搜索时间
  auto now = std::chrono::steady_clock::now();
  auto elapsed =
      std::chrono::duration_cast<std::chrono::seconds>(now - _searchStartTime);
  int seconds = static_cast<int>(elapsed.count());

  // 更新搜索时间标签
  char timeStr[64];
  snprintf(timeStr, sizeof(timeStr), "搜索时间: %d秒", seconds);
  _searchTimeLabel->setString(timeStr);
}

Widget* AttackLayer::createLevelItem(int levelId, const std::string& name,
                                     const std::string& path, int stars) {
  float width = 700.0f;
  float height = 100.0f;

  auto widget = Layout::create();
  widget->setContentSize(Size(width, height));
  widget->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
  widget->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
  widget->setBackGroundColor(COLOR_ITEM_BG);

  // Level Name
  auto nameLabel = createLabel(name, 24);
  nameLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
  nameLabel->setPosition(Vec2(20.0f, height / 2.0f));
  widget->addChild(nameLabel);

  // Stars
  std::string starStr = "星星: ";
  for (int i = 0; i < stars; ++i) starStr += "★";
  for (int i = stars; i < 3; ++i) starStr += "☆";

  auto starLabel = createLabel(starStr, 20, Color4B::YELLOW);
  starLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
  starLabel->setPosition(Vec2(300.0f, height / 2.0f));
  widget->addChild(starLabel);

  // Attack Button
  auto btn = Button::create("images/ui/Bar.png");
  btn->setScale(0.4f);
  btn->setTitleText("进攻");
  btn->setTitleFontSize(24);
  btn->setPosition(Vec2(width - 80.0f, height / 2.0f));
  btn->addClickEventListener([this, path, name](Ref*) {
    // 使用从 JSON 读取的路径和名称
    // 创建并切换到 AttackScene，传入关卡名称
    auto attackScene = AttackScene::createScene(path, name);
    if (attackScene) {
      auto director = Director::getInstance();
      director->replaceScene(attackScene);
    }
  });
  widget->addChild(btn);

  return widget;
}
