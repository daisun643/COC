#include "ReplayLayer.h"

#include <tuple>
#include <vector>

#include "Container/Scene/Record/RecordScene.h"
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

ReplayLayer* ReplayLayer::create() {
  ReplayLayer* layer = new (std::nothrow) ReplayLayer();
  if (layer && layer->init()) {
    layer->autorelease();
    return layer;
  }
  CC_SAFE_DELETE(layer);
  return nullptr;
}

bool ReplayLayer::init() {
  if (!LayerColor::initWithColor(Color4B(0, 0, 0, 180))) {
    return false;
  }

  buildUI();

  auto listener = EventListenerTouchOneByOne::create();
  listener->setSwallowTouches(true);
  listener->onTouchBegan = [this](Touch* touch, Event* event) {
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
// TODO 删除
void ReplayLayer::setOnReplaySelectedCallback(
    std::function<void(const std::string& recordPath)> callback) {
  _onReplaySelected = callback;
}

void ReplayLayer::buildUI() {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

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

  auto title = createLabel("战斗回放", 32);
  title->setPosition(Vec2(_panel->getContentSize().width / 2.0f,
                          _panel->getContentSize().height - 35.0f));
  title->enableBold();
  _panel->addChild(title);

  auto closeBtn = Button::create("images/ui/Bar.png");
  closeBtn->setScale(0.5f);
  closeBtn->setTitleText("关闭");
  closeBtn->setTitleFontSize(20);
  closeBtn->setPosition(Vec2(_panel->getContentSize().width - 60.0f,
                             _panel->getContentSize().height - 35.0f));
  closeBtn->addClickEventListener([this](Ref*) { this->removeFromParent(); });
  _panel->addChild(closeBtn);

  _scrollView = ScrollView::create();
  _scrollView->setContentSize(Size(760.0f, 500.0f));
  _scrollView->setDirection(ScrollView::Direction::VERTICAL);
  _scrollView->setBounceEnabled(true);
  _scrollView->setPosition(Vec2(20.0f, 20.0f));
  _panel->addChild(_scrollView);

  // 读取Resources/record/summary.json中的records数组
  float itemHeight = 100.0f;
  float spacing = 10.0f;
  FileUtils* fileUtils = FileUtils::getInstance();
  std::vector<std::tuple<std::string, std::string, std::string, std::string>>
      records;  // <name, mapPath, recordPath, time>

  rapidjson::Document doc;
  std::string summaryPath = "record/summary.json";
  std::string fullPath = fileUtils->fullPathForFilename(summaryPath);

  if (!fullPath.empty() && fileUtils->isFileExist(fullPath)) {
    std::string content = fileUtils->getStringFromFile(fullPath);
    if (!content.empty()) {
      doc.Parse(content.c_str());
      if (!doc.HasParseError() && doc.HasMember("records") &&
          doc["records"].IsArray()) {
        const rapidjson::Value& recordsArray = doc["records"];
        for (rapidjson::SizeType i = 0; i < recordsArray.Size(); i++) {
          const rapidjson::Value& recordObj = recordsArray[i];
          if (recordObj.IsObject() && recordObj.HasMember("name") &&
              recordObj.HasMember("mapPath") &&
              recordObj.HasMember("recordPath") &&
              recordObj.HasMember("time")) {
            std::string name = recordObj["name"].IsString()
                                   ? recordObj["name"].GetString()
                                   : "";
            std::string mapPath = recordObj["mapPath"].IsString()
                                      ? recordObj["mapPath"].GetString()
                                      : "";
            std::string recordPath = recordObj["recordPath"].IsString()
                                         ? recordObj["recordPath"].GetString()
                                         : "";
            std::string time = recordObj["time"].IsString()
                                   ? recordObj["time"].GetString()
                                   : "";
            if (!name.empty() && !mapPath.empty() && !recordPath.empty() &&
                !time.empty()) {
              records.push_back(
                  std::make_tuple(name, mapPath, recordPath, time));
            }
          }
        }
      }
    }
  }

  // 如果没有读取到记录，使用空列表
  if (records.empty()) {
    CCLOG("ReplayLayer: No records found in summary.json, using empty list");
  }

  // 根据实际记录数量调整滚动视图大小
  int itemCount = records.size();
  float innerHeight = (itemHeight + spacing) * itemCount;
  if (innerHeight < _scrollView->getContentSize().height) {
    innerHeight = _scrollView->getContentSize().height;
  }
  _scrollView->setInnerContainerSize(
      Size(_scrollView->getContentSize().width, innerHeight));

  // 创建回放项（按时间倒序，最新的在前）
  for (size_t i = 0; i < records.size(); ++i) {
    const auto& record =
        records[records.size() - 1 - i];  // 倒序显示，最新的在前
    std::string name = std::get<0>(record);
    std::string mapPath = std::get<1>(record);
    std::string recordPath = std::get<2>(record);
    std::string timeStr = std::get<3>(record);

    // 格式化时间显示（从 YYYYMMDD_HHMMSS 转换为 YYYY-MM-DD HH:MM:SS）
    std::string formattedTime = timeStr;
    if (timeStr.length() == 15) {  // YYYYMMDD_HHMMSS
      formattedTime = timeStr.substr(0, 4) + "-" + timeStr.substr(4, 2) + "-" +
                      timeStr.substr(6, 2) + " " + timeStr.substr(9, 2) + ":" +
                      timeStr.substr(11, 2) + ":" + timeStr.substr(13, 2);
    }

    // 传递 mapPath 和 recordPath（使用 recordPath 作为主要标识）
    auto item =
        createReplayItem(recordPath, mapPath, name, true, formattedTime);
    item->setPosition(Vec2(_scrollView->getContentSize().width / 2.0f,
                           innerHeight - (i + 0.5f) * (itemHeight + spacing)));
    _scrollView->addChild(item);
  }
}

Widget* ReplayLayer::createReplayItem(const std::string& recordPath,
                                      const std::string& mapPath,
                                      const std::string& opponentName,
                                      bool isVictory,
                                      const std::string& timeStr) {
  float width = 700.0f;
  float height = 100.0f;

  auto widget = Layout::create();
  widget->setContentSize(Size(width, height));
  widget->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
  widget->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
  widget->setBackGroundColor(COLOR_ITEM_BG);

  // Result Icon/Text
  auto resultLabel = createLabel(isVictory ? "胜利" : "失败", 28,
                                 isVictory ? Color4B::GREEN : Color4B::RED);
  resultLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
  resultLabel->setPosition(Vec2(20.0f, height / 2.0f + 15.0f));
  widget->addChild(resultLabel);

  // Opponent Name
  auto nameLabel = createLabel("VS " + opponentName, 20);
  nameLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
  nameLabel->setPosition(Vec2(120.0f, height / 2.0f + 15.0f));
  widget->addChild(nameLabel);

  // Time
  auto timeLabel = createLabel(timeStr, 16, Color4B::GRAY);
  timeLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
  timeLabel->setPosition(Vec2(20.0f, height / 2.0f - 20.0f));
  widget->addChild(timeLabel);

  // Replay Button
  auto btn = Button::create("images/ui/Bar.png");
  btn->setScale(0.4f);
  btn->setTitleText("观看");
  btn->setTitleFontSize(24);
  btn->setPosition(Vec2(width - 80.0f, height / 2.0f));
  // 地图 json 和 布兵 json
  btn->addClickEventListener([this, recordPath, mapPath](Ref*) {
    // 创建并切换到 RecordScene
    // RecordScene 需要地图路径（用于 BasicScene）和记录路径（用于加载回放数据）
    auto recordScene = RecordScene::createScene(mapPath, recordPath);
    if (recordScene) {
      auto director = Director::getInstance();
      director->replaceScene(recordScene);
    }
  });
  widget->addChild(btn);

  return widget;
}
