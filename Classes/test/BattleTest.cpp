#include "gtest/gtest.h"
#include "Utils/API/Battle/Battle.h"

TEST(BattleTest, StartBattle) {
    Battle battle;
    bool started = battle.start("user3", "user4");
    EXPECT_TRUE(started);
}

TEST(BattleTest, GetBattleResult) {
    Battle battle;
    battle.start("user3", "user4");
    battle.setWinner("user3");
    EXPECT_EQ(battle.getWinner(), "user3");
}