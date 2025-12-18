#include "JoinClansLayer.h"

#include "Utils/API/Clans/Clans.h"
#include "Utils/Profile/Profile.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace cocos2d::ui;

namespace {
const Color3B COLOR_BUTTON_BG(128, 128, 128);  // 灰色背景
const Color3B COLOR_BUTTON_BG_SELECTED(100, 100, 100);  // 选中时的深灰色背景
const Color3B COLOR_LIST_ITEM_BG(100, 100, 100);
const std::string FONT_NAME = "Arial";

Label* createLabel(const std::string& text, int fontSize,
                   const Color4B& color = Color4B::WHITE) {
  auto label = Label::createWithSystemFont(text, FONT_NAME, fontSize);
  label->setTextColor(color);
  return label;
}

// 创建圆角背景的 DrawNode
DrawNode* createRoundedBackground(const Size& size, float radius = 8.0f, bool isSelected = false) {
  auto drawNode = DrawNode::create();
  Color4F grayColor;
  if (isSelected) {
    grayColor = Color4F(100.0f / 255.0f, 100.0f / 255.0f, 100.0f / 255.0f, 1.0f);
  } else {
    grayColor = Color4F(128.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f, 1.0f);
  }

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

  _contentArea = nullptr;
  _overviewLabel = nullptr;
  _overviewBg = nullptr;
  _searchLabel = nullptr;
  _searchBg = nullptr;
  _searchTextField = nullptr;
  _searchConfirmButton = nullptr;
  _scrollView = nullptr;
  _listContainer = nullptr;
  _isOverviewSelected = true;  // 默认选中"概览"

  buildUI();
  return true;
}

void JoinClansLayer::buildUI() {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 创建内容区域
  _contentArea = Layer::create();
  _contentArea->setContentSize(Size(760.0f, 450.0f));
  this->addChild(_contentArea);

  // 两个按钮：概览、搜索
  float buttonHeight = 50.0f;
  float buttonWidth = 200.0f;
  float spacing = 20.0f;
  float topY = _contentArea->getContentSize().height - 30.0f;
  float totalWidth = 2 * buttonWidth + spacing;
  float startX = (_contentArea->getContentSize().width - totalWidth) / 2.0f;
  float radius = 8.0f;

  // "概览" 按钮
  float overviewCenterX = startX + buttonWidth / 2.0f;
  float overviewCenterY = topY - buttonHeight / 2.0f;
  _overviewBg = createRoundedBackground(Size(buttonWidth, buttonHeight), radius, true);  // 默认选中
  _overviewBg->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
  _overviewBg->setPosition(Vec2(overviewCenterX - buttonWidth / 2.0f, overviewCenterY - buttonHeight / 2.0f));
  _contentArea->addChild(_overviewBg, 0);  // 背景在底层

  _overviewLabel = createLabel("概览", 28);
  _overviewLabel->setPosition(Vec2(overviewCenterX, overviewCenterY));
  _contentArea->addChild(_overviewLabel, 10);  // Label 在上层

  // 为"概览"添加触摸事件（注册到 contentArea 上，使用世界坐标检测）
  auto overviewListener = EventListenerTouchOneByOne::create();
  overviewListener->setSwallowTouches(true);
  overviewListener->onTouchBegan = [this, overviewCenterX, overviewCenterY, buttonWidth, buttonHeight](Touch* touch, Event* event) {
    Vec2 location = touch->getLocation();
    Vec2 contentPos = _contentArea->convertToNodeSpace(location);
    // 检查是否在概览按钮区域内
    Rect overviewRect(overviewCenterX - buttonWidth / 2.0f, 
                      overviewCenterY - buttonHeight / 2.0f, 
                      buttonWidth, buttonHeight);
    if (overviewRect.containsPoint(contentPos)) {
      return true;
    }
    return false;
  };
  overviewListener->onTouchEnded = [this](Touch* touch, Event* event) {
    this->onOverviewClick();
  };
  _eventDispatcher->addEventListenerWithSceneGraphPriority(overviewListener, _contentArea);

  // "搜索" 按钮
  float searchCenterX = startX + buttonWidth + spacing + buttonWidth / 2.0f;
  float searchCenterY = topY - buttonHeight / 2.0f;
  _searchBg = createRoundedBackground(Size(buttonWidth, buttonHeight), radius, false);  // 默认未选中
  _searchBg->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
  _searchBg->setPosition(Vec2(searchCenterX - buttonWidth / 2.0f, searchCenterY - buttonHeight / 2.0f));
  _contentArea->addChild(_searchBg, 0);  // 背景在底层

  _searchLabel = createLabel("搜索", 28);
  _searchLabel->setPosition(Vec2(searchCenterX, searchCenterY));
  _contentArea->addChild(_searchLabel, 10);  // Label 在上层

  // 为"搜索"添加触摸事件（注册到 contentArea 上，使用世界坐标检测）
  auto searchListener = EventListenerTouchOneByOne::create();
  searchListener->setSwallowTouches(true);
  searchListener->onTouchBegan = [this, searchCenterX, searchCenterY, buttonWidth, buttonHeight](Touch* touch, Event* event) {
    Vec2 location = touch->getLocation();
    Vec2 contentPos = _contentArea->convertToNodeSpace(location);
    // 检查是否在搜索按钮区域内
    Rect searchRect(searchCenterX - buttonWidth / 2.0f, 
                    searchCenterY - buttonHeight / 2.0f, 
                    buttonWidth, buttonHeight);
    if (searchRect.containsPoint(contentPos)) {
      return true;
    }
    return false;
  };
  searchListener->onTouchEnded = [this](Touch* touch, Event* event) {
    this->onSearchClick();
  };
  _eventDispatcher->addEventListenerWithSceneGraphPriority(searchListener, _contentArea);
  
  // 默认显示概览列表
  showOverviewList();
}

void JoinClansLayer::onOverviewClick() {
  if (_isOverviewSelected) {
    return;  // 已经选中，不需要切换
  }

  _isOverviewSelected = true;
  updateTabSelection(true);

  // 隐藏搜索UI
  if (_searchTextField) {
    _searchTextField->setVisible(false);
  }
  if (_searchConfirmButton) {
    _searchConfirmButton->setVisible(false);
  }

  // 显示概览列表
  showOverviewList();
}

void JoinClansLayer::onSearchClick() {
  if (!_isOverviewSelected) {
    return;  // 已经选中，不需要切换
  }

  _isOverviewSelected = false;
  updateTabSelection(false);

  // 显示搜索UI
  showSearchUI();
}

void JoinClansLayer::showOverviewList() {
  // 清除现有列表（参考 AttackLayer 的方式）
  if (_scrollView) {
    _scrollView->removeFromParent();
    _scrollView = nullptr;
  }
  _listContainer = nullptr;

  // 创建滚动视图，位置在按钮下方
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
  
  // 创建列表容器（不再使用，直接添加到 ScrollView）
  _listContainer = nullptr;

  // 调用API获取所有部落
  Clans::getAllClansInfo([this](const ClansListResult& result) {
    if (result.success) {
      this->displayClansList(result.clans);
    } else {
      // 显示错误消息
      auto errorLabel = createLabel("获取失败: " + result.message, 24,
                                    Color4B::RED);
      errorLabel->setPosition(
          Vec2(_contentArea->getContentSize().width / 2.0f,
               _contentArea->getContentSize().height / 2.0f));
      _contentArea->addChild(errorLabel);
    }
  });
}

void JoinClansLayer::showSearchUI() {
  // 清除现有列表（参考 AttackLayer 的方式）
  if (_scrollView) {
    _scrollView->removeFromParent();
    _scrollView = nullptr;
  }
  _listContainer = nullptr;

  // 创建搜索框
  if (!_searchTextField) {
    _searchTextField = TextField::create("输入部落名称", FONT_NAME, 24);
    _searchTextField->setContentSize(Size(400.0f, 40.0f));
    _searchTextField->setPosition(
        Vec2(_contentArea->getContentSize().width / 2.0f - 60.0f,
             _contentArea->getContentSize().height - 120.0f));
    _contentArea->addChild(_searchTextField);
  }
  _searchTextField->setVisible(true);
  _searchTextField->setString("");

  // 创建确认搜索按钮
  if (!_searchConfirmButton) {
    _searchConfirmButton = Button::create();
    _searchConfirmButton->setTitleText("确认搜索");
    _searchConfirmButton->setTitleFontSize(24);
    _searchConfirmButton->setContentSize(Size(120.0f, 40.0f));
    _searchConfirmButton->setPosition(
        Vec2(_contentArea->getContentSize().width / 2.0f + 200.0f,
             _contentArea->getContentSize().height - 120.0f));
    _searchConfirmButton->addTouchEventListener(
        [this](Ref* sender, Widget::TouchEventType type) {
          if (type == Widget::TouchEventType::ENDED) {
            this->onSearchConfirm();
          }
        });
    _contentArea->addChild(_searchConfirmButton);
  }
  _searchConfirmButton->setVisible(true);
}

void JoinClansLayer::onSearchConfirm() {
  if (!_searchTextField) {
    return;
  }

  std::string keyword = _searchTextField->getString();
  if (keyword.empty()) {
    return;
  }

  // 清除现有列表（参考 AttackLayer 的方式）
  if (_scrollView) {
    _scrollView->removeFromParent();
    _scrollView = nullptr;
  }
  _listContainer = nullptr;

  // 创建滚动视图，位置在按钮下方
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
  
  // 创建列表容器（不再使用，直接添加到 ScrollView）
  _listContainer = nullptr;

  // 调用API搜索部落
  Clans::searchClans(keyword, [this](const ClansListResult& result) {
    if (result.success) {
      this->displayClansList(result.clans);
    } else {
      // 显示错误消息
      auto errorLabel = createLabel("搜索失败: " + result.message, 24,
                                    Color4B::RED);
      errorLabel->setPosition(
          Vec2(_contentArea->getContentSize().width / 2.0f,
               _contentArea->getContentSize().height / 2.0f));
      _contentArea->addChild(errorLabel);
    }
  });
}

void JoinClansLayer::displayClansList(const std::vector<ClanInfo>& clans) {
  if (!_scrollView) {
    return;
  }

  // 清除现有内容（参考 AttackLayer 的方式）
  _scrollView->removeAllChildren();

  if (clans.empty()) {
    auto emptyLabel = createLabel("没有找到部落", 24, Color4B::WHITE);
    emptyLabel->setPosition(
        Vec2(_scrollView->getContentSize().width / 2.0f,
             _scrollView->getContentSize().height / 2.0f));
    _scrollView->addChild(emptyLabel);
    _scrollView->setInnerContainerSize(_scrollView->getContentSize());
    return;
  }

  // 计算列表总高度（参考 AttackLayer 的方式）
  float itemHeight = 60.0f;
  float spacing = 10.0f;
  int clanCount = clans.size();
  float innerHeight = (itemHeight + spacing) * clanCount;
  if (innerHeight < _scrollView->getContentSize().height) {
    innerHeight = _scrollView->getContentSize().height;
  }

  _scrollView->setInnerContainerSize(
      Size(_scrollView->getContentSize().width, innerHeight));

  // 创建列表项（参考 AttackLayer 的位置计算方式）
  for (size_t i = 0; i < clans.size(); i++) {
    auto item = createClanItem(clans[i]);
    item->setPosition(Vec2(_scrollView->getContentSize().width / 2.0f,
                          innerHeight - (i + 0.5f) * (itemHeight + spacing)));
    _scrollView->addChild(item);
  }
}

cocos2d::ui::Widget* JoinClansLayer::createClanItem(const ClanInfo& clan) {
  // 参考 AttackLayer 的 createLevelItem 方式
  float width = 700.0f;
  float height = 60.0f;
  
  auto container = Layout::create();
  container->setContentSize(Size(width, height));
  container->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
  container->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
  container->setBackGroundColor(Color3B(100, 100, 100));

  // 部落名称（参考 AttackLayer 的布局方式）
  auto nameLabel = createLabel(clan.name, 24, Color4B::WHITE);
  nameLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
  nameLabel->setPosition(Vec2(20.0f, height / 2.0f));
  container->addChild(nameLabel);

  // 成员数量
  std::string memberText = "成员: " + std::to_string(clan.member_count);
  auto memberLabel = createLabel(memberText, 20, Color4B(200, 200, 200, 255));
  memberLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_RIGHT);
  
