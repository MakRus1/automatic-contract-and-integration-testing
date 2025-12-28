#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "services/user_service.hpp"
#include "services/database.hpp"

using namespace services;
using namespace contracts;

class UserServiceUnitTest : public ::testing::Test {
protected:
    void SetUp() override {
        database_ = std::make_shared<InMemoryDatabase>();
        userService_ = std::make_unique<UserService>(database_);
    }

    std::shared_ptr<InMemoryDatabase> database_;
    std::unique_ptr<UserService> userService_;
};

TEST_F(UserServiceUnitTest, CreateUser_ValidData_ReturnsPositiveId) {
    int id = userService_->createUser("John Doe", "john@example.com");
    EXPECT_GT(id, 0);
}

TEST_F(UserServiceUnitTest, CreateUser_EmptyName_ReturnsMinusOne) {
    int id = userService_->createUser("", "john@example.com");
    EXPECT_EQ(id, -1);
}

TEST_F(UserServiceUnitTest, CreateUser_InvalidEmail_ReturnsMinusOne) {
    int id = userService_->createUser("John Doe", "invalid-email");
    EXPECT_EQ(id, -1);
}

TEST_F(UserServiceUnitTest, GetUser_ExistingUser_ReturnsUser) {
    int id = userService_->createUser("John Doe", "john@example.com");
    auto user = userService_->getUser(id);
    
    ASSERT_TRUE(user.has_value());
    EXPECT_EQ(user->name, "John Doe");
    EXPECT_EQ(user->email, "john@example.com");
    EXPECT_TRUE(user->is_active);
}

TEST_F(UserServiceUnitTest, GetUser_NonExistingUser_ReturnsNullopt) {
    auto user = userService_->getUser(999);
    EXPECT_FALSE(user.has_value());
}

TEST_F(UserServiceUnitTest, GetActiveUsers_ReturnsOnlyActiveUsers) {
    int id1 = userService_->createUser("User1", "user1@test.com");
    int id2 = userService_->createUser("User2", "user2@test.com");
    userService_->deactivateUser(id1);

    auto activeUsers = userService_->getActiveUsers();
    
    ASSERT_EQ(activeUsers.size(), 1);
    EXPECT_EQ(activeUsers[0].id, id2);
}

TEST_F(UserServiceUnitTest, DeactivateUser_ExistingUser_ReturnsTrue) {
    int id = userService_->createUser("John Doe", "john@example.com");
    bool result = userService_->deactivateUser(id);
    
    EXPECT_TRUE(result);
    auto user = userService_->getUser(id);
    EXPECT_FALSE(user->is_active);
}

TEST_F(UserServiceUnitTest, DeactivateUser_NonExistingUser_ReturnsFalse) {
    bool result = userService_->deactivateUser(999);
    EXPECT_FALSE(result);
}

TEST_F(UserServiceUnitTest, UserExists_ExistingUser_ReturnsTrue) {
    int id = userService_->createUser("John Doe", "john@example.com");
    EXPECT_TRUE(userService_->userExists(id));
}

TEST_F(UserServiceUnitTest, UserExists_NonExistingUser_ReturnsFalse) {
    EXPECT_FALSE(userService_->userExists(999));
}

