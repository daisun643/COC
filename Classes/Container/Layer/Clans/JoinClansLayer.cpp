#include "JoinClansLayer.h"

#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace cocos2d::ui;

namespace {
const Color3B COLOR_BUTTON_BG(128, 128, 128);  // 灰色背景
const std::string FONT_NAME = "Arial";

Label* createLabel(const std::string& text, int fontSize,
                   const Color4B& color = Color4B::WHITE) {
  auto label = Label::createWithSystemFont(text, FONT_NAME, fontSize);
  label->setTextColor(color);
  return label;
}

// 创建圆角背景的 DrawNode
DrawNode* createRoundedBackground(const Size& size, float radius = 8.0f) {
  auto drawNode = DrawNode::create();
  Color4F grayColor(128.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f, 1.0f);

  float width = size.width;
  float height = size.height;

  // 绘制中心矩形
  drawNode->drawSolidRect(Vec2(radius, radius),
                          Vec2(width - radius, height - radius), grayColor);

  // 绘制四个圆角
  drawNode->drawSolidCircle(Vec2(radius, radius), radius, 0, 20, grayColor);
  drawNode->drawSolidCircle(Vec2(width - radius, radius), radius, 0, 20,
                            grayColor);
  drawNode->drawSolidCircle(Vec2(radius, height - radius), radius, 0, 20,
                            grayColor);
  drawNode->drawSolidCircle(Vec2(width - radius, height - radius), radius, 0,
                            20, grayColor);

  // 绘制连接圆角的矩形
  drawNode->drawSolidRect(Vec2(radius, 0), Vec2(width - radius, height),
                          grayColor);
  drawNode->drawSolidRect(Vec2(0, radius), Vec2(width, height - radius),
                          grayColor);

  return drawNode;
}
}  // namespace

JoinClansLayer* JoinClansLayer::create() {
  JoinClansLayer* layer = new (std::nothrow) JoinClansLayer();
  if (layer && layer->init()) {
    layer->autorelease();
    return layer;
  }
  CC_SAFE_DELETE(layer);
  return nullptr;
}

bool JoinClansLayer::init() {
  if (!Layer::init()) {
    return false;
  }

  buildUI();
  return true;
}

void JoinClansLayer::buildUI() {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 创建内容区域
  auto contentArea = Layer::create();
  contentArea->setContentSize(Size(760.0f, 450.0f));
  this->addChild(contentArea);

  // 两个按钮：概览、搜索
  float buttonHeight = 50.0f;
  float buttonWidth = 200.0f;
  float spacing = 20.0f;
  float topY = contentArea->getContentSize().height - 30.0f;
  float totalWidth = 2 * buttonWidth + spacing;
  float startX = (contentArea->getContentSize().width - totalWidth) / 2.0f;
  float radius = 8.0f;

  // "概览" 按钮
  float overviewCenterX = startX + buttonWidth / 2.0f;
  float overviewCenterY = topY - buttonHeight / 2.0f;
  auto overviewBg = createRoundedBackground(Size(buttonWidth, buttonHeight), radius);
  overviewBg->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
  overviewBg->setPosition(Vec2(overviewCenterX - buttonWidth / 2.0f, overviewCenterY - buttonHeight / 2.0f));
  contentArea->addChild(overviewBg);

  auto overviewLabel = createLabel("概览", 28);
  overviewLabel->setPosition(Vec2(overviewCenterX, overviewCenterY));
  contentArea->addChild(overviewLabel);

  // "搜索" 按钮
  float searchCenterX = startX + buttonWidth + spacing + buttonWidth / 2.0f;
  float searchCenterY = topY - buttonHeight / 2.0f;
  auto searchBg = createRoundedBackground(Size(buttonWidth, buttonHeight), radius);
  searchBg->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
  searchBg->setPosition(Vec2(searchCenterX - buttonWidth / 2.0f, searchCenterY - buttonHeight / 2.0f));
  contentArea->addChild(searchBg);

  auto searchLabel = createLabel("搜索", 28);
  searchLabel->setPosition(Vec2(searchCenterX, searchCenterY));
  contentArea->addChild(searchLabel);
}

