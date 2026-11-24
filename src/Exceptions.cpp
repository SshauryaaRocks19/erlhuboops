#include "Exceptions.h"

#include <iomanip>
#include <sstream>

namespace ReliefOps {

namespace {

std::string overweightMessage(const std::string& callSign, double attemptedKg, double limitKg)
{
    std::ostringstream os;
    os << std::fixed << std::setprecision(1)
       << callSign << ": payload would reach " << attemptedKg
       << " kg, rated limit is " << limitKg << " kg";
    return os.str();
}

std::string fuelMessage(const std::string& callSign, double requiredL, double availableL)
{
    std::ostringstream os;
    os << std::fixed << std::setprecision(1)
       << callSign << ": sortie needs " << requiredL
       << " L of fuel, only " << availableL << " L on board";
    return os.str();
}

std::string stockMessage(const std::string& depot, const std::string& item,
                         int requested, int available)
{
    std::ostringstream os;
    os << depot << ": requested " << requested << " x " << item
       << " but only " << available << " can be released";
    return os.str();
}

}  // namespace

CargoOverweightException::CargoOverweightException(const std::string& callSign,
                                                   double attemptedKg, double limitKg)
    : ReliefOpsException(overweightMessage(callSign, attemptedKg, limitKg)),
      attemptedKg_(attemptedKg),
      limitKg_(limitKg)
{
}

FuelDepletionException::FuelDepletionException(const std::string& callSign,
                                               double requiredL, double availableL)
    : ReliefOpsException(fuelMessage(callSign, requiredL, availableL)),
      requiredL_(requiredL),
      availableL_(availableL)
{
}

DamagedCorridorException::DamagedCorridorException(const std::string& corridor,
                                                   const std::string& reason)
    : ReliefOpsException("corridor " + corridor + " unusable: " + reason),
      corridor_(corridor)
{
}

InsufficientStockException::InsufficientStockException(const std::string& depot,
                                                       const std::string& item,
                                                       int requested, int available)
    : ReliefOpsException(stockMessage(depot, item, requested, available))
{
}

DuplicateCargoException::DuplicateCargoException(const std::string& manifestId)
    : ReliefOpsException("cargo lot " + manifestId + " has already been booked in")
{
}

PersistenceException::PersistenceException(const std::string& detail)
    : ReliefOpsException("persistence: " + detail)
{
}

}  // namespace ReliefOps
