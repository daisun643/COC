#include "Building.h"

#include <cmath>

#include "Manager/Config/ConfigManager.h"
#include "Manager/PlayerManager.h"

Building::Building()
    : _buildingType(BuildingType::TOWN_HALL),
      _level(1),
      _maxLevel(3),
      _infoLabel(nullptr),
      _buildingName("Building"),
      _centerX(0.0f),
      _centerY(0.0f),
      _gridCount(1),
      _row(0),
      _col(0),
      _isDragging(false),
      _glowNode(nullptr),
      _anchorNode(nullptr),
      _glowAction(nullptr),
      _glowColor(1.0f, 1.0f, 0.0f, 0.6f),
      _maxHP(1000.0f),
      _currentHP(1000.0f),
      _hpBarBackground(nullptr),
      _hpBarForeground(nullptr),
      _state(State::NORMAL),
      _upgradeTotalTime(0.0f),
      _upgradeTimer(0.0f),
      _progressBar(nullptr),
      _progressBarBg(nullptr),
      _timeLabel(nullptr) {}

Building::~Building() {
  // 注意：在Cocos2d-x中，父节点销毁时会自动清理所有子节点
  // 这里只需要将指针置空，避免重复析构
  // 如果子节点已经被移除，removeFromParent()是安全的（会检查父节点）
  if (_hpBarBackground && _hpBarBackground->getParent()) {
    _hpBarBackground->removeFromParent();
  }
  if (_hpBarForeground && _hpBarForeground->getParent()) {
    _hpBarForeground->removeFromParent();
  }
  // 清理UI指针
  removeUpgradeUI();
  // 将指针置空，避免悬空指针
  _hpBarBackground = nullptr;
  _hpBarForeground = nullptr;
}

bool Building::init(const std::string& imagePath, BuildingType type,
                    const int& level, const int& gridCount,
                    const float& anchorRatioX, const float& anchorRatioY,
                    const float& imageScale) {
  _buildingType = type;
  _level = level;
  _gridCount = gridCount;

  // 尝试加载图片，如果失败则创建默认外观
  if (imagePath.empty() || !Sprite::initWithFile(imagePath)) {
    // 如果路径为空或加载失败，不调用initWithFile的默认行为（它可能已经失败了），
    // 而是确保Sprite被初始化（即使是空的）以便作为容器使用
    if (imagePath.empty()) {
      Sprite::init();  // 初始化为空Sprite
    }
    createDefaultAppearance();
  }

  this->setScale(imageScale);

  // 设置锚点
  this->setAnchorPoint(
      Vec2(_anchorRatioX = anchorRatioX, _anchorRatioY = anchorRatioY));

  // 创建信息标签（初始隐藏）
  _infoLabel = Label::createWithSystemFont("", "Arial", 12);
  _infoLabel->setPosition(Vec2(this->getContentSize().width / 2,
                               this->getContentSize().height + 20));
  _infoLabel->setColor(Color3B::WHITE);
  _infoLabel->setVisible(false);
  this->addChild(_infoLabel, 10);

  // 创建光晕效果节点（初始隐藏）
  _glowNode = DrawNode::create();
  _glowNode->setVisible(false);
  this->addChild(_glowNode, 6);  // 放在建筑前面，防止被大建筑遮挡

  // 创建锚点标记节点（红点）
  _anchorNode = DrawNode::create();
  // 绘制一个红色圆点（在锚点位置，即本地坐标系原点）
  auto width = this->getContentSize().width;
  auto height = this->getContentSize().height;
  _anchorNode->drawDot(Vec2(anchorRatioX * width, anchorRatioY * height), 5.0f,
                       Color4F(1.0f, 0.0f, 0.0f, 1.0f));  // 红色，半径5像素
  this->addChild(_anchorNode, 10);  // 放在最前面，确保可见

  // 创建生命值条
  _hpBarBackground = DrawNode::create();
  _hpBarForeground = DrawNode::create();
  this->addChild(_hpBarBackground, 10);
  this->addChild(_hpBarForeground, 11);
  updateHPBar();

  // 开启update调度，用于处理倒计时
  this->scheduleUpdate();

  return true;
}

