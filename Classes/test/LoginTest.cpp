#include "gtest/gtest.h"
#include "Utils/API/Authentication/Login.h"

TEST(LoginTest, ValidLogin) {
    Login loginService;
    bool success = loginService.authenticate("user3", "password123");
    EXPECT_TRUE(success);
}

TEST(LoginTest, InvalidPassword) {
    Login loginService;
    bool success = loginService.authenticate("user3", "wrongpass");
    EXPECT_FALSE(success);
}