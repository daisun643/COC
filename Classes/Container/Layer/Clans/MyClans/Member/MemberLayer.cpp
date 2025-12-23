#include "MemberLayer.h"

#include "Utils/API/Clans/Clans.h"
#include "Utils/Profile/Profile.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace cocos2d::ui;

namespace {
const Color3B COLOR_LIST_ITEM_BG(100, 100, 100);
const std::string FONT_NAME = "Arial";

Label* createLabel(const std::string& text, int fontSize,
                   const Color4B& color = Color4B::WHITE) {
  auto label = Label::createWithSystemFont(text, FONT_NAME, fontSize);
  label->setTextColor(color);
  return label;
}
}  // namespace

MemberLayer* MemberLayer::create() {
  MemberLayer* layer = new (std::nothrow) MemberLayer();
  if (layer && layer->init()) {
    layer->autorelease();
    return layer;
  }
  CC_SAFE_DELETE(layer);
  return nullptr;
}

bool MemberLayer::init() {
  if (!Layer::init()) {
    return false;
  }

  _contentArea = nullptr;
  _scrollView = nullptr;

  buildUI();
  return true;
}

void MemberLayer::buildUI() {
  // 创建内容区域
  _contentArea = Layer::create();
  _contentArea->setContentSize(Size(760.0f, 450.0f));
  this->addChild(_contentArea);

  // 检查用户是否在部落中
  auto profile = Profile::getInstance();
  if (!profile || profile->getClansId() <= 0) {
    // 用户尚未加入部落，显示提示信息
    auto tipLabel = createLabel("你尚未加入一个部落", 28, Color4B::WHITE);
    tipLabel->setPosition(
        Vec2(_contentArea->getContentSize().width / 2.0f,
             _contentArea->getContentSize().height / 2.0f));
    _contentArea->addChild(tipLabel);
    return;
  }

  // 用户已加入部落，加载成员列表
  loadMembers();
}

void MemberLayer::loadMembers() {
  auto profile = Profile::getInstance();
  if (!profile) {
    return;
  }

  int clanId = profile->getClansId();
  std::string clanIdStr = std::to_string(clanId);

  // 调用API获取部落成员
  Clans::getClanMembers(clanIdStr, [this](bool success, const std::string& message,
                                           const std::vector<std::string>& members) {
    if (success) {
      this->displayMembersList(members);
    } else {
      // 显示错误消息
      if (_scrollView) {
        _scrollView->removeFromParent();
        _scrollView = nullptr;
      }
      auto errorLabel = createLabel("获取失败: " + message, 24, Color4B::RED);
      errorLabel->setPosition(
          Vec2(_contentArea->getContentSize().width / 2.0f,
               _contentArea->getContentSize().height / 2.0f));
      _contentArea->addChild(errorLabel);
    }
  });
}

void MemberLayer::displayMembersList(const std::vector<std::string>& members) {
  // 清除现有列表
  if (_scrollView) {
    _scrollView->removeFromParent();
    _scrollView = nullptr;
  }

  // 参考 JoinClansLayer 的布局方式，为顶部按钮留出空间
  float buttonHeight = 50.0f;
  float topY = _contentArea->getContentSize().height - 30.0f;
  float buttonBottomY = topY - buttonHeight - 20.0f;  // 按钮下方留20像素间距
  float scrollViewHeight = buttonBottomY - 20.0f;  // 从按钮下方到底部，底部留20像素间距

  _scrollView = ScrollView::create();
  _scrollView->setContentSize(Size(_contentArea->getContentSize().width, scrollViewHeight));
  _scrollView->setPosition(Vec2(0.0f, 20.0f));  // 从底部开始，留20像素间距
  _scrollView->setDirection(ScrollView::Direction::VERTICAL);
  _scrollView->setBounceEnabled(true);
  _contentArea->addChild(_scrollView);

  if (members.empty()) {
    auto emptyLabel = createLabel("没有成员", 24, Color4B::WHITE);
    emptyLabel->setPosition(
        Vec2(_scrollView->getContentSize().width / 2.0f,
             _scrollView->getContentSize().height / 2.0f));
    _scrollView->addChild(emptyLabel);
    _scrollView->setInnerContainerSize(_scrollView->getContentSize());
    return;
  }

  // 计算列表总高度
  float itemHeight = 50.0f;
  float spacing = 10.0f;
  int memberCount = members.size();
  float innerHeight = (itemHeight + spacing) * memberCount;
  if (innerHeight < _scrollView->getContentSize().height) {
    innerHeight = _scrollView->getContentSize().height;
  }

  _scrollView->setInnerContainerSize(
      Size(_scrollView->getContentSize().width, innerHeight));

  // 创建列表项
  for (size_t i = 0; i < members.size(); i++) {
    auto item = createMemberItem(members[i]);
    item->setPosition(Vec2(_scrollView->getContentSize().width / 2.0f,
                           innerHeight - (i + 0.5f) * (itemHeight + spacing)));
    _scrollView->addChild(item);
  }
}

cocos2d::ui::Widget* MemberLayer::createMemberItem(const std::string& memberName) {
  float width = 700.0f;
  float height = 50.0f;

  auto container = Layout::create();
  container->setContentSize(Size(width, height));
  container->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
  container->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
  container->setBackGroundColor(COLOR_LIST_ITEM_BG);

  // 成员名称
  auto nameLabel = createLabel(memberName, 22, Color4B::WHITE);
  nameLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
  nameLabel->setPosition(Vec2(20.0f, height / 2.0f));
  container->addChild(nameLabel);

  return container;
}

