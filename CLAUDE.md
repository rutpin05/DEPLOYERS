# CLAUDE.md - DEPLOYERS v2

## IMPORTANT: Running Simulations (Read First!)

### Build Location (Windows)
MSBuild is at: `C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe`

```powershell
# Build Release
& 'C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe' Deployers.sln /p:Configuration=Release /p:Platform=x64 /m

# Build Debug
& 'C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe' Deployers.sln /p:Configuration=Debug /p:Platform=x64 /m
```

### Running Simulations - CRITICAL
1. **ALWAYS set `Autorun 1`** in input/reload .dep files, otherwise the GUI will wait for user interaction
2. **Monitor progress** by tailing the log file (`*_Log.dep`), not with sleep loops
3. **Performance**: Simulation runs ~3 years (36 months) per minute
4. **Check for errors**: Look for `*_Error.dep` files if simulation fails unexpectedly
5. **Avoid timeout loops**: Don't use `Start-Sleep` in polling loops - tail the log file instead

### Example: Running a simulation correctly
```powershell
# 1. Ensure Autorun is set in the .dep file
# 2. Start simulation
Start-Process -FilePath .\DeployersRelease.exe -ArgumentList 'MyInput.dep' -WorkingDirectory (Get-Location)

# 3. Monitor progress (don't use sleep loops!)
Get-Content 'SimName_Log.dep' -Wait -Tail 20
```

---

## Project Overview

DEPLOYERS v2 is an agent-based macroeconomic simulator that reconstructs complete economic systems through Darwinian evolutionary mechanisms. Unlike traditional economic models, it:

- Deploys economies endogenously to reproduce a target Social Accounting Matrix (SAM) from real countries
- Implements boundedly rational agents (workers, producers, government, banks) with no knowledge of economic theories
- Uses assisted production followed by calibration to guide simulated flows toward empirical targets
- Applies natural selection where firms survive based on market performance

## Build Commands

```powershell
# Windows (Visual Studio) - Release build
msbuild .\Deployers.sln /p:Configuration=Release /p:Platform=x64

# Windows - Debug build
msbuild .\Deployers.sln /p:Configuration=Debug /p:Platform=x64

# Run simulation
.\DeployersRelease.exe __InitialInput.dep
```

```bash
# Linux (CMake)
cmake -S centos/source -B centos/build
cmake --build centos/build
./centos/build/Deployers config.dep
```

## Project Structure

```
source/                    # All C++ implementation (~21K lines)
├── pch.h                 # Master header: ALL class declarations
├── DEPLOYERS.cpp         # Entry point, global state management
├── World.cpp             # Core simulation engine (3530 lines)
├── CData.cpp             # Parameter management & metrics (3024 lines)
├── Producer.cpp          # Producer/firm logic (2227 lines)
├── SAM.cpp               # Social Accounting Matrix (2078 lines)
├── Agent.cpp             # Base agent economics
├── Banks.cpp             # Banking system
└── [other source files]
include/                  # Third-party headers (GlgApi)
lib/                      # Precompiled third-party libraries
DOCS/                     # Documentation (migration plans, architecture)
```

## Key Files

| File | Purpose |
|------|---------|
| `pch.h` | Master header with ALL class declarations and type definitions |
| `DEPLOYERS.cpp` | Entry point, global RNG, simulation stage helpers |
| `World.cpp` | Main simulation loop, agent interactions, I/O |
| `CData.cpp` | Parameter parsing, GDP/CPI trackers, metrics |
| `Producer.cpp` | Firm production cycle (planning, procurement, execution) |
| `SAM.cpp` | Social Accounting Matrix: sector definitions, IO flows |

## Critical Patterns

### Serialization (STRICT backward compatibility)
- Every persistent class implements `operator<<` / `operator>>`
- **RULE**: Add new fields ONLY at END of serialization block
- Preserve exact spacing and brace layout
- Never change field order in existing serialization

### Randomness
- Always use `getRandom01()` global wrapper, never call `myRandomEngine()` directly
- `GoodQtty doubleToGQtty(double d)` implements probabilistic rounding
- RNG state persisted via `readrndstatus()` / `writerndstatus()`

### Monthly Producer Flow (DO NOT REORDER)
```cpp
1. monthInitialize()                    // Reset budgets
2. PlanProductionAndProcurement()       // Stock targets, procurement queries
3. ExecuteProcurement()                 // BuyGoods() rounds
4. ExecuteProduction()                  // RunProducer() with labor/capital/IC
5. ExecuteFixedCapitalInvestment()      // Depreciation & capital formation
6. ManageFinancing()                    // Operational credit or bootstrap
7. FinalizeMonthActivity()              // Update histories
```

### Accounting Validation
- `CData::CheckAccountingBalance()` validates cash conservation
- All transactions must update both sides (buyer, seller)
- SAM flows updated symmetrically for consumption, production, trade

## Type System

- `GoodType` (long): Index for products/sectors
- `AgentID` (long): Index for agents
- `AgentType` (long): -1=Worker, -2=Government, -3=CentralBank, -4=PrivateBank, >=0=Producer type
- `GoodQtty` (long long or double): Physical quantity of goods
- `AgentTypeID` (pair<AgentType, AgentID>): Complete agent identifier

## Global Singletons

Access via accessor functions:
- `World()` / `getWorld()` - CWorld instance
- `CentralBank()` - CCentralBank instance
- `Government()` - CGovernment instance
- `DEPData()` - CData instance

