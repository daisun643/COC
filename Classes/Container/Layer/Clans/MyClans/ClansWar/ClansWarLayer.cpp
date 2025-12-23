#include "ClansWarLayer.h"

#include <sstream>

#include "Container/Scene/AttackScence/AttackScene.h"
#include "Utils/API/Clans/Clans.h"
#include "Utils/API/Clans/ClansWar.h"
#include "Utils/PathUtils.h"
#include "Utils/Profile/Profile.h"
#include "network/HttpClient.h"
#include "ui/CocosGUI.h"

using namespace cocos2d;
using namespace cocos2d::network;

ClansWarLayer* ClansWarLayer::create() {
  ClansWarLayer* p = new (std::nothrow) ClansWarLayer();
  if (p && p->init()) {
    p->autorelease();
    return p;
  }
  CC_SAFE_DELETE(p);
  return nullptr;
}

bool ClansWarLayer::init() {
  if (!Layer::init()) return false;
  _isOwner = false;
  _warStarted = false;
  buildUI();
  return true;
}

void ClansWarLayer::buildUI() {
  _contentArea = Layer::create();
  _contentArea->setContentSize(Size(760.0f, 450.0f));
  this->addChild(_contentArea);

  // 检查用户是否在部落中
  auto profile = Profile::getInstance();
  if (!profile || profile->getClansId() <= 0) {
    // 用户尚未加入部落，显示提示信息
    auto tipLabel =
        Label::createWithSystemFont("你尚未加入一个部落", "Arial", 28);
    tipLabel->setColor(Color3B::WHITE);
    tipLabel->setPosition(Vec2(_contentArea->getContentSize().width / 2.0f,
                               _contentArea->getContentSize().height / 2.0f));
    _contentArea->addChild(tipLabel);
    return;
  }

  // 用户已加入部落，创建部落战界面
  float buttonHeight = 50.0f;
  float topY = _contentArea->getContentSize().height - 30.0f;
  float buttonBottomY = topY - buttonHeight - 20.0f;  // 按钮下方留20像素间距

  // 创建滚动视图区域（顶部按钮下方）
  float scrollViewHeight = buttonBottomY - 20.0f;  // 按钮下方留20像素间距

  _scrollView = ui::ScrollView::create();
  _scrollView->setContentSize(
      Size(_contentArea->getContentSize().width, scrollViewHeight));
  _scrollView->setPosition(
      Vec2(0.0f, buttonHeight - 20.0f));  // 从按钮下方开始，留20像素间距
  _scrollView->setDirection(ui::ScrollView::Direction::VERTICAL);
  _scrollView->setBounceEnabled(true);
  _contentArea->addChild(_scrollView);

  // 创建开始部落战按钮
  buttonBg = LayerColor::create(Color4B(128, 128, 128, 255), 200, 50);
  _startWarButton = ui::Button::create();
  _startWarButton->setTitleText("开启部落战");
  _startWarButton->setTitleFontSize(20);
  _startWarButton->setContentSize(Size(200.0f, buttonHeight));
  _startWarButton->setPosition(Vec2(buttonBg->getContentSize().width / 2,
                                    buttonBg->getContentSize().height / 2));
  buttonBg->setPosition(Vec2(_contentArea->getContentSize().width / 2.0f - 100,
                             buttonBottomY - 100.0f));
  buttonBg->addChild(_startWarButton);
  _startWarButton->addClickEventListener([this](Ref*) { onStartWarPressed(); });
  _contentArea->addChild(buttonBg);

  // 默认状态：未开启部落战
  buttonBg->setVisible(false);
  _statusLabel = Label::createWithSystemFont("部落战尚未开启", "Arial", 24);
  _statusLabel->setColor(Color3B::WHITE);
  _statusLabel->setPosition(
      Vec2(_contentArea->getContentSize().width / 2.0f, buttonBottomY - 20.0f));
  _contentArea->addChild(_statusLabel);

  // 初次刷新界面
  refresh();
}

void ClansWarLayer::refresh() {
  // 查询后端 overview
  checkOwner();
  loadOverview();
}

void ClansWarLayer::showNotStartedUI() {
  buttonBg->setVisible(_isOwner);
  _scrollView->setVisible(false);
  _statusLabel->setString("部落战尚未开启");
}

void ClansWarLayer::showStartedUI(const std::string& warId) {
  _currentWarId = warId;
  buttonBg->setVisible(false);
  _scrollView->setVisible(true);
  _statusLabel->setString("部落战进行中");
}

void ClansWarLayer::onStartWarPressed() {
  // 使用封装的 ClansWar API
  Profile* profile = Profile::getInstance();
  if (!profile || profile->getClansId() <= 0) {
    _statusLabel->setString("未加入部落，无法开启部落战");
    return;
  }
  std::string clansId = std::to_string(profile->getClansId());
  if (!_isOwner) {
    _statusLabel->setString("只有部落所有者可以开启部落战");
    return;
  }
  ClansWar::startWar(clansId, [this](bool success, const std::string& message,
                                     const std::string& war_id,
                                     const std::string& history_path) {
    if (!success) {
      _statusLabel->setString(message.empty() ? "开启部落战失败" : message);
      return;
    }
    // 刷新 overview
    loadOverview();
  });
}

Label* createLabel(const std::string& text, int fontSize,
                   const Color4B& color = Color4B::WHITE) {
  auto label = Label::createWithSystemFont(text, "Arial", fontSize);
  label->setTextColor(color);
  return label;
}

