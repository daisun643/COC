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
  _createClanLabel = nullptr;
  _createClanBg = nullptr;
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

  // 三个按钮：概览、搜索、创建部落
  float buttonHeight = 50.0f;
  float buttonWidth = 140.0f;  // 缩小按钮宽度以容纳三个按钮
  float spacing = 20.0f;
  float topY = _contentArea->getContentSize().height - 30.0f;
  float totalWidth = 3 * buttonWidth + 2 * spacing;  // 三个按钮，两个间距
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
  
  // "创建部落" 按钮（与"概览"和"搜索"并排）
  float createClanCenterX = startX + 2 * buttonWidth + 2 * spacing + buttonWidth / 2.0f;
  float createClanCenterY = topY - buttonHeight / 2.0f;
  _createClanBg = createRoundedBackground(Size(buttonWidth, buttonHeight), radius, false);
  _createClanBg->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
  _createClanBg->setPosition(Vec2(createClanCenterX - buttonWidth / 2.0f, createClanCenterY - buttonHeight / 2.0f));
  _contentArea->addChild(_createClanBg, 0);
  
  _createClanLabel = createLabel("创建部落", 28);
  _createClanLabel->setPosition(Vec2(createClanCenterX, createClanCenterY));
  _contentArea->addChild(_createClanLabel, 10);
  
  // 为"创建部落"按钮添加触摸事件
  auto createClanListener = EventListenerTouchOneByOne::create();
  createClanListener->setSwallowTouches(true);
  createClanListener->onTouchBegan = [this, createClanCenterX, createClanCenterY, buttonWidth, buttonHeight](Touch* touch, Event* event) {
    Vec2 location = touch->getLocation();
    Vec2 contentPos = _contentArea->convertToNodeSpace(location);
    Rect createClanRect(createClanCenterX - buttonWidth / 2.0f, 
                       createClanCenterY - buttonHeight / 2.0f, 
                       buttonWidth, buttonHeight);
    if (createClanRect.containsPoint(contentPos)) {
      // 要判断 profile->getClanId() == -1 才能创建部落
      if (Profile::getInstance()->getClansId() != -1) {
        // 提示用户不能创建部落
        CCLOG("JoinClansLayer: Already in a clan, cannot create new clan");
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
        return false;
      }
      return true;
    }
    return false;
  };
  createClanListener->onTouchEnded = [this](Touch* touch, Event* event) {
    this->onCreateClanClick();
  };
  _eventDispatcher->addEventListenerWithSceneGraphPriority(createClanListener, _contentArea);
  
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
  float buttonWidth = 140.0f;  // 与 buildUI 中的 buttonWidth 保持一致
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

void JoinClansLayer::onCreateClanClick() {
  showCreateClanDialog();
}

