#include "gtest/gtest.h"
#include "Utils/Profile/Profile.h"

TEST(ProfileTest, LoadProfile) {
    Profile profile;
    bool loaded = profile.load("me.json");
    EXPECT_TRUE(loaded);
}

TEST(ProfileTest, SaveProfile) {
    Profile profile;
    profile.load("me.json");
    bool saved = profile.save();
    EXPECT_TRUE(saved);
}