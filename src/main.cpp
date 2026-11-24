#include "Allocation.h"
#include "DistributionDepot.h"
#include "Exceptions.h"
#include "IncidentCommander.h"
#include "ManifestFormat.h"
#include "Persistence.h"
#include "PriorityDispatchQueue.h"
#include "ReliefVehicle.h"

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

using namespace ReliefOps;
using namespace ReliefOps::Inventory;
using namespace ReliefOps::Logistics;

namespace {

const GeoPoint kHub{30.7268, 78.4354};
const GeoPoint kBhatwari{30.8096, 78.6158};
const GeoPoint kManeri{30.7393, 78.5316};
const GeoPoint kGangori{30.7580, 78.4440};

const char* const kInboundManifest =
    "RC-9001 RATION     400 12.0  0.030 GREEN\n"
    "RC-9002 TRAUMA_MED 200 4.5   0.012 RED\n"
    "RC-9003 WATER      20  210.0 0.220 GREEN\n"
    "RC-9004 BLANKET    600 0.9   0.004 GREEN\n"
    "RC-9002 TRAUMA_MED 200 4.5   0.012 RED\n";

struct Fleet {
    std::vector<ReliefVehicle*> vehicles;

    ReliefVehicle* add(ReliefVehicle* v)
    {
        vehicles.push_back(v);
        return v;
    }

