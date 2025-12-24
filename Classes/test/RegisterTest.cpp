#include "gtest/gtest.h"
#include "Utils/API/Authentication/Register.h"

TEST(RegisterTest, ValidRegistration) {
    Register registerService;
    bool result = registerService.registerUser("newuser", "password123");
    EXPECT_TRUE(result);
}

TEST(RegisterTest, InvalidUsername) {
    Register registerService;
    bool result = registerService.registerUser("", "password123");
    EXPECT_FALSE(result);
}