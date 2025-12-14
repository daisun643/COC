#include "MapEditLayer.h"

USING_NS_CC;
using namespace cocos2d::ui;

namespace {
const Color3B COLOR_PANEL_BG(40, 42, 54);
const Color3B COLOR_CARD_BG(58, 60, 72);
const std::string FONT_NAME = "Arial";
const std::string CUSTOM_FONT = "fonts/NotoSansSC-VariableFont_wght.ttf";
static bool s_fontChecked = false;
static bool s_useCustomFont = false;

Label* createLabel(const std::string& text, int fontSize,
                   const Color3B& color = Color3B::WHITE) {
  if (!s_fontChecked) {
    s_useCustomFont = FileUtils::getInstance()->isFileExist(CUSTOM_FONT);
    s_fontChecked = true;
  }

  Label* label;
  if (s_useCustomFont) {
    TTFConfig ttfConfig(CUSTOM_FONT, fontSize);
    label = Label::createWithTTF(ttfConfig, text);
  } else {
    label = Label::createWithSystemFont(text, FONT_NAME, fontSize);
  }
  label->setColor(color);
  return label;
}
}  // namespace

MapEditLayer* MapEditLayer::createWithItems(
    const std::vector<ShopItem>& allItems) {
  MapEditLayer* layer = new (std::nothrow) MapEditLayer();
  if (layer && layer->initWithItems(allItems)) {
    layer->autorelease();
    return layer;
  }
  CC_SAFE_DELETE(layer);
  return nullptr;
}

bool MapEditLayer::initWithItems(const std::vector<ShopItem>& allItems) {
  if (!Layer::init()) {
    return false;
  }

  _allItems = allItems;
  buildUI();

  return true;
}

void MapEditLayer::buildUI() {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // Right side buttons
  float btnX = origin.x + visibleSize.width - 80;
  float startY = origin.y + visibleSize.height - 100;
  float gapY = 80;

  // Remove All Button
  _removeAllButton = createSideButton("移除全部", Vec2(btnX, startY), [this]() {
    if (_onRemoveAll) _onRemoveAll();
  });
  this->addChild(_removeAllButton);

  // Save Button
  _saveButton = createSideButton("保存", Vec2(btnX, startY - gapY), [this]() {
    if (_onSave) _onSave();
  });
  this->addChild(_saveButton);

  // Cancel Button
  _cancelButton =
      createSideButton("取消", Vec2(btnX, startY - gapY * 2), [this]() {
        if (_onCancel) _onCancel();
      });
  this->addChild(_cancelButton);

  // Bottom ScrollView
  auto bg = Layout::create();
  bg->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
  bg->setBackGroundColor(COLOR_PANEL_BG);
  bg->setBackGroundColorOpacity(200);
  bg->setContentSize(Size(visibleSize.width, 160));
  bg->setPosition(Vec2(origin.x, origin.y));
  this->addChild(bg);

  _scrollView = ScrollView::create();
  _scrollView->setDirection(ScrollView::Direction::HORIZONTAL);
  _scrollView->setContentSize(Size(visibleSize.width, 150));
  _scrollView->setInnerContainerSize(Size(visibleSize.width, 150));
  _scrollView->setPosition(Vec2(origin.x, origin.y + 5));
  this->addChild(_scrollView);
}

cocos2d::ui::Button* MapEditLayer::createSideButton(
    const std::string& title, const cocos2d::Vec2& position,
    const std::function<void()>& callback) {
  auto button = Button::create("images/ui/Bar.png");
  button->setTitleText(title);
  button->setTitleFontSize(20);
  button->setScaleX(0.6f);
  button->setScaleY(0.8f);
  button->getTitleRenderer()->setScaleX(1.33f);  // Compensate for X scale
  button->setPosition(position);
  button->addClickEventListener([callback](Ref*) {
    if (callback) callback();
  });
  return button;
}

void MapEditLayer::updateInventory(
    const std::map<std::string, int>& inventory) {
  _currentInventory = inventory;
  populateInventory();

  // Check if inventory has items
  bool hasItems = false;
  for (const auto& pair : _currentInventory) {
    if (pair.second > 0) {
      hasItems = true;
      break;
    }
  }

  if (_saveButton) {
    _saveButton->setEnabled(!hasItems);
    _saveButton->setBright(!hasItems);
    if (hasItems) {
      _saveButton->setTitleText("请先放置");
    } else {
      _saveButton->setTitleText("保存");
    }
  }
}

void MapEditLayer::populateInventory() {
  _scrollView->removeAllChildren();

  float itemWidth = 120.0f;
  float itemHeight = 140.0f;
  float margin = 10.0f;
  float startX = margin + itemWidth / 2.0f;
  float y = _scrollView->getContentSize().height / 2.0f;

  int index = 0;
  for (const auto& pair : _currentInventory) {
    std::string id = pair.first;
    int count = pair.second;

    if (count <= 0) continue;

    // Find ShopItem info
    auto it = std::find_if(_allItems.begin(), _allItems.end(),
                           [&](const ShopItem& item) { return item.id == id; });

    if (it != _allItems.end()) {
      auto card = createInventoryCard(*it, count);
      card->setPosition(Vec2(startX + index * (itemWidth + margin), y));
      _scrollView->addChild(card);
      index++;
    }
  }

  float innerWidth = std::max(_scrollView->getContentSize().width,
                              (itemWidth + margin) * index + margin);
  _scrollView->setInnerContainerSize(
      Size(innerWidth, _scrollView->getContentSize().height));
}

cocos2d::ui::Layout* MapEditLayer::createInventoryCard(const ShopItem& item,
                                                       int count) {
  auto layout = Layout::create();
  layout->setContentSize(Size(120, 140));
  layout->setAnchorPoint(Vec2(0.5f, 0.5f));
  layout->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
  layout->setBackGroundColor(COLOR_CARD_BG);
  layout->setTouchEnabled(true);
  layout->addClickEventListener([this, item](Ref*) {
    if (_onItemClick) {
      _onItemClick(item.id);
    }
  });

  // Image
  auto sprite = Sprite::create(item.imagePath);
  if (sprite) {
    float scale = 80.0f / sprite->getContentSize().width;
    sprite->setScale(scale);
    sprite->setPosition(Vec2(60, 80));
    layout->addChild(sprite);
  }

  // Name
  auto nameLabel = createLabel(item.displayName, 14);
  nameLabel->setPosition(Vec2(60, 30));
  layout->addChild(nameLabel);

  // Count
  auto countLabel =
      createLabel("x" + std::to_string(count), 16, Color3B::GREEN);
  countLabel->setPosition(Vec2(100, 120));
  layout->addChild(countLabel);

  return layout;
}

void MapEditLayer::setOnRemoveAllCallback(std::function<void()> callback) {
  _onRemoveAll = callback;
}

void MapEditLayer::setOnSaveCallback(std::function<void()> callback) {
  _onSave = callback;
}

void MapEditLayer::setOnCancelCallback(std::function<void()> callback) {
  _onCancel = callback;
}

void MapEditLayer::setOnItemClickCallback(
    std::function<void(const std::string&)> callback) {
  _onItemClick = callback;
}
