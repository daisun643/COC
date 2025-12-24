#include <gtest.h>

#include "Manager/Config/ConfigManager.h"
#include "Utils/GridUtils.h"
#include "Utils/PathFinder.h"

// Mock ConfigManager for testing
class MockConfigManager : public ConfigManager {
 public:
  ConstantConfig getConstantConfig() override {
    ConstantConfig config;
    config.deltaX = 50.0f;
    config.deltaY = 25.0f;
    config.gridSize = 10;
    return config;
  }

  static MockConfigManager* getInstance() {
    static MockConfigManager instance;
    return &instance;
  }
};

TEST(PathFinderTest, FindPath_StartEndSame_ReturnsSinglePoint) {
  // Arrange
  ConfigManager::setInstance(MockConfigManager::getInstance());
  Vec2 startPos(100, 200);
  Vec2 endPos(100, 200);
  Vec2 p00(100, 200);
  auto isWalkable = [](int row, int col) { return true; };

  // Act
  auto path = PathFinder::findPath(startPos, endPos, p00, isWalkable);

  // Assert
  ASSERT_EQ(path.size(), 1);
  EXPECT_EQ(path[0], endPos);
}

TEST(PathFinderTest, FindPath_ValidPath_ReturnsCorrectPath) {
  // Arrange
  ConfigManager::setInstance(MockConfigManager::getInstance());
  Vec2 startPos = GridUtils::gridToScene(0, 0, Vec2(100, 200));
  Vec2 endPos = GridUtils::gridToScene(2, 2, Vec2(100, 200));
  Vec2 p00(100, 200);
  auto isWalkable = [](int row, int col) { return true; };

  // Act
  auto path = PathFinder::findPath(startPos, endPos, p00, isWalkable);

  // Assert
  ASSERT_GT(path.size(), 1);
  EXPECT_EQ(path.front(), startPos);
  EXPECT_EQ(path.back(), endPos);
}

TEST(PathFinderTest, FindPath_BlockedPath_ReturnsEmpty) {
  // Arrange
  ConfigManager::setInstance(MockConfigManager::getInstance());
  Vec2 startPos = GridUtils::gridToScene(0, 0, Vec2(100, 200));
  Vec2 endPos = GridUtils::gridToScene(2, 2, Vec2(100, 200));
  Vec2 p00(100, 200);
  auto isWalkable = [](int row, int col) { return false; };

  // Act
  auto path = PathFinder::findPath(startPos, endPos, p00, isWalkable);

  // Assert
  EXPECT_TRUE(path.empty());
}