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
    this->addChild(_background);
  }

  Size bgSize = _background ? _background->getContentSize() : Size(100, 20);

  // 2. 创建进度条 (LoadingBar)
  // 使用用户提供的纹理
  std::string barTexture =
      (type == Type::GOLD) ? "images/ui/Yellow.png" : "images/ui/Purple.png";

  _progressBar = cocos2d::ui::LoadingBar::create(barTexture);
  if (_progressBar) {
    _progressBar->setDirection(
        cocos2d::ui::LoadingBar::Direction::RIGHT);  // 从右向左填充
    _progressBar->setPercent(0);
    _progressBar->setPosition(Vec2::ZERO);  // 居中显示

    // 自动调整进度条尺寸以匹配背景条
    Size barSize = _progressBar->getContentSize();
    if (barSize.width > 0 && barSize.height > 0) {
      // 宽度缩小至 95%，高度缩小至 80% 以露出上下黑框
      float scaleX = (bgSize.width * 0.95f) / barSize.width;
      float scaleY = (bgSize.height * 0.80f) / barSize.height;
      _progressBar->setScale(scaleX, scaleY);
    }

    // 向左平移一点，解决左侧空隙问题，右侧由图标遮挡
    _progressBar->setPosition(Vec2(-5, 0));

    this->addChild(_progressBar, 1);  // 层级在背景之上
  }

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
  float labelFontSize = 28;

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

  // 整体缩小组件，因为素材本身较大
  this->setScale(0.75f);

  return true;
}

void ResourceWidget::updateAmount(int amount, int maxAmount) {
  _currentAmount = amount;
  _maxAmount = maxAmount;

  if (_amountLabel) {
    _amountLabel->setString(std::to_string(amount));
  }

  // 更新进度条
  if (_progressBar && maxAmount > 0) {
    float percent = (float)amount / maxAmount * 100.0f;
    if (percent > 100.0f) percent = 100.0f;
    if (percent < 0.0f) percent = 0.0f;

    _progressBar->setPercent(percent);
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

  auto createLabel = [&](const std::string& text, const Color4B& color) {
    Label* label;
    if (useTTF) {
      TTFConfig ttfConfig(fontFile, fontSize);
      label = Label::createWithTTF(ttfConfig, text);
    } else {
      label = Label::createWithSystemFont(text, "Arial", fontSize);
    }
    label->enableBold();
    label->setTextColor(color);
    label->enableOutline(Color4B::BLACK, 1);
    return label;
  };

  // 最大储量文本
  std::string maxStr = "最大储量: " + std::to_string(_maxAmount);
  auto maxLabel = createLabel(maxStr, Color4B::WHITE);
  maxLabel->setPosition(Vec2(100, 55));
  bg->addChild(maxLabel);

  // 产量文本
  std::string prodStr = "产量: " + std::to_string(_production) + "/小时";
  auto prodLabel = createLabel(prodStr, Color4B::GREEN);
  prodLabel->setPosition(Vec2(100, 25));
  bg->addChild(prodLabel);
}

void ResourceWidget::hideInfoPopup() {
  if (_infoPopup) {
    _infoPopup->removeFromParent();
    _infoPopup = nullptr;
  }
}