  // 创建"加入"按钮
  auto joinButton = Button::create();
  joinButton->setTitleText("加入");
  joinButton->setTitleFontSize(20);
  joinButton->setContentSize(Size(80.0f, 40.0f));
  joinButton->setPosition(Vec2(width - 60.0f, height / 2.0f));
  
  // 调整成员数量标签位置，为按钮留出空间
  memberLabel->setPosition(Vec2(width - 160.0f, height / 2.0f));
  container->addChild(memberLabel);
  
  // 为"加入"按钮添加点击事件
  joinButton->addTouchEventListener([this, clan](Ref* sender, Widget::TouchEventType type) {
    if (type == Widget::TouchEventType::ENDED) {
      this->onJoinClanClick(clan);
    }
  });
  container->addChild(joinButton);

  return container;
}

void JoinClansLayer::updateTabSelection(bool isOverview) {
  float buttonWidth = 200.0f;
  float buttonHeight = 50.0f;
  float radius = 8.0f;
  
  if (isOverview) {
    // "概览" 选中，"搜索" 未选中
    // 移除旧的背景并创建新的
    if (_overviewBg) {
      Vec2 oldPos = _overviewBg->getPosition();
      int oldZOrder = _overviewBg->getLocalZOrder();
      _overviewBg->removeFromParent();
      _overviewBg = createRoundedBackground(Size(buttonWidth, buttonHeight), radius, true);
      _overviewBg->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
      _overviewBg->setPosition(oldPos);
      _contentArea->addChild(_overviewBg, oldZOrder);  // 使用原来的 z-order
    }
    
    if (_searchBg) {
      Vec2 oldPos = _searchBg->getPosition();
      int oldZOrder = _searchBg->getLocalZOrder();
      _searchBg->removeFromParent();
      _searchBg = createRoundedBackground(Size(buttonWidth, buttonHeight), radius, false);
      _searchBg->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
      _searchBg->setPosition(oldPos);
      _contentArea->addChild(_searchBg, oldZOrder);  // 使用原来的 z-order
    }
  } else {
    // "搜索" 选中，"概览" 未选中
    // 移除旧的背景并创建新的
    if (_overviewBg) {
      Vec2 oldPos = _overviewBg->getPosition();
      int oldZOrder = _overviewBg->getLocalZOrder();
      _overviewBg->removeFromParent();
      _overviewBg = createRoundedBackground(Size(buttonWidth, buttonHeight), radius, false);
      _overviewBg->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
      _overviewBg->setPosition(oldPos);
      _contentArea->addChild(_overviewBg, oldZOrder);  // 使用原来的 z-order
    }
    
    if (_searchBg) {
      Vec2 oldPos = _searchBg->getPosition();
      int oldZOrder = _searchBg->getLocalZOrder();
      _searchBg->removeFromParent();
      _searchBg = createRoundedBackground(Size(buttonWidth, buttonHeight), radius, true);
      _searchBg->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
      _searchBg->setPosition(oldPos);
      _contentArea->addChild(_searchBg, oldZOrder);  // 使用原来的 z-order
    }
  }
  
  // 确保 Label 在背景之上（提高 z-order）
  if (_overviewLabel) {
    _overviewLabel->setLocalZOrder(10);
  }
  if (_searchLabel) {
    _searchLabel->setLocalZOrder(10);
  }
}

