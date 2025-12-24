#include "gtest/gtest.h"
#include "Utils/API/Clans/ClansWar.h"

TEST(ClansWarTest, InitializeWar) {
    ClansWar war;
    war.initialize("War-5", 100);
    EXPECT_EQ(war.getWarId(), "War-5");
    EXPECT_EQ(war.getMaxParticipants(), 100);
}

TEST(ClansWarTest, AddParticipant) {
    ClansWar war;
    war.initialize("War-5", 100);
    bool added = war.addParticipant("user3");
    EXPECT_TRUE(added);
    EXPECT_EQ(war.getParticipantCount(), 1);
}