// 实现 update 函数
void Building::update(float dt) {
  // 只有在升级状态下才更新
  if (_state == State::UPGRADING) {
    _upgradeTimer -= dt;

    // 更新进度条百分比
    if (_progressBar && _upgradeTotalTime > 0) {
      float percent = 100.0f * (1.0f - (_upgradeTimer / _upgradeTotalTime));
      _progressBar->setPercentage(percent);
    }

    // 更新倒计时文字
    if (_timeLabel) {
      // 向上取整，避免显示 0s 时其实还有 0.5s
      int seconds = static_cast<int>(ceil(_upgradeTimer));
      if (seconds < 0) seconds = 0;
      _timeLabel->setString(StringUtils::format("%ds", seconds));
    }

    // 倒计时结束逻辑
    if (_upgradeTimer <= 0.0f) {
      completeUpgrade();
    }
  }
}

void Building::createDefaultAppearance() {
  // 根据建筑类型创建不同颜色的默认外观
  Color4B color = Color4B::WHITE;
  switch (_buildingType) {
    case BuildingType::TOWN_HALL:
      color = Color4B(139, 69, 19, 255);  // 棕色
      break;
    case BuildingType::DEFENSE:
      color = Color4B(255, 0, 0, 255);  // 红色
      break;
    case BuildingType::RESOURCE:
      color = Color4B(255, 215, 0, 255);  // 金色
      break;
    case BuildingType::STORAGE:
      color = Color4B(0, 0, 255, 255);  // 蓝色
      break;
    case BuildingType::BARRACKS:
      color = Color4B(0, 255, 0, 255);  // 绿色
      break;
    case BuildingType::WALL:
      color = Color4B(128, 128, 128, 255);  // 灰色
      break;
  }

  // 创建彩色矩形作为默认外观
  auto layer = LayerColor::create(color, 80, 80);
  layer->setPosition(Vec2::ZERO);
  this->addChild(layer);

  // 设置内容大小
  this->setContentSize(Size(80, 80));
}

bool Building::isOutOfBounds(int gridSize) const {
  float topRow, topCol, rightRow, rightCol, bottomRow, bottomCol, leftRow,
      leftCol;

  float halfGridCount = _gridCount / 2.0f;
  topRow = _row + halfGridCount;
  topCol = _col - halfGridCount;
  rightRow = _row + halfGridCount;
  rightCol = _col + halfGridCount;
  bottomRow = _row - halfGridCount;
  bottomCol = _col + halfGridCount;
  leftRow = _row - halfGridCount;
  leftCol = _col - halfGridCount;

  // 检查四个角是否都在有效范围内 [0, gridSize]
  if (topRow < 0 || topRow >= gridSize || topCol < 0 || topCol >= gridSize)
    return true;
  if (rightRow < 0 || rightRow >= gridSize || rightCol < 0 ||
      rightCol >= gridSize)
    return true;
  if (bottomRow < 0 || bottomRow >= gridSize || bottomCol < 0 ||
      bottomCol >= gridSize)
    return true;
  if (leftRow < 0 || leftRow >= gridSize || leftCol < 0 || leftCol >= gridSize)
    return true;

  return false;
}

void Building::showGlow() {
  if (!_glowNode) {
    return;
  }

  _glowNode->setVisible(true);

  // 获取配置管理器中的GlowDelay
  auto configManager = ConfigManager::getInstance();
  float glowDelay = 0.5f;
  if (configManager) {
    auto constantConfig = configManager->getConstantConfig();
    glowDelay = constantConfig.glowDelay;
  }

  // 先绘制一次光晕
  updateGlowDrawing();

  // 停止之前的动画
  if (_glowAction) {
    _glowNode->stopAction(_glowAction);
  }

  // 创建明暗交替动画
  // 使用FadeTo实现明暗交替（从0.3到0.8透明度，0-255范围）
  auto fadeToBright =
      FadeTo::create(glowDelay, static_cast<GLubyte>(0.8f * 255.0f));
  auto fadeToDark =
      FadeTo::create(glowDelay, static_cast<GLubyte>(0.3f * 255.0f));
  auto sequence = Sequence::create(fadeToBright, fadeToDark, nullptr);
  auto repeat = RepeatForever::create(sequence);

  // 设置初始透明度（0-255范围）
  _glowNode->setOpacity(static_cast<GLubyte>(0.6f * 255.0f));

  // 开始动画
  _glowAction = repeat;
  _glowNode->runAction(_glowAction);
}