void JoinClansLayer::onJoinClanClick(const ClanInfo& clan) {
  // 获取当前用户ID
  auto profile = Profile::getInstance();
  if (!profile) {
    CCLOG("JoinClansLayer: Profile is null");
    return;
  }

  // 检查是否已经加入部落
  if (profile->getClansId() != -1) {
    CCLOG("JoinClansLayer: Had joined a clan, need to leave current clan first");
    // 显示错误消息
    auto errorLabel = createLabel("已经加入部落，需要先退出当前部落", 24, Color4B::RED);
    errorLabel->setPosition(
        Vec2(_contentArea->getContentSize().width / 2.0f,
             _contentArea->getContentSize().height / 2.0f));
    _contentArea->addChild(errorLabel, 1000);
    // 3秒后移除提示
    auto delay = DelayTime::create(3.0f);
    auto remove = CallFunc::create([errorLabel]() {
      errorLabel->removeFromParent();
    });
    errorLabel->runAction(Sequence::create(delay, remove, nullptr));
    return;
  }
  
  int userId = profile->getId();
  if (userId <= 0) {
    CCLOG("JoinClansLayer: Invalid user ID");
    return;
  }
  
  // 检查部落ID是否有效
  if (clan.id.empty()) {
    CCLOG("JoinClansLayer: Clan ID is empty");
    return;
  }
  
  // 调用API加入部落
  Clans::joinClan(clan.id, userId, [this, clan](bool success, const std::string& message) {
    if (success) {
      CCLOG("JoinClansLayer: Successfully joined clan %s", clan.name.c_str());
      // 更新Profile中的部落信息
      auto profile = Profile::getInstance();
      if (profile) {
        // 假设clan.id是字符串，需要转换为int（如果服务器返回的是数字字符串）
        try {
          int clanId = std::stoi(clan.id);
          profile->setClansId(clanId);
        } catch (...) {
          // 如果转换失败，使用默认值或跳过
          CCLOG("JoinClansLayer: Failed to convert clan ID to int: %s", clan.id.c_str());
        }
        profile->setClansName(clan.name);
      }
      
      // 显示成功消息（可以添加一个提示标签）
      auto successLabel = createLabel("加入成功: " + clan.name, 24, Color4B::GREEN);
      successLabel->setPosition(
          Vec2(_contentArea->getContentSize().width / 2.0f,
               _contentArea->getContentSize().height / 2.0f));
      _contentArea->addChild(successLabel, 1000);
      
      // 3秒后移除提示
      auto delay = DelayTime::create(3.0f);
      auto remove = CallFunc::create([successLabel]() {
        successLabel->removeFromParent();
      });
      successLabel->runAction(Sequence::create(delay, remove, nullptr));
    } else {
      CCLOG("JoinClansLayer: Failed to join clan: %s", message.c_str());
      // 显示错误消息
      auto errorLabel = createLabel("加入失败: " + message, 24, Color4B::RED);
      errorLabel->setPosition(
          Vec2(_contentArea->getContentSize().width / 2.0f,
               _contentArea->getContentSize().height / 2.0f));
      _contentArea->addChild(errorLabel, 1000);
      
      // 3秒后移除提示
      auto delay = DelayTime::create(3.0f);
      auto remove = CallFunc::create([errorLabel]() {
        errorLabel->removeFromParent();
      });
      errorLabel->runAction(Sequence::create(delay, remove, nullptr));
    }
  });
}