    ~Fleet()
    {
        for (ReliefVehicle* v : vehicles) {
            delete v;
        }
    }
};

void section(const std::string& title)
{
    std::cout << '\n' << rule(kManifestWidth, '=') << title << '\n' << rule(kManifestWidth, '=');
}

void escalate(MissionParameters& mission, TriageLevel level)
{
    if (level > mission.priority) {
        mission.priority = level;
    }
}

RouteAssessment dispatch(DistributionDepot& depot, ReliefVehicle& vehicle, const Corridor& corridor,
                         std::vector<TransitRecord>& log, std::vector<ReliefCargo>& audit)
{
    const double payload = vehicle.payloadKg();
    std::size_t deducted = 0;
    try {
        for (; deducted < vehicle.cargoCount(); ++deducted) {
            depot -= vehicle.cargoAt(deducted);
        }
        const RouteAssessment route = vehicle.depart(corridor);
        log.push_back({vehicle.callSign(), corridor.name, payload, route.distanceKm, true});
        for (std::size_t i = 0; i < vehicle.cargoCount(); ++i) {
            audit.push_back(vehicle.cargoAt(i));
        }
        std::cout << std::fixed << std::setprecision(1)
                  << "  " << vehicle.callSign() << " departed on " << corridor.name << " via "
                  << route.note << ": " << route.distanceKm << " km, " << route.hours * 60.0
                  << " min, " << route.fuelL << " L\n";
        std::cout.unsetf(std::ios::fixed);
        vehicle.unloadAll();
        return route;
    } catch (const ReliefOpsException&) {
        for (std::size_t i = 0; i < deducted; ++i) {
            const ReliefCargo& lot = vehicle.cargoAt(i);
            depot.stockOf(lot.type()) += lot.units();
        }
        log.push_back({vehicle.callSign(), corridor.name, payload, 0.0, false});
        throw;
    }
}

void printRouteTable(const Fleet& fleet, const std::vector<Corridor>& corridors)
{
    for (const Corridor& c : corridors) {
        std::cout << c.name << '\n';
        for (const ReliefVehicle* v : fleet.vehicles) {
            const RouteAssessment r = v->evaluateRoute(c);
            std::cout << "  " << std::left << std::setw(10) << v->callSign()
                      << std::setw(20) << v->kind();
            if (r.feasible) {
                std::cout << std::fixed << std::setprecision(1) << std::right
                          << std::setw(7) << r.distanceKm << " km" << std::setw(8)
                          << r.hours * 60.0 << " min  (" << r.note << ")\n";
                std::cout.unsetf(std::ios::fixed);
            } else {
                std::cout << "blocked: " << r.note << '\n';
            }
            std::cout << std::right;
        }
    }
}

void run()
{
    section("ERL-HUB  Uttarkashi field distribution hub");

    DistributionDepot depot("Hub-Alpha", kHub);
    depot.setReserveFloor(SupplyType::TraumaMedicine, 80);

    std::istringstream inbound(kInboundManifest);
    ReliefCargo lot;
    while (inbound >> lot) {
        try {
            depot += lot;
            std::cout << "  received " << lot << '\n';
        } catch (const DuplicateCargoException& e) {
            std::cout << "  rejected: " << e.what() << '\n';
        }
    }
    std::cout << '\n' << depot;

    section("Fleet");
    Fleet fleet;
    ReliefVehicle* heli = fleet.add(new AerialTransport("HELI-01", 3000, 1800, 4.0, kHub, 220,
                                                        EmergencyLifeSupportUnit(4, 3000)));
    ReliefVehicle* quad = fleet.add(new AerialTransport("QUAD-07", 25, 3.0, 0.2, kHub, 60,
                                                        EmergencyLifeSupportUnit(0, 0)));
    ReliefVehicle* convoy = fleet.add(new GroundConvoy("CONVOY-A", 12000, 400, 0.9, kHub, 40));
    fleet.add(new AmphibiousCraft("BOAT-3", 1500, 200, 1.5, kHub, 25));
    ReliefVehicle* amu = fleet.add(new AmphibiousMedicalUnit("AMU-1", 2000, 300, 1.2, kHub, 35, 20,
                                                             EmergencyLifeSupportUnit(6, 4000)));
    std::cout << "Operational vehicles: " << TransportVehicle::operationalCount() << '\n';

    const std::vector<Corridor> corridors = {
        {"C1 NH-34 to Bhatwari", kHub, kBhatwari, 0.25, false, true},
        {"C2 Maneri spillway", kHub, kManeri, 0.10, true, true},
        {"C3 Gangori link road", kHub, kGangori, 0.80, false, false},
    };

    section("Route evaluation (ReliefVehicle* dispatch)");
    printRouteTable(fleet, corridors);

    section("Dispatch queue");
    PriorityDispatchQueue<DispatchRequest, UrgentMedicalFirst> queue;
    queue.push({"REQ-11", "Bhatwari", TriageLevel::Yellow, 40, false});
    queue.push({"REQ-12", "Maneri", TriageLevel::Red, 18, true});
    queue.push({"REQ-13", "Gangori", TriageLevel::Green, 120, false});
    queue.push({"REQ-14", "Bhatwari", TriageLevel::Red, 25, true});

    PriorityDispatchQueue<DispatchRequest, UrgentMedicalFirst> preview = queue;
    while (!preview.empty()) {
        const DispatchRequest r = preview.pop();
        std::cout << "  " << r.id << "  " << std::left << std::setw(10) << r.sector << std::right
                  << std::setw(7) << toString(r.triage) << std::setw(5) << r.casualties
                  << (r.medical ? "  medical" : "") << '\n';
    }

    IncidentCommander commander("IC Rawat");
    std::vector<TransitRecord> transitLog;
    std::vector<ReliefCargo> audit;

    section("REQ-14  trauma kits to Bhatwari");
    {
        const ReliefCargo kits = allocateSupplies(depot, TriageLevel::Red, 25);
        *heli += kits;
        escalate(heli->mission(), TriageLevel::Red);
        std::cout << *heli;
        dispatch(depot, *heli, corridors[0], transitLog, audit);
    }

    section("REQ-12  trauma kits to Maneri");
    {
        try {
            allocateSupplies(depot, TriageLevel::Red, 18);
        } catch (const InsufficientStockException& e) {
            std::cout << "  " << e.what() << '\n';
            commander.overrideReserve(depot, SupplyType::TraumaMedicine, 40);
            std::cout << "  " << depot.overrideLog().back() << '\n';
        }
        *amu += allocateSupplies(depot, TriageLevel::Red, 18);
        escalate(amu->mission(), TriageLevel::Red);
        std::cout << *amu;
        dispatch(depot, *amu, corridors[1], transitLog, audit);
    }

    section("REQ-11  rations to Bhatwari");
    {
        const ReliefCargo rations = allocateSupplies(depot, SupplyType::RationKit, 6.0);
        *convoy += rations;
        try {
            dispatch(depot, *convoy, corridors[0], transitLog, audit);
        } catch (const DamagedCorridorException& e) {
            std::cout << "  " << e.what() << '\n'
                      << "  stock rolled back, rations on hand: " << depot.stockOf(SupplyType::RationKit) << '\n';
            convoy->unloadAll();
            *heli += rations;
            dispatch(depot, *heli, corridors[0], transitLog, audit);
        }
    }

    section("REQ-13  blankets to Gangori");
    {
        *convoy += allocateSupplies(depot, SupplyType::ThermalBlanket, 0.48);
        dispatch(depot, *convoy, corridors[2], transitLog, audit);
    }

    section("QUAD-07  drone medical drop");
    {
        const ReliefCargo drop = ReliefCargo::standardLot(SupplyType::TraumaMedicine, 6, TriageLevel::Red);
        try {
            *quad += drop;
        } catch (const CargoOverweightException& e) {
            std::cout << "  " << e.what() << '\n';
            commander.authoriseOverload(*quad, 2.5);
            *quad += drop;
            std::cout << "  overload authorised by " << commander.name() << '\n';
        }
        try {
            dispatch(depot, *quad, corridors[0], transitLog, audit);
        } catch (const FuelDepletionException& e) {
            std::cout << "  " << e.what() << '\n';
            quad->unloadAll();
        }
    }

    section("Standby truck (copy constructor)");
    {
        TransportVehicle templateTruck("TRUCK-T", 8000, 300);
        templateTruck += ReliefCargo::standardLot(SupplyType::WaterTank, 10);
        templateTruck += ReliefCargo::standardLot(SupplyType::RationKit, 100);

        TransportVehicle standby(templateTruck);
        std::cout << "  manifests identical: " << std::boolalpha << (standby == templateTruck) << '\n';
        standby += ReliefCargo::standardLot(SupplyType::ThermalBlanket, 200);
        std::cout << "  after adding blankets to standby: " << (standby == templateTruck)
                  << ", template still has " << templateTruck.cargoCount() << " lots\n";
        std::cout << standby;
    }

    section("Persistence");
    std::filesystem::create_directories("out");
    saveTransitLog("out/transit.log.bin", transitLog);
    saveManifestAudit("out/manifest_audit.bin", audit);

    std::cout << "Transit log (reloaded)\n";
    for (const TransitRecord& r : loadTransitLog("out/transit.log.bin")) {
        std::cout << "  " << std::left << std::setw(10) << r.callSign << std::setw(24) << r.corridor
                  << std::right << tonnes(r.payloadKg) << (r.delivered ? "  delivered" : "  aborted") << '\n';
    }
    std::cout << "\nManifest audit (reloaded)\n" << manifestHeader;
    for (const ReliefCargo& c : loadManifestAudit("out/manifest_audit.bin")) {
        std::cout << row(c);
    }

    section("Hub status");
    std::cout << depot;
    std::cout << "Dispatched so far: " << std::fixed << std::setprecision(3)
              << TransportVehicle::totalDispatchedTonnes() << " t\n";
    std::cout << "Operational vehicles: " << TransportVehicle::operationalCount() << '\n';

    section("Standing down fleet");
}

}  // namespace

int main()
{
    try {
        run();
    } catch (const std::exception& e) {
        std::cerr << "fatal: " << e.what() << '\n';
        return 1;
    }
    std::cout << "Operational vehicles after stand-down: " << TransportVehicle::operationalCount() << '\n';
    return 0;
}
