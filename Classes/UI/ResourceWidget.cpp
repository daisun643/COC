#include "ResourceWidget.h"

USING_NS_CC;

ResourceWidget* ResourceWidget::create(Type type) {
  ResourceWidget* pRet = new (std::nothrow) ResourceWidget();
  if (pRet && pRet->init(type)) {
    pRet->autorelease();
    return pRet;
  } else {
    delete pRet;
    pRet = nullptr;
    return nullptr;
  }
}

bool ResourceWidget::init(Type type) {
  if (!Node::init()) {
    return false;
  }

  _type = type;
  _currentAmount = 0;
  _maxAmount = 0;
  _production = 0;
  _infoPopup = nullptr;

  // 1. 加载背景条 (Bar.png)
  _background = Sprite::create("images/ui/Bar.png");
  if (_background) {
    // 尝试将背景变暗，以防图片本身太亮导致文字看不清
    // 如果图片本身是深色的，这会让它更深，通常没问题
    _background->setColor(Color3B(80, 80, 80));
    this->addChild(_background);
  }

  Size bgSize = _background ? _background->getContentSize() : Size(100, 20);

  // 2. 创建进度条 (LoadingBar)
  // 由于没有专门的进度条纹理，我们创建一个纯色的纹理
  // 金币用黄色，圣水用紫色
  Color3B barColor =
      (type == Type::GOLD) ? Color3B(255, 215, 0) : Color3B(255, 0, 255);

  // 创建一个1x1的白色纹理
  auto whitePixel = Sprite::create();
  whitePixel->setTextureRect(Rect(0, 0, 1, 1));
  whitePixel->setColor(barColor);

  // 使用 LoadingBar
  _progressBar = cocos2d::ui::LoadingBar::create();
  _progressBar->setScale9Enabled(true);  // 开启九宫格缩放，以便拉伸纯色纹理
  _progressBar->loadTexture(
      whitePixel->getTexture()
          ->getPath());  // 这里可能需要更复杂的处理来使用纯色，或者直接用LayerColor模拟
  // 简单起见，我们用 LayerColor 模拟进度条，因为 LoadingBar 需要纹理路径
  // 重新实现：使用 LayerColor + ClippingNode 或者直接改变 LayerColor 的宽度
  // 但为了支持 "从右向左填充"，改变 LayerColor 的宽度并设置锚点为 (1, 0.5)
  // 是最简单的

  // 移除上面的 LoadingBar 代码，改用 LayerColor
  auto barLayer = LayerColor::create(Color4B(barColor), bgSize.width - 4,
                                     bgSize.height - 4);
  barLayer->setIgnoreAnchorPointForPosition(false);
  barLayer->setAnchorPoint(Vec2(1.0f, 0.5f));            // 锚点设在右侧中心
  barLayer->setPosition(Vec2(bgSize.width / 2 - 2, 0));  // 放在背景条内部右侧
  this->addChild(barLayer, 1);

  // 保存 barLayer 指针以便更新，这里我们用 tag 或者成员变量
  // 由于 LayerColor 没有 setPercent，我们需要手动计算宽度
  barLayer->setTag(999);  // Tag 999 for progress bar

  // 3. 根据类型加载图标
  std::string iconPath =
      (type == Type::GOLD) ? "images/ui/Gold.png" : "images/ui/Elixir.png";
  _icon = Sprite::create(iconPath);
  if (_icon) {
    // 调整图标位置：放在背景条右侧，稍微向左偏移一点，避免完全溢出
    // 假设背景条中心是 (0,0)
    float iconX = bgSize.width / 2;  // 稍微突出一点
    _icon->setPosition(Vec2(iconX, 0));
    this->addChild(_icon, 3);  // 图标层级最高
  }

  // 4. 创建数字标签
  // 增大字号，提高清晰度
  float labelFontSize = 24;

  TTFConfig ttfConfig("fonts/NotoSansSC-VariableFont_wght.ttf", labelFontSize);
  // 检查文件是否存在，如果不存在则使用 Arial
  if (!FileUtils::getInstance()->isFileExist(
          "fonts/NotoSansSC-VariableFont_wght.ttf")) {
    _amountLabel = Label::createWithSystemFont("0", "Arial", labelFontSize);
    _amountLabel->setTextColor(Color4B::WHITE);
    _amountLabel->enableBold();
    _amountLabel->enableOutline(Color4B::BLACK, 1);
  } else {
    _amountLabel = Label::createWithTTF(ttfConfig, "0");
    _amountLabel->setTextColor(Color4B::WHITE);
    _amountLabel->enableBold();
    _amountLabel->enableOutline(Color4B::BLACK, 1);
  }
  if (_amountLabel) {
    // 放在背景条中间
    _amountLabel->setPosition(Vec2::ZERO);
    this->addChild(_amountLabel, 2);
  }

  // 5. 添加点击事件监听
  auto listener = EventListenerTouchOneByOne::create();
  listener->setSwallowTouches(true);  // 吞噬触摸，防止穿透
  listener->onTouchBegan = [this, bgSize](Touch* touch, Event* event) -> bool {
    Vec2 location = touch->getLocation();
    Vec2 nodePos = this->convertToNodeSpace(location);

    // 检查点击是否在背景条范围内
    Rect rect(-bgSize.width / 2, -bgSize.height / 2, bgSize.width,
              bgSize.height);
    if (rect.containsPoint(nodePos)) {
      showInfoPopup();
      return true;
    }
    return false;
  };

  // 点击屏幕其他地方关闭弹窗
  auto closeListener = EventListenerTouchOneByOne::create();
  closeListener->onTouchBegan = [this](Touch* touch, Event* event) -> bool {
    if (_infoPopup && _infoPopup->isVisible()) {
      hideInfoPopup();
    }
    return false;  // 不吞噬，让其他控件也能响应
  };

  _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
  _eventDispatcher->addEventListenerWithSceneGraphPriority(closeListener, this);

  return true;
}

void ResourceWidget::updateAmount(int amount, int maxAmount) {
  _currentAmount = amount;
  _maxAmount = maxAmount;

  if (_amountLabel) {
    _amountLabel->setString(std::to_string(amount));
  }

  // 更新进度条
  auto barLayer = dynamic_cast<LayerColor*>(this->getChildByTag(999));
  if (barLayer && _background && maxAmount > 0) {
    float percent = (float)amount / maxAmount;
    if (percent > 1.0f) percent = 1.0f;
    if (percent < 0.0f) percent = 0.0f;

    Size bgSize = _background->getContentSize();
    float maxWidth = bgSize.width - 4;  // 减去边距

    // 改变宽度
    barLayer->setContentSize(Size(maxWidth * percent, bgSize.height - 4));
  }
}

void ResourceWidget::setProduction(int production) { _production = production; }

void ResourceWidget::showInfoPopup() {
  if (_infoPopup) {
    _infoPopup->removeFromParent();
    _infoPopup = nullptr;
  }

  // 创建背景
  auto bg = LayerColor::create(Color4B(0, 0, 0, 200), 200, 80);
  bg->setIgnoreAnchorPointForPosition(false);
  bg->setAnchorPoint(Vec2(0.5f, 1.0f));  // 顶部中心对齐
  bg->setPosition(Vec2(0, -25));         // 在资源条下方

  // 修改：将弹窗添加到父节点 (MainUILayer)，Z-Order 设为高于资源条即可
  // 这样既不会被兄弟节点遮挡，也不会覆盖所有 Scene 的内容
  auto parent = this->getParent();
  if (parent) {
    // 转换坐标：从 本地(0, -25) -> 世界 -> 父节点本地
    Vec2 worldPos = this->convertToWorldSpace(Vec2(0, -25));
    Vec2 parentPos = parent->convertToNodeSpace(worldPos);

    bg->setPosition(parentPos);
    // 设为 100，通常足以覆盖同一层级的其他 UI 元素
    parent->addChild(bg, 100);
    _infoPopup = bg;
  } else {
    // 回退方案
    bg->setPosition(Vec2(0, -25));
    this->addChild(bg, 100);
    _infoPopup = bg;
  }

  // 字体配置
  std::string fontFile = "fonts/NotoSansSC-VariableFont_wght.ttf";
  bool useTTF = FileUtils::getInstance()->isFileExist(fontFile);
  float fontSize = 24;

  // 最大储量文本
  std::string maxStr = "最大储量: " + std::to_string(_maxAmount);
  Label* maxLabel;
  if (useTTF) {
    TTFConfig ttfConfig(fontFile, fontSize);
    maxLabel = Label::createWithTTF(ttfConfig, maxStr);
    maxLabel->enableBold();
  } else {
    maxLabel = Label::createWithSystemFont(maxStr, "Arial", fontSize);
    maxLabel->enableBold();
  }
  maxLabel->setTextColor(Color4B::WHITE);
  maxLabel->enableOutline(Color4B::BLACK, 1);
  maxLabel->setPosition(Vec2(100, 55));
  bg->addChild(maxLabel);

  // 产量文本
  std::string prodStr = "产量: " + std::to_string(_production) + "/小时";
  Label* prodLabel;
  if (useTTF) {
    TTFConfig ttfConfig(fontFile, fontSize);
    prodLabel = Label::createWithTTF(ttfConfig, prodStr);
    prodLabel->enableBold();
  } else {
    prodLabel = Label::createWithSystemFont(prodStr, "Arial", fontSize);
    prodLabel->enableBold();
  }
  prodLabel->setTextColor(Color4B::GREEN);
  prodLabel->enableOutline(Color4B::BLACK, 1);
  prodLabel->setPosition(Vec2(100, 25));
  bg->addChild(prodLabel);
}

void ResourceWidget::hideInfoPopup() {
  if (_infoPopup) {
    _infoPopup->removeFromParent();
    _infoPopup = nullptr;
  }
}