## Simulation Stages

```
Initialization → PreCalibration → AssistedCalibration →
TransitionCalibration → RealMarketSimulation
```

## Adding New Features

### New Input Parameter
1. Default value in `CData::initializeInputParameters()`
2. Read logic in `CData::readInputParameters()` if special parsing needed
3. Access via `getInputParameter("ParamName")`
4. Preserve field order in `writeInputParameters()`

### New Cash-Holding Entity
1. Update `CData::initialize()` to include in accounting
2. Extend `CData::CheckAccountingBalance()` for validation

### New Plot/Metric
1. Aggregate data in `CData::AnalizeLastMonth()`
2. Define window metadata in `readPlotsInputParameters()` / `writePlotsInputParameters()`

## What NOT to Change

- ___InitialInput.dep should never be modified, work on a copy
- Serialization block order/spacing (breaks snapshot compatibility)
- `CHistory` length semantics (cascades to all time-series)
- Month progression order in `RunSimulation()`
- Cash accounting logic (breaks SAM validation)
- SAM flow updates (must maintain double-entry symmetry)
- Neighborhood calculation (affects reproducibility)

## Persistence Verification

### How to Verify Persistence Works

**CRITICAL**: The correct persistence test procedure is:

1. **Run full simulation from initial input** with settings like:
   ```
   SimulateNYearsAfterCalibration 5
   SaveMonthsAfterCalibration 4      # saves at calibration+4 months (e.g., month 180)
   SaveMonthsFromMonth0 9999         # not needed for this test
   Autorun 1
   ```
   This creates snapshots: `..._156_Calibrated.dep`, `..._180.dep`, `..._215.dep`

2. **Rename the intermediate snapshot** for comparison:
   ```bash
   mv SimName_AT_180.dep SimName_AT_180_first.dep
   ```

3. **Reload FROM the calibrated snapshot** (NOT from initial input):
   ```bash
   ./DeployersRelease.exe SimName_AT_156_Calibrated.dep
   ```
   This creates a NEW `..._180.dep` snapshot

4. **Compare the two snapshots at month 180**:
   ```bash
   # Compare FirmBirths/FirmDeaths vectors
   grep "FirmBirths\|FirmDeaths" SimName_AT_180_first.dep
   grep "FirmBirths\|FirmDeaths" SimName_AT_180.dep
   # Values MUST be IDENTICAL
   ```

**Why this works**: Both runs go through months 156→180 with identical RNG state. If persistence is correct, the stochastic outcomes (firm births/deaths) will match exactly.

### Key Comparison Points
- **Firm death counts**: `// Firm Death Age Distribution - Month NNN (X deaths this month)`
- **Year distribution**: `Year 0: XX.X%` lines after firm death entries
- **FirmBirths/FirmDeaths vectors** in snapshot DEPData section

### Locating Curve Data in Snapshot Files

Snapshot files (`*_NNN.dep`, NOT `*_NNN_Log.dep`) contain serialized state. To find specific curve data:

#### FirmsBirth&Death Plot Data
```bash
# Find FirmBirths/FirmDeaths vectors in DEPData section
grep -n "FirmBirths\|FirmDeaths" SimName_AT_NNN.dep
```
**Format**: `FirmBirths { v1 v2 v3 ... v12 }` - last 12 months of birth counts

#### Plot Curve Data (Y-values)
```bash
# Find specific plot window data
grep -n "FirmsBirth&Death\|Indices_and" SimName_AT_NNN.dep
```
Plot data is in `PlotsData` section with structure:
```
PlotWindowName { nCurves N Curve0 { Yvalues { ... } } Curve1 { ... } }
```

#### Key Data Locations in CData Serialization
| Data | Search Pattern | Line Context |
|------|----------------|--------------|
| Firm births/deaths | `FirmBirths {` | DEPData section |
| Market prices | `MarketPrices_currMonth {` | DEPData section |
| GDP/CPI trackers | `GDPtracker {` / `CPItracker {` | DEPData section |
| Plot Y-values | `Yvalues {` | PlotsData subsection |

### Example: Verify FirmsBirth&Death Persistence
```powershell
# 1. Extract from baseline snapshot
Select-String "FirmBirths|FirmDeaths" ATw16_AT_215.dep

# 2. Compare firm death counts in logs
Select-String "Firm Death.*Month (200|210)" ATw16_AT_215_Log.dep
Select-String "Firm Death.*Month (200|210)" ATw16_AT_Log.dep  # reload log

# 3. Values must be IDENTICAL for persistence to be valid
```

### Persistence Test Checklist
- [ ] Baseline run completes without errors
- [ ] Intermediate snapshot loads successfully
- [ ] Reload run produces identical monthly metrics
- [ ] FirmBirths/FirmDeaths vectors match at comparison points
- [ ] RNG state restored (firm death distributions match exactly)

---

## Current Branch

**ToCOVID19** - Migration plan for implementing pandemic simulation features.

See `DOCS/toCov19_migration_plan.md` for active development phases.

## Documentation

- `.github/copilot-instructions.md` - Detailed AI coding guidelines
- `DOCS/Neighborhoods_Documentation.md` - Bounded rationality system
- `DOCS/LoadWorld_RNG_Refactor_Design.md` - Persistence mechanics
- `DOCS/DEPLOYERS_Comprehensive_Analysis*.md` - Full architecture
