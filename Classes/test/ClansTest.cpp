#include "gtest/gtest.h"
#include "Utils/API/Clans/Clans.h"

TEST(ClansTest, CreateClan) {
    Clans clans;
    bool created = clans.createClan("DragonClan", "user3");
    EXPECT_TRUE(created);
}

TEST(ClansTest, GetClanMembers) {
    Clans clans;
    clans.createClan("DragonClan", "user3");
    clans.addMember("DragonClan", "user4");
    EXPECT_EQ(clans.getMemberCount("DragonClan"), 2);
}