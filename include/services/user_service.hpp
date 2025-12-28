#pragma once

#include "contracts/user_contract.hpp"
#include "contracts/database_contract.hpp"
#include <memory>

namespace services {

/**
 * @brief Реализация сервиса пользователей
 * 
 * Реализует контракт IUserService с использованием
 * абстракции базы данных для хранения.
 */
class UserService : public contracts::IUserService {
public:
    explicit UserService(std::shared_ptr<contracts::IDatabase> database);

    int createUser(const std::string& name, const std::string& email) override;
    std::optional<contracts::User> getUser(int id) const override;
    std::vector<contracts::User> getActiveUsers() const override;
    bool deactivateUser(int id) override;
    bool userExists(int id) const override;

private:
    std::shared_ptr<contracts::IDatabase> database_;

    // Валидация согласно контракту
    bool isValidName(const std::string& name) const;
    bool isValidEmail(const std::string& email) const;
};

} // namespace services