void JoinClansLayer::showCreateClanDialog() {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 创建半透明背景层
  auto bgLayer = LayerColor::create(Color4B(0, 0, 0, 0), visibleSize.width, visibleSize.height);
  bgLayer->setPosition(origin);
  bgLayer->setName("CreateClanDialogBackground");
  this->addChild(bgLayer, 1000);

  // 创建对话框背景
  float dialogWidth = 400.0f;
  float dialogHeight = 250.0f;
  auto dialogBg = LayerColor::create(Color4B(50, 50, 50, 255), dialogWidth, dialogHeight);
  dialogBg->setAnchorPoint(Vec2::ANCHOR_MIDDLE);  // 设置锚点为中心
  dialogBg->setPosition(Vec2(origin.x + dialogWidth / 2, origin.y + 50));  // 屏幕中心
  dialogBg->setName("CreateClanDialog");
  this->addChild(dialogBg, 1001);

  // 创建标题标签
  auto titleLabel = createLabel("创建部落", 24);
  titleLabel->setColor(Color3B::WHITE);
  titleLabel->setPosition(Vec2(dialogWidth / 2.0f, dialogHeight - 50.0f));
  dialogBg->addChild(titleLabel);

  // 创建输入框标签
  auto nameLabel = createLabel("部落名称:", 18);
  nameLabel->setColor(Color3B::WHITE);
  nameLabel->setPosition(Vec2(50.0f, dialogHeight - 100.0f));
  nameLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
  dialogBg->addChild(nameLabel);

  // 创建输入框
  auto textField = TextField::create("", FONT_NAME, 20);
  textField->setPlaceHolder("请输入部落名称");
  textField->setPlaceHolderColor(Color4B::GRAY);
  textField->setMaxLength(20);
  textField->setContentSize(Size(300.0f, 40.0f));
  textField->setPosition(Vec2(dialogWidth / 2.0f, dialogHeight - 130.0f));
  textField->setTextColor(Color4B::WHITE);
  dialogBg->addChild(textField);

  // 创建确定按钮
  float btnWidth = 80.0f;
  float btnHeight = 35.0f;
  float confirmBtnX = dialogWidth / 2.0f - btnWidth - 10.0f;
  float confirmBtnY = 30.0f;
  auto confirmBtnBg = LayerColor::create(Color4B(100, 150, 100, 255), btnWidth, btnHeight);
  confirmBtnBg->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
  confirmBtnBg->setPosition(Vec2(confirmBtnX, confirmBtnY));
  auto confirmLabel = createLabel("确定", 18);
  confirmLabel->setPosition(Vec2(btnWidth / 2.0f, btnHeight / 2.0f));
  confirmBtnBg->addChild(confirmLabel);
  dialogBg->addChild(confirmBtnBg);

  // 创建取消按钮
  float cancelBtnX = dialogWidth / 2.0f + 10.0f;
  float cancelBtnY = 30.0f;
  auto cancelBtnBg = LayerColor::create(Color4B(150, 100, 100, 255), btnWidth, btnHeight);
  cancelBtnBg->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
  cancelBtnBg->setPosition(Vec2(cancelBtnX, cancelBtnY));
  auto cancelLabel = createLabel("取消", 18);
  cancelLabel->setPosition(Vec2(btnWidth / 2.0f, btnHeight / 2.0f));
  cancelBtnBg->addChild(cancelLabel);
  dialogBg->addChild(cancelBtnBg);

  // 添加按钮点击事件
  auto listener = EventListenerTouchOneByOne::create();
  listener->setSwallowTouches(true);
  listener->onTouchBegan = [this, bgLayer, dialogBg, textField, confirmBtnBg, cancelBtnBg,
                             confirmBtnX, confirmBtnY, cancelBtnX, cancelBtnY,
                             btnWidth, btnHeight](Touch* touch, Event* event) -> bool {
    Vec2 location = touch->getLocation();
    // 将触摸位置转换为相对于 dialogBg 的坐标
    Vec2 dialogLocalPos = dialogBg->convertToNodeSpace(location);

    // 检查是否点击了确定按钮
    Rect confirmRect(confirmBtnX, confirmBtnY, btnWidth, btnHeight);
    if (confirmRect.containsPoint(dialogLocalPos)) {
      // 获取输入的部落名称
      std::string clanName = textField->getString();
      if (clanName.empty() || clanName.find_first_not_of(' ') == std::string::npos) {
        // 部落名称为空或只有空格，显示错误提示
        auto errorLabel = createLabel("部落名称不能为空", 18, Color4B::RED);
        errorLabel->setPosition(Vec2(dialogBg->getContentSize().width / 2.0f, 80.0f));
        dialogBg->addChild(errorLabel, 100);
        auto delay = DelayTime::create(2.0f);
        auto remove = CallFunc::create([errorLabel]() {
          errorLabel->removeFromParent();
        });
        errorLabel->runAction(Sequence::create(delay, remove, nullptr));
        return true;
      }

      // 移除对话框
      this->removeChild(bgLayer);
      this->removeChild(dialogBg);

      // 调用创建部落 API
      auto profile = Profile::getInstance();
      if (!profile || profile->getId() <= 0) {
        CCLOG("JoinClansLayer: User not logged in");
        return true;
      }

      int userId = profile->getId();
      Clans::createClan(clanName, userId, [this, clanName](bool success, const std::string& message, const std::string& clan_id) {
        if (success) {
          CCLOG("JoinClansLayer: Successfully created clan %s (ID: %s)", clanName.c_str(), clan_id.c_str());
          
          // 更新Profile中的部落信息
          auto profile = Profile::getInstance();
          if (profile) {
            try {
              int clanId = std::stoi(clan_id);
              profile->setClansId(clanId);
            } catch (...) {
              CCLOG("JoinClansLayer: Failed to convert clan ID to int: %s", clan_id.c_str());
            }
            profile->setClansName(clanName);
            profile->save();
          }
          
          // 显示成功消息
          auto successLabel = createLabel("创建成功: " + clanName, 24, Color4B::GREEN);
          successLabel->setPosition(
              Vec2(_contentArea->getContentSize().width / 2.0f,
                   _contentArea->getContentSize().height / 2.0f));
          _contentArea->addChild(successLabel, 1000);
          
          // 3秒后关闭当前层
          auto delay = DelayTime::create(3.0f);
          auto remove = CallFunc::create([this, successLabel]() {
            successLabel->removeFromParent();
            // this->removeFromParent();
          });
          successLabel->runAction(Sequence::create(delay, remove, nullptr));
        } else {
          CCLOG("JoinClansLayer: Failed to create clan: %s", message.c_str());
          // 显示错误消息
          auto errorLabel = createLabel("创建失败: " + message, 24, Color4B::RED);
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

      return true;
    }

    // 检查是否点击了取消按钮
    Rect cancelRect(cancelBtnX, cancelBtnY, btnWidth, btnHeight);
    if (cancelRect.containsPoint(dialogLocalPos)) {
      this->removeChild(bgLayer);
      this->removeChild(dialogBg);
      return true;
    }

    // 点击背景也关闭对话框
    Vec2 bgLocalPos = bgLayer->convertToNodeSpace(location);
    if (bgLocalPos.x >= 0 && bgLocalPos.x <= bgLayer->getContentSize().width &&
        bgLocalPos.y >= 0 && bgLocalPos.y <= bgLayer->getContentSize().height) {
      this->removeChild(bgLayer);
      this->removeChild(dialogBg);
      return true;
    }
    return false;
  };

  _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, dialogBg);
}