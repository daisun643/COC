#include "gtest/gtest.h"
#include "Utils/API/URLEncoder.h"

TEST(URLEncoderTest, EncodeBasicString) {
    std::string input = "Hello World!";
    std::string expected = "Hello%20World!";
    EXPECT_EQ(URLEncoder::encode(input), expected);
}

TEST(URLEncoderTest, EncodeSpecialCharacters) {
    std::string input = "@#$%^&*()";
    std::string expected = "%40%23%24%25%5E%26%2A%28%29";
    EXPECT_EQ(URLEncoder::encode(input), expected);
}