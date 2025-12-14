#include "ShopLayer.h"

#include <algorithm>

USING_NS_CC;
using namespace cocos2d::ui;

namespace {
const Color3B COLOR_PANEL_BG(40, 42, 54);
const Color3B COLOR_CARD_BG(58, 60, 72);
const Color4B COLOR_TEXT_NORMAL(200, 200, 200, 255);
const Color4B COLOR_TEXT_DESC(210, 210, 210, 255);
const Color4B COLOR_TEXT_PRICE(255, 215, 0, 255);
const std::string FONT_NAME = "Arial";
const std::string CUSTOM_FONT = "fonts/NotoSansSC-VariableFont_wght.ttf";
static bool s_fontChecked = false;
static bool s_useCustomFont = false;

Label* createLabel(const std::string& text, int fontSize,
                   const Color4B& color = Color4B::WHITE,
                   const Vec2& anchor = Vec2::ANCHOR_MIDDLE) {
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
  label->setTextColor(color);
  label->setAnchorPoint(anchor);
  return label;
}
}  // namespace

ShopLayer* ShopLayer::createWithItems(const std::vector<ShopItem>& items) {
  ShopLayer* layer = new (std::nothrow) ShopLayer();
  if (layer && layer->initWithItems(items)) {
    layer->autorelease();
    return layer;
  }
  CC_SAFE_DELETE(layer);
  return nullptr;
}

bool ShopLayer::initWithItems(const std::vector<ShopItem>& items) {
  if (!LayerColor::initWithColor(Color4B(0, 0, 0, 180))) {
    return false;
  }

  _items = items;

  buildUI();
  populateItems();

  auto listener = EventListenerTouchOneByOne::create();
  listener->setSwallowTouches(true);
  listener->onTouchBegan =
      CC_CALLBACK_2(ShopLayer::onBackgroundTouchBegan, this);
  _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

  return true;
}

void ShopLayer::setPurchaseCallback(
    const std::function<bool(const ShopItem&)>& callback) {
  _purchaseCallback = callback;
}

void ShopLayer::buildUI() {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 主面板
  _panel = Layout::create();
  _panel->setContentSize(Size(900.0f, 600.0f));
  _panel->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
  _panel->setBackGroundColor(COLOR_PANEL_BG);
  _panel->setBackGroundColorOpacity(235);
  _panel->setPosition(Vec2(
      origin.x + (visibleSize.width - _panel->getContentSize().width) / 2.0f,
      origin.y +
          (visibleSize.height - _panel->getContentSize().height) / 2.0f));
  this->addChild(_panel);

  // 标题
  auto title = createLabel("商店", 32);
  title->setPosition(Vec2(_panel->getContentSize().width / 2.0f,
                          _panel->getContentSize().height - 35.0f));
  title->enableBold();
  _panel->addChild(title);

  // 关闭按钮
  auto closeBtn = Button::create("images/ui/Bar.png");
  closeBtn->setScale(0.5f);
  closeBtn->setTitleText("关闭");
  closeBtn->setTitleFontSize(20);
  closeBtn->setPosition(Vec2(_panel->getContentSize().width - 60.0f,
                             _panel->getContentSize().height - 35.0f));
  closeBtn->addClickEventListener([this](Ref*) { this->removeFromParent(); });
  _panel->addChild(closeBtn);

  // 信息提示
  _messageLabel = createLabel("左右滑动查看更多建筑，点击购买放置", 22);
  _messageLabel->setPosition(
      Vec2(_panel->getContentSize().width / 2.0f, 50.0f));
  _panel->addChild(_messageLabel);

  // 滚动视图
  const Size scrollSize(840.0f, 420.0f);
  float scrollX = (_panel->getContentSize().width - scrollSize.width) / 2.0f;
  float scrollY = 100.0f;

  _scrollView = ScrollView::create();
  _scrollView->setContentSize(scrollSize);
  _scrollView->setDirection(ScrollView::Direction::HORIZONTAL);
  _scrollView->setBounceEnabled(true);
  _scrollView->setInertiaScrollEnabled(true);
  _scrollView->setScrollBarEnabled(false);
  _scrollView->setInnerContainerSize(scrollSize);
  _scrollView->setPosition(Vec2(scrollX, scrollY));
  _panel->addChild(_scrollView);

  // 空状态提示
  _emptyLabel = createLabel("暂时没有可购买的建筑", 22);
  _emptyLabel->setPosition(Vec2(_panel->getContentSize().width / 2.0f,
                                scrollY + scrollSize.height / 2.0f));
  _emptyLabel->setVisible(false);
  _panel->addChild(_emptyLabel);
}

void ShopLayer::populateItems() {
  if (!_scrollView) {
    return;
  }

  _scrollView->removeAllChildren();

  const float cardWidth = 240.0f;
  const float cardHeight = 360.0f;
  const float spacing = 30.0f;
  const float sidePadding = 20.0f;

  if (_items.empty()) {
    _scrollView->setInnerContainerSize(_scrollView->getContentSize());
    _scrollView->jumpToLeft();
    if (_emptyLabel) {
      _emptyLabel->setVisible(true);
    }
    showMessage("商店暂未上架任何建筑");
    return;
  }

  if (_emptyLabel) {
    _emptyLabel->setVisible(false);
  }
  showMessage("左右滑动查看更多建筑，点击购买放置");

  float innerHeight = _scrollView->getContentSize().height;
  float baseY = (innerHeight - cardHeight) / 2.0f;
  if (baseY < 0.0f) {
    baseY = 0.0f;
  }

  float currentX = sidePadding;
  for (size_t i = 0; i < _items.size(); ++i) {
    auto card = createItemCard(_items[i], static_cast<int>(i));
    if (!card) {
      continue;
    }
    card->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
    card->setPosition(Vec2(currentX, baseY));
    _scrollView->addChild(card);
    currentX += cardWidth + spacing;
  }

  size_t itemCount = _items.size();
  float totalWidth =
      sidePadding * 2.0f + cardWidth * static_cast<float>(itemCount) +
      spacing * static_cast<float>((itemCount > 0) ? (itemCount - 1) : 0);
  float innerWidth = std::max(_scrollView->getContentSize().width, totalWidth);
  _scrollView->setInnerContainerSize(Size(innerWidth, innerHeight));
  _scrollView->jumpToLeft();
}

