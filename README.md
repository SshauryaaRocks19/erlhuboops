# ERL-HUB

Post-earthquake emergency relief logistics hub, written in C++17 for the OOP course project (Option 2).

The program simulates a field distribution hub after an earthquake. It books in relief cargo, keeps a reserve of trauma medicine, and ranks incoming requests with medical cases first. It then checks which vehicles can use each damaged corridor and sends out sorties. Every sortie is written to binary logs.

## Build and run

Needs a C++17 compiler (tested with Apple clang 17).

```
make
make run
```

`make clean` removes `build/` and `out/`. The binary logs are written to `out/` when the program runs.

## Layout

```
include/   headers
src/       implementation + main.cpp (demo scenario)
```

| File | Contents |
|------|----------|
| `Supplies.h` | `SupplyType`, `TriageLevel`, unit mass/volume per item |
| `ReliefCargo.h` | one cargo lot (`==`, `<<`, `>>`, binary read/write) |
| `DistributionDepot.h` | depot stock in `unordered_map`, reserve floors, `+=` / `-=` |
| `TransportVehicle.h` | dynamic manifest array, copy constructor, static fleet counters |
| `ReliefVehicle.h` | vehicle hierarchy and the amphibious medical unit (diamond) |
| `Equipment.h` | `GPSBeacon`, `EmergencyLifeSupportUnit` |
| `IncidentCommander.h` | friend class used for reserve and payload overrides |
| `Allocation.h` | overloaded `allocateSupplies` |
| `PriorityDispatchQueue.h` | heap-based template queue and the `UrgentMedicalFirst` functor |
| `ManifestFormat.h` | stream manipulators for the manifest tables |
| `Exceptions.h` | exception classes derived from `std::exception` |
| `BinaryIO.h`, `Persistence.h` | binary save/load for the transit log and manifest audit |
| `Geo.h` | `GeoPoint` and inline distance / fuel / payload helpers |

## Where each requirement is covered

**Unit I**
- Namespaces: `ReliefOps::Inventory` (cargo, depot, allocation) and `ReliefOps::Logistics` (vehicles, dispatch, persistence).
- Function overloading: `allocateSupplies(depot, SupplyType, double cargoSpaceM3)` for volume-based loads and `allocateSupplies(depot, TriageLevel, int casualties)` for trauma kits.
- Call by reference: `dispatch(DistributionDepot&, ReliefVehicle&, ...)` in `main.cpp` and `escalate(MissionParameters&, ...)`.
- Return by reference: `DistributionDepot::stockOf()` returns `int&`. This is used to roll stock back when a sortie fails. `TransportVehicle::mission()` and `cargoAt()` also return references.
- Inline functions: `haversineKm`, `payloadUtilisation`, `fuelForDistance` and `travelHours` in `Geo.h`.
- Friend class: `IncidentCommander` is a friend of `DistributionDepot` and `TransportVehicle`. It can lower a reserve floor or raise a vehicle's payload limit by up to 10%.

**Unit II**
- `ReliefCargo`, `DistributionDepot` and `TransportVehicle` keep all data private.
- Static members: `TransportVehicle::operationalCount()` and `totalDispatchedTonnes()`.
- Dynamic memory: `TransportVehicle` allocates its manifest with `new[]` in the constructor and grows it when full. It has a deep copy constructor, a copy assignment operator and a destructor that frees the array.
- Operators:
  - `depot += lot` restocks the depot and rejects duplicate lots.
  - `depot -= lot` deducts stock when a vehicle departs.
  - `lot == lot` compares manifest IDs.
  - `vehicle == vehicle` compares two manifests.
  - `<<` / `>>` read and write cargo lots. The inbound manifest is parsed with `>>`.

**Unit III**
- `ReliefVehicle` (abstract) has three subclasses: `AerialTransport`, `GroundConvoy` and `AmphibiousCraft`.
- `AmphibiousMedicalUnit` inherits from both `GroundConvoy` and `AmphibiousCraft`. Both use `virtual` inheritance from `ReliefVehicle`, so the unit has only one base object.
- Containership: every vehicle has a `GPSBeacon`. The aerial and amphibious medical units also have an `EmergencyLifeSupportUnit`.
- Runtime polymorphism: the route table calls `evaluateRoute()` through `ReliefVehicle*`. The fleet is deleted through base pointers at the end of the run, and the destructor trace shows the full chain for each vehicle.

**Unit IV**
- Exceptions: `CargoOverweightException`, `FuelDepletionException`, `DamagedCorridorException`, `InsufficientStockException`, `DuplicateCargoException` and `PersistenceException`. All of them derive from `ReliefOpsException`, which derives from `std::exception`.
- Binary files:
  - The transit log and the manifest audit are written to `<file>.tmp`, then renamed over the real file. A crash mid-write therefore never leaves a half-written log.
  - Each file has a header with a magic number, a version and a checksum, all checked on load.
  - Files are written in host byte order.
- STL and templates:
  - `PriorityDispatchQueue<T, Compare>` is built on `std::vector` plus the heap algorithms.
  - `UrgentMedicalFirst` is the comparator functor.
  - Stock is kept in `std::unordered_map`.
- I/O manipulators: `manifestHeader`, `rule(n)`, `row(lot)` and `tonnes(kg)`.

## Scenario in main.cpp

1. Book in four lots. The fifth line is a duplicate and gets rejected.
2. Commission five vehicles and evaluate three damaged corridors.
3. Queue four requests. The medical ones come out first.
4. REQ-14 goes out by helicopter.
5. REQ-12 runs short of trauma kits. The commander lowers the reserve, and the kits go out on the amphibious medical unit over water.
6. REQ-11 is sent by road first and fails with `DamagedCorridorException`. Stock is rolled back and the load is re-sent by helicopter.
7. REQ-13 goes by convoy.
8. The drone drop hits `CargoOverweightException`. The commander approves a small overload, then the drop fails with `FuelDepletionException`.
9. A standby truck is copy-constructed to show deep copy.
10. The logs are saved, reloaded and printed, then the fleet is stood down.