void ClansWarLayer::loadOverview() {
  Profile* profile = Profile::getInstance();
  if (!profile || profile->getClansId() <= 0) {
    _statusLabel->setString("未加入部落");
    return;
  }
  std::string clansId = std::to_string(profile->getClansId());
  ClansWar::getWarOverview(clansId, [this](const WarOverviewResult& result) {
    if (!result.success) {
      // 如果是未开始部落战，后端 message 会包含提示
      if (result.message.find("未开始") != std::string::npos) {
        _warStarted = false;
        _scrollView->setVisible(false);
        _statusLabel->setString("部落战尚未开启");
        // 根据 owner 状态显示按钮（如果 owner）
        buttonBg->setVisible(_isOwner);
      } else {
        _statusLabel->setString(result.message.empty() ? "获取部落战信息失败"
                                                       : result.message);
      }
      return;
    }

    // 展示已开启界面
    _warStarted = true;
    buttonBg->setVisible(false);
    showStartedUI("");
    // 清空 scrollView
    _scrollView->removeAllChildren();

    float itemH = 60.0f;
    float spacing = 10.0f;
    float width = _scrollView->getContentSize().width;
    float y = 0.0f;

    for (const auto& m : result.maps) {
      auto container =
          LayerColor::create(Color4B(200, 200, 200, 100), width, itemH);
      container->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
      container->setPosition(Vec2(0, y));

      auto idLabel =
          Label::createWithSystemFont(std::string("地图:") + m.id, "Arial", 20);
      idLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
      idLabel->setPosition(Vec2(10, itemH / 2));
      container->addChild(idLabel);

      auto info = Label::createWithSystemFont(
          "stars:" + std::to_string(m.stars) + " cnt:" + std::to_string(m.cnt),
          "Arial", 18);
      info->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
      info->setPosition(Vec2(200, itemH / 2));
      container->addChild(info);

      auto attackBtn = ui::Button::create();
      attackBtn->setTitleText("进攻");
      attackBtn->setPosition(Vec2(width - 60, itemH / 2));
      std::string mapId = m.id;
      int cnt = m.cnt;
      attackBtn->addClickEventListener([this, cnt, mapId](Ref*) {
        if (cnt == 0) {
          return;
          // 提示进攻次数用完
          CCLOG("ClansWarLayer::loadOverView 进攻次数已经用完");
          // 显示错误提示
          auto errorLabel =
              createLabel("进攻次数已经用完，无法再次进攻", 24, Color4B::RED);
          errorLabel->setPosition(
              Vec2(_contentArea->getContentSize().width / 2.0f,
                   _contentArea->getContentSize().height / 2.0f));
          this->getParent()->addChild(errorLabel, 1000);
          // 3秒后移除提示
          auto delay = DelayTime::create(3.0f);
          auto remove = CallFunc::create(
              [errorLabel]() { errorLabel->removeFromParent(); });
          errorLabel->runAction(Sequence::create(delay, remove, nullptr));
        }
        onAttackPressed(mapId);
      });
      container->addChild(attackBtn);

      _scrollView->addChild(container);
      y += itemH + spacing;
    }

    // 调整 inner container 大小
    _scrollView->setInnerContainerSize(Size(width, y));
  });
}

void ClansWarLayer::checkOwner() {
  Profile* profile = Profile::getInstance();
  if (!profile || profile->getClansId() <= 0) {
    _isOwner = false;
    buttonBg->setVisible(false);
    return;
  }
  std::string clansId = std::to_string(profile->getClansId());
  int userId = profile->getId();
  Clans::getClanOwner(
      clansId,
      [this, userId](bool success, const std::string& message, int owner_id) {
        if (success) {
          _isOwner = (owner_id == userId);
        } else {
          _isOwner = false;
        }
        // 如果当前部落战未开始，则根据 owner 状态显示按钮
        if (!_warStarted) {
          buttonBg->setVisible(_isOwner);
        }
      });
}

void ClansWarLayer::onAttackPressed(const std::string& mapId) {
  auto profile = Profile::getInstance();
  if (profile && profile->getClansId() == -1) {
    CCLOG("ClansWarLayer::onAttackPressed profile problem");
  }
  ClansWar::getWarMap(
      std::to_string(profile->getClansId()), mapId,
      [profile, mapId](bool success, const std::string& message,
                       const std::string& map_data_json) {
        // 使用 mapJsonData 创建 AttackScene
        std::string tmpPath = "level/_tmp.json";
        // 使用 PathUtils 获取真实路径（forWrite=true
        // 表示用于写入）
        std::string fullPath = PathUtils::getRealFilePath(tmpPath, false);
        // 确保目录存在
        if (!PathUtils::ensureDirectoryExists(fullPath)) {
          CCLOG("Failed to create directory for temp map file: %s",
                fullPath.c_str());
          return;
        }

        // 将 mapJsonData 写入到 fullPath 文件
        auto fileUtils = FileUtils::getInstance();
        if (!fileUtils->writeStringToFile(map_data_json, fullPath)) {
          CCLOG("Failed to write map data to file: %s", fullPath.c_str());
          return;
        }

        CCLOG("Map data written to: %s", fullPath.c_str());
        std::string opp = std::string("Clans-War-") + mapId;
        auto attackScene =
            dynamic_cast<AttackScene*>(AttackScene::createScene(tmpPath, opp));
        attackScene->setClansWarInfo(std::to_string(profile->getClansId()),
                                     mapId);
        if (attackScene) {
          auto director = Director::getInstance();
          director->replaceScene(attackScene);
        } else {
          CCLOG("Failed to create AttackScene from file: %s", fullPath.c_str());
        }
      });
}