void Building::updateGlowDrawing() {
  if (!_glowNode) {
    return;
  }

  _glowNode->clear();

  // 获取配置管理器
  auto configManager = ConfigManager::getInstance();
  if (!configManager) {
    return;
  }
  auto constantConfig = configManager->getConstantConfig();
  float deltaX = constantConfig.deltaX;
  float deltaY = constantConfig.deltaY;

  // 获取当前缩放比例
  float scaleX = this->getScaleX();
  float scaleY = this->getScaleY();
  // 防止除以零
  if (scaleX == 0.0f) scaleX = 1.0f;
  if (scaleY == 0.0f) scaleY = 1.0f;

  // 绘制光晕边框（使用黄色半透明）
  // 线宽也需要反向缩放，以保持视觉一致性
  float glowWidth = 3.0f / ((scaleX + scaleY) / 2.0f);

  int gridCount = _gridCount;  // 使用getter方法获取宽度
  Vec2 center(this->getContentSize().width * _anchorRatioX,
              this->getContentSize().height * _anchorRatioY);

  // 坐标偏移量需要反向缩放，以抵消节点的缩放影响，确保光晕大小与地图网格一致
  Vec2 top = center + Vec2(0, gridCount * deltaY / scaleY);
  Vec2 right = center + Vec2(gridCount * deltaX / scaleX, 0);
  Vec2 bottom = center + Vec2(0, -gridCount * deltaY / scaleY);
  Vec2 left = center + Vec2(-gridCount * deltaX / scaleX, 0);
  // Vec2 top(_gridCount * deltaX, 2 * _gridCount * deltaY);
  // Vec2 right(2 * _gridCount * deltaX, _gridCount * deltaY);
  // Vec2 bottom(_gridCount * deltaX, 0);
  // Vec2 left(0, _gridCount * deltaY);
  // 绘制四条边
  _glowNode->drawSegment(top, right, glowWidth, _glowColor);
  _glowNode->drawSegment(right, bottom, glowWidth, _glowColor);
  _glowNode->drawSegment(bottom, left, glowWidth, _glowColor);
  _glowNode->drawSegment(left, top, glowWidth, _glowColor);
}

void Building::setPlacementValid(bool isValid) {
  if (isValid) {
    // 有效：黄色光晕，正常颜色
    _glowColor = Color4F(1.0f, 1.0f, 0.0f, 0.6f);
  } else {
    // 无效：红色光晕
    _glowColor = Color4F(1.0f, 0.0f, 0.0f, 0.8f);
  }
  // 立即更新光晕
  updateGlowDrawing();
}

void Building::hideGlow() {
  if (_glowNode) {
    _glowNode->setVisible(false);
    _glowNode->clear();
    if (_glowAction) {
      _glowNode->stopAction(_glowAction);
      _glowAction = nullptr;
    }
  }
}
// TODO bug
bool Building::inDiamond(const Vec2& pos) const {
  // Layer 坐标系下
  auto constantConfig = ConfigManager::getInstance()->getConstantConfig();
  float deltaX = constantConfig.deltaX;
  float deltaY = constantConfig.deltaY;
  // 将点转换为相对于锚点的坐标
  Vec2 relativePos = pos - this->getPosition();

  // 菱形的边界：以锚点为中心，gridSize为边长的菱形
  // 在等距投影中，菱形的边界条件：
  // |x / deltaX| + |y / deltaY| <= gridCount

  if (deltaX <= 0 || deltaY <= 0) {
    return false;
  }

  float dx = relativePos.x / deltaX;
  float dy = relativePos.y / deltaY;

  // 曼哈顿距离
  float manhattanDist = std::abs(dx) + std::abs(dy);

  return manhattanDist <= _gridCount;
}

