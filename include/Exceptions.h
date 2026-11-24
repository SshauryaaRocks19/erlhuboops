#pragma once

#include <exception>
#include <string>

namespace ReliefOps {

class ReliefOpsException : public std::exception {
public:
    explicit ReliefOpsException(std::string message) : message_(std::move(message)) {}
    const char* what() const noexcept override { return message_.c_str(); }

private:
    std::string message_;
};

class CargoOverweightException : public ReliefOpsException {
public:
    CargoOverweightException(const std::string& callSign, double attemptedKg, double limitKg);

    double attemptedKg() const noexcept { return attemptedKg_; }
    double limitKg() const noexcept { return limitKg_; }

private:
    double attemptedKg_;
    double limitKg_;
};

class FuelDepletionException : public ReliefOpsException {
public:
    FuelDepletionException(const std::string& callSign, double requiredL, double availableL);

    double shortfallL() const noexcept { return requiredL_ - availableL_; }

private:
    double requiredL_;
    double availableL_;
};

class DamagedCorridorException : public ReliefOpsException {
public:
    DamagedCorridorException(const std::string& corridor, const std::string& reason);

    const std::string& corridor() const noexcept { return corridor_; }

private:
    std::string corridor_;
};

class InsufficientStockException : public ReliefOpsException {
public:
    InsufficientStockException(const std::string& depot, const std::string& item,
                               int requested, int available);
};

class DuplicateCargoException : public ReliefOpsException {
public:
    explicit DuplicateCargoException(const std::string& manifestId);
};

class PersistenceException : public ReliefOpsException {
public:
    explicit PersistenceException(const std::string& detail);
};

}  // namespace ReliefOps