Layout* ShopLayer::createItemCard(const ShopItem& item, int globalIndex) {
  const float cardWidth = 240.0f;
  const float cardHeight = 360.0f;
  const float padding = 15.0f;

  auto card = Layout::create();
  card->setContentSize(Size(cardWidth, cardHeight));
  card->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
  card->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
  card->setBackGroundColor(COLOR_CARD_BG);
  card->setBackGroundColorOpacity(220);

  // 1. 建筑图片 (顶部)
  float previewHeight = 140.0f;
  auto preview = Layout::create();
  preview->setContentSize(Size(cardWidth - 2 * padding, previewHeight));
  preview->setBackGroundColorType(
      Layout::BackGroundColorType::NONE);  // 透明背景
  preview->setPosition(Vec2(padding, cardHeight - padding - previewHeight));
  card->addChild(preview);

  if (!item.imagePath.empty()) {
    auto sprite = Sprite::create(item.imagePath);
    if (sprite) {
      // 缩放图片以适应预览区域
      float scaleX = (cardWidth - 2 * padding) / sprite->getContentSize().width;
      float scaleY = previewHeight / sprite->getContentSize().height;
      float scale = std::min(scaleX, scaleY) * 0.9f;  // 留一点边距
      sprite->setScale(scale);
      sprite->setPosition(Vec2(preview->getContentSize().width / 2.0f,
                               preview->getContentSize().height / 2.0f));
      preview->addChild(sprite);
    } else {
      // 图片加载失败，回退到色块
      preview->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
      preview->setBackGroundColor(Color3B(item.placeholderColor.r,
                                          item.placeholderColor.g,
                                          item.placeholderColor.b));
      preview->setBackGroundColorOpacity(230);
    }
  } else {
    // 没有图片路径，回退到色块
    preview->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
    preview->setBackGroundColor(Color3B(item.placeholderColor.r,
                                        item.placeholderColor.g,
                                        item.placeholderColor.b));
    preview->setBackGroundColorOpacity(230);
  }

  // 序号
  auto indexLabel = createLabel(StringUtils::format("#%d", globalIndex + 1), 16,
                                COLOR_TEXT_NORMAL, Vec2::ANCHOR_TOP_RIGHT);
  indexLabel->setPosition(Vec2(cardWidth - 10.0f, cardHeight - 10.0f));
  card->addChild(indexLabel);

  // 2. 名称 (预览图下方)
  auto nameLabel =
      createLabel(item.displayName, 22, Color4B::WHITE, Vec2::ANCHOR_TOP_LEFT);
  nameLabel->setPosition(Vec2(padding, preview->getPositionY() - 10.0f));
  nameLabel->enableBold();
  card->addChild(nameLabel);

  // 3. 描述 (名称下方)
  // 预留高度给描述
  float descHeight = 70.0f;
  auto descLabel =
      createLabel(item.description, 18, COLOR_TEXT_DESC, Vec2::ANCHOR_TOP_LEFT);
  descLabel->setDimensions(cardWidth - 2 * padding, descHeight);
  descLabel->setPosition(Vec2(padding, nameLabel->getPositionY() - 30.0f));
  descLabel->setAlignment(TextHAlignment::LEFT);
  descLabel->setVerticalAlignment(TextVAlignment::TOP);
  card->addChild(descLabel);

  // 4. 价格 (描述下方/底部上方)
  std::string priceText = StringUtils::format("金币: %d   圣水: %d",
                                              item.costGold, item.costElixir);
  auto priceLabel =
      createLabel(priceText, 16, COLOR_TEXT_PRICE, Vec2::ANCHOR_MIDDLE_BOTTOM);
  priceLabel->setPosition(Vec2(cardWidth / 2.0f, 75.0f));
  card->addChild(priceLabel);

  // 5. 购买按钮 (底部)
  auto buyBtn = Button::create("images/ui/Bar.png");
  buyBtn->setScale(0.5f);
  buyBtn->setTitleText("购买");
  buyBtn->setTitleFontSize(20);
  buyBtn->setTitleColor(Color3B::WHITE);
  buyBtn->setPosition(Vec2(cardWidth / 2.0f, 35.0f));
  buyBtn->addClickEventListener([this, item](Ref*) {
    if (_purchaseCallback) {
      bool ok = _purchaseCallback(item);
      if (ok) {
        this->removeFromParent();
      }
    }
  });
  card->addChild(buyBtn);

  return card;
}

bool ShopLayer::onBackgroundTouchBegan(Touch* touch, Event* event) {
  if (!_panel) {
    return true;
  }
  Vec2 location = touch->getLocation();
  Vec2 panelPos = _panel->convertToNodeSpace(location);
  Rect panelRect(0.0f, 0.0f, _panel->getContentSize().width,
                 _panel->getContentSize().height);
  if (!panelRect.containsPoint(panelPos)) {
    this->removeFromParent();
  }
  return true;
}

void ShopLayer::showMessage(const std::string& text) {
  if (_messageLabel) {
    _messageLabel->setString(text);
  }
}