// 修改 upgrade 入口函数
void Building::upgrade() {
  if (_level >= _maxLevel) {
    CCLOG("Building %s reached max level %d", _buildingName.c_str(), _maxLevel);
    return;
  }

  if (_state == State::UPGRADING) {
    return;
  }

  // 1. 获取下一等级的配置，查看消耗
  // 注意：我们升级是为了去下一级，所以应该读取 _level + 1 的消耗配置
  auto nextLevelConfig = ConfigManager::getInstance()->getBuildingConfig(
      _buildingName, _level + 1);

  int cost = nextLevelConfig.upgradeCost;
  std::string costType = nextLevelConfig.upgradeCostType;
  // 如果配置文件里没有配 upgradeCostType，默认可以用金币
  if (costType.empty()) costType = "Gold";

  // 2. 检查并扣除资源
  bool success = false;
  if (costType == "Gold") {
    success = PlayerManager::getInstance()->consumeGold(cost);
  } else if (costType == "Elixir") {
    success = PlayerManager::getInstance()->consumeElixir(cost);
  } else {
    // 如果类型未知（例如宝石），默认成功或者自行处理
    success = true;
  }

  if (!success) {
    CCLOG("Not enough %s to upgrade! Need %d", costType.c_str(), cost);

    // 这里可以弹出一个 "资源不足" 的提示 Label
    auto failLabel = Label::createWithSystemFont("资源不足!", "Arial", 32);
    failLabel->setColor(Color3B::RED);
    failLabel->setPosition(
        Vec2(getContentSize().width / 2, getContentSize().height + 120));
    this->addChild(failLabel, 100);
    auto seq = Sequence::create(MoveBy::create(1.0f, Vec2(0, 50)),
                                RemoveSelf::create(), nullptr);
    failLabel->runAction(seq);

    return;  // 资源不足，终止升级
  }

  // 3. 资源扣除成功，开始升级流程

  // 弹出提示词：“消耗 xx 金币”
  // 建议使用中文提示，或者根据 costType 转换
  std::string costName = (costType == "Gold") ? "金币" : "圣水";
  std::string hintText =
      StringUtils::format("消耗 %d %s", cost, costName.c_str());

  auto hintLabel = Label::createWithSystemFont(hintText, "Arial", 28);
  // 金色或亮色字体
  hintLabel->setColor(Color3B(255, 215, 0));
  hintLabel->enableOutline(Color4B::BLACK, 2);
  hintLabel->setPosition(Vec2(getContentSize().width / 2,
                              getContentSize().height + 100));  // 在建筑上方
  this->addChild(hintLabel, 100);

  // 提示动画：向上飘动并淡出
  auto moveUp = MoveBy::create(1.5f, Vec2(0, 80));
  auto fadeOut = FadeOut::create(1.5f);
  auto spawn = Spawn::create(moveUp, fadeOut, nullptr);
  auto sequence = Sequence::create(spawn, RemoveSelf::create(), nullptr);
  hintLabel->runAction(sequence);

  // 4. 正常切换状态并开始倒计时
  _state = State::UPGRADING;

  // 使用配置中的建造时间，如果没配则默认10秒
  float configTime = nextLevelConfig.buildTime;
  if (configTime <= 0) configTime = 10.0f;

  _upgradeTotalTime = configTime;
  _upgradeTimer = _upgradeTotalTime;

  createUpgradeUI();

  CCLOG("Started upgrading %s. Time: %.1f. Consumed: %d %s",
        _buildingName.c_str(), _upgradeTotalTime, cost, costType.c_str());
}

