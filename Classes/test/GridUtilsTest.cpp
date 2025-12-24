#include <gtest.h>

#include "Manager/Config/ConfigManager.h"
#include "Utils/GridUtils.h"

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

TEST(GridUtilsTest, GridToSceneConversion) {
  // Arrange
  ConfigManager::setInstance(MockConfigManager::getInstance());
  Vec2 p00(100, 200);

  // Act
  Vec2 result = GridUtils::gridToScene(2, 3, p00);

  // Assert
  EXPECT_NEAR(result.x, 100 + (2 + 3) * 50, 0.001);
  EXPECT_NEAR(result.y, 200 + (3 - 2) * 25, 0.001);
}

TEST(GridUtilsTest, ScreenToGridConversion) {
  // Arrange
  ConfigManager::setInstance(MockConfigManager::getInstance());
  Vec2 p00(100, 200);
  Vec2 screenPos(350, 225);  // Should map to (2, 3)
  float row, col;

  // Act
  bool result = GridUtils::screenToGrid(screenPos, p00, row, col);

  // Assert
  EXPECT_TRUE(result);
  EXPECT_NEAR(row, 2.0f, 0.001);
  EXPECT_NEAR(col, 3.0f, 0.001);
}

TEST(GridUtilsTest, FindNearestGrassVertex) {
  // Arrange
  ConfigManager::setInstance(MockConfigManager::getInstance());
  Vec2 p00(100, 200);
  Vec2 screenPos(360, 230);  // Near (2, 3)
  float row, col;
  Vec2 nearestPos;

  // Act
  bool result =
      GridUtils::findNearestGrassVertex(screenPos, p00, row, col, nearestPos);

  // Assert
  EXPECT_TRUE(result);
  EXPECT_EQ(static_cast<int>(row), 2);
  EXPECT_EQ(static_cast<int>(col), 3);
}