#include "gtest/gtest.h"
#include "Utils/API/User/User.h"

TEST(UserTest, Initialization) {
    User user;
    EXPECT_FALSE(user.isNull());
}

TEST(UserTest, GetUserInfo) {
    User user;
    user.setId(123);
    EXPECT_EQ(user.getId(), 123);
}