// 新增：完成升级的实际逻辑（原 upgrade 函数内容）
void Building::completeUpgrade() {
  _state = State::NORMAL;
  removeUpgradeUI();

  _level++;

  // 使用新的 ConfigManager 接口
  auto config =
      ConfigManager::getInstance()->getBuildingConfig(_buildingName, _level);

  if (!config.image.empty()) {
    auto texture =
        Director::getInstance()->getTextureCache()->addImage(config.image);
    if (texture) {
      this->setTexture(texture);
      this->setTextureRect(Rect(0, 0, texture->getContentSize().width,
                                texture->getContentSize().height));
      this->setContentSize(texture->getContentSize());
    }
  }

  if (_infoLabel) {
    _infoLabel->setPosition(Vec2(this->getContentSize().width / 2,
                                 this->getContentSize().height + 20));
  }

  updateGlowDrawing();

  if (_anchorNode) {
    _anchorNode->clear();
    float width = this->getContentSize().width;
    float height = this->getContentSize().height;
    _anchorNode->drawDot(Vec2(_anchorRatioX * width, _anchorRatioY * height),
                         5.0f, Color4F(1.0f, 0.0f, 0.0f, 1.0f));
  }

  _maxHP = config.maxHP;
  _currentHP = _maxHP;
  updateHPBar();

  CCLOG("Upgraded %s to level %d COMPLETED.", _buildingName.c_str(), _level);
}

// 新增：取消升级
void Building::cancelUpgrade() {
  if (_state == State::UPGRADING) {
    _state = State::NORMAL;
    _upgradeTimer = 0.0f;
    removeUpgradeUI();
    CCLOG("Upgrade cancelled for %s", _buildingName.c_str());
    // 注意：这里未实现退款逻辑，如需退款可在 PlayerManager 添加
    // addGold/addElixir
  }
}

// 新增：立即完成
void Building::finishUpgradeImmediately() {
  if (_state != State::UPGRADING) return;

  int gemCost = 1;  // 每次立即完成消耗1个宝石
  if (PlayerManager::getInstance()->consumeGems(gemCost)) {
    CCLOG("Instant finish used. Consumed %d gem.", gemCost);
    completeUpgrade();
  } else {
    CCLOG("Not enough gems to finish immediately!");
    // 这里可以弹出提示 "宝石不足"
  }
}

// UI 创建与销毁辅助函数
void Building::createUpgradeUI() {
  if (_progressBar) return;

  Size size = this->getContentSize();

  // 1. 创建背景（底框）
  // 使用 Bar.png 作为容器/背景
  _progressBarBg = Sprite::create("images/ui/Bar.png");
  if (_progressBarBg) {
    _progressBarBg->setColor(Color3B::GRAY);  // 稍微变暗作为背景
    _progressBarBg->setPosition(Vec2(size.width / 2, size.height + 50));
    _progressBarBg->setScale(0.8f);
    this->addChild(_progressBarBg, 20);
  }

  // 2. 创建进度条（填充物）
  // 使用 Yellow.png，这通常是一张实心的黄色图片
  // 这样当它作为 ProgressTimer 时，就能看到明显的黄色条在增长
  auto barSprite = Sprite::create("images/ui/Yellow.png");
  if (barSprite) {
    // 如果 Yellow.png 只是一个小方块，我们需要把它拉伸到和背景条一样大
    if (_progressBarBg) {
      Size bgSize = _progressBarBg->getContentSize();
      Size yellowSize = barSprite->getContentSize();
      // 这里我们手动补偿一点点宽度
      float scaleX = (bgSize.width / yellowSize.width) * 1.5f;
      // 计算缩放比例，让黄色图片适配背景框的大小
      barSprite->setScaleX(bgSize.width / yellowSize.width);
      barSprite->setScaleY(bgSize.height / yellowSize.height);
    }

    _progressBar = ProgressTimer::create(barSprite);
    _progressBar->setType(ProgressTimer::Type::BAR);
    _progressBar->setMidpoint(Vec2(0, 0.5));     // 从左边开始
    _progressBar->setBarChangeRate(Vec2(1, 0));  // 水平增长
    _progressBar->setPercentage(0);              // 初始状态为 0% (空)

    // 位置与背景重合
    _progressBar->setPosition(Vec2(size.width / 2, size.height + 50));
    // 整体缩放跟随背景
    // (注意：如果上面已经对barSprite做过拉伸，这里的setScale会叠加)
    // 简单起见，这里设置为与背景相同的整体缩放

    this->addChild(_progressBar, 21);
  }

  // 3. 倒计时文字
  // 创建大字体的倒计时
  int initialSeconds = static_cast<int>(ceil(_upgradeTotalTime));
  // 字体大小改为 25 (原为 16)，确保看清楚
  _timeLabel = Label::createWithSystemFont(
      StringUtils::format("%ds", initialSeconds), "Arial", 25);
  _timeLabel->setPosition(
      Vec2(size.width / 2, size.height + 80));  // 文字放在进度条上方
  // 增加黑色描边，宽度为 2，增加对比度
  _timeLabel->enableOutline(Color4B::BLACK, 2);
  // 设为明亮的颜色，如黄色或白色
  _timeLabel->setColor(Color3B::WHITE);
  this->addChild(_timeLabel, 22);
}

void Building::removeUpgradeUI() {
  if (_progressBar) {
    _progressBar->removeFromParent();
    _progressBar = nullptr;
  }
  if (_progressBarBg) {
    _progressBarBg->removeFromParent();
    _progressBarBg = nullptr;
  }
  if (_timeLabel) {
    _timeLabel->removeFromParent();
    _timeLabel = nullptr;
  }
}

void Building::updateHPBar() {
  if (!_hpBarBackground || !_hpBarForeground) {
    return;
  }

  // 生命值条尺寸（根据建筑大小调整）
  float barWidth =
      this->getContentSize().width * 0.8f;  // 血条宽度为建筑宽度的80%
  if (barWidth < 40.0f) {
    barWidth = 40.0f;  // 最小宽度
  }
  float barHeight = 5.0f;  // 血条高度

  // 在Cocos2d-x中，设置锚点后，本地坐标系的原点(0,0)就是锚点位置
  float barY = this->getContentSize()
                   .height;  // 血条的Y坐标（相对于锚点，即本地坐标系原点）
  float anchorX = this->getContentSize().width * _anchorRatioX;
  float anchorY = this->getContentSize().height * _anchorRatioY;
  // 清除之前的绘制
  _hpBarBackground->clear();
  _hpBarForeground->clear();

  // 绘制背景（深红色/暗红色）
  // 血条中心在(barX, barY)，所以起点和终点相对于中心
  Vec2 bgStart(anchorX - barWidth / 2, barY);
  Vec2 bgEnd(anchorX + barWidth / 2, barY);
  _hpBarBackground->drawSegment(bgStart, bgEnd, barHeight,
                                Color4F(0.3f, 0.0f, 0.0f, 1.0f));

  // 绘制前景（红色，根据生命值比例）
  if (_maxHP > 0) {
    float hpRatio = _currentHP / _maxHP;
    if (hpRatio < 0) {
      hpRatio = 0;
    }
    if (hpRatio > 1) {
      hpRatio = 1;
    }

    float foregroundWidth = barWidth * hpRatio;
    if (foregroundWidth > 0) {
      Vec2 fgStart(anchorX - barWidth / 2, barY);
      Vec2 fgEnd(anchorX - barWidth / 2 + foregroundWidth, barY);

      // 使用红色血条
      Color4F barColor = Color4F(1.0f, 0.0f, 0.0f, 1.0f);  // 红色

      _hpBarForeground->drawSegment(fgStart, fgEnd, barHeight, barColor);
    }
  }
}

void Building::setCurrentHPAndUpdate(float hp) {
  _currentHP = hp;
  if (_currentHP < 0) {
    _currentHP = 0;
  }
  if (_currentHP > _maxHP) {
    _currentHP = _maxHP;
  }
  updateHPBar();
}

void Building::setHealthBarVisible(bool visible) {
  if (_hpBarBackground) {
    _hpBarBackground->setVisible(visible);
  }
  if (_hpBarForeground) {
    _hpBarForeground->setVisible(visible);
  }
}

void Building::takeDamage(float damage) {
  if (!isAlive()) {
    return;
  }

  _currentHP -= damage;
  if (_currentHP < 0) {
    _currentHP = 0;
  }
  updateHPBar();

  // 如果HP为0，隐藏建筑并移除
  if (_currentHP <= 0) {
    if (_onDeathCallback) {
      _onDeathCallback(this);
    }
    this->setVisible(false);
    this->removeFromParent();
  }
}

bool Building::isAlive() const { return _currentHP > 0; }
