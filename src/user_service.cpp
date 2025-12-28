#include "services/user_service.hpp"
#include <algorithm>

namespace services {

UserService::UserService(std::shared_ptr<contracts::IDatabase> database)
    : database_(std::move(database)) {}

int UserService::createUser(const std::string& name, const std::string& email) {
    // Проверка контракта: имя и email должны быть валидны
    if (!isValidName(name) || !isValidEmail(email)) {
        return -1;
    }

    contracts::User user;
    user.id = 0;  // Будет присвоен базой данных
    user.name = name;
    user.email = email;
    user.is_active = true;

    return database_->saveUser(user);
}

std::optional<contracts::User> UserService::getUser(int id) const {
    return database_->findUserById(id);
}

std::vector<contracts::User> UserService::getActiveUsers() const {
    auto all_users = database_->findAllUsers();
    std::vector<contracts::User> active_users;
    
    std::copy_if(all_users.begin(), all_users.end(), 
                 std::back_inserter(active_users),
                 [](const contracts::User& u) { return u.is_active; });
    
    return active_users;
}

bool UserService::deactivateUser(int id) {
    auto user = database_->findUserById(id);
    if (!user.has_value()) {
        return false;
    }

    user->is_active = false;
    return database_->updateUser(*user);
}

bool UserService::userExists(int id) const {
    return database_->findUserById(id).has_value();
}

bool UserService::isValidName(const std::string& name) const {
    // Контракт: имя не должно быть пустым
    return !name.empty();
}

bool UserService::isValidEmail(const std::string& email) const {
    // СЛОМАНО: убрали проверку @ в email
    // Теперь любой непустой email считается валидным
    // Это нарушает контракт!
    return !email.empty();
}

} // namespace services

