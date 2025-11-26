#include "HelloWorldScene.h"

USING_NS_CC;

Scene *HelloWorld::createScene() { return HelloWorld::create(); }

bool HelloWorld::init() {
  if (!Scene::init()) {
    return false;
  }

  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 添加背景颜色（避免加载不存在的图片）
  auto background = LayerColor::create(Color4B(50, 50, 50, 255));
  this->addChild(background, -1);

  // 添加欢迎标签（使用系统字体，避免字体文件不存在）
  auto welcomeLabel =
      Label::createWithSystemFont("Welcome to COC Game", "Arial", 48);
  if (welcomeLabel) {
    welcomeLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                   origin.y + visibleSize.height / 2 + 100));
    welcomeLabel->setColor(Color3B::WHITE);
    this->addChild(welcomeLabel, 1);
  }

  // 添加菜单
  MenuItem *closeItem = nullptr;
  auto imageItem =
      MenuItemImage::create("CloseNormal.png", "CloseSelected.png",
                            CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));

  if (imageItem == nullptr || imageItem->getContentSize().width <= 0 ||
      imageItem->getContentSize().height <= 0) {
    // 如果没有图片，创建一个文本菜单项
    closeItem = MenuItemFont::create(
        "Exit", CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));
  } else {
    closeItem = imageItem;
    float x =
        origin.x + visibleSize.width - closeItem->getContentSize().width / 2;
    float y = origin.y + closeItem->getContentSize().height / 2;
    closeItem->setPosition(Vec2(x, y));
  }

  auto menu = Menu::create(closeItem, NULL);
  menu->setPosition(Vec2::ZERO);
  this->addChild(menu, 1);

  // 添加标题标签（使用系统字体）
  auto titleLabel = Label::createWithSystemFont("COC Game", "Arial", 32);
  if (titleLabel) {
    titleLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                 origin.y + visibleSize.height - 50));
    titleLabel->setColor(Color3B::YELLOW);
    this->addChild(titleLabel, 1);
  }

  return true;
}

void HelloWorld::menuCloseCallback(cocos2d::Ref *pSender) {
  Director::getInstance()->end();
}
