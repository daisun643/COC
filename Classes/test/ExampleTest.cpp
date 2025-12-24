#include "gtest/gtest.h"
#include "Utils/API/example.h"

TEST(ExampleTest, BasicFunctionality) {
    Example example;
    EXPECT_EQ(example.getVersion(), "1.0.0");
}

TEST(ExampleTest, ProcessData) {
    Example example;
    std::string result = example.processData("test");
    EXPECT_FALSE(result.empty());
}