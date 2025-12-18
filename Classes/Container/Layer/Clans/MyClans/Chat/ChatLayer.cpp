#include "ChatLayer.h"

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

ChatLayer* ChatLayer::create() {
  ChatLayer* layer = new (std::nothrow) ChatLayer();
  if (layer && layer->init()) {
    layer->autorelease();
    return layer;
  }
  CC_SAFE_DELETE(layer);
  return nullptr;
}

bool ChatLayer::init() {
  if (!Layer::init()) {
    return false;
  }

  _contentArea = nullptr;
  _scrollView = nullptr;
  _inputTextField = nullptr;
  _sendButton = nullptr;

  buildUI();
  return true;
}

void ChatLayer::buildUI() {
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

  // 用户已加入部落，创建聊天界面
  // 参考 JoinClansLayer 的布局方式，为顶部按钮留出空间
  float buttonHeight = 50.0f;
  float topY = _contentArea->getContentSize().height - 30.0f;
  float buttonBottomY = topY - buttonHeight - 20.0f;  // 按钮下方留20像素间距
  
  // 创建输入区域（底部）
  float inputHeight = 50.0f;
  float inputY = 10.0f;

  // 输入框
  _inputTextField = TextField::create("输入消息...", FONT_NAME, 20);
  _inputTextField->setContentSize(Size(600.0f, 40.0f));
  _inputTextField->setPosition(Vec2(320.0f, inputY + inputHeight / 2.0f));
  _contentArea->addChild(_inputTextField);

  // 发送按钮
  _sendButton = Button::create();
  _sendButton->setTitleText("发送");
  _sendButton->setTitleFontSize(20);
  _sendButton->setContentSize(Size(100.0f, 40.0f));
  _sendButton->setPosition(Vec2(680.0f, inputY + inputHeight / 2.0f));
  _sendButton->addTouchEventListener([this](Ref* sender, Widget::TouchEventType type) {
    if (type == Widget::TouchEventType::ENDED) {
      this->onSendButtonClick();
    }
  });
  _contentArea->addChild(_sendButton);

  // 创建消息列表区域（在按钮下方和输入框上方之间）
  // 从按钮下方到底部输入框上方，留出间距
  float scrollViewHeight = buttonBottomY - inputHeight - 20.0f;  // 按钮下方到输入框上方，留20像素间距

  _scrollView = ScrollView::create();
  _scrollView->setContentSize(Size(_contentArea->getContentSize().width, scrollViewHeight));
  _scrollView->setPosition(Vec2(0.0f, inputHeight + 10.0f));  // 从底部输入框上方开始，留10像素间距
  _scrollView->setDirection(ScrollView::Direction::VERTICAL);
  _scrollView->setBounceEnabled(true);
  _contentArea->addChild(_scrollView);

  // 加载消息
  loadMessages();
}

void ChatLayer::loadMessages() {
  auto profile = Profile::getInstance();
  if (!profile) {
    return;
  }

  int clanId = profile->getClansId();
  std::string clanIdStr = std::to_string(clanId);

  // 调用API获取聊天消息（不限制条数，获取所有消息）
  Clans::getClanChatMessages(clanIdStr, 0, [this](bool success, const std::string& message,
                                                   const std::vector<Clans::ChatMessage>& messages,
                                                   int count) {
    if (success) {
      this->displayMessagesList(messages);
    } else {
      // 显示错误消息
      if (_scrollView) {
        _scrollView->removeAllChildren();
        auto errorLabel = createLabel("获取失败: " + message, 24, Color4B::RED);
        errorLabel->setPosition(
            Vec2(_scrollView->getContentSize().width / 2.0f,
                 _scrollView->getContentSize().height / 2.0f));
        _scrollView->addChild(errorLabel);
        _scrollView->setInnerContainerSize(_scrollView->getContentSize());
      }
    }
  });
}

void ChatLayer::displayMessagesList(const std::vector<Clans::ChatMessage>& messages) {
  if (!_scrollView) {
    return;
  }

  // 清除现有内容
  _scrollView->removeAllChildren();

  if (messages.empty()) {
    auto emptyLabel = createLabel("暂无消息", 24, Color4B::WHITE);
    emptyLabel->setPosition(
        Vec2(_scrollView->getContentSize().width / 2.0f,
             _scrollView->getContentSize().height / 2.0f));
    _scrollView->addChild(emptyLabel);
    _scrollView->setInnerContainerSize(_scrollView->getContentSize());
    return;
  }

  // 计算列表总高度
  float itemHeight = 80.0f;  // 消息项高度，需要显示发送者、内容和时间
  float spacing = 10.0f;
  int messageCount = messages.size();
  float innerHeight = (itemHeight + spacing) * messageCount;
  if (innerHeight < _scrollView->getContentSize().height) {
    innerHeight = _scrollView->getContentSize().height;
  }

  _scrollView->setInnerContainerSize(
      Size(_scrollView->getContentSize().width, innerHeight));

  // 创建列表项（消息已按时间降序排序，最新的在前）
  for (size_t i = 0; i < messages.size(); i++) {
    auto item = createMessageItem(messages[i]);
    item->setPosition(Vec2(_scrollView->getContentSize().width / 2.0f,
                           innerHeight - (i + 0.5f) * (itemHeight + spacing)));
    _scrollView->addChild(item);
  }

  // 滚动到底部（显示最新消息）
  _scrollView->scrollToBottom(0.1f, false);
}

cocos2d::ui::Widget* ChatLayer::createMessageItem(const Clans::ChatMessage& msg) {
  float width = 700.0f;
  float height = 80.0f;

  auto container = Layout::create();
  container->setContentSize(Size(width, height));
  container->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
  container->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
  container->setBackGroundColor(COLOR_LIST_ITEM_BG);

  // 发送者名称（顶部左侧）
  auto senderLabel = createLabel(msg.sender, 18, Color4B(200, 200, 255, 255));
  senderLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
  senderLabel->setPosition(Vec2(10.0f, height - 15.0f));
  container->addChild(senderLabel);

  // 时间（顶部右侧）
  auto timeLabel = createLabel(msg.time, 14, Color4B(150, 150, 150, 255));
  timeLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_RIGHT);
  timeLabel->setPosition(Vec2(width - 10.0f, height - 15.0f));
  container->addChild(timeLabel);

  // 消息内容（中间）
  auto contentLabel = createLabel(msg.content, 18, Color4B::WHITE);
  contentLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
  contentLabel->setPosition(Vec2(10.0f, height / 2.0f - 10.0f));
  // 设置最大宽度，超出部分自动换行
  contentLabel->setDimensions(width - 20.0f, 0);
  contentLabel->setAlignment(TextHAlignment::LEFT);
  container->addChild(contentLabel);

  return container;
}

void ChatLayer::onSendButtonClick() {
  if (!_inputTextField) {
    return;
  }

  std::string content = _inputTextField->getString();
  if (content.empty() || content.find_first_not_of(" \t\n\r") == std::string::npos) {
    return;  // 消息为空或只有空白字符
  }

  auto profile = Profile::getInstance();
  if (!profile) {
    return;
  }

  int clanId = profile->getClansId();
  int userId = profile->getId();
  std::string clanIdStr = std::to_string(clanId);

  // 清空输入框
  _inputTextField->setString("");

  // 调用API发送消息
  Clans::sendClanChatMessage(clanIdStr, userId, content, [this](bool success, const std::string& message) {
    if (success) {
      // 发送成功，刷新消息列表
      this->refreshMessages();
    } else {
      // 发送失败，显示错误提示（可以添加一个提示标签）
      CCLOG("ChatLayer: Failed to send message: %s", message.c_str());
    }
  });
}

void ChatLayer::refreshMessages() {
  // 重新加载消息列表
  loadMessages();
}

