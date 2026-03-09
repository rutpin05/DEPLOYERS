
//  World.cpp

//   DEPLOYERS v2

// >>>>>>  READ notes at the beginning of pch.h  <<<<<<<<<

#include "./pch.h"

std::vector<int> CWorld::_yearlyDeathsByAge;  // Death counts by age for current year

// ======================  CMovingAverage  ==================================
// Constructor

CMovingAverage::CMovingAverage() : windowSize(10), sum(0.0) {};

CMovingAverage::CMovingAverage(size_t size) : windowSize(size), sum(0.0)
{
}

// Clear all values from the window
void CMovingAverage::clear() {
	window.clear();
	sum = 0.0;
}
// Add a new value and return the new average
double CMovingAverage::update(double newValue) {
	// Add the new value
	window.push_back(newValue);
	sum += newValue;

	// Remove oldest value if window is full
	if (window.size() > windowSize) {
		sum -= window.front();
		window.pop_front();
	}

	// Return current average
	return sum / window.size();
}

double CMovingAverage::getCurrentAverage() const {
	if (window.empty()) return 0.0;
	return sum / window.size();
}

// ===============    Neighbors  ==================

	// Save the complete state of the manager (objects and RNG state) to a file
bool CNeighborsVectorManager::saveState(const std::string& filename) const {
	try {
		std::ofstream outFile(filename, std::ios::binary);
		if (!outFile) {
			CWorld::ERRORmsg("Error: Could not open RNG state file " + filename + " for writing");
			return false;
		}

		// 1. Save the number of objects
		size_t objectCount = getAgentsPtrs().size();
		outFile.write(reinterpret_cast<const char*>(&objectCount), sizeof(objectCount));

		// 2. Save each object's ID
		for (const auto& obj : getAgentsPtrs()) {
			CAgentFID fid = obj->getFID();
			int type = fid._type;
			int id = fid._id;
			outFile.write(reinterpret_cast<const char*>(&type), sizeof(type));
			outFile.write(reinterpret_cast<const char*>(&id), sizeof(id));
		}

		// 3. Save the RNG state
		std::stringstream rngState;
		rngState << rng;
		std::string stateStr = rngState.str();

		// Save the size of the RNG state string and then the state itself
		size_t stateSize = stateStr.size();
		outFile.write(reinterpret_cast<const char*>(&stateSize), sizeof(stateSize));
		outFile.write(stateStr.c_str(), stateSize);

		outFile.close();
		// std::cout << "Complete manager state saved to " << filename << "\n";
		return true;
	}
	catch (const std::exception& e) {
		CWorld::ERRORmsg("Error saving manager state: " + string(e.what()));
		return false;
	}
}

// Load the complete state of the manager (objects and RNG state) from a file
bool CNeighborsVectorManager::loadState(const std::string& filename) {
	try {
		std::ifstream inFile(filename, std::ios::binary);
		if (!inFile) {
			CWorld::ERRORmsg("Error: Could not open RNG state file " + filename + " for reading");
			return false;
		}

		// Clear current objects
		AgentsPtrs().clear();

		// 1. Read the number of objects
		size_t objectCount;
		inFile.read(reinterpret_cast<char*>(&objectCount), sizeof(objectCount));

		// 2. Read and recreate each object
		for (size_t i = 0; i < objectCount; ++i) {
			int type, id;
			inFile.read(reinterpret_cast<char*>(&type), sizeof(type));
			inFile.read(reinterpret_cast<char*>(&id), sizeof(id));
			CAgentFID fid(type, id);
			CAgent* pAgent = getWorld().getFIDAgent(fid);
			if (type >= 0)
				pAgent = getWorld().getpProducers()->at(id);
			else if (type == -1)
				pAgent = getWorld().getpWorkers()->at(id);
			else
				CWorld::ERRORmsg("Error loading manager state: invalid object type " + to_string(type));

		AgentsPtrs().push_back(pAgent);
	}

	_isDirty = true;  // Invalidate cache after loading state

	// 3. Read the RNG state
	size_t stateSize;
	inFile.read(reinterpret_cast<char*>(&stateSize), sizeof(stateSize));		std::vector<char> stateBuffer(stateSize);
		inFile.read(stateBuffer.data(), stateSize);

		// Convert buffer to string and load into RNG
		std::string stateStr(stateBuffer.data(), stateSize);
		std::stringstream state(stateStr);
		state >> rng;

		inFile.close();
		//std::cout << "Complete manager state loaded from " << filename << "\n";
		return true;
	}
	catch (const std::exception& e) {
		//	std::cerr << "Error loading manager state: " << e.what() << "\n";
		CWorld::ERRORmsg("Error loading manager state: " + string(e.what()));
		return false;
	}
}

// Save the state of the random number generator to a file
bool CNeighborsVectorManager::saveRNGState(const std::string& filename) {
	try {
		std::ofstream outFile(filename, std::ios::binary);
		if (!outFile) {
			std::cerr << "Error: Could not open file " << filename << " for writing\n";
			return false;
		}

		// Convert RNG state to a string
		std::stringstream state;
		state << rng;
		std::string stateStr = state.str();

		// Save the size of the state string and then the state itself
		size_t stateSize = stateStr.size();
		outFile.write(reinterpret_cast<const char*>(&stateSize), sizeof(stateSize));
		outFile.write(stateStr.c_str(), stateSize);

		outFile.close();
		return true;
	}
	catch (const std::exception& e) {
		std::cerr << "Error saving RNG state: " << e.what() << "\n";
		return false;
	}
}

// Load the state of the random number generator from a file
bool CNeighborsVectorManager::loadRNGState(const std::string& filename) {
	try {
		std::ifstream inFile(filename, std::ios::binary);
		if (!inFile) {
			std::cerr << "Error: Could not open file " << filename << " for reading\n";
			return false;
		}

		// Read the size of the state string
		size_t stateSize;
		inFile.read(reinterpret_cast<char*>(&stateSize), sizeof(stateSize));

		// Read the state string
		std::vector<char> stateBuffer(stateSize);
		inFile.read(stateBuffer.data(), stateSize);

		// Convert buffer to string
		std::string stateStr(stateBuffer.data(), stateSize);

		// Load the state into the RNG
		std::stringstream state(stateStr);
		state >> rng;

		inFile.close();
		return true;
	}
	catch (const std::exception& e) {
		std::cerr << "Error loading RNG state: " << e.what() << "\n";
		return false;
	}
}

// Clear all objects and reset the manager
void CNeighborsVectorManager::clear() {
	AgentsPtrs().clear();
	_isDirty = true;  // Invalidate cache
	// Re-seed the random number generator if desired
	rng.seed(std::random_device{}());
	//CWorld::ERRORmsg("NeighborsVectorManager has been reset", false);
}

// Get sorted IDs (with caching for performance optimization)
const std::vector<AgentID>& CNeighborsVectorManager::getSortedIDs() const {
	if (_isDirty) {
		_sortedIDs.clear();
		_sortedIDs.reserve(getAgentsPtrs().size());
		
		for (auto* agent : getAgentsPtrs()) {
			_sortedIDs.push_back(agent->getID());
		}
		
		std::sort(_sortedIDs.begin(), _sortedIDs.end());
		_isDirty = false;  // Cache is now fresh
	}
	return _sortedIDs;
}

// Insert an agent pointer at a random position
void CNeighborsVectorManager::randomInsert(CAgent* ptr) {
	if (!ptr) {
		CWorld::ERRORmsg("Error: Cannot insert null pointer");
		return;
	}
	if (ptr->getAgentType() < GovernmentType)
	{
		CWorld::ERRORmsg("Error: Invalid agent type " + to_string(ptr->getAgentType()));
		return;
	}

	// If the vector is empty, simply add the agent
	if (AgentsPtrs().empty()) {
		AgentsPtrs().push_back(ptr);
		//	CWorld::ERRORmsg("Object with ID " + to_string(ptr->getID())
		//		+ " inserted at position 0", false);
		return;
	}

	// Generate a random position
	std::uniform_int_distribution<size_t> dist(0, AgentsPtrs().size());
	size_t position = dist(rng);

	// Insert the agent at the random position
	AgentsPtrs().insert(AgentsPtrs().begin() + position, ptr);
	_isDirty = true;  // Invalidate cache
	//CWorld::ERRORmsg("Agent with ID " + to_string(ptr->getID())
	//	+ " inserted at position " + to_string(position), false);
}

// Remove an agent by its FID
bool CNeighborsVectorManager::removeNeighbor(CAgentFID fid) {
	for (auto it = AgentsPtrs().begin(); it != AgentsPtrs().end(); ++it) {
		if ((*it)->getFID() == fid) {
			AgentsPtrs().erase(it);
			_isDirty = true;  // Invalidate cache
			//CWorld::ERRORmsg("Object with ID " + to_string(id) + " removed", false);
			return true;
		}
	}

	CWorld::ERRORmsg("No agent found of type " + to_string(fid._type) + " and ID " + to_string(fid._id));
	return false;
}

// Get a pointer to an agent by its ID
CAgent* CNeighborsVectorManager::getAgentPtr(CAgentFID fid) {
	for (auto it = AgentsPtrs().begin(); it != AgentsPtrs().end(); ++it) {
		if ((*it)->getFID() == fid) {
			// Return a pointer to the object in the vector
			return (*it);
		}
	}

	CWorld::ERRORmsg("No Agent found with type= " + to_string(fid._type) + " and ID= " + to_string(fid._id), false);
	return nullptr;
}

//======================  CPandemicState  ===================================
// COVID-19 pandemic state management (Darwinian ABM - no BLE)

const char* getPandemicPhaseName(PandemicPhase phase) {
	switch (phase) {
		case PandemicPhase::PrePandemic: return "PrePandemic";
		case PandemicPhase::AcuteLockdown: return "AcuteLockdown";
		case PandemicPhase::GradualRecovery: return "GradualRecovery";
		case PandemicPhase::NewNormal: return "NewNormal";
		case PandemicPhase::PostPandemic: return "PostPandemic";
		default: return "Unknown";
	}
}

void CPandemicState::initialize() {
	_isPandemicActive = (getInputParameter("PandemicActive") == 1.0);

	if (!_isPandemicActive) return;

	// Compute actual pandemic start month: FinishCalibrationAt + PandemicStartMonthAfterCalibrated
	long finishCalibAt = (long)getInputParameter("FinishCalibrationAt");
	long monthsAfterCalib = (long)getInputParameter("PandemicStartMonthAfterCalibrated");
	long pandemicDuration = (long)getInputParameter("PandemicDurationMonths");

	_pandemicStartMonth = finishCalibAt + monthsAfterCalib;
	_pandemicEndMonth = _pandemicStartMonth + pandemicDuration;

	_acutePhaseMonths = (long)getInputParameter("PandemicAcutePhaseMonths");
	_recoveryPhaseMonths = (long)getInputParameter("PandemicRecoveryMonths");
	_furloughSubsidyRate = getInputParameter("FurloughSubsidyRate");
	_furloughCoverageRate = getInputParameter("FurloughCoverageRate");
	_creditAvailabilityFactor = getInputParameter("CreditTighteningFactor");

	// Check for historical validation mode
	_useHistoricalData = (getInputParameter("UseHistoricalData") == 1.0);
	if (_useHistoricalData) {
		// Load historical data file
		const string& histFile = getDEPData().getHistoricalDataFile();
		if (!histFile.empty()) {
			loadHistoricalData(histFile);
			// Extend pandemic end month if historical data is longer than default duration
			if (!_historicalConfinement.empty()) {
				long historicalEnd = _pandemicStartMonth + (long)_historicalConfinement.size();
				if (historicalEnd > _pandemicEndMonth) {
					_pandemicEndMonth = historicalEnd;
				}
			}
		} else {
			LogFile() << "[PANDEMIC] WARNING: UseHistoricalData=1 but no HistoricalDataFile specified\n";
			_useHistoricalData = false;
		}
	}

	// Initialize sector impacts and confinement factors (only used in non-historical mode)
	if (!_useHistoricalData) {
		loadSectorImpacts();
		loadSectorConfinement();
	}

	LogFile() << "[PANDEMIC] Initialized: Active=" << _isPandemicActive
		<< ", FinishCalibrationAt=" << finishCalibAt
		<< ", MonthsAfterCalibrated=" << monthsAfterCalib
		<< ", ComputedStart=" << _pandemicStartMonth
		<< ", ComputedEnd=" << _pandemicEndMonth
		<< ", AcutePhase=" << _acutePhaseMonths << " months"
		<< ", RecoveryPhase=" << _recoveryPhaseMonths << " months"
		<< ", HistoricalMode=" << _useHistoricalData << "\n";
}

void CPandemicState::reinitializeTiming() {
	// Recalculate pandemic timing now that FinishCalibrationAt is finalized
	if (!_isPandemicActive) return;

	long finishCalibAt = (long)getInputParameter("FinishCalibrationAt");
	long monthsAfterCalib = (long)getInputParameter("PandemicStartMonthAfterCalibrated");
	long pandemicDuration = (long)getInputParameter("PandemicDurationMonths");

	long oldStart = _pandemicStartMonth;
	long oldEnd = _pandemicEndMonth;

	_pandemicStartMonth = finishCalibAt + monthsAfterCalib;
	_pandemicEndMonth = _pandemicStartMonth + pandemicDuration;

	LogFile() << "[PANDEMIC] Reinitializing timing after calibration:"
		<< " OldStart=" << oldStart << "->" << _pandemicStartMonth
		<< ", OldEnd=" << oldEnd << "->" << _pandemicEndMonth
		<< " (FinishCalibrationAt=" << finishCalibAt << ")\n";
}

void CPandemicState::updatePhase(long currentMonth) {
	if (currentMonth < _pandemicStartMonth) {
		_currentPhase = PandemicPhase::PrePandemic;
		_lockdownIntensity = 0.0;
		_exportDemandFactor = 1.0;
		_importSupplyFactor = 1.0;
		return;
	}

	if (currentMonth >= _pandemicEndMonth) {
		_currentPhase = PandemicPhase::PostPandemic;
		_lockdownIntensity = 0.0;
		_exportDemandFactor = 1.0;
		_importSupplyFactor = 1.0;
		return;
	}

	long monthsIntoPandemic = currentMonth - _pandemicStartMonth;
	long acuteEndMonth = _acutePhaseMonths;

	// NAIVE TEST MODE: Binary lockdown (full lockdown then instant open)
	// No gradual recovery - recovery EMERGES from the free market
	if (monthsIntoPandemic < acuteEndMonth) {
		// LOCKDOWN: Government ordered closure (EXOGENOUS)
		_currentPhase = PandemicPhase::AcuteLockdown;
		_lockdownIntensity = 1.0;  // Full lockdown (was 0.5)
		_exportDemandFactor = getInputParameter("ExportDemandShockAcute");
		_importSupplyFactor = getInputParameter("ImportSupplyShockAcute");
	}
	else {
		// FULLY OPEN: Recovery EMERGES from here (no imposed gradual recovery)
		_currentPhase = PandemicPhase::PostPandemic;
		_lockdownIntensity = 0.0;
		_exportDemandFactor = 1.0;
		_importSupplyFactor = 1.0;
	}
}

double CPandemicState::getDemandShock(GoodType sector) const {
	if (!_isPandemicActive || _lockdownIntensity == 0.0) return 1.0;

	auto it = _sectorDemandShock.find(sector);
	if (it == _sectorDemandShock.end()) return 1.0;

	// Interpolate based on lockdown intensity
	double baseShock = it->second;
	return 1.0 - _lockdownIntensity * (1.0 - baseShock);
}

double CPandemicState::getSupplyShock(GoodType sector) const {
	if (!_isPandemicActive || _lockdownIntensity == 0.0) return 1.0;

	auto it = _sectorSupplyShock.find(sector);
	if (it == _sectorSupplyShock.end()) return 1.0;

	double baseShock = it->second;
	return 1.0 - _lockdownIntensity * (1.0 - baseShock);
}

double CPandemicState::getLaborShock(GoodType sector) const {
	if (!_isPandemicActive || _lockdownIntensity == 0.0) return 1.0;

	auto it = _sectorLaborShock.find(sector);
	if (it == _sectorLaborShock.end()) return 1.0;

	double baseShock = it->second;
	return 1.0 - _lockdownIntensity * (1.0 - baseShock);
}

double CPandemicState::getConfinementFactor(GoodType sector) const {
	// Returns how OPEN a sector is (0 = closed, 1 = fully open)
	// This is PHYSICAL availability, not preference

	if (!_isPandemicActive || _lockdownIntensity == 0.0) return 1.0;

	auto it = _sectorConfinement.find(sector);
	if (it == _sectorConfinement.end()) {
		// Default: non-specified sectors are mostly open
		return 1.0;
	}

	double baseConfinement = it->second;  // Confinement at FULL lockdown

	// Interpolate based on lockdown intensity
	// At full lockdown (intensity=1.0): return baseConfinement
	// At no lockdown (intensity=0.0): return 1.0
	// Linear interpolation between
	return 1.0 - _lockdownIntensity * (1.0 - baseConfinement);
}

bool CPandemicState::isActive(long month) const {
	return _isPandemicActive &&
		month >= _pandemicStartMonth &&
		month < _pandemicEndMonth;
}

bool CPandemicState::isInPandemicPeriod() const {
	return isActive(currMonth());
}

void CPandemicState::loadSectorImpacts() {
	// Sector impacts controlled by AmplifyPandemicTo20PercentGDP parameter:
	// 0 = Poledna-realistic results (~6% GDP drop) - mild shocks (0.90-0.98)
	// 1 = Amplified for visual testing (~20% GDP drop) - stronger shocks

	const bool amplify = (getInputParameter("AmplifyPandemicTo20PercentGDP") == 1.0);
	LogFile() << "[PANDEMIC] loadSectorImpacts: amplified=" << amplify << "\n";

	auto getSectorType = [](const string& code) -> GoodType {
		return CProducer::getProducerTypeOfLabel("P_" + code);
	};

	// Lambda to set demand shock with realistic and amplified values
	auto setShock = [this, amplify](GoodType sector, double realistic, double amplified) {
		if (sector != UndefAgentType) {
			_sectorDemandShock[sector] = amplify ? amplified : realistic;
		}
	};

	string cc = getSAM().CountryCode();

	// HIGH IMPACT sectors (hospitality, travel, entertainment)
	setShock(getSectorType(cc + "_I"), 0.95, 0.55);      // Hotels/Restaurants
	setShock(getSectorType(cc + "_R90T92"), 0.95, 0.50); // Arts/Entertainment
	setShock(getSectorType(cc + "_R93"), 0.95, 0.60);    // Sports/Recreation
	setShock(getSectorType(cc + "_N79"), 0.95, 0.45);    // Travel agencies
	setShock(getSectorType(cc + "_H51"), 0.95, 0.55);    // Air transport
	setShock(getSectorType(cc + "_S96"), 0.95, 0.70);    // Personal services

	// MEDIUM IMPACT sectors (retail, manufacturing, transport)
	setShock(getSectorType(cc + "_G47"), 0.97, 0.80);    // Retail
	setShock(getSectorType(cc + "_G45"), 0.97, 0.75);    // Motor trade
	setShock(getSectorType(cc + "_H49"), 0.97, 0.75);    // Land transport
	setShock(getSectorType(cc + "_C29"), 0.97, 0.70);    // Motor vehicles mfg
	setShock(getSectorType(cc + "_C13T15"), 0.97, 0.75); // Textiles
	setShock(getSectorType(cc + "_F"), 0.97, 0.85);      // Construction

	// LOW IMPACT sectors (essential, can work from home)
	setShock(getSectorType(cc + "_J62T63"), 0.99, 0.95); // IT services
	setShock(getSectorType(cc + "_K64"), 0.99, 0.95);    // Financial services
	setShock(getSectorType(cc + "_Q"), 0.99, 1.10);      // Healthcare (increased demand)
	setShock(getSectorType(cc + "_D"), 0.99, 1.00);      // Utilities
	setShock(getSectorType(cc + "_A01"), 0.99, 1.00);    // Agriculture

	LogFile() << "[PANDEMIC] Loaded " << _sectorDemandShock.size() << " sector demand shocks\n";
}

void CPandemicState::loadSectorConfinement() {
	// PHYSICAL CONFINEMENT FACTORS - Based on Poledna et al. (2023)
	// These represent PHYSICAL CLOSURE of sectors during FULL LOCKDOWN
	// 0.0 = completely closed (cannot buy anything)
	// 1.0 = fully open (normal operations)
	//
	// Key difference from demand shocks:
	// - Demand shock: "People don't WANT to buy" -> money spent elsewhere
	// - Confinement: "People CAN'T buy" -> money becomes forced savings

	const bool amplify = (getInputParameter("AmplifyPandemicTo20PercentGDP") == 1.0);
	LogFile() << "[PANDEMIC] loadSectorConfinement: amplified=" << amplify << "\n";

	auto getSectorType = [](const string& code) -> GoodType {
		return CProducer::getProducerTypeOfLabel("P_" + code);
	};

	auto setConfinement = [this, amplify](GoodType sector, double realistic, double amplified) {
		if (sector != UndefAgentType) {
			_sectorConfinement[sector] = amplify ? amplified : realistic;
		}
	};

	string cc = getSAM().CountryCode();

	// CLOSED sectors during lockdown (5-15% open)
	setConfinement(getSectorType(cc + "_I"), 0.15, 0.05);      // Hotels/Restaurants
	setConfinement(getSectorType(cc + "_R90T92"), 0.05, 0.02); // Arts/Entertainment
	setConfinement(getSectorType(cc + "_R93"), 0.05, 0.02);    // Sports/Recreation
	setConfinement(getSectorType(cc + "_N79"), 0.05, 0.02);    // Travel agencies
	setConfinement(getSectorType(cc + "_H51"), 0.10, 0.05);    // Air transport
	setConfinement(getSectorType(cc + "_S96"), 0.10, 0.05);    // Personal services

	// PARTIALLY OPEN sectors (40-60% open)
	setConfinement(getSectorType(cc + "_G47"), 0.50, 0.30);    // Retail (non-essential)
	setConfinement(getSectorType(cc + "_G45"), 0.40, 0.25);    // Motor trade
	setConfinement(getSectorType(cc + "_F"), 0.60, 0.40);      // Construction
	setConfinement(getSectorType(cc + "_C29"), 0.50, 0.30);    // Motor vehicles mfg

	// MOSTLY OPEN sectors (70-90% open)
	setConfinement(getSectorType(cc + "_G46"), 0.80, 0.60);    // Wholesale
	setConfinement(getSectorType(cc + "_M69T70"), 0.80, 0.60); // Legal/accounting
	setConfinement(getSectorType(cc + "_M71"), 0.75, 0.55);    // Architecture/engineering

	// ESSENTIAL/FULLY OPEN sectors (100% open)
	setConfinement(getSectorType(cc + "_C10T12"), 1.00, 0.90); // Food processing
	setConfinement(getSectorType(cc + "_G47"), 0.70, 0.50);    // Retail (essential)
	setConfinement(getSectorType(cc + "_Q"), 1.00, 1.00);      // Healthcare
	setConfinement(getSectorType(cc + "_D"), 1.00, 1.00);      // Utilities
	setConfinement(getSectorType(cc + "_J62T63"), 0.95, 0.90); // IT (work from home)
	setConfinement(getSectorType(cc + "_K64"), 0.90, 0.80);    // Finance

	LogFile() << "[PANDEMIC] Loaded " << _sectorConfinement.size() << " sector confinement factors\n";
}

void CPandemicState::loadPolednaLaborShocks() {
	// Load sector-specific labor shocks based on AMS (Austrian Public Employment Service)
	// March 2020 unemployment inflow data, as used by Poledna et al. (2023)
	//
	// These represent LABOR AVAILABILITY (supply-side shock)
	// 0.20 = 80% labor reduction (severe unemployment surge)
	// 1.00 = no labor shock
	// >1.0 = increased labor demand (e.g., healthcare)

	LogFile() << "[PANDEMIC] loadPolednaLaborShocks: Loading AMS March 2020 labor shocks\n";

	auto getSectorType = [](const string& code) -> GoodType {
		return CProducer::getProducerTypeOfLabel("P_" + code);
	};

	string cc = getSAM().CountryCode();

	// Clear existing labor shocks
	_sectorLaborShock.clear();

	// SEVERE IMPACT (70-85% labor reduction) - AMS "Very High" unemployment inflow
	// Accommodation, arts, travel - effectively shut down
	if (getSectorType(cc + "_I") != UndefAgentType)
		_sectorLaborShock[getSectorType(cc + "_I")] = 0.20;           // Accommodation & food
	if (getSectorType(cc + "_R90T92") != UndefAgentType)
		_sectorLaborShock[getSectorType(cc + "_R90T92")] = 0.25;      // Arts & entertainment
	if (getSectorType(cc + "_R93") != UndefAgentType)
		_sectorLaborShock[getSectorType(cc + "_R93")] = 0.25;         // Sports & recreation
	if (getSectorType(cc + "_N79") != UndefAgentType)
		_sectorLaborShock[getSectorType(cc + "_N79")] = 0.15;         // Travel agencies

	// HIGH IMPACT (40-50% labor reduction) - AMS "High" unemployment inflow
	// Construction, retail, transport - significant disruption
	if (getSectorType(cc + "_F") != UndefAgentType)
		_sectorLaborShock[getSectorType(cc + "_F")] = 0.40;           // Construction
	if (getSectorType(cc + "_G47") != UndefAgentType)
		_sectorLaborShock[getSectorType(cc + "_G47")] = 0.50;         // Retail trade
	if (getSectorType(cc + "_G45") != UndefAgentType)
		_sectorLaborShock[getSectorType(cc + "_G45")] = 0.45;         // Motor vehicle trade
	if (getSectorType(cc + "_H49") != UndefAgentType)
		_sectorLaborShock[getSectorType(cc + "_H49")] = 0.45;         // Land transport
	if (getSectorType(cc + "_H51") != UndefAgentType)
		_sectorLaborShock[getSectorType(cc + "_H51")] = 0.30;         // Air transport
	if (getSectorType(cc + "_S96") != UndefAgentType)
		_sectorLaborShock[getSectorType(cc + "_S96")] = 0.40;         // Personal services

	// MEDIUM IMPACT (70-75% labor available) - AMS "Medium" unemployment inflow
	// Manufacturing, professional services - moderate disruption
	if (getSectorType(cc + "_C29") != UndefAgentType)
		_sectorLaborShock[getSectorType(cc + "_C29")] = 0.70;         // Motor vehicle manufacturing
	if (getSectorType(cc + "_C13T15") != UndefAgentType)
		_sectorLaborShock[getSectorType(cc + "_C13T15")] = 0.70;      // Textiles
	if (getSectorType(cc + "_M69T70") != UndefAgentType)
		_sectorLaborShock[getSectorType(cc + "_M69T70")] = 0.75;      // Legal & accounting
	if (getSectorType(cc + "_M71") != UndefAgentType)
		_sectorLaborShock[getSectorType(cc + "_M71")] = 0.75;         // Architecture & engineering
	if (getSectorType(cc + "_N") != UndefAgentType)
		_sectorLaborShock[getSectorType(cc + "_N")] = 0.70;           // Admin & support services

	// LOW IMPACT (95-100% labor available) - AMS "Low" or "Unchanged"
	// Finance, IT, essential services - minimal disruption
	if (getSectorType(cc + "_K64") != UndefAgentType)
		_sectorLaborShock[getSectorType(cc + "_K64")] = 0.95;         // Financial services
	if (getSectorType(cc + "_K65") != UndefAgentType)
		_sectorLaborShock[getSectorType(cc + "_K65")] = 0.95;         // Insurance
	if (getSectorType(cc + "_J62T63") != UndefAgentType)
		_sectorLaborShock[getSectorType(cc + "_J62T63")] = 1.00;      // IT services (work from home)
	if (getSectorType(cc + "_J58T60") != UndefAgentType)
		_sectorLaborShock[getSectorType(cc + "_J58T60")] = 0.95;      // Publishing & broadcasting
	if (getSectorType(cc + "_D") != UndefAgentType)
		_sectorLaborShock[getSectorType(cc + "_D")] = 1.00;           // Utilities
	if (getSectorType(cc + "_E") != UndefAgentType)
		_sectorLaborShock[getSectorType(cc + "_E")] = 1.00;           // Water & waste
	if (getSectorType(cc + "_A01") != UndefAgentType)
		_sectorLaborShock[getSectorType(cc + "_A01")] = 1.00;         // Agriculture

	// INCREASED DEMAND - Healthcare and essential food
	if (getSectorType(cc + "_Q") != UndefAgentType)
		_sectorLaborShock[getSectorType(cc + "_Q")] = 1.05;           // Healthcare (increased)
	if (getSectorType(cc + "_C10T12") != UndefAgentType)
		_sectorLaborShock[getSectorType(cc + "_C10T12")] = 1.00;      // Food processing

	LogFile() << "[PANDEMIC] Loaded " << _sectorLaborShock.size()
		<< " AMS-based labor shocks for Poledna replication\n";
}

void CPandemicState::loadHistoricalData(const string& filename) {
	// Load historical confinement data from CSV file for Austria validation test
	// Format: Month,Phase,I,R90T92,...,KurzarbeitRate,ExportFactor,ImportFactor
	
	ifstream file(filename);
	if (!file.is_open()) {
		LogFile() << "[PANDEMIC] ERROR: Cannot open historical data file: " << filename << "\n";
		return;
	}
	
	// Clear any existing historical data
	_historicalConfinement.clear();
	_historicalKurzarbeit.clear();
	_historicalExports.clear();
	_historicalImports.clear();
	_historicalPhaseNames.clear();
	
	string line;
	
	// Skip comment lines (starting with #)
	while (getline(file, line)) {
		if (line.empty()) continue;
		if (line[0] != '#') break;
	}
	
	// Parse header line to get sector column positions
	// Header format: Month,Phase,I,R90T92,R93,G47,...,KurzarbeitRate,ExportFactor,ImportFactor
	vector<string> headers;
	stringstream headerss(line);
	string token;
	while (getline(headerss, token, ',')) {
		// Trim whitespace
		size_t start = token.find_first_not_of(" \t\r\n");
		size_t end = token.find_last_not_of(" \t\r\n");
		if (start != string::npos) {
			headers.push_back(token.substr(start, end - start + 1));
		} else {
			headers.push_back(token);
		}
	}
	
	// Map column indices to sector GoodTypes (skip Month=0, Phase=1, and last 3 columns)
	map<int, GoodType> colToSector;
	string cc = getSAM().CountryCode();
	
	for (size_t i = 2; i < headers.size() - 3; i++) {
		string sectorCode = headers[i];
		GoodType gt = CProducer::getProducerTypeOfLabel("P_" + cc + "_" + sectorCode);
		if (gt != UndefAgentType) {
			colToSector[(int)i] = gt;
			// LogFile() << "[PANDEMIC] Mapped column " << i << " (" << sectorCode << ") to GoodType " << gt << "\n";
		} else {
			LogFile() << "[PANDEMIC] WARNING: Unknown sector code '" << sectorCode << "' in historical data\n";
		}
	}
	
	// Parse data rows
	while (getline(file, line)) {
		if (line.empty() || line[0] == '#') continue;
		
		// Parse row values
		vector<string> values;
		stringstream rowss(line);
		while (getline(rowss, token, ',')) {
			// Trim whitespace
			size_t start = token.find_first_not_of(" \t\r\n");
			size_t end = token.find_last_not_of(" \t\r\n");
			if (start != string::npos) {
				values.push_back(token.substr(start, end - start + 1));
			} else {
				values.push_back(token);
			}
		}
		
		if (values.size() < headers.size()) {
			LogFile() << "[PANDEMIC] WARNING: Incomplete row in historical data: " << line << "\n";
			continue;
		}
		
		// Store phase name (column 1)
		_historicalPhaseNames.push_back(values[1]);
		
		// Extract sector confinement factors
		map<GoodType, double> monthConfinement;
		for (const auto& pair : colToSector) {
			try {
				monthConfinement[pair.second] = stod(values[pair.first]);
			} catch (...) {
				monthConfinement[pair.second] = 1.0;  // Default to fully open on parse error
			}
		}
		_historicalConfinement.push_back(monthConfinement);
		
		// Extract policy parameters from last 3 columns
		int n = (int)values.size();
		try {
			_historicalKurzarbeit.push_back(stod(values[n-3]));
			_historicalExports.push_back(stod(values[n-2]));
			_historicalImports.push_back(stod(values[n-1]));
		} catch (...) {
			_historicalKurzarbeit.push_back(0.0);
			_historicalExports.push_back(1.0);
			_historicalImports.push_back(1.0);
		}
	}
	
	file.close();
	
	_useHistoricalData = true;
	_historicalDataFile = filename;
	
	LogFile() << "[PANDEMIC] Loaded " << _historicalConfinement.size() 
		<< " months of historical data from " << filename << "\n";
	LogFile() << "[PANDEMIC] Mapped " << colToSector.size() << " sector columns to GoodTypes\n";
}

void CPandemicState::updatePhaseHistorical(long currentMonth) {
	// HISTORICAL VALIDATION MODE: Apply actual Austrian confinement data month-by-month
	// This enables comparison of DEPLOYERS output to observed economic outcomes
	
	if (!_useHistoricalData) {
		updatePhase(currentMonth);  // Fall back to standard (Naive) logic
		return;
	}
	
	long m = currentMonth - _pandemicStartMonth;
	
	if (m < 0) {
		// Before pandemic starts
		_currentPhase = PandemicPhase::PrePandemic;
		_lockdownIntensity = 0.0;
		_exportDemandFactor = 1.0;
		_importSupplyFactor = 1.0;
		return;
	}
	
	if (m >= (long)_historicalConfinement.size()) {
		// After historical data ends: fully open
		_currentPhase = PandemicPhase::PostPandemic;
		_lockdownIntensity = 0.0;
		_exportDemandFactor = 1.0;
		_importSupplyFactor = 1.0;
		// Restore default confinement factors
		loadSectorConfinement();
		return;
	}
	
	// Apply historical confinement factors for this month
	_sectorConfinement = _historicalConfinement[m];
	
	// Apply historical policy parameters
	_furloughCoverageRate = _historicalKurzarbeit[m];
	_exportDemandFactor = _historicalExports[m];
	_importSupplyFactor = _historicalImports[m];
	
	// Calculate average lockdown intensity from sector confinement
	double sumClosedness = 0.0;
	for (const auto& pair : _sectorConfinement) {
		sumClosedness += (1.0 - pair.second);  // Closedness = 1 - openness
	}
	_lockdownIntensity = _sectorConfinement.empty() ? 0.0 : 
		sumClosedness / (double)_sectorConfinement.size();
	
	// Determine phase based on lockdown intensity
	if (_lockdownIntensity > 0.4) {
		_currentPhase = PandemicPhase::AcuteLockdown;
	} else if (_lockdownIntensity > 0.1) {
		_currentPhase = PandemicPhase::GradualRecovery;
	} else if (_lockdownIntensity > 0.02) {
		_currentPhase = PandemicPhase::NewNormal;
	} else {
		_currentPhase = PandemicPhase::PostPandemic;
	}
	
	// Log monthly status
	LogFile() << "[PANDEMIC HISTORICAL] Month " << currentMonth 
		<< " (m=" << m << "): Phase=" << (_historicalPhaseNames.size() > (size_t)m ? _historicalPhaseNames[m] : "?")
		<< ", AvgLockdown=" << (int)(_lockdownIntensity * 100) << "%"
		<< ", Kurzarbeit=" << (int)(_furloughCoverageRate * 100) << "%"
		<< ", Export=" << _exportDemandFactor
		<< ", Import=" << _importSupplyFactor << "\n";
}

void CPandemicState::resetMonthlyStats() {
	_firmsOnKurzarbeit = 0;
	_workersOnKurzarbeit = 0;
	_monthlyKurzarbeitPaid = 0.0;
	_monthlyForcedSavings = 0.0;
}

// Serialization operators for CPandemicState
ofstream& operator<<(ofstream& ofstrm, const CPandemicState& state) {
	ofstrm << "\n PandemicState {";
	ofstrm << "\n  isPandemicActive " << state._isPandemicActive;
	ofstrm << "\n  pandemicStartMonth " << state._pandemicStartMonth;
	ofstrm << "\n  pandemicEndMonth " << state._pandemicEndMonth;
	ofstrm << "\n  currentPhase " << (int)state._currentPhase;
	ofstrm << "\n  lockdownIntensity " << state._lockdownIntensity;
	ofstrm << "\n  acutePhaseMonths " << state._acutePhaseMonths;
	ofstrm << "\n  recoveryPhaseMonths " << state._recoveryPhaseMonths;
	ofstrm << "\n  exportDemandFactor " << state._exportDemandFactor;
	ofstrm << "\n  importSupplyFactor " << state._importSupplyFactor;
	ofstrm << "\n  furloughSubsidyRate " << state._furloughSubsidyRate;
	ofstrm << "\n  furloughCoverageRate " << state._furloughCoverageRate;
	ofstrm << "\n  creditAvailabilityFactor " << state._creditAvailabilityFactor;
	ofstrm << "\n  totalKurzarbeitPaid " << state._totalKurzarbeitPaid;
	ofstrm << "\n  firmsOnKurzarbeit " << state._firmsOnKurzarbeit;
	ofstrm << "\n  workersOnKurzarbeit " << state._workersOnKurzarbeit;
	ofstrm << "\n  monthlyKurzarbeitPaid " << state._monthlyKurzarbeitPaid;
	ofstrm << "\n  totalForcedSavings " << state._totalForcedSavings;
	ofstrm << "\n  monthlyForcedSavings " << state._monthlyForcedSavings;
	ofstrm << "\n }";
	return ofstrm;
}

ifstream& operator>>(ifstream& ifstrm, CPandemicState& state) {
	string word, bracket;
	int phaseInt;

	ifstrm >> word >> bracket;  // "PandemicState {"
	ifstrm >> word >> state._isPandemicActive;
	ifstrm >> word >> state._pandemicStartMonth;
	ifstrm >> word >> state._pandemicEndMonth;
	ifstrm >> word >> phaseInt;
	state._currentPhase = (PandemicPhase)phaseInt;
	ifstrm >> word >> state._lockdownIntensity;
	ifstrm >> word >> state._acutePhaseMonths;
	ifstrm >> word >> state._recoveryPhaseMonths;
	ifstrm >> word >> state._exportDemandFactor;
	ifstrm >> word >> state._importSupplyFactor;
	ifstrm >> word >> state._furloughSubsidyRate;
	ifstrm >> word >> state._furloughCoverageRate;
	ifstrm >> word >> state._creditAvailabilityFactor;
	ifstrm >> word >> state._totalKurzarbeitPaid;
	ifstrm >> word >> state._firmsOnKurzarbeit;
	ifstrm >> word >> state._workersOnKurzarbeit;

	// Handle newer fields for backward compatibility
	ifstrm >> word;
	if (word == "monthlyKurzarbeitPaid") {
		ifstrm >> state._monthlyKurzarbeitPaid;
		ifstrm >> word;
	}
	if (word == "totalForcedSavings") {
		ifstrm >> state._totalForcedSavings;
		ifstrm >> word;
	}
	if (word == "monthlyForcedSavings") {
		ifstrm >> state._monthlyForcedSavings;
		ifstrm >> bracket;  // "}"
	} else {
		// Old format - word is already the bracket
		state._monthlyKurzarbeitPaid = 0.0;
		state._totalForcedSavings = 0.0;
		state._monthlyForcedSavings = 0.0;
		bracket = word;
	}

	// Reload sector impacts after loading state
	state.loadSectorImpacts();
	state.loadSectorConfinement();

	return ifstrm;
}

// CWorld pandemic management methods
void CWorld::initializePandemic() {
	_pandemicState.initialize();
}

void CWorld::updatePandemicState() {
	if (!_pandemicState.isPandemicActive())
		return;

	// Track phase changes for logging
	static PandemicPhase lastLoggedPhase = PandemicPhase::PrePandemic;

	// Update pandemic phase based on current month
	// Use historical mode if enabled, otherwise use naive binary mode
	if (_pandemicState.isUsingHistoricalData()) {
		_pandemicState.updatePhaseHistorical(currMonth());
	} else {
		_pandemicState.updatePhase(currMonth());
	}

	bool phaseChanged = (_pandemicState.getCurrentPhase() != lastLoggedPhase);
	bool isPandemicMonth = _pandemicState.isActive(currMonth());

	if (phaseChanged) {
		LogFile() << "\n[PANDEMIC] Month " << currMonth() << ": Phase changed to "
			<< getPandemicPhaseName(_pandemicState.getCurrentPhase())
			<< " (Lockdown intensity: " << (_pandemicState.getLockdownIntensity() * 100) << "%)\n";
		lastLoggedPhase = _pandemicState.getCurrentPhase();
	}

	// Log pandemic status every 12 months or on phase change during active pandemic
	if (isPandemicMonth && (phaseChanged || currMonth() % 12 == 0 || currMonth() == _pandemicState.getPandemicStartMonth())) {
		LogFile() << "[PANDEMIC] Month " << currMonth()
			<< ": Lockdown=" << (_pandemicState.getLockdownIntensity() * 100) << "%"
			<< ", ImportFactor=" << _pandemicState.getImportSupplyFactor()
			<< ", ExportFactor=" << _pandemicState.getExportDemandFactor() << "\n";
	}

	// Reset monthly statistics
	_pandemicState.resetMonthlyStats();
}

//======================  BLE (Behavioral Learning Equilibrium)  ================
// Implementation of Poledna et al. (2023) expectation mechanism for fair comparison
// THREE-PHASE OPERATION: OFF -> TRAINING -> ACTIVE

void CBLEExpectations::initialize() {
	_alpha_Y = 0.99;      // Near unit root for output (high persistence)
	_beta_Y = 0.0;
	_alpha_pi = 0.95;     // Slightly lower persistence for inflation
	_beta_pi = 0.0;

	_outputHistory.clear();
	_inflationHistory.clear();

	_expectedGrowth = 0.0;
	_expectedInflation = 0.0;
	_isInitialized = true;
	_lastUpdateMonth = -1;

	// Initialize in OFF phase - will transition to TRAINING after calibration
	_phase = BLEPhase::OFF;
	_trainingStartMonth = -1;
	_trainingEndMonth = -1;

	// Use parameters from input if available
	_learningWindow = static_cast<int>(getInputParameter("BLELearningWindow"));
	_learningGain = getInputParameter("BLELearningGain");
	_minTrainingMonths = static_cast<int>(getInputParameter("BLEMinTrainingMonths"));
}

void CBLEExpectations::resetForTraining() {
	// Clear any data learned during calibration (which is artificial)
	_outputHistory.clear();
	_inflationHistory.clear();
	_expectedGrowth = 0.0;
	_expectedInflation = 0.0;
	_beta_Y = 0.0;
	_beta_pi = 0.0;
	_lastUpdateMonth = -1;
}

void CBLEExpectations::startTraining(long month) {
	if (_phase != BLEPhase::OFF) return;

	// Reset any stale data from initialization/calibration period
	resetForTraining();

	_phase = BLEPhase::TRAINING;
	_trainingStartMonth = month;
	_trainingEndMonth = month + _minTrainingMonths;

	LogFile() << "[BLE] === TRAINING PHASE STARTED ===" 
	          << "\n      Month: " << month
	          << "\n      Will activate at month: " << _trainingEndMonth
	          << " (after " << _minTrainingMonths << " months of free-market observation)"
	          << "\n      During training: learning ON, influence OFF\n";
}

void CBLEExpectations::activateIfReady(long month) {
	if (_phase != BLEPhase::TRAINING) return;

	// Check if we have enough training data
	bool enoughTime = (month >= _trainingEndMonth);
	bool enoughData = (_outputHistory.size() >= static_cast<size_t>(_learningWindow));

	if (enoughTime && enoughData) {
		_phase = BLEPhase::ACTIVE;

		LogFile() << "[BLE] === ACTIVE PHASE STARTED ===" 
		          << "\n      Month: " << month
		          << "\n      Trained for: " << (month - _trainingStartMonth) << " months"
		          << "\n      History size: " << _outputHistory.size() << " months"
		          << "\n      Learned parameters: beta_Y=" << _beta_Y << ", beta_pi=" << _beta_pi
		          << "\n      Current expectations: growth=" << (_expectedGrowth * 100) << "%, inflation=" << (_expectedInflation * 100) << "%"
		          << "\n      BLE now influences agent decisions\n";
	}
}

void CBLEExpectations::updateExpectations(double currentLogGDP, double currentInflation, long currentMonth) {
	if (!_isInitialized) {
		initialize();
	}

	// Only update during TRAINING or ACTIVE phases
	if (_phase == BLEPhase::OFF) {
		return;
	}

	// Avoid duplicate updates in same month
	if (currentMonth == _lastUpdateMonth) {
		return;
	}
	_lastUpdateMonth = currentMonth;

	// Store history
	_outputHistory.push_back(currentLogGDP);
	_inflationHistory.push_back(currentInflation);

	// Keep only recent history within learning window
	while (_outputHistory.size() > static_cast<size_t>(_learningWindow)) {
		_outputHistory.pop_front();
		_inflationHistory.pop_front();
	}

	// Need at least 2 data points to form expectations
	if (_outputHistory.size() < 2) {
		_expectedGrowth = 0.0;
		_expectedInflation = currentInflation;
		return;
	}

	// Form expectations using AR(1) rule
	// E[Y(t+1)] = alpha_Y * Y(t) + beta_Y  (log level)
	// We track growth rate, so: E[growth] = alpha_Y * last_growth + beta_Y
	double lastGrowth = _outputHistory.back() - _outputHistory[_outputHistory.size() - 2];
	_expectedGrowth = _alpha_Y * lastGrowth + _beta_Y;

	// Inflation expectation
	_expectedInflation = _alpha_pi * currentInflation + _beta_pi;

	// Learn parameters if we have enough history
	if (_outputHistory.size() >= static_cast<size_t>(_learningWindow)) {
		learnParameters();
	}
}

void CBLEExpectations::learnParameters() {
	// Constant gain learning (simplified Recursive Least Squares)
	// Updates parameters based on recent forecast errors

	if (_outputHistory.size() < 3) return;

	// Calculate forecast error for output growth
	double prevGrowth = _outputHistory[_outputHistory.size() - 2] - _outputHistory[_outputHistory.size() - 3];
	double prevExpectedGrowth = _alpha_Y * prevGrowth + _beta_Y;
	double actualGrowth = _outputHistory.back() - _outputHistory[_outputHistory.size() - 2];
	double forecastError_Y = actualGrowth - prevExpectedGrowth;

	// Update beta_Y with constant gain learning
	// beta captures the "intercept" - systematic bias in expectations
	_beta_Y += _learningGain * forecastError_Y;

	// Clamp beta to reasonable bounds
	_beta_Y = max(-0.1, min(0.1, _beta_Y));

	// Similar for inflation
	if (_inflationHistory.size() >= 2) {
		double prevInflation = _inflationHistory[_inflationHistory.size() - 2];
		double prevExpectedInflation = _alpha_pi * prevInflation + _beta_pi;
		double actualInflation = _inflationHistory.back();
		double forecastError_pi = actualInflation - prevExpectedInflation;

		_beta_pi += _learningGain * forecastError_pi;
		_beta_pi = max(-0.05, min(0.05, _beta_pi));
	}
}

// Serialization for BLE state
ofstream& operator<<(ofstream& ofstrm, const CBLEExpectations& ble) {
	ofstrm << "\n BLEExpectations {";
	ofstrm << "\n  isInitialized " << ble._isInitialized;
	ofstrm << "\n  lastUpdateMonth " << ble._lastUpdateMonth;
	ofstrm << "\n  phase " << static_cast<int>(ble._phase);
	ofstrm << "\n  trainingStartMonth " << ble._trainingStartMonth;
	ofstrm << "\n  trainingEndMonth " << ble._trainingEndMonth;
	ofstrm << "\n  minTrainingMonths " << ble._minTrainingMonths;
	ofstrm << "\n  alpha_Y " << ble._alpha_Y;
	ofstrm << "\n  beta_Y " << ble._beta_Y;
	ofstrm << "\n  alpha_pi " << ble._alpha_pi;
	ofstrm << "\n  beta_pi " << ble._beta_pi;
	ofstrm << "\n  expectedGrowth " << ble._expectedGrowth;
	ofstrm << "\n  expectedInflation " << ble._expectedInflation;
	ofstrm << "\n  learningWindow " << ble._learningWindow;
	ofstrm << "\n  learningGain " << ble._learningGain;

	// Serialize history
	ofstrm << "\n  outputHistorySize " << ble._outputHistory.size();
	ofstrm << "\n  outputHistory {";
	for (const auto& val : ble._outputHistory) {
		ofstrm << " " << val;
	}
	ofstrm << " }";

	ofstrm << "\n  inflationHistorySize " << ble._inflationHistory.size();
	ofstrm << "\n  inflationHistory {";
	for (const auto& val : ble._inflationHistory) {
		ofstrm << " " << val;
	}
	ofstrm << " }";

	ofstrm << "\n }";
	return ofstrm;
}

ifstream& operator>>(ifstream& ifstrm, CBLEExpectations& ble) {
	string word, bracket;
	size_t historySize;
	int phaseInt;

	ifstrm >> word >> bracket;  // "BLEExpectations {"
	ifstrm >> word >> ble._isInitialized;
	ifstrm >> word >> ble._lastUpdateMonth;
	ifstrm >> word >> phaseInt;
	ble._phase = static_cast<CBLEExpectations::BLEPhase>(phaseInt);
	ifstrm >> word >> ble._trainingStartMonth;
	ifstrm >> word >> ble._trainingEndMonth;
	ifstrm >> word >> ble._minTrainingMonths;
	ifstrm >> word >> ble._alpha_Y;
	ifstrm >> word >> ble._beta_Y;
	ifstrm >> word >> ble._alpha_pi;
	ifstrm >> word >> ble._beta_pi;
	ifstrm >> word >> ble._expectedGrowth;
	ifstrm >> word >> ble._expectedInflation;
	ifstrm >> word >> ble._learningWindow;
	ifstrm >> word >> ble._learningGain;

	// Read output history
	ifstrm >> word >> historySize;  // "outputHistorySize N"
	ifstrm >> word >> bracket;      // "outputHistory {"
	ble._outputHistory.clear();
	for (size_t i = 0; i < historySize; ++i) {
		double val;
		ifstrm >> val;
		ble._outputHistory.push_back(val);
	}
	ifstrm >> bracket;  // "}"

	// Read inflation history
	ifstrm >> word >> historySize;  // "inflationHistorySize N"
	ifstrm >> word >> bracket;      // "inflationHistory {"
	ble._inflationHistory.clear();
	for (size_t i = 0; i < historySize; ++i) {
		double val;
		ifstrm >> val;
		ble._inflationHistory.push_back(val);
	}
	ifstrm >> bracket;  // "}"
	ifstrm >> bracket;  // final "}"

	return ifstrm;
}

// CWorld BLE management methods
void CWorld::initializeBLE() {
	if (getInputParameter("UseBLE") == 1.0) {
		_bleExpectations.initialize();
		LogFile() << "[BLE] Behavioral Learning Equilibrium initialized in OFF phase"
			<< "\n      Window=" << _bleExpectations.getLearningWindow()
			<< ", Gain=" << _bleExpectations.getLearningGain()
			<< ", MinTrainingMonths=" << _bleExpectations.getMinTrainingMonths()
			<< "\n      Will start TRAINING after FinishCalibrationAt\n";
	}
}

void CWorld::updateBLEExpectations() {
	if (getInputParameter("UseBLE") != 1.0) {
		return;
	}

	long finishCalibAt = static_cast<long>(getInputParameter("FinishCalibrationAt"));

	// === PHASE TRANSITIONS ===

	// Start training when calibration ends (first month after FinishCalibrationAt)
	if (currMonth() == finishCalibAt + 1 && _bleExpectations.isOff()) {
		_bleExpectations.startTraining(currMonth());
	}

	// Check if ready to activate (during training phase)
	if (_bleExpectations.isTraining()) {
		_bleExpectations.activateIfReady(currMonth());
	}

	// === LEARNING (during TRAINING or ACTIVE phase) ===

	if (_bleExpectations.isOff()) {
		return;  // Still in OFF phase, don't learn
	}

	// Get current real GDP and calculate log
	double realGDP = getDEPData().getGDPtracker().getreal_gdp();
	if (realGDP <= 0) {
		return;  // Skip if no valid GDP data yet
	}
	double logGDP = log(realGDP);

	// Get current inflation rate from CPI tracker
	double inflation = getDEPData().getCPItracker().getCPIInflationRate();

	// Update BLE expectations (learning happens in both TRAINING and ACTIVE)
	_bleExpectations.updateExpectations(logGDP, inflation, currMonth());

	// Log BLE state periodically (every 12 months) if logging enabled
	if (getInputParameter("LogBLEExpectations") == 1.0 && currMonth() % 12 == 0) {
		LogFile() << "[BLE] Month " << currMonth() << " [" << _bleExpectations.getPhaseString() << "]"
			<< ": ExpGrowth=" << (_bleExpectations.getExpectedGrowth() * 100) << "%"
			<< ", ExpInflation=" << (_bleExpectations.getExpectedInflation() * 100) << "%"
			<< ", alpha_Y=" << _bleExpectations.getAlphaY()
			<< ", beta_Y=" << _bleExpectations.getBetaY()
			<< ", HistorySize=" << _bleExpectations.getLearningWindow() << "\n";
	}
}

//======================  CWorld  ===================================

// static
string CWorld::_SimulationName;
map<string, CSimulatedCountry*>* CWorld::_pSimulatedCountries;

CCentralBank*& pCentralBank() { return _pCentralBank; };
const CCentralBank* getpCentralBank() { return _pCentralBank; };
CCentralBank& CentralBank() { return *_pCentralBank; };
const CCentralBank& getCentralBank() { return *_pCentralBank; };

AgentType& CentralBankType() { return _CentralBankType; };
const AgentType getCentralBankType() { return _CentralBankType; };
AgentType& PrivateBankType() { return _PrivateBankType; };
const AgentType getPrivateBankType() { return _PrivateBankType; };

bool& BanksAreProducers() { return _BanksAreProducers; }
bool getBanksAreProducers() { return _BanksAreProducers; }

bool IsCentralBankType(GoodType gType) { return gType == getCentralBankType(); }
bool IsPrivateBankType(GoodType gType) { return gType == getPrivateBankType(); }
bool IsBankType(GoodType gTy) { return (gTy == getCentralBankType() || gTy == getPrivateBankType()); }
bool IsProducerBankType(GoodType gTy)
{
	return (IsBankType(gTy) && BanksAreProducers());
}
bool IsNonProducerBankType(GoodType gTy)
{
	return (IsBankType(gTy) && !BanksAreProducers());
}

CWorld::CWorld()
{
	pWorld() = this;

	_pSimulatedCountries = new map<string, CSimulatedCountry*>;
	_pSAM = nullptr;
	_pFIGARO = nullptr;

	_pGovernment = nullptr;

	pCentralBank() = nullptr;

	_pRandomListOfIndivIDs = new vector<AgentID>;
	_pInteractingAgents = new vector<CAgent*>;

	_pWorkers = new vector<CWorker*>;
	_pProducers = new vector<CProducer*>;

	_pFinancialMarket = nullptr;

	pExtSectorsToLabor() = nullptr;
}
CWorld::~CWorld()
{
	delete _pWorkers;
	delete _pProducers;
	delete _pInteractingAgents;
	delete _pRandomListOfIndivIDs;
}

ofstream& operator<<(ofstream& ofstrm, const CWorld& world)
{
	ofstrm << "//   DEPLOYERS 2.0  -----   \n\n";

	getDEPData().writeInputParameters(ofstrm);
	getSAM().writeSAMTXT(ofstrm);
	world.writeSnapshot(ofstrm);

	return ofstrm;
}
ifstream& operator>>(ifstream& ifstrm, CWorld& world)
{
	char a = 0;
	string word;
	double value = 0;

	ifstrm >> word;
	while (!ifstrm.eof())
	{
		if (word == "//")
		{
			getline(ifstrm, word); // line comment
			ifstrm >> word;
			continue;
		}
		else if (word == "/*")
		{
			while (ifstrm >> word, word != "*/"); // multiline comment
			ifstrm >> word;
			continue;
		}
		else if (word == "INPUT_PARAMETERS")
		{
			DEPData().readInputParameters();
			ifstrm >> word;

			continue;
		}
		else if (word == "COUNTRIES" || word == "rndstatus")
		{
			// they're read by SAM().readSAMTXT() and readrndstatus()
			const long maxchars = 30000;
			char* pField = new char[maxchars];
			while (ifstrm.peek() != '}')
				ifstrm.getline(pField, maxchars);
			ifstrm >> word;
			ifstrm >> word;
			delete[] pField;
			continue;
		}
		else if (word == "SNAPSHOT")
		{
			ifstrm >> word;
			continue;
		}
		else if (word == "MAP")
			break;
		else
		{
			CWorld::ERRORmsg(
				"Unexpected word while reading input " + word, true);
		}
	}

	return ifstrm;
}

/////////////////////  Static agents  & functions  /////////////////////////

void CWorld::ERRORmsg(string str, bool quit)
{
#ifdef DEPLOYERS_GRAPHICS
	char* msg = (char*)str.c_str();
	GlgError(GLG_USER_ERROR, msg);

	if (quit && getInputParameter("ExitOnQuit"))
	{
		ofstream ofstrm;
		ofstrm.open("_ERROR-message.dep");
		if (ofstrm.is_open())
		{
			ofstrm << endl << str << endl;
			ofstrm.close();
		}
		else
		{
			// Handle the error of opening the file
		}
		exit(-1);
	}
#elif defined(LINUX_VERSION)
	// NoGRAPH, Linux
	cout << str << endl;
	if (quit)
		exit(-2);
#endif
};
const double CWorld::getRandom01()
{
	uniform_real_distribution<> myRand01(0, 1);

	double rnd = myRand01(myRandomEngine());

	if (DebugLevel() > 5 && currMonth() >= getInputParameter("DebugFromMonth"))
		LogFile() << " rnd" << rnd;

	return rnd;
}

CAgent* CWorld::getFIDAgent(CAgentFID fid) const
{
	CAgent* pAgent = nullptr;
	if (fid._type >= 0)
		pAgent = getWorld().getpProducers()->at(fid._id);
	else if (fid._type == -1)
		pAgent = getWorld().getpWorkers()->at(fid._id);
	else
		CWorld::ERRORmsg("Error loading manager state: invalid object type "
			+ to_string(fid._type) + " and ID " + to_string(fid._id));

	return pAgent;
}

bool CWorld::getbRunning() const
{
	return _bRunning;
};
bool CWorld::getbFinished() const
{
	return _bFinished;
};
bool CWorld::getbPaused() const
{
	return _bPaused;
};
bool& CWorld::bRunning() { return _bRunning; };
bool& CWorld::bFinished() { return _bFinished; };
bool& CWorld::bPaused() { return _bPaused; };
bool& CWorld::bPlotBorders() { return _bPlotBorders; };
void CWorld::writeTimeAndDate(ofstream& ofstrm) const
{
#if defined _WINDOWS
	time_t rawtime;
	time(&rawtime); // get current calendar time

	struct tm timeinfo;
	localtime_s(&timeinfo, &rawtime);

	const long buffer_size = 256;
	char buffer[256];
	asctime_s(buffer, buffer_size, &timeinfo); //do the conversion

	ofstrm << buffer; // << endl;
#endif
}

void CWorld::addDemandEvent(AgentID buyerID, GoodType gType)
{
	if (gType < 0 || gType >= getSAM().getnPProducerTypes())
		return;

	long maxHistorySize = 2 * getSAM().getnPProducerTypes();
	if (maxHistorySize <= 0)
		return;

	_recentDemandEvents.push_back({ buyerID, gType });

	// Keep the deque size limited to the history size
	while (_recentDemandEvents.size() > (size_t)maxHistorySize) {
		_recentDemandEvents.pop_front();
	}
};

GoodType CWorld::getRandomGoodDemandedInNeighborhood(AgentID agentID)
{
	if (_recentDemandEvents.empty())
		return UndefGoodType;

	// Get the agent's neighborhood
	CAgent* pAgent = getFIDAgent(CAgentFID(WorkerType, agentID));
	if (!pAgent) return UndefGoodType; // Should not happen if called from a valid worker

	vector<CAgent*> neighbors;
	getNeighborsWorkersOf(pAgent, neighbors);

	// Create a set of neighbor IDs for quick lookup, including the agent itself
	std::set<AgentID> neighborhoodIDs;
	neighborhoodIDs.insert(agentID);
	for (const auto& neighbor : neighbors) {
		neighborhoodIDs.insert(neighbor->getID());
	}

	// Filter the demand events to find those from the neighborhood
	std::vector<GoodType> neighborhoodDemands;
	for (const auto& demandEvent : _recentDemandEvents) {
		if (neighborhoodIDs.count(demandEvent.first)) {
			neighborhoodDemands.push_back(demandEvent.second);
		}
	}

	// If there's no demand in this neighborhood, fall back to global demand
	if (neighborhoodDemands.empty()) {
		long randomIndex = getRandom01() * _recentDemandEvents.size();
		return _recentDemandEvents[randomIndex].second;
	}

	// Pick a random good from the neighborhood's demand
	long randomIndex = getRandom01() * neighborhoodDemands.size();
	return neighborhoodDemands[randomIndex];
};

CTypeDoubleMap& CWorld::ConsumPXFactor() { return _ConsumPXFactor; };
CTypeDoubleMap CWorld::getConsumPXFactor() const { return _ConsumPXFactor; };
CTypeDoubleMap& CWorld::RefTotalHouseholdPXConsum() { return _RefTotalHouseholdPXConsum; }
CTypeDoubleMap CWorld::getRefTotalHouseholdPXConsum() const { return _RefTotalHouseholdPXConsum; };
CTypeDoubleMap& CWorld::GovConsumPXFactor() { return _GovConsumPXFactor; };
CTypeDoubleMap CWorld::getGovConsumPXFactor() const { return _GovConsumPXFactor; };
CTypeDoubleMap& CWorld::RefGovConsumPX() { return _RefGovConsum; }
CTypeDoubleMap CWorld::getRefGovConsumPX() const { return _RefGovConsum; };

CTypeDoubleMap& CWorld::FixCapitalProductivity() { return _FixCapitalProductivity; }
CTypeDoubleMap CWorld::getFixCapitalProductivity() const { return _FixCapitalProductivity; };

CWorker* CWorld::newWorker()
{
	long ID;
	for (ID = 0; ID < (long)pWorkers()->size(); ++ID)
		if (pWorkers()->at(ID) == nullptr)
		{
			pWorkers()->at(ID) = new CWorker(ID);
			return pWorkers()->at(ID);
		}

	ID = (long)pWorkers()->size();

	CWorker* pWorker = new CWorker(ID);
	pWorkers()->push_back(pWorker);

	NeighboringWorkersManager().randomInsert(pWorker);

	return pWorker;
};
vector<CWorker*>*& CWorld::pWorkers() { return _pWorkers; };
vector<CWorker*>& CWorld::Workers() { return *_pWorkers; };
const vector<CWorker*>* CWorld::getpWorkers() const { return _pWorkers; };
long CWorld::getNWorkers() const { return (long)getpWorkers()->size(); }
void CWorld::setupExtSectorsToLaborArray() // static
{
	// ExtSectors may pay salaries to employees (see SAMEXT90agreg)

	delete pExtSectorsToLabor();
	pExtSectorsToLabor() = new vector<vector<GoodQtty>>(getExtSectors().size(),
		vector<GoodQtty>(SAM().AccountGroups().at("L").size(), 0));
	long ixSect = -1;
	for (auto& pairExtSect : getExtSectors())
	{
		++ixSect;
		auto ExtSectcol = pairExtSect.first;
		auto pExtSect = pairExtSect.second;

		long ixL = -1;
		for (auto& pLrowAcc : SAM().AccountGroups().at("L"))
		{
			++ixL;
			auto Lrow = pLrowAcc->accN();

			(*pExtSectorsToLabor())[ixSect][ixL] = (GoodQtty)(getSAM().getRowCol(Lrow, ExtSectcol)
				/ (12.0 * getSAM().getActive() * 0.01 * getSAM().getInitUnemploymentPercent()));
		}
	}
}

void CWorld::writeWorkers(ofstream& ofstrm) const
{
	ofstrm << "\n PropToConsume " << CWorker::getPropToConsume() << endl;
	ofstrm << "\n" << "Workers " << getpWorkers()->size() << " {";
	for (auto pIndiv : *getpWorkers())
	{
		ofstrm << *pIndiv;
	}
	ofstrm << " }\n";
};
void CWorld::readWorkers(ifstream& ifstrm)
{
	string word, bracket;
	long id = -1;

	ifstrm >> word >> CWorker::PropToConsume();
	ifstrm >> word >> id >> bracket; // "Workers: nn {"
	for (auto pInd : *pWorkers())// (long n = 0; n < nIndiv; ++n)
	{
		ifstrm >> word >> word; // "Indiv {";
		ifstrm >> word >> id;
		CWorker* pIndiv = pWorkers()->at(id);

		ifstrm >> *pIndiv;
		ifstrm >> bracket;// " }"
	}
	ifstrm >> bracket;// " }"
};

void CWorld::initializeWorkers()
{
	auto initialCash = DEPData().InputParameter("nInitSalaries") * getSAM().getInitSalary(); // some years salary;
	auto GFCFtype = getSAM().GFCFtype();
	CBank* pBank0 = nullptr;

	for (auto& pWorker : *pWorkers())
	{
		// force call to avoid not calling Random01(), due to Release optimization, if BondAssetsFraction is zero
		double rnd = getRandom01();
		pWorker->_BondToSharesRatio = rnd * DEPData().InputParameter("BondToSharesRatio");

		pWorker->assignHandLgroups();

		pWorker->myGoodsIwish().clear();

		pWorker->InitCash() = initialCash;
		pWorker->Cash() = initialCash;

		// ============ Open a Bank from start, for Indivs and Producers =============

		if (pWorker->getID() == 0)
		{
			if (DEPData().InputParameter("MaxNBanks") > 1)
				pWorkers()->at(0)->TryToFoundCommercialBank();
			pBank0 = pWorkers()->at(0)->pOwnedBank();
		}

		pWorker->pUsedBank() = pBank0;
	}

	CWorker::PropToConsume() = getInputParameter("PropToConsume");
	RefTotalHouseholdPXConsum().clear();
	ConsumPXFactor().clear();
	RefGovConsumPX().clear();
	GovConsumPXFactor().clear();

	// Workers have their own idiosyncratic preferences around average values:
	double IwishSpreadFraction = getInputParameter("IwishSpreadFraction");

	DEPData().InitialProducerTypes().clear();// ---------------   Start one Producer of each type
	for (GoodType gType = 0; gType < getSAM().getnPProducerTypes(); ++gType)
	{
		DEPData().InitialProducerTypes().push_back(gType);
		DEPData().nCurrentPProducers()[gType] = 0;
	}

	for (long gType = 0; gType < getSAM().getnPXproducerTypes(); ++gType)
	{
		RefTotalHouseholdPXConsum()[gType] = 0;
		ConsumPXFactor()[gType] = getInputParameter("InitialConsumPXFactor");

		RefGovConsumPX()[gType] = 0;
		GovConsumPXFactor()[gType] = 1.;
	}

	// Household cols
	for (auto& pWorker : *pWorkers())
	{
		pWorker->GoodsIhave()[GFCFtype] = 0; // GoodsIhave(GFCF) is flow, _myGFCF is the stock
		pWorker->MonthlyActivityMonth() = (long)(getRandom01() * getInputParameter("WorkDaysPerMonth"));

		for (long gType = 0; gType < getSAM().getnPXproducerTypes(); ++gType)
		{
			// The gType fraction of GFCF is included in its buyQtty below

			// ---------------   Start one PProducer of each type  -------------------
			if (gType < getSAM().getnPProducerTypes()
				&& DEPData().nCurrentPProducers().at(gType) < getInputParameter("MinCurrentPProducers"))
			{
				if (pWorker->getID() == gType
					&& getSAM().getSAMGrossOutput_mu(gType) > 0)
				{
					// World().newProducer will do this: if (getSAM().IsPublicSector(producerType))		pOwner = pGovernment();
					CProducer* pnewProd = newProducer(gType, pWorker);
				}
			}

			pWorker->setmyPriceOf(gType, 1.0);

			// Use Hcol data for each individual
			GoodType HCol = pWorker->getmyHgroupN();

			// total units to buy per year
			double buyQtty = (double)getSAM().getRowCol(gType, HCol);

			// Add this gType fraction of the GFCF qtty
			buyQtty += getSAM().getRowCol("GFCF", HCol) * SAM().GFCFfractionOf(gType);

			// Workers have their own preferences around the average value
			buyQtty *= ((1.0 - 0.5 * IwishSpreadFraction) + IwishSpreadFraction * getRandom01());

			// downscale to units per month, per active (nWorkers)
			GoodQtty buyQ = doubleToGQtty(buyQtty / (12.0 * getSAM().getActive()));

			// all sectors, no	if (buyQ > 0)
			pWorker->myGoodsIwish()[gType] = buyQ;

			pWorker->GoodsIhave()[gType] = 0; // no initial stock
		}
	}
};

CProducer* CWorld::newProducer(AgentType producerType, CAgent* pinitialOwner)
{
	if (producerType < 0 || producerType >= getSAM().getnPProducerTypes()
		|| getSAM().getSAMGrossOutput_mu(producerType) == 0)
		return nullptr;

	CProducer* pProducer = nullptr;
	long ID;
	for (ID = 0; ID < (long)pProducers()->size(); ++ID)
		if (pProducers()->at(ID) == nullptr) // available ID found, otherwise ID=size()
			break;

	if (getSAM().IsPublicSector(producerType))
		pinitialOwner = pGovernment();

	pProducer = new CProducer(ID, producerType, pinitialOwner);

	++DEPData().FirmBirths()[currMonth() % DEPData().FirmBirths().size()];

	pProducer->initialize();
	pProducer->MonthlyActivityMonth() = (long)(getRandom01() * getInputParameter("WorkDaysPerMonth"));

	int WorkDaysPerMonth = getInputParameter("WorkDaysPerMonth"); // 20 labor days
	int Nproduction_days = getInputParameter("Nproduction_days"); // 4 once a week
	int interval = WorkDaysPerMonth / Nproduction_days;
	pProducer->myFirstProductionDay() = (long)(getRandom01() * interval);

	if (ID < pProducers()->size())
		pProducers()->at(ID) = pProducer; // vector of all Producers
	else
		pProducers()->push_back(pProducer);

	if (pProducer != nullptr && producerType >= 0)
		NeighboringProducersManager().randomInsert(pProducer);

	if (pinitialOwner == nullptr) // ExtSector Producers
	{
		pProducer->pUsedBank() = nullptr;
	}
	else
	{
		if (pinitialOwner->getmyUsedBank() != nullptr)
			pProducer->pUsedBank() = pinitialOwner->getmyUsedBank(); // a private Bank
		else if (DEPData().InputParameter("MaxNBanks") == 1) // CentralBank only
			pProducer->pUsedBank() = pCentralBank();
		else if (DEPData().InputParameter("MaxNBanks") > 1)
			pProducer->pUsedBank() = pCentralBank()->getRandomBank();
	}

	/*
		// Lightweight birth log to trace startup dynamics
		try {
			LogFile() << "\n[BIRTH] Month:" << currMonth()
				<< " ID:" << pProducer->getID()
				<< " Type:" << getSAM().getAccNameOfN(producerType)
				<< " OwnerType:" << (pinitialOwner ? pinitialOwner->getAgentType() : UndefAgentType)
				<< " Cash:" << pProducer->getCash()
				<< " Bank:" << (pProducer->getmyUsedBank() ? pProducer->getmyUsedBank()->getAgentName() : string("None"));
			LogFile().flush();
		}
		catch (...) {  } // logging must never break simulation
	*/

	return pProducer;
};
vector<CProducer*>*& CWorld::pProducers() { return _pProducers; };
vector<CProducer*>& CWorld::Producers() { return *_pProducers; };
const vector<CProducer*>* CWorld::getpProducers() const { return _pProducers; };
long CWorld::getNProducers() const { return (long)getpProducers()->size(); }

void CWorld::defineProductsAndProducersFromSAM() const
{
	// rowQtties are interpreted as qtty units
	// Each product unit is the qtty of product purchased by 1 mu at the start of the simulation
	// Therefore, all initial prices are 1.0
	// A special case is CompEmployees: its unit is 1 worker and its 
	// initial price ("salary") is InitSalary * InitSalaryCalibFactor

	// PRODUCTs DEFINITIONS  ==================================================================

	//M J X needed?  >>>>>>>>>>>> nPXproducerTypes includes groups P, X  <<<<<<<<<<<<<<<<<<

	CGoods::clearGoodsDefinitions();

	CProducer::ProductSpecs().clear();
	auto nPXproducerTypes = getSAM().getnPXproducerTypes();// Types include groups P, X
	auto nPProducerTypes = getSAM().getnPProducerTypes();// Types include groups P, X
	// Define product specs from account data

	// Intermediate comsumption square matrix  ================================================

	CProducer::ProducerTypeOfLabel().clear();
	CProducer::ProducerLabelOfType().resize(nPXproducerTypes);

	string prevAccStr = "";
	bool newGroup = true;
	// colTypes include groups P, X
	for (GoodType colType = 0; colType < nPXproducerTypes; ++colType)
	{
		string gName = getSAM().getAccNameOfN(colType);
		CGoods::defineNewGoodType(gName);

		// Define its Producer type and name
		auto& account = SAM().Account(colType);
		string producerName = account.label(); // only one GoodType per ProducerType for SAMs
		CProducer::ProducerTypeOfLabel()[producerName] = colType;
		CProducer::ProducerLabelOfType()[colType] = producerName;

		CProductSpecs& productSpecs = *new CProductSpecs(colType);
		CProducer::ProductSpecs()[colType] = &productSpecs;

		productSpecs.bIsService() = true;

		// ExtSectors column values are Exports purchased by an ExtSect
		if (Account(colType).label().at(0) == 'X')
		{
			productSpecs.bImported() = true;
			//M J needed?productSpecs.GrossOpSurplus() = 1.0; 
			//M J needed?productSpecs.GrossOutput_mu() = 1;  to avoid div. by 0

			continue;
		}
		else for (int rowType = 0; rowType < nPXproducerTypes; ++rowType) // colTypes P only
		{
			productSpecs.bImported() = false;
			// Intermediate consumptions of colType's for current rowType
			double qtty = (double)Account(rowType).rowQtties()[colType]; // to be normalized below
			productSpecs.Needed_Init(rowType) = qtty; // to be normalized below
			// productSpecs.NeededPerUnit(rowType) will be defined below in GrossOutputAndDepreciationRate
		}
	}

	// PRODUCERs DEFINITIONS  ==================================================================

	CProducer::mProducerSpecs().clear();
	ExtSectors().clear();
	for (int producerType = 0; producerType < nPXproducerTypes; ++producerType)
	{
		GoodType gType = producerType; // one ProducerType (sector) per GoodType
		CProducerSpecs& producerSpecs = *new CProducerSpecs(gType);

		CProducer::mProducerSpecs()[gType] = &producerSpecs;

		// Products of each producer: ONLY ONE, productType = producerType

		if (getSAM().IsExtSectType(producerType)) // each ExtSect type is unique Producer
		{
			CExtSect* pExtS = new CExtSect(producerType, producerType);
			addExtSect(producerType, pExtS); // build map: ExtSect(producerType)
		}
	}

	GrossOutputAndDepreciationRate();
};

void CWorld::GrossOutputAndDepreciationRate() const
{
	assert(DEPData().getDepreciationRate().size() == 0); // initialization at currMonth = -1

	for (long gType = 0; gType < getSAM().getnPProducerTypes(); ++gType)
	{
		DEPData().DepreciationRate()[gType] = getInputParameter("DepreciationRate");
		DEPData().DepreciationFraction()[gType] = 0.;
		DEPData().prevDepreciationFraction()[gType] = 0.;

		CProductSpecs& productSpecs = *CProducer::ProductSpecs()[gType];
		productSpecs.GrossOutput_mu() = getSAM().getSAMGrossOutput_mu(gType);
	}

	// NORMALIZATION with productSpecs.GrossOutput_mu() from above ================================

	GoodType gType = UndefGoodType;
	for (int colType = 0; colType < getSAM().getnPProducerTypes(); ++colType)
	{
		auto& acc = Account(colType);
		auto& productSpecs = *CProducer::ProductSpecs()[colType];

		for (auto& pair : productSpecs.Needed_Init())
			productSpecs.NeededPerUnit(pair.first) = pair.second / productSpecs.GrossOutput_mu();

		// ============ = Possible (implemented) account rows below the Producers rows  ======================== =

		productSpecs.CompEmployees() =
			Account(getSAM().getAccNofName("CompEmployees")).rowQtties().at(colType)
			/ productSpecs.GrossOutput_mu();

		productSpecs.GrossOpSurplus() =
			Account(getSAM().getAccNofName("GrossOpSurplus")).rowQtties().at(colType)
			/ productSpecs.GrossOutput_mu();

		if (productSpecs.GrossOpSurplus() < 0)
		{
			auto sectorName = getSAM().getAccNameOfN(colType);
			long valM = productSpecs.GrossOpSurplus() * 1.e-6;
			CWorld::ERRORmsg("WARNING: GrossOpSurplus of " + sectorName + " sector = "
				+ to_string(valM) + " millions < 0. *** Setting it to 0 ***", false);
			productSpecs.GrossOpSurplus() = 0.;
		}

		if (gType = getSAM().getAccNofName("TaxProduction"), gType != UndefGoodType)
			productSpecs.TaxProduction() = Account(gType).rowQtties().at(colType)
			/ productSpecs.GrossOutput_mu();

		if (gType = getSAM().getAccNofName("TaxProducts"), gType != UndefGoodType)
			productSpecs.TaxProducts() = Account(gType).rowQtties().at(colType)
			/ productSpecs.GrossOutput_mu();

		if (gType = getSAM().getAccNofName("IRPF"), gType != UndefGoodType)
			productSpecs.IRPF() = Account(gType).rowQtties().at(colType)
			/ productSpecs.GrossOutput_mu();

		if (gType = getSAM().getAccNofName("TaxImportCE"), gType != UndefGoodType)
			productSpecs.TaxImportCE() = Account(gType).rowQtties().at(colType)
			/ productSpecs.GrossOutput_mu();

		if (gType = getSAM().getAccNofName("TaxImportRW"), gType != UndefGoodType)
			productSpecs.TaxImportRW() = Account(gType).rowQtties().at(colType)
			/ productSpecs.GrossOutput_mu();

		if (gType = getSAM().getAccNofName("Households"), gType != UndefGoodType)
			productSpecs.Households() = Account(gType).rowQtties().at(colType)
			/ productSpecs.GrossOutput_mu();

		if (gType = getSAM().getAccNofName("Government"), gType != UndefGoodType)
			productSpecs.Government() = Account(gType).rowQtties().at(colType)
			/ productSpecs.GrossOutput_mu();
	}
};

void CWorld::writeProducers(ofstream& ofstrm) const
{
	ofstrm << "\n" << "Producers " << getpProducers()->size() << " {";
	const CProducer* pProducer = nullptr;
	for (auto pProducer : *getpProducers())
	{
		if (pProducer != nullptr)
		{
			ofstrm << "\n";
			ofstrm << *pProducer << endl;
		}
	}

	ofstrm << " }\n";
};
void CWorld::readProducers(ifstream& ifstrm)
{
	string name, word, bracket;
	long id = -1;

	// set up IDs and pointers to producers and individuals

	long nProducers = 0, nIndiv;

	ifstrm >> name >> nProducers;
	ifstrm >> name >> nIndiv;

	pWorkers()->clear();
	pProducers()->clear();

	for (long n = 0; n < nIndiv; ++n)
		CWorker* pIndiv = newWorker();

	long indivN = 0;
	string producerLabel;
	CWorker* pIndiv = nullptr;
	ifstrm >> word >> nProducers >> bracket; // "Producers: nProds {"

	pProducers()->resize(nProducers, nullptr);
	while (ifstrm >> producerLabel, producerLabel != "}")
	{
		// "producerLabel { ID id"
		ifstrm >> bracket >> word >> id;

		(*pProducers())[id] = new CProducer(id, getSAM().getAccNofLabel(producerLabel), nullptr);

		ifstrm >> *(*pProducers())[id];
	}
};

void CWorld::readDEPData(string& filename)
{
	ifstream& ifstrm = *new ifstream();
	ifstrm.open(filename);
	if (ifstrm.fail())
		CWorld::ERRORmsg("Couldn't read ****" + filename, true);

	DEPData().readPlotsInputParameters(filename);

	DEPData().initializePlotsInputParameters();

	string name;
	while (ifstrm >> name, !ifstrm.eof() && name != "DEPData");

	if (!ifstrm.eof())
	{
		ifstrm >> *pDEPData();
		ifstrm.close();
	}

	return;
}

void CWorld::writeSnapshot(ofstream& ofstrm) const
{
	ofstrm << "\nSNAPSHOT {\n";

	ofstrm << "\n currYear " << getDEPData().getCurrYear()
		<< " currMonth " << currMonth()
		<< " bRunning " << getbRunning()
		<< " bFinished " << getbFinished()
		<< " bPaused 1" << endl;

	ofstrm << "\n InitSalaryCalibFactor " << getInitSalaryCalibFactor();

	ofstrm << "\n ConsumPXFactor ";
	ofstrm << getConsumPXFactor() << endl;
	ofstrm << "\n RefTotalHouseholdPXConsum ";
	ofstrm << getRefTotalHouseholdPXConsum() << endl;

	ofstrm << "\n GovConsumPXFactor ";
	ofstrm << getGovConsumPXFactor() << endl;
	ofstrm << "\n RefGovConsum ";
	ofstrm << getRefGovConsumPX() << endl;

	ofstrm << "\n RefGDP_VA " << getRefGDP_VA() << endl;

	// GFCF stability and wealth target tracking for dynamic AssistedProductionUpto
	ofstrm << "\n bIndivsWealthTargetReached " << _bIndivsWealthTargetReached;
	ofstrm << " monthIndivsWealthTargetReached " << _monthIndivsWealthTargetReached;
	ofstrm << "\n bGFCFStabilized " << _bGFCFStabilized;
	ofstrm << " monthGFCFStabilized " << _monthGFCFStabilized;
	ofstrm << " prevGFCF " << _prevGFCF;
	ofstrm << " consecutiveStableGFCFMonths " << _consecutiveStableGFCFMonths;
	ofstrm << " bCalibrationEndpointSet " << _bCalibrationEndpointSet << endl;

	ofstrm << "\n FixCapitalProductivity ";
	ofstrm << getFixCapitalProductivity();

	ofstrm << "\n\n nProducers " << getpProducers()->size();
	ofstrm << "\n nWorkers " << getpWorkers()->size() << endl;
	assert(getnWorkers() == getpWorkers()->size());

	writeProducers(ofstrm);
	writeWorkers(ofstrm);

	ofstrm << "\n Government {";
	ofstrm << getGovernment();
	ofstrm << " }" << endl;

	ofstrm << "\n ExtSectors {";
	for (auto& gpair : _pExtSect)
	{
		ofstrm << "\n " << getSAM().getAccNameOfN(gpair.first) << " {" << endl;
		ofstrm << *gpair.second;
		ofstrm << " }" << endl;
	}
	ofstrm << " }" << endl;

	if (DEPData().InputParameter("MaxNBanks") > 0)
	{
		ofstrm << "\n CentralBank {";
		ofstrm << getCentralBank();
		ofstrm << " }" << endl;

		ofstrm << "\n Banks {";
		for (const auto& bpair : getCentralBank().getBanks())
		{
			ofstrm << "\n PrivateBank {";
			ofstrm << *bpair.second;
			ofstrm << " }" << endl;
		}
		ofstrm << " }" << endl;
	}

	ofstrm << "\n FinancialMarket {";
	ofstrm << getFinancialMarket();
	ofstrm << " }" << endl;

	ofstrm << "\n RecentDemandEvents {";
	for (const auto& event : _recentDemandEvents)
	{
		ofstrm << " " << event.first << " " << event.second;
	}
	ofstrm << " }" << endl;

	// COVID-19 pandemic state (Darwinian ABM approach)
	ofstrm << _pandemicState;

	// BLE (Behavioral Learning Equilibrium) state - Poledna parallel comparison
	if (getInputParameter("UseBLE") == 1.0) {
		ofstrm << _bleExpectations;
	}

	ofstrm << "}\n";
};
void CWorld::readSnapshot(string fileNameExt)
{
	string word, word1, word2;
	long currentYear = 0, currentMonth = 0;

	ifstream ifstrm(fileNameExt);
	if (ifstrm.fail())
		ERRORmsg("Failed to open " + fileNameExt, true);

	while (ifstrm >> word, word != "SNAPSHOT")
		if (ifstrm.eof())
			return;

	ifstrm >> word; // "{"
	ifstrm
		>> word >> currentYear // getCurrYear() is calculated from currMonth value
		>> word >> currentMonth
		>> word >> bRunning()
		>> word >> bFinished()
		>> word >> bPaused();

	InitMonth() = currentMonth;
	bRunning() = true;
	bFinished() = false;
	bPaused() = true;

	_currMonth = currentMonth;

	ifstrm >> word >> InitSalaryCalibFactor();

	ifstrm >> word;
	ifstrm >> ConsumPXFactor();
	ifstrm >> word;
	ifstrm >> RefTotalHouseholdPXConsum();

	ifstrm >> word;
	ifstrm >> GovConsumPXFactor();
	ifstrm >> word;
	ifstrm >> RefGovConsumPX();

	ifstrm >> word >> RefGDP_VA();

	// GFCF stability and wealth target tracking for dynamic AssistedProductionUpto
	ifstrm >> word >> _bIndivsWealthTargetReached;
	ifstrm >> word >> _monthIndivsWealthTargetReached;
	ifstrm >> word >> _bGFCFStabilized;
	ifstrm >> word >> _monthGFCFStabilized;
	ifstrm >> word >> _prevGFCF;
	ifstrm >> word >> _consecutiveStableGFCFMonths;
	ifstrm >> word >> _bCalibrationEndpointSet;

	ifstrm >> word;
	ifstrm >> FixCapitalProductivity();

	readProducers(ifstrm);
	readWorkers(ifstrm);
	nWorkers() = (long)getpWorkers()->size();

	ifstrm >> word >> word1; // "Government {"
	ifstrm >> *pGovernment();
	ifstrm >> word; // "}"

	ifstrm >> word >> word1; // "ExtSectors {"
	while (ifstrm >> word, word != "}")
	{
		auto aType = getSAM().getAccNofName(word);
		while (ifstrm >> word, word != "}")
		{
			ifstrm >> ExtSect(aType);
		}
	}

	if (DEPData().InputParameter("MaxNBanks") > 0)
	{
		ifstrm >> word >> word1; // "CentralBank {"
		ifstrm >> *pCentralBank();

		ifstrm >> word >> word1; // "Banks {"
		ifstrm >> word >> word1; // "PrivateBank {"
		ifstrm >> *pCentralBank()->Banks()[0];
		ifstrm >> word; // "}"
	}

	ifstrm >> word >> word1; // "FinancialMarket {"
	ifstrm >> *pFinancialMarket();
	ifstrm >> word; // "}"

	ifstrm >> word >> word1; // "RecentDemandEvents {"
	_recentDemandEvents.clear();
	AgentID agentID;
	GoodType gType;
	while (ifstrm >> word, word != "}")
	{
		agentID = stol(word);
		ifstrm >> gType;
		_recentDemandEvents.push_back({ agentID, gType });
	}

	// Read pandemic state if present (handle backward compatibility)
	ifstrm >> word;
	if (word == "PandemicState") {
		// Put word back and read full pandemic state
		ifstrm.seekg(-static_cast<int>(word.length()) - 1, ios::cur);
		ifstrm >> _pandemicState;
		ifstrm >> word; // Move to next word after pandemic state
	}

	// Read BLE state if present (handle backward compatibility)
	if (word == "BLEExpectations") {
		// Put word back and read full BLE state
		ifstrm.seekg(-static_cast<int>(word.length()) - 1, ios::cur);
		ifstrm >> _bleExpectations;
		ifstrm >> word; // Move to next word after BLE state
	}

	// Handle old snapshot format that may have CohortBirthCounts and CohortDeathsByAge
	if (word == "CohortBirthCounts") {
		// Skip old cohort data for backwards compatibility
		while (ifstrm >> word, word != "}") { }
		ifstrm >> word >> word; // "CohortDeathsByAge {"
		while (ifstrm >> word, word != "}") { }
		ifstrm >> word; // final "}"
	}
};

void CWorld::writerndstatus(ofstream& ofstrm) const
{
	ofstrm << "\nrndstatus {\n" << myRandomEngine() << "\n}" << endl;
};

bool CWorld::hasSnapshotSection(const string& filename) const
{
	ifstream ifstrm(filename);
	if (ifstrm.fail())
		return false;
	string word;
	while (ifstrm >> word) {
		if (word == "SNAPSHOT")
			return true;
	}
	return false;
}

bool CWorld::readrndstatus(string fileNameExt)
{
	string word;

	ifstream& ifstrm = *new ifstream(fileNameExt);
	while (ifstrm >> word, word != "rndstatus" && !ifstrm.eof());
	delete pmyRandomEngine();
	pmyRandomEngine() = new mt19937();
	if (word == "rndstatus")
	{
		ifstrm >> word; // "{"
		ifstrm >> myRandomEngine();
		ifstrm >> word; // "}"
	}
	else
		myRandomEngine().seed(5489);

	ifstrm.close();
	return true;
}
void CWorld::assignPointersToBanks(string fileNameExt)
{
	string word;
	AgentID agentID, bankID;
	AgentType bankType;
	long nProducers = 0, nWorkers = 0, nOpen = 0;;
	ifstream ifstrm(fileNameExt);

	while (ifstrm >> word, !ifstrm.eof())
		if (word == "Producers")
			break;

	if (!ifstrm.eof())
	{
		ifstrm >> nProducers >> word; // "62 {"
		nOpen = 1;

		while (ifstrm >> word, word != "}") // " >> P_producername
		{
			ifstrm >> word >> word >> agentID; // "{ ID id"
			nOpen = 2;
			while (ifstrm >> word, word != "UsedBank")
			{
				if (word == "}")
					--nOpen;
				else if (word == "{")
					++nOpen;
			};

			ifstrm >> word >> bankType >> word >> bankID;
			if (bankType == getCentralBankType())
				pWorld()->pProducers()->at(agentID)->pUsedBank() = pCentralBank();
			else if (bankType == PrivateBankType())
			{
				pWorld()->pProducers()->at(agentID)->pUsedBank()
					= pCentralBank()->Banks().at(bankID);
			}

			while (nOpen > 1) // "}" of P_producername {
			{
				ifstrm >> word;
				if (word == "}")
					--nOpen;
				else if (word == "{")
					++nOpen;
			};
		}
	}

	while (ifstrm >> word, !ifstrm.eof() && word != "Workers");
	if (!ifstrm.eof())
	{
		ifstrm >> nWorkers >> word; // "200 {"
		nOpen = 1;

		while (ifstrm >> word, word != "}") // " >> Indiv
		{
			ifstrm >> word >> word >> agentID; // "{ ID id"

			// UsedBank
			nOpen = 2;
			while (ifstrm >> word, !ifstrm.eof() && word != "UsedBank")
			{
				if (word == "}")
					--nOpen;
				else if (word == "{")
					++nOpen;
			};

			assert(!ifstrm.eof());

			ifstrm >> word >> bankType >> word >> bankID;
			if (bankType == getCentralBankType())
				pWorld()->pWorkers()->at(agentID)->pUsedBank() = pCentralBank();
			else if (bankType == PrivateBankType())
			{
				pWorld()->pWorkers()->at(agentID)->pUsedBank()
					= pCentralBank()->Banks().at(bankID);
			}

			// OwnedBank

			while (ifstrm >> word, !ifstrm.eof() && word != "OwnedBank")
			{
				if (word == "}")
					--nOpen;
				else if (word == "{")
					++nOpen;
			};

			assert(!ifstrm.eof());

			ifstrm >> bankID; // should be a PrivateBank type
			if (bankID >= 0)
				pWorkers()->at(agentID)->pOwnedBank() = pCentralBank()->Banks().at(bankID);

			while (nOpen > 1) // "}" of Indiv {
			{
				ifstrm >> word;
				if (word == "}")
					--nOpen;
				else if (word == "{")
					++nOpen;
			};
		}
	}
	ifstrm.close();
}

void CWorld::SetPricesToOne()
{
	for (auto& pIndiv : Workers())
		for (auto& pair : pIndiv->myPrice())
		{
			if (pair.first == getSAM().GFCFtype()) // this is 1 + loan interestRate
				continue;

			pIndiv->setmyPriceOf(pair.first, 1.0);
		}

	for (auto& pProducer : Producers())
		if (pProducer != nullptr)
		{
			pProducer->productionPrice() = 1.0;

			for (auto& pair : pProducer->myPrice())
			{
				if (pair.first == getSAM().GFCFtype()) // this is 1 + loan interestRate
					continue;

				pProducer->setmyPriceOf(pair.first, 1.0);
			}
		}
}

/////////////////////  running the World  ////////////////////////////

void CWorld::recalculateSaveMonths()
{
	// Recalculate SaveMonths from SaveMonthsAfterCalibration and SaveMonthsFromMonth0
	// This ensures SaveMonths is properly set on reload (since it's not serialized)
	long finishCalibAt = getDEPData().getFinishCalibrationAt();

	DEPData().SaveMonths().clear();

	// Only add FinishCalibrationAt if it's greater than current month (future save point)
	if (finishCalibAt > currMonth())
		DEPData().SaveMonths().push_back(finishCalibAt);

	// Add months relative to calibration end
	for (const auto& relativeMonth : getDEPData().getSaveMonthsAfterCalibration())
	{
		int absoluteMonth = finishCalibAt + relativeMonth;
		if (absoluteMonth > currMonth())  // Only future months
			DEPData().SaveMonths().push_back(absoluteMonth);
	}

	// Add absolute months from SaveMonthsFromMonth0
	for (const auto& absoluteMonth : getDEPData().getSaveMonthsFromMonth0())
	{
		if (absoluteMonth > currMonth())  // Only future months
			DEPData().SaveMonths().push_back(absoluteMonth);
	}
}

void CWorld::cleanIOFilesForReload()
{
	if (getInputParameter("LoadMonthN") <= 0)
		return;

	string simulationName = getSimulationName();

#ifdef _WINDOWS
	// Windows implementation using system command
	string command = "del /Q " + simulationName + "*_IO*.dep 2>nul";
	system(command.c_str());
#else
	// Unix/Linux implementation
	string command = "rm -f " + simulationName + "*_IO*.dep 2>/dev/null";
	system(command.c_str());
#endif

	//	LogFile() << "Cleaned I/O synchronization files for reload at month "
	//		<< getInputParameter("LoadMonthN") << endl;
}

bool CWorld::LoadWorld()
{
	// Clean I/O files before loading if this is a reload
	cleanIOFilesForReload();

	string fileNameExt = getDEPData().getInputFileName() + ".dep";
	
	// Check if this is a reload (has SNAPSHOT section) BEFORE initializing
	// This is crucial for correct RNG sequence restoration
	bool isReload = hasSnapshotSection(fileNameExt);

	initializeWorld();

	LogFile() << "\n // Input file name " << fileNameExt << ",  ";
	writeTimeAndDate(LogFile());

	// 1. Copy input file to output log file

	ifstream ifstrm(fileNameExt);
	string line;
	while (getline(ifstrm, line))
		LogFile() << line << endl;
	LogFile().flush();
	ifstrm.close();

	// -------------------------------------

	readSnapshot(fileNameExt);

	assignPointersToBanks(fileNameExt);

	setupExtSectorsToLaborArray();

	readDEPData(fileNameExt);

	// Recalculate SaveMonths from SaveMonthsAfterCalibration and SaveMonthsFromMonth0
	// This is needed because SaveMonths is not serialized and must be recomputed on reload
	recalculateSaveMonths();

	// -------------------------------------------------------------------

	LogFile() << "\n Log begins:\n\n";
	LogFile().flush();

	// CRITICAL: Restore RNG state AFTER all loading is complete
	// For fresh runs: seeds with default (5489)
	// For reloads: restores saved RNG state from snapshot
	// This ensures no RNG consumption happens after state restoration
	readrndstatus(fileNameExt);
	
	if (isReload)
		loadNeighborsRNGStates(fileNameExt);

	// Initialize pandemic state from input parameters
	// This is done after loading so current run's pandemic settings apply
	initializePandemic();

	// Reinitialize pandemic timing if we're reloading and calibration endpoint was set
	if (isReload && _pandemicState.isPandemicActive()) {
		_pandemicState.reinitializeTiming();
	}

	return true;
};

void CWorld::loadNeighborsRNGStates(const string& fileNameExt)
{
	ifstream ifstrm(fileNameExt);
	string word;

	// Find the NeighborsRNGStates section
	while (ifstrm >> word, word != "NeighborsRNGStates" && !ifstrm.eof());

	if (ifstrm.eof())
		return;

	ifstrm >> word; // "{"

	// Load Workers Manager
	ifstrm >> word >> word; // "WorkersManager {"
	loadNeighborsRNGStateFromStream(ifstrm, NeighboringWorkersManager());
	ifstrm >> word; // "}"

	// Load Producers Manager
	ifstrm >> word >> word; // "ProducersManager {"
	loadNeighborsRNGStateFromStream(ifstrm, NeighboringProducersManager());
	ifstrm >> word; // "}"

	ifstrm >> word; // "}"
	ifstrm.close();
}

void CWorld::loadNeighborsRNGStateFromStream(ifstream& ifstrm, CNeighborsVectorManager& manager)
{
	string word;
	size_t objectCount;

	// Read object count
	ifstrm >> word >> objectCount; // "ObjectCount n"

	// Clear current objects
	manager.AgentsPtrs().clear();

	// Read agent FIDs
	ifstrm >> word; // "AgentFIDs"
	for (size_t i = 0; i < objectCount; ++i) {
		int type, id;
		ifstrm >> type >> id;
		CAgentFID fid(type, id);
		CAgent* pAgent = getFIDAgent(fid);
		manager.AgentsPtrs().push_back(pAgent);
	}

	// Read RNG state
	ifstrm >> word; // "RNGState"
	string rngStateStr;
	getline(ifstrm, rngStateStr);
	std::stringstream state(rngStateStr);
	state >> manager.rng;
}

bool CWorld::SaveWorld() const
{
	if (currMonth() < 0)
		return true;

	// save month (config) file  ------------------------------------------------------

	string SimulationName = CWorld::getSimulationName();
	string outputFileName = SimulationName;
	if (getpFigaro() != nullptr)
		outputFileName += "_" + getSAM().CountryCode();
	outputFileName += "_" + to_string(currMonth());
	
	// Add "_Calibrated" suffix when saving at calibration endpoint
	if (currMonth() == getDEPData().getFinishCalibrationAt())
		outputFileName += "_Calibrated";

	ofstream outfile(outputFileName + ".dep");
	if (outfile.fail())
		return false;

	outfile.precision(17); // double: up to all 17 significant digits
	outfile << "\n // " << outputFileName << "  ";
	writeTimeAndDate(outfile);

	DEPData().writeInputParameters(outfile);

	getDEPData().writePlotsInputParameters(outfile);

	getSAM().writeSAMTXT(outfile);

	outfile.flush();

	writeSnapshot(outfile);

	outfile << getDEPData();

	writerndstatus(outfile);

	// NEW: Save RNG states at the end of the config file instead of separate files
	outfile << "\nNeighborsRNGStates {" << endl;
	outfile << "WorkersManager {" << endl;
	saveNeighborsRNGState(outfile, getNeighboringWorkersManager());
	outfile << "}" << endl;
	outfile << "ProducersManager {" << endl;
	saveNeighborsRNGState(outfile, getNeighboringProducersManager());
	outfile << "}" << endl;
	outfile << "}" << endl;

	outfile.close();

	// save Log file  ------------------------------------------------------

	string LogFileName = SimulationName;
	if (getpFigaro() != nullptr)
		LogFileName += "_" + getSAM().CountryCode();
	LogFileName += "_Log.dep";

	string newLogFileName = outputFileName + "_Log.dep"; // includes month n

	string comand =
#ifdef _WINDOWS
		" copy "
#else
		" cp "
#endif // _WINDOWS
		+ LogFileName + " " + newLogFileName;
	int a = system(comand.c_str());

	return true;
};

void CWorld::saveNeighborsRNGState(ofstream& outfile, const CNeighborsVectorManager& manager) const
{
	// Save the number of objects
	size_t objectCount = manager.getAgentsPtrs().size();
	outfile << "ObjectCount " << objectCount << endl;

	// Save each object's FID
	outfile << "AgentFIDs ";
	for (const auto& obj : manager.getAgentsPtrs()) {
		CAgentFID fid = obj->getFID();
		outfile << "" << fid._type << " " << fid._id << " ";
	}
	outfile << endl;

	// Save the RNG state
	std::stringstream rngState;
	rngState << manager.getRNG();
	outfile << "RNGState " << rngState.str() << endl;
}

void CWorld::initializeWorld()
{
	std::remove("_ERROR-message.dep");

	DEPData().initialize_constructor(); // again

	delete _pSimulatedCountries;
	_pSimulatedCountries = new map<string, CSimulatedCountry*>;
	_pSAM = nullptr;
	_pFIGARO = nullptr;

	_pGovernment = nullptr;

	pCentralBank() = nullptr;

	delete _pRandomListOfIndivIDs;
	_pRandomListOfIndivIDs = new vector<AgentID>;
	delete _pInteractingAgents;
	_pInteractingAgents = new vector<CAgent*>;

	delete _pWorkers;
	_pWorkers = new vector<CWorker*>;
	delete _pProducers;
	_pProducers = new vector<CProducer*>;

	_pFinancialMarket = nullptr;

#ifdef DEPLOYERS_GRAPHICS
	if (ColorDefinitions().size() == 0)
		defineColors();
#endif

	pExtSectorsToLabor() = nullptr;

	// -----------------------------------------------------------

	pLogFile() = new ofstream();
	_pExternalSectorsIO = new ofstream();

	_currMonth = -1;
	InitMonth() = 0;

	string InputFileExt = getDEPData().getInputFileName() + ".dep";

	// RNG state is now restored at end of LoadWorld() to avoid corruption
	// during snapshot reload (where created workers get overwritten anyway)
	// For fresh runs, RNG is seeded with default in readrndstatus()

	InitSalaryCalibFactor() = 1.0;

	delete _pSAM;
	_pSAM = new CSAM();
	_pFIGARO = nullptr;

	bRunning() = false;
	bFinished() = false;
	bPaused() = false;

	SAM().readSAMTXT(InputFileExt); // keep a text copy of SAM, to write it to output file

	SAM().readSAM(InputFileExt); // builds the GoodType and ProducerType lists

	// New formula: nWorkers = nSimulatedWorkersPerSector * nSectors
	// This keeps simulation size consistent across countries (unlike the old formula
	// which scaled with country population, making large countries like USA too slow)
	World().nWorkers() = (long)getInputParameter("nSimulatedWorkersPerSector")
		* getSAM().getnPProducerTypes();

	defineProductsAndProducersFromSAM(); // defines each ProductSpecs and ProducerSpecs

	DEPData().NYears() = (long)DEPData().InputParameter("NYears");
	DebugLevel() = (long)DEPData().InputParameter("DebugLevel");

	InitializeAllNeighbors();

	//  -------  CentralBank  -----------------

	delete pCentralBank();
	pCentralBank() = nullptr;
	if (DEPData().InputParameter("MaxNBanks") > 0)
		pCentralBank() = new CCentralBank();

	//  -------  Workers  -----------------

	for (auto pIndiv : *pWorkers())
		delete (CWorker*)pIndiv;

	pWorkers()->clear();

	// Add new nWorkers only to initial, empty snapshot:
	long nIndiv = getWorld().getnWorkers();
	for (long n = 0; n < nIndiv; ++n)
		CWorker* pIndiv = newWorker();

	//  -------  Producers  -----------------

	for (auto pProd : *pProducers())
		if (pProd != nullptr && !pProd->IsCentralBank() && !pProd->IsPrivateBank())
			delete (CProducer*)pProd;

	pProducers()->clear();

	//  -------  Government  -----------------
	delete pGovernment();
	pGovernment() = new CGovernment(GovernmentType, GovernmentType);

	// --------------------------------------------------------------

	delete pFinancialMarket();
	pFinancialMarket() = new CFinancialMarket();

	DEPData().initialize();
	// start Log output file

	string logFname = CWorld::getSimulationName();
	if (getpFigaro() != nullptr)
		logFname += "_" + getFigaro()._thisCountryCode;
	logFname += "_Log.dep";
	LogFile().open(logFname);
}
void CWorld::InitializeAllNeighbors()
{
	// Workers manager  --------------------------------------------
	// Neighbor limits scale with nSimulatedWorkersPerSector to maintain
	// consistent market coverage as simulation size changes

	NeighboringWorkersManager().clear();

	NeighboringWorkersManager()._MaxNeighboringWorkers =
		getInputParameter("MaxNeighboringWorkers")
		* getInputParameter("nSimulatedWorkersPerSector");

	NeighboringWorkersManager()._MaxNeighboringProducers = getSAM().getnPProducerTypes()
		* getInputParameter("MaxNeighboringProducersPerSector")
		* getInputParameter("nSimulatedWorkersPerSector");

	// Producers manager  --------------------------------------------

	NeighboringProducersManager().clear();

	NeighboringProducersManager()._MaxNeighboringWorkers =
		getInputParameter("MaxNeighboringWorkers")
		* getInputParameter("nSimulatedWorkersPerSector");

	NeighboringProducersManager()._MaxNeighboringProducers = getSAM().getnPProducerTypes()
		* getInputParameter("MaxNeighboringProducersPerSector")
		* getInputParameter("nSimulatedWorkersPerSector");
};

void CWorld::getNeighborsWorkersOf(CAgent* pAgent, vector<CAgent*>& myNeighbors)
{
	AgentID id = pAgent->getID();;

	int MaxNneighbors = NeighboringWorkersManager()._MaxNeighboringWorkers;
	myNeighbors.clear();

	// Get a reference to all worker agents
	auto& allWorkers = NeighboringWorkersManager().getAgentsPtrs();
	int totalWorkers = (int)allWorkers.size();

	// Make sure we don't try to get more neighbors than there are agents
	MaxNneighbors = min(MaxNneighbors, totalWorkers - 1);

	// If no agents or requesting 0 neighbors, return empty vector
	if (totalWorkers == 0 || MaxNneighbors <= 0) {
		return;
	}

	// Reserve space to avoid reallocations
	myNeighbors.reserve(MaxNneighbors);

	// Get sorted IDs from cache (will rebuild if dirty)
	const auto& allIDs = NeighboringWorkersManager().getSortedIDs();

	// Find the position of the given ID in the sorted array
	// If ID doesn't exist, find the closest position
	size_t position = 0;
	bool exactMatch = false;

	for (size_t i = 0; i < allIDs.size(); ++i) {
		if (allIDs[i] == id) {
			position = i;
			exactMatch = true;
			break;
		}
		else if (allIDs[i] > id) {
			position = i;
			break;
		}
	}

	// If we went through the array without finding a spot, position is at the end
	if (!exactMatch && position == 0 && !allIDs.empty() && allIDs[0] < id) {
		position = allIDs.size();
	}

	// Calculate the fraction of the way through the ID space
	double fractionPosition;
	if (exactMatch) {
		fractionPosition = static_cast<double>(position) / allIDs.size();
	}
	else {
		// For non-exact matches, scale the position
		// We place it proportionally between the surrounding existing IDs
		if (position == 0) {
			fractionPosition = 0.0;
		}
		else if (position >= allIDs.size()) {
			fractionPosition = 1.0;
		}
		else {
			// Interpolate between the two surrounding IDs
			AgentID prevID = allIDs[position - 1];
			AgentID nextID = allIDs[position];
			double range = nextID - prevID;
			double offset = id - prevID;
			double localFraction = (range > 0) ? offset / range : 0.5;

			fractionPosition = (position - 1 + localFraction) / allIDs.size();
		}
	}

	// Collect myNeighbors based on this fractional position
	for (int i = 1; i <= MaxNneighbors; ++i) {
		// Calculate index based on the fractional position
		double neighborFraction = fractionPosition + (static_cast<double>(i) / allIDs.size());
		while (neighborFraction >= 1.0) neighborFraction -= 1.0;

		size_t neighborIndex = static_cast<size_t>(neighborFraction * allIDs.size()) % allIDs.size();

		// Find the agent with this ID
		AgentID neighborID = allIDs[neighborIndex];
		for (auto* agent : allWorkers) {
			if (agent->getID() == neighborID) {
				myNeighbors.push_back(agent);
				break;
			}
		}
	}
}

void CWorld::getNeighborsProducersOf(CAgent* pAgent, vector<CAgent*>& myNeighbors)
{
	AgentID id = pAgent->getID();;
	if (pAgent->getAgentType() < WorkerType) // Gov, ExtSects, CentralBank, PrivateBank
		id = pAgent->getmyProxyBuyerID();

	int MaxNneighbors = NeighboringProducersManager()._MaxNeighboringProducers;
	myNeighbors.clear();

	// Get a reference to all producer agents
	auto& allProducers = NeighboringProducersManager().getAgentsPtrs();
	int totalProducers = (int)allProducers.size();

	// Make sure we don't try to get more neighbors than there are agents
	MaxNneighbors = min(MaxNneighbors, totalProducers - 1);

	// If no agents or requesting 0 neighbors, return empty vector
	if (totalProducers == 0 || MaxNneighbors <= 0) {
		return;
	}

	// Reserve space to avoid reallocations
	myNeighbors.reserve(MaxNneighbors);

	// Get sorted IDs from cache (will rebuild if dirty)
	const auto& allIDs = NeighboringProducersManager().getSortedIDs();

	// Find the position of the given ID in the sorted array
	// If ID doesn't exist, find the closest position
	size_t position = 0;
	bool exactMatch = false;

	for (size_t i = 0; i < allIDs.size(); ++i) {
		if (allIDs[i] == id) {
			position = i;
			exactMatch = true;
			break;
		}
		else if (allIDs[i] > id) {
			position = i;
			break;
		}
	}

	// If we went through the array without finding a spot, position is at the end
	if (!exactMatch && position == 0 && !allIDs.empty() && allIDs[0] < id) {
		position = allIDs.size();
	}

	// Calculate the fraction of the way through the ID space
	double fractionPosition;
	if (exactMatch) {
		fractionPosition = static_cast<double>(position) / allIDs.size();
	}
	else {
		// For non-exact matches, scale the position
		// We place it proportionally between the surrounding existing IDs
		if (position == 0) {
			fractionPosition = 0.0;
		}
		else if (position >= allIDs.size()) {
			fractionPosition = 1.0;
		}
		else {
			// Interpolate between the two surrounding IDs
			AgentID prevID = allIDs[position - 1];
			AgentID nextID = allIDs[position];
			double range = nextID - prevID;
			double offset = id - prevID;
			double localFraction = (range > 0) ? offset / range : 0.5;

			fractionPosition = (position - 1 + localFraction) / allIDs.size();
		}
	}

	// Collect myNeighbors based on this fractional position
	for (int i = 1; i <= MaxNneighbors; ++i) {
		// Calculate index based on the fractional position
		double neighborFraction = fractionPosition + (static_cast<double>(i) / allIDs.size());
		while (neighborFraction >= 1.0) neighborFraction -= 1.0;

		size_t neighborIndex = static_cast<size_t>(neighborFraction * allIDs.size()) % allIDs.size();

		// Find the agent with this ID
		AgentID neighborID = allIDs[neighborIndex];
		for (auto* agent : allProducers) {
			if (agent->getID() == neighborID) {
				myNeighbors.push_back(agent);
				break;
			}
		}
	}
}

void CWorld::InitializeSimulation()
{
	// CAgent::PriceAdaptFactor() = getInputParameter("PriceAdaptFactor");// 1.005;

	if (pGovernment() != nullptr)
		pGovernment()->initialize();

	for (auto& gpair : getExtSectors())
		gpair.second->initialize();

	initializeWorkers();

	FixCapitalProductivity().clear();

	DEPData().initialize();

	// Initialize BLE expectations (if enabled)
	initializeBLE();

	// Initialize pandemic (if enabled)
	initializePandemic();

	writeExternalSectorsIO();
	_currMonth = 0;

	//  ----------------------------------------------------------------------

	for (auto& pAgent : *pProducers())
		if (pAgent != nullptr)
			pAgent->MonthlyActivityMonth() = (long)(getRandom01() * getInputParameter("WorkDaysPerMonth"));

	if (pGovernment() != nullptr)
		pGovernment()->MonthlyActivityMonth() = (long)(getRandom01() * getInputParameter("WorkDaysPerMonth"));

	if (pCentralBank() != nullptr)
		pCentralBank()->MonthlyActivityMonth() = (long)(getRandom01() * getInputParameter("WorkDaysPerMonth"));

	if (pCentralBank() != nullptr && pCentralBank()->Banks().size() > 0)
		for (auto& pair : pCentralBank()->Banks())
			pair.second->MonthlyActivityMonth() = (long)(getRandom01() * getInputParameter("WorkDaysPerMonth"));

	for (auto& pair : ExtSectors())
		pair.second->MonthlyActivityMonth() = (long)(getRandom01() * getInputParameter("WorkDaysPerMonth"));

	RefTotalHouseholdPXConsum().clear();
	for (const auto pAcc : getAccountGroups().at("H"))
	{
		auto Hcol = pAcc->accN();

		for (long gType = 0; gType < getSAM().getnPXproducerTypes(); ++gType)
			RefTotalHouseholdPXConsum()[gType] += (double)getSAM().getRowCol(gType, Hcol);

		RefTotalHouseholdPXConsum()[getSAM().GFCFtype()] += (double)getSAM().getRowCol("GFCF", Hcol);
	}

	RefGovConsumPX().clear();
	for (long gType = 0; gType < getSAM().getnPXproducerTypes(); ++gType)
		RefGovConsumPX()[gType] += (double)getSAM().getRowCol(gType, "Government");

	RefGovConsumPX()[getSAM().GFCFtype()] += (double)getSAM().getRowCol("GFCF", "Government");

	RefGDP_VA() = 0;
	if (getpFigaro() != nullptr)
	{
		for (long gType = 0; gType < getSAM().getnPProducerTypes(); ++gType)
		{
			RefGDP_VA() += (double)getSAM().getRowCol("CompEmployees", gType)
				+ (double)getSAM().getRowCol("GrossOpSurplus", gType)
				+ (double)getSAM().getRowCol("TaxProduction", gType)
				+ (double)getSAM().getRowCol("TaxProducts", gType);
		}
	}
	else
		ERRORmsg("No Figaro object in InitializeSimulation");
};
bool CWorld::RunSimulation()
{
	// return false if not finished

	if (getDEPData().getNYears() <= 0
		|| (currMonth() >= getDEPData().getEndMonth()))// using getEndMonth() for consistency
	{
		bFinished() = true;
		bRunning() = false;
		bPaused() = false;
		bPlotBorders() = true;

		return true;
	}

	_currMonth++;
	runOneMonth(); // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

	if (currMonth() + 1 < getDEPData().getEndMonth())
	{
		for (const auto& monthN : getDEPData().getSaveMonths())
		{
			if (monthN == currMonth())
			{
				SaveWorld();
				break;
			}
		}

		return false; // not finished NYears
	}
	else
	{
		bRunning() = false;
		bFinished() = true;

		SaveWorld();

		return true; // finished NYears
	}
};

void CWorld::writeExternalSectorsIO()
{
	if (_pFIGARO == nullptr || getExtSectors().size() <= 1 // size==1: RW
		|| currMonth() < getInputParameter("ReadIOfilesFromMonth"))
		return;

	if (currMonth() > 1 + getInputParameter("ReadIOfilesFromMonth")
		&& currMonth() > InitMonth() + 1) // InitMonth>0: LoadMonth
		waitMyExternalCountriesToReadMyIO();

	string thisCountryCode = Figaro()._thisCountryCode;
	string filename = CWorld::getSimulationName() + "_" + thisCountryCode + "_IO.dep";
	ofstream outf(filename);

#ifdef LINUX_VERSION
	cout << "thisCountryCode " << Figaro()._thisCountryCode << " , currMonth " << currMonth() << endl;
#endif

	outf.precision(17); // double: up to all 17 significant digits
#ifdef WINDOWS_VERSION
	writeTimeAndDate(outf);
#endif
	string SAMname = SAM()._SAMname;
	outf << "thisCountryCode " << Figaro()._thisCountryCode << " currMonth " << currMonth() << endl;

	//  --------------------------  IMPORTS  --------------

	outf << "IMPORTS {" << endl;

	string thisCC = (_pFIGARO != nullptr ? Figaro()._thisCountryCode : "");

	// loop over every external sector and dump its import flows
	// grouping by country code is not necessary when we simply want each sector visible
	// we'll still keep the previous structure when Figaro lists are available

	if (_pFIGARO != nullptr && getInputParameter("TradeDisaggMode") == 1)
	{
		// Sector mode: iterate over all ExtSectors by sector name (no country grouping)
		outf << "PurchasedFromCountryCode RW { " << endl;
		for (const auto& eS : ExtSectors())
		{
			auto& eSectRowN = eS.first;
			auto& eSect = *eS.second;
			outf << " from_ExtSector " << eSectRowN << " " << eSect.name()
				<< " SectorCode " << eSect.name() << " { to_mySectors ";
			for (long producerCol = 0; producerCol < getSAM().getnPProducerTypes(); ++producerCol)
				outf << " " << producerCol << " " << eSect.getImports()[producerCol];
			outf << " }" << endl;
		}
		outf << " }" << endl;
	}
	else if (_pFIGARO != nullptr)
	{
		// Country mode: first disaggregated, then aggregated countries
		for (const auto& extCountryCode : *Figaro()._pDisaggExtSectCountries)
		{
			outf << "PurchasedFromCountryCode " << extCountryCode << " { " << endl;
			for (const auto& eS : ExtSectors())
			{
				auto& eSectRowN = eS.first;
				auto& eSect = *eS.second;
				string eSectCountryCode = eSect.name().substr(0, 2);
				if (eSectCountryCode != extCountryCode)
					continue;

				string sectorCode = (eSect.name().size() > 2 ? eSect.name().substr(2) : eSect.name());
				outf << " from_ExtSector " << eSectRowN << " " << eSect.name()
					<< " SectorCode " << sectorCode << " { to_mySectors ";
				for (long producerCol = 0; producerCol < getSAM().getnPProducerTypes(); ++producerCol)
					outf << " " << producerCol << " " << eSect.getImports()[producerCol];

				outf << " }" << endl;
			}

			outf << " }" << endl;
		}

		outf << endl;

		// aggregated countries (including RW)
		for (const auto& extCountryCode : *Figaro()._pAggExtSectCountries)
		{
			outf << "PurchasedFromCountryCode " << extCountryCode << " { " << endl;
			string eSectName = extCountryCode;
			GoodType eSectRowN = getSAM().getAccNofName(eSectName);

			outf << " from_aggExtSector " << eSectRowN << " " << eSectName << " { to_mySectors ";
			for (long producerCol = 0; producerCol < getSAM().getnPProducerTypes(); ++producerCol)
				outf << " " << producerCol << " " << getExtSect(eSectRowN).getImports()[producerCol];

			outf << " }" << endl;

			outf << " }" << endl;
		}
	}
	else
	{
		// no Figaro available – just dump every ext sector under a generic RW block
		outf << "PurchasedFromCountryCode RW { " << endl;
		for (const auto& eS : ExtSectors())
		{
			auto& eSectRowN = eS.first;
			auto& eSect = *eS.second;
			string sectorCode = (eSect.name().size() > 2 ? eSect.name().substr(2) : eSect.name());
			outf << " from_ExtSector " << eSectRowN << " " << eSect.name()
				<< " SectorCode " << sectorCode << " { to_mySectors ";
			for (long producerCol = 0; producerCol < getSAM().getnPProducerTypes(); ++producerCol)
				outf << " " << producerCol << " " << eSect.getImports()[producerCol];

			outf << " }" << endl;
		}
		outf << " }" << endl;
	}

	outf << "}" << endl;

	//  --------------------------  EXPORTS  --------------

	outf << "EXPORTS {" << endl;

	// -------  EXports section: each external sector writes its exports back to this country

	if (_pFIGARO != nullptr && getInputParameter("TradeDisaggMode") == 1)
	{
		// Sector mode: iterate over all ExtSectors by sector name
		outf << "SoldToCountryCode RW { " << endl;
		for (long producerRowN = 0; producerRowN < getSAM().getnPProducerTypes(); ++producerRowN)
		{
			outf << " to_extSector " << producerRowN << " " << getSAM().getAccNameOfN(producerRowN)
				<< " { from_mySectors ";
			for (const auto& es : ExtSectors())
				outf << " " << es.first << " " << es.second->getExports().at(producerRowN);
			outf << " }" << endl;
		}
		outf << " }" << endl;
	}
	else if (_pFIGARO != nullptr)
	{
		// Country mode: disaggregated countries
		for (const auto& extCountryCode : *Figaro()._pDisaggExtSectCountries)
		{
			GoodType baseExtSectN = getSAM().getAccNofName(extCountryCode) - 1;
			outf << "SoldToCountryCode " << extCountryCode << " { " << endl;
			for (long producerRowN = 0; producerRowN < getSAM().getnPProducerTypes(); ++producerRowN)
			{
				outf << " to_extSector " << producerRowN << " " << getSAM().getAccNameOfN(producerRowN)
					<< " { from_mySectors ";

				for (const auto& es : ExtSectors())
				{
					auto& eSectColN = es.first;
					auto& eSect = *es.second;
					if (eSect.name().substr(0, 2) != extCountryCode)
						continue;

					outf << " " << eSectColN << " " << eSect.getExports().at(producerRowN);
				}

				outf << " }" << endl;
			}
			outf << " }" << endl;
		}

		outf << endl;

		// Aggregated countries (including RW)
		for (const auto& extCountryCode : *Figaro()._pAggExtSectCountries)
		{
			GoodType ExtSectorN = getSAM().getAccNofName(extCountryCode);

			outf << "SoldToCountryCode " << extCountryCode << " { " << endl;
			outf << " to_aggExtSector " << ExtSectorN << " " << extCountryCode << " { from_mySectors ";

			for (long productRow = 0; productRow < getSAM().getnPProducerTypes(); ++productRow)
				outf << " " << productRow << " " << getExtSectors().at(ExtSectorN)->getExports().at(productRow);

			outf << " }" << endl;

			outf << " }" << endl;
		}
	}
	else
	{
		// no Figaro – just write every ext sector as its own sell-to block under RW
		for (const auto& es : ExtSectors())
		{
			auto& eSect = *es.second;
			string groupCode = "RW";
			outf << "SoldToCountryCode " << groupCode << " { " << endl;
			for (long producerRowN = 0; producerRowN < getSAM().getnPProducerTypes(); ++producerRowN)
			{
				outf << " to_extSector " << producerRowN << " " << getSAM().getAccNameOfN(producerRowN)
					<< " { from_mySectors ";

				for (const auto& inner : ExtSectors())
				{
					if (inner.first != es.first) continue;
					outf << " " << inner.first << " " << inner.second->getExports().at(producerRowN);
				}

				outf << " }" << endl;
			}
			outf << " }" << endl;
		}
	}

	outf << "}" << endl;

	outf.flush();
	outf.close();

	// update written month flag file

	filename = CWorld::getSimulationName() + "_" + thisCountryCode
		+ "_IO_written.dep";
	ofstream flagFile(filename);
	flagFile << " " << currMonth();
	flagFile.flush();
	flagFile.close();
}
void CWorld::waitMyExternalCountriesToReadMyIO() const
{
	string thisCountryCode = getFigaro()._thisCountryCode;

	set<string> MyExtCountriesCodes;
	for (const auto& code : *getFigaro()._pDisaggExtSectCountries)
		MyExtCountriesCodes.insert(code);
	for (const auto& code : *getFigaro()._pAggExtSectCountries)
		MyExtCountriesCodes.insert(code);
	MyExtCountriesCodes.erase("RW");
	set<string> extCountriesCodes = MyExtCountriesCodes;

	ifstream flagFile;
	string filename;
	long filemonth = 0;
	for (const auto& extCountryCode : MyExtCountriesCodes)
	{
		if (extCountryCode == "RW")
			ERRORmsg("extCountryCode == RW", true);

		filename = CWorld::getSimulationName() + "_" + thisCountryCode + "_IO_readBy_" + extCountryCode + ".dep";

		long m = 0;
		bool fileRead = false;
		while (!fileRead) {
			flagFile.open(filename);
			if (flagFile.fail()) {
				flagFile.close();
				crossPlatformSleep(getInputParameter("sleep_seconds"));
				if (m++ > getInputParameter("NsleepForIO"))
				{
					ERRORmsg(thisCountryCode + ": couldn't open flagFile " + filename, true);
					exit(-3); // give up
				}
				continue; // keep waiting
			}
			else // flagFile open, read it
			{
				flagFile >> filemonth;
				flagFile.close();
				if (filemonth < currMonth() - 1) // not read yet
				{
					crossPlatformSleep(getInputParameter("sleep_seconds"));
					if (m++ > getInputParameter("NsleepForIO"))
					{
						ERRORmsg(thisCountryCode + ": couldn't open flagFile " + filename, true);
						exit(-3); // give up
					}
					continue; // keep waiting
				}
				if (filemonth == currMonth() - 1) // OK, extCountry has read my previous IO file
				{
					fileRead = true;
					break;
				}
				else
					if (filemonth >= currMonth())
					{
						if (getInputParameter("LoadMonthN") > 0 && filemonth > getInputParameter("LoadMonthN"))
						{
							fileRead = true;
							break;
						}
						else
						{
							ERRORmsg(thisCountryCode + ": flagFile " + filename + " read ahead my currMonth", true); // exit
							return; // dummy
						}
					}
			}
		}

		extCountriesCodes.erase(extCountryCode); // don't change the for range
		if (extCountriesCodes.size() == 0)
			break;
	}
}

void CWorld::readMyExternalSectorsIO() const
{
	if (getInputParameter("TradeDisaggMode") == 1) // Sector mode: no inter-country IO files
		return;

	if (_pFIGARO == nullptr || getExtSectors().size() <= 1 // size==1: RW
		|| currMonth() < getInputParameter("ReadIOfilesFromMonth"))
		return;

	waitMyExternalCountriesToWriteTheirIO();

	long abscurrMonth = 0;
	string fname, countryCode, SectorCode, SectName, ExtSectName, word, word1, word2;
	GoodQtty SectorN = -1, qtty = 0;
	double d0 = 0., d1 = 0.;
	GoodType gType = undefinedGoodType;
	char line[1000] = { " " };

	ifstream ExtSectIO;

	string thisCountryCode = getFigaro()._thisCountryCode;

	set<string> MyExtCountriesCodes;
	for (const auto& code : *getFigaro()._pDisaggExtSectCountries)
		MyExtCountriesCodes.insert(code);
	for (const auto& code : *getFigaro()._pAggExtSectCountries)
		MyExtCountriesCodes.insert(code);
	MyExtCountriesCodes.erase("RW");
	set<string> extCountriesCodes = MyExtCountriesCodes;

	ifstream flagFile;
	string filename;
	long currmonth = 0;
	for (auto& pair : ExtSectors())
	{
		auto& ExtSect = *pair.second;
		ExtSect.Exports().clear();
		ExtSect.Exports().resize(getSAM().getnPXproducerTypes(), 0);
		ExtSect.Exports_Init().clear();
		ExtSect.Exports_Init().resize(getSAM().getnPXproducerTypes(), 0);
		ExtSect.Imports().clear();
		ExtSect.Imports().resize(getSAM().getnPXproducerTypes(), 0);
	}

	for (const auto& extCountryCode : MyExtCountriesCodes)
	{
		bool IsDisaggExtCountry = false;
		if (getFigaro()._pDisaggExtSectCountries->find(extCountryCode)
			!= getFigaro()._pDisaggExtSectCountries->end())
			IsDisaggExtCountry = true;

		string filename = CWorld::getSimulationName() + "_"
			+ extCountryCode + "_IO.dep";

		ExtSectIO.open(filename);
		if (!ExtSectIO.is_open())
			ERRORmsg("Couldn't open " + filename, true);

#ifdef WINDOWS_VERSION
		ExtSectIO.getline(line, 999, '\n'); // time date
#endif

		ExtSectIO >> word >> countryCode >> word;
		assert(countryCode == extCountryCode);

		ExtSectIO >> abscurrMonth;

		// ==========   EXPORTS    ============================

		// This country ES reading _FR_IO.dep, go to:
		// "IMPORTS
		//     PurchasedFromCountryCode ES
		//		from_aggExtSector [nn] XES { to_mySectors [mm...]"

		ExtSectIO >> word >> word1; // IMPORTS {

		while (true) {
			while (word != "PurchasedFromCountryCode")
			{
				ExtSectIO >> word;
				if (word != "PurchasedFromCountryCode")
					ExtSectIO.getline(line, 999, '\n');
			}
			ExtSectIO >> countryCode; // PurchasedFromCountryCode extCountryCode
			if (countryCode == thisCountryCode)
			{
				ExtSectIO >> word; // {
				break;
			}
			else
				word = "";
		}

		GoodQtty extSectorN = -1;
		while (ExtSectIO >> word, word != "}") // from_(agg)ExtSector
		{
			ExtSectIO >> extSectorN >> ExtSectName;

			if (ExtSectName.length() == 2) // if it's a country code: aggregated
			{
				//IsDisaggExtCountry is false;
				ExtSectIO >> word >> word1; // { to_mySectors:
			}
			else
			{
				//IsDisaggExtCountry is true;
				ExtSectIO >> word >> word >> word >> word1; // { to_mySectors:
			}

			GoodType myExtSectorN = getSAM().getAccNofName(extCountryCode);

			while (ExtSectIO >> word, word != "}") // read this row of sectors values
			{
				SectorN = atoi(word.c_str());
				ExtSectIO >> qtty;
				ExtSectors().at(myExtSectorN)->Exports()[SectorN] = qtty;
				ExtSectors().at(myExtSectorN)->Exports_Init()[SectorN] = qtty;
				// Exports_Init is used as ExtSector.GoodsIwish and for normalization in allocateBuyersQttyAndMoney
			}

			if (!IsDisaggExtCountry)	//M J ?? not verified for Aggregated
				break;
		}

		// ==========   IMPORTS    ============================

		// find again thisCountryCode (now in EXPORTS of extCountryCode)

		while (true)
		{
			while (word != "SoldToCountryCode")
			{
				ExtSectIO >> word;
				if (word != "SoldToCountryCode")
					ExtSectIO.getline(line, 999, '\n');
			}
			ExtSectIO >> countryCode; // PurchasedFromCountryCode extCountryCode
			if (countryCode == thisCountryCode)
			{
				ExtSectIO >> word; // {
				break;
			}
			else
				word = "";
		}

		while (ExtSectIO >> word, word != "}") // to_(agg)extSector
		{
			ExtSectIO >> extSectorN >> ExtSectName;

			ExtSectIO >> word >> word1; // { from_mySectors:

			GoodType myExtSectorN = getSAM().getAccNofName(extCountryCode);

			//---------------ExtSectIO >> SectorN >> SectName >> word >> word1; // { from_mySectors:

			while (ExtSectIO >> word, word != "}")
			{
				SectorN = atoi(word.c_str());
				ExtSectIO >> qtty;
				ExtSectors().at(myExtSectorN)->Imports()[SectorN] = qtty; // used as ToSell here
			}

			if (!IsDisaggExtCountry)
				break;
		}

		ExtSectIO.close();

		// update read month flag file

		filename = CWorld::getSimulationName() + "_"
			+ extCountryCode + "_IO_readBy_"
			+ thisCountryCode + ".dep";
		ofstream flagFile(filename);
		flagFile << " " << abscurrMonth;
		flagFile.flush();
		flagFile.close();
	}


	// After processing all simulated countries, handle RW specially
	if (getExtSectors().find(getSAM().getAccNofName("RW")) != getExtSectors().end())
	{
		GoodType RWType = getSAM().getAccNofName("RW");
		CExtSect* pRWExtSect = pExtSect(RWType);

		// Since RW doesn't write IO files, use SAM values for its exports
		// These represent what AT can import from RW
		for (long gType = 0; gType < getSAM().getnPProducerTypes(); ++gType)
		{
			// Column RW in SAM: what AT imports from RW
			GoodQtty importFromRW = getSAM().getRowCol(gType, RWType);
			pRWExtSect->Exports()[gType] = importFromRW;
			pRWExtSect->Exports_Init()[gType] = importFromRW;
		}

		// Handle GFCF if applicable
		GoodType GFCFtype = getSAM().GFCFtype();
		/*
		if (GFCFtype <= getSAM().getnPXproducerTypes())
		{
			GoodQtty gfcfFromRW = getSAM().getRowCol(GFCFtype, RWType);
			pRWExtSect->Exports()[GFCFtype] = gfcfFromRW;
			pRWExtSect->Exports_Init()[GFCFtype] = gfcfFromRW;
		}
		*/
	}
};

// new version with more robust parsing of IO files (handles disaggregated external sectors)
void CWorld::readMyExternalSectorsIO_v2() const
{
    if (getInputParameter("TradeDisaggMode") == 1)
        return;

    if (_pFIGARO == nullptr || getExtSectors().size() <= 1
        || currMonth() < getInputParameter("ReadIOfilesFromMonth"))
        return;

    waitMyExternalCountriesToWriteTheirIO();

    long abscurrMonth = 0;
    string fname, countryCode, SectorCode, SectName, ExtSectName, word, word1, word2;
    GoodQtty SectorN = -1, qtty = 0;
    GoodType extSectorN = 0;
    double d0 = 0., d1 = 0.;
    GoodType gType = undefinedGoodType;
    char line[1000] = { " " };

    ifstream ExtSectIO;
    string thisCountryCode = getFigaro()._thisCountryCode;

    set<string> MyExtCountriesCodes;
    for (const auto& code : *getFigaro()._pDisaggExtSectCountries)
        MyExtCountriesCodes.insert(code);
    for (const auto& code : *getFigaro()._pAggExtSectCountries)
        MyExtCountriesCodes.insert(code);
    MyExtCountriesCodes.erase("RW");
    set<string> extCountriesCodes = MyExtCountriesCodes;

    for (auto& pair : ExtSectors())
    {
        auto& ExtSect = *pair.second;
        ExtSect.Exports().assign(getSAM().getnPXproducerTypes(), 0);
        ExtSect.Exports_Init().assign(getSAM().getnPXproducerTypes(), 0);
        ExtSect.Imports().assign(getSAM().getnPXproducerTypes(), 0);
    }

    for (const auto& extCountryCode : MyExtCountriesCodes)
    {
        string filename = CWorld::getSimulationName() + "_" + extCountryCode + "_IO.dep";
        ExtSectIO.open(filename);
        if (!ExtSectIO.is_open())
            ERRORmsg("Couldn't open " + filename, true);

#ifdef WINDOWS_VERSION
        ExtSectIO.getline(line, 999, '\n');
#endif
        ExtSectIO >> word >> countryCode >> word;
        assert(countryCode == extCountryCode);
        ExtSectIO >> abscurrMonth;

        // IMPORTS section
        ExtSectIO >> word >> word1; // should be "IMPORTS {"
        while (true)
        {
            while (word != "PurchasedFromCountryCode")
            {
                ExtSectIO >> word;
                if (word != "PurchasedFromCountryCode")
                    ExtSectIO.getline(line, 999, '\n');
            }
            ExtSectIO >> countryCode;
            if (countryCode == thisCountryCode)
            {
                ExtSectIO >> word;
                break;
            }
            else
                word = "";
        }
        while (ExtSectIO >> word && word != "}")
        {
            if (word == "from_ExtSector" || word == "from_aggExtSector")
            {
                ExtSectIO >> extSectorN >> ExtSectName;
                ExtSectIO >> word;
                if (word == "SectorCode")
                {
                    ExtSectIO >> SectorCode;
                    ExtSectIO >> word;
                }
                if (word != "{")
                {
                    while (word != "{" && ExtSectIO >> word);
                }
                ExtSectIO >> word; // to_mySectors
                GoodType myExtSectorN = getSAM().getAccNofName(ExtSectName);
                while (ExtSectIO >> word && word != "}")
                {
                    SectorN = atoi(word.c_str());
                    ExtSectIO >> qtty;
                    ExtSectors().at(myExtSectorN)->Exports()[SectorN] = qtty;
                    ExtSectors().at(myExtSectorN)->Exports_Init()[SectorN] = qtty;
                }
            }
            else
            {
                ExtSectIO.getline(line, 999, '\n');
            }
        }

        // EXPORTS section
        while (true)
        {
            while (word != "SoldToCountryCode")
            {
                ExtSectIO >> word;
                if (word != "SoldToCountryCode")
                    ExtSectIO.getline(line, 999, '\n');
            }
            ExtSectIO >> countryCode;
            if (countryCode == thisCountryCode)
            {
                ExtSectIO >> word;
                break;
            }
            else
                word = "";
        }
        while (ExtSectIO >> word && word != "}")
        {
            if (word == "to_extSector" || word == "to_aggExtSector")
            {
                ExtSectIO >> extSectorN >> ExtSectName;
                ExtSectIO >> word;
                if (word != "{")
                {
                    while (word != "{" && ExtSectIO >> word);
                }
                ExtSectIO >> word; // from_mySectors
                GoodType myExtSectorN = getSAM().getAccNofName(ExtSectName);
                while (ExtSectIO >> word && word != "}")
                {
                    SectorN = atoi(word.c_str());
                    ExtSectIO >> qtty;
                    ExtSectors().at(myExtSectorN)->Imports()[SectorN] = qtty;
                }
            }
            else
            {
                ExtSectIO.getline(line, 999, '\n');
            }
        }
        ExtSectIO.close();

        filename = CWorld::getSimulationName() + "_" + extCountryCode + "_IO_readBy_" + thisCountryCode + ".dep";
        ofstream flagFile(filename);
        flagFile << " " << abscurrMonth;
        flagFile.flush();
        flagFile.close();
    }

    // handle RW as before
    if (getExtSectors().find(getSAM().getAccNofName("RW")) != getExtSectors().end())
    {
        GoodType RWType = getSAM().getAccNofName("RW");
        CExtSect* pRWExtSect = pExtSect(RWType);
        for (long gType = 0; gType < getSAM().getnPProducerTypes(); ++gType)
        {
            GoodQtty importFromRW = getSAM().getRowCol(gType, RWType);
            pRWExtSect->Exports()[gType] = importFromRW;
            pRWExtSect->Exports_Init()[gType] = importFromRW;
        }
        GoodType GFCFtype = getSAM().GFCFtype();
        /*
        if (GFCFtype <= getSAM().getnPXproducerTypes())
        {
            GoodQtty gfcfFromRW = getSAM().getRowCol(GFCFtype, RWType);
            pRWExtSect->Exports()[GFCFtype] = gfcfFromRW;
            pRWExtSect->Exports_Init()[GFCFtype] = gfcfFromRW;
        }
        */
    }
};

void CWorld::waitMyExternalCountriesToWriteTheirIO() const
{
	string thisCountryCode = getFigaro()._thisCountryCode;

	set<string> MyExtCountriesCodes;
	for (const auto& code : *getFigaro()._pDisaggExtSectCountries)
		MyExtCountriesCodes.insert(code);
	for (const auto& code : *getFigaro()._pAggExtSectCountries)
		MyExtCountriesCodes.insert(code);
	MyExtCountriesCodes.erase("RW");

	ifstream flagFile;
	string filename;
	long currmonth = 0;
	set<string> extCountriesCodes = MyExtCountriesCodes; // make an erasable, countdown copy
	for (const auto& extCountryCode : MyExtCountriesCodes)
	{
		if (extCountryCode == "RW")
			ERRORmsg("ERROR: extCountryCode == RW", true);

		filename = CWorld::getSimulationName() + "_" + extCountryCode + "_IO_written.dep";

		while (true)
		{
			long m = 0;
			bool fileOpen = false;
			while (!fileOpen)
			{
				flagFile.open(filename);
				if (flagFile.fail())
				{
					flagFile.close();
					crossPlatformSleep(getInputParameter("sleep_seconds"));
					if (m++ > getInputParameter("NsleepForIO"))
					{
						ERRORmsg(thisCountryCode + ": couldn't open flagFile " + filename, true);
						return;
					}
				}
				else
					fileOpen = true;
			}

			flagFile >> currmonth;
			flagFile.close();
			crossPlatformSleep(getInputParameter("sleep_seconds"));

			if (currmonth == currMonth() - 1)
				break;
			else
				if (currmonth >= currMonth())
				{
					if (getInputParameter("LoadMonthN") > 0 && currmonth > getInputParameter("LoadMonthN"))
						break;
					else
					{
						ERRORmsg(thisCountryCode + ": flagFile " + filename + " past my currMonth", true); // exit
						return; // dummy
					}
				}
		}

		extCountriesCodes.erase(extCountryCode);
		if (extCountriesCodes.size() == 0)
			break;
	}
}

void CWorld::DismantleAndStartNewProducers()
{
	// Dismantle Unused Producers

	DEPData().FirmBirths()[currMonth() % DEPData().FirmBirths().size()] = 0;
	DEPData().FirmDeaths()[currMonth() % DEPData().FirmDeaths().size()] = 0;

	for (auto& pProducer : *pProducers())
	{
		if (pProducer == nullptr)
			continue;

		auto prodType = pProducer->getAgentType();

		if (prodType < 0
			|| pProducer->IsExtSect()
			|| pProducer->IsCentralBank() || pProducer->IsPrivateBank())
			continue;

		if (getDEPData().getnCurrentPProducers().at(prodType)
			<= getInputParameter("MinCurrentPProducers"))
			continue;

		if (pProducer->getWealthPrevMonth() < 0)
			pProducer->incrementConsecutiveMonthsNegativeWealth();
		else
			pProducer->resetConsecutiveMonthsNegativeWealth();

		if (pProducer->getConsecutiveMonthsNegativeWealth()
			>= getInputParameter("MonthsWithNegativeWealthToDismantle"))
		{
			// Log dismantle trigger with context
			/*
			try {
				LogFile() << "\n[DEATH_TRIGGER] Month:" << currMonth()
					<< " ID:" << pProducer->getID()
					<< " Type:" << getSAM().getAccNameOfN(prodType)
					<< " AgeM:" << (currMonth() - pProducer->getinitialMonth())
					<< " ConsecutiveNegW:" << pProducer->getConsecutiveMonthsNegativeWealth()
					<< " PrevWealth:" << pProducer->getWealthPrevMonth()
					<< " Cash:" << pProducer->getCash()
					<< " BankBal:" << pProducer->getmyBankBalance()
					<< " Employees:" << pProducer->getEmployees().size();
				LogFile().flush();
			}
			catch (...) {}
			*/

			DismantleProducer(pProducer);
		}
	}

	// Start new Producers

	for (auto pWorker : *pWorkers())
		pWorker->TryToStartupNewProducer();

	if (currMonth() >= 9999) DEPData().CheckAccountingBalance();
};
long CWorld::DismantleProducer(CProducer*& pProducer)
{
	assert(getDEPData().getnCurrentPProducers().at(pProducer->getAgentType()) > 0);
	--DEPData().nCurrentPProducers().at(pProducer->getAgentType());

	DEPData().Dismantled() += pProducer->getGoodsIhave();
	DEPData().Dismantled() += pProducer->getToSell();

	while (pProducer->getEmployees().size() > 0)
		pProducer->ReleaseLastEmployee();

	auto fid = pProducer->getFID();
	if (fid._type < 0)
		ERRORmsg("DismantleProducer: fid._type < 0", true);

	NeighboringProducersManager().removeNeighbor(fid);

	// LogFile() << "\n month " << currMonth() << " " << pProducer->getAgentName() << " dismantled. ";

	if (getInputParameter("MaxNBanks") > 0
		|| pProducer->pUsedBank() != nullptr)
	{
		auto cash = pProducer->getCash();
		CBankEntry currStat =
			pProducer->pUsedBank()->ClientCashOperation(pProducer, cash);

		// close pProducer's account: transfer balance to Bank, not to owner

		pProducer->myBankAccountStatus() =
			pProducer->getmyUsedBank()->ClientDefaultClose(pProducer);
	}
	else if (pProducer->pOwners()->size() > 0)
	{
		auto cash = pProducer->getCash();
		pProducer->pOwners()->begin()->second->Cash() += cash;
		pProducer->Cash() -= cash;
	}

	map<CAgentFID, CAgent*> Owners = *pProducer->pOwners();
	for (auto& pair : Owners)
	{
		auto& owner = *pair.second;
		pProducer->myShares(pProducer->getFID()) += owner.myShares(pProducer->getFID());// -Prod +Owner
		owner.myShares().erase(pProducer->getFID());
		owner.pmyProducers()->erase(pProducer->getFID());
		pProducer->pOwners()->erase(owner.getFID());
	}
	pProducer->myShares().erase(pProducer->getFID());

	// Only track death age distribution after AssistedProductionUpto
	long assistedProductionUpto = (long)getInputParameter("AssistedProductionUpto");
	if (currMonth() >= assistedProductionUpto) {
		// Calculate firm age using the producer's stored _initialMonth
		int creationMonth = pProducer->getinitialMonth();
		int ageInMonths = currMonth() - creationMonth;
		int ageInYears = ageInMonths / DEPData().getMonthsPerYear();

		// Log detailed death record
		/*
		try {
			LogFile() << "\n[DEATH] Month:" << currMonth()
				<< " ID:" << pProducer->getID()
				<< " Type:" << getSAM().getAccNameOfN(pProducer->getAgentType())
				<< " AgeM:" << ageInMonths << " AgeY:" << ageInYears
				<< " WealthPrev:" << pProducer->getWealthPrevMonth()
				<< " Cash:" << pProducer->getCash()
				<< " BankBal:" << pProducer->getmyBankBalance()
				<< " FixCap:" << pProducer->getmyFixCapital()
				<< " Employees:" << pProducer->getEmployees().size();
			LogFile().flush();
		}
		catch (...) {}
		*/

		// Ensure we have enough space in our vector
		if (ageInYears >= (int)_yearlyDeathsByAge.size()) {
			_yearlyDeathsByAge.resize(ageInYears + 1, 0);
		}

		// Increment the death count for this age
		_yearlyDeathsByAge[ageInYears]++;
	}

	delete pProducers()->at(fid._id);
	pProducers()->at(fid._id) = nullptr;

	++DEPData().FirmDeaths()[currMonth() % DEPData().FirmDeaths().size()];

	return 1; // nDismantled
};

void CWorld::writeFirmBirthDeathHistogram() {
	// Only proceed after AssistedProductionUpto
	long assistedProductionUpto = (long)getInputParameter("AssistedProductionUpto");
	
	if (currMonth() < assistedProductionUpto) return;
	
	// Check if we have any deaths to report this month
	int totalDeaths = 0;
	for (size_t i = 0; i < _yearlyDeathsByAge.size(); i++) {
		totalDeaths += _yearlyDeathsByAge[i];
	}
	
	if (totalDeaths == 0) return;
	
	// Output header with month info
	LogFile() << "\n// Firm Death Age Distribution - Month " << currMonth();
	LogFile() << " (" << totalDeaths << " deaths this month)";
	LogFile() << "\n// (% of dead firms that were N years old)\n";
	
	// Output age distribution of this month's deaths (starting from age 0)
	int maxAge = min((int)_yearlyDeathsByAge.size(), 11);  // Show up to 10 years (0-10)
	for (int age = 0; age < maxAge; ++age) {
		if (age < (int)_yearlyDeathsByAge.size() && _yearlyDeathsByAge[age] > 0) {
			double percent = 100.0 * _yearlyDeathsByAge[age] / totalDeaths;
			LogFile() << "Year " << age << ": " << std::fixed << std::setprecision(1) << percent << "%\n";
		}
	}
	
	// Reset death counts for next month
	_yearlyDeathsByAge.clear();
	_yearlyDeathsByAge.resize(DEPData().getNYears(), 0);
}

// Update AssistedProductionUpto dynamically based on BOTH:
// 1. IndivsWealth reaching target
// 2. HH GFCF (dGFCF_Households) stabilizing at a constant level
// Producers need at least 50% assistance until BOTH conditions are met
void CWorld::updateAssistedProductionUptoFromGFCFStability()
{
	double currentGFCF = getDEPData().getDataHouseholdsGFCF_mu();
	double gfcfStabilityTolerance = getInputParameter("GFCFStabilityTolerance"); // e.g., 0.02 (2%)
	long stabilityCheckStartMonth = (long)getInputParameter("GFCFStabilityCheckStartMonth"); // e.g., 36
	int requiredStableMonths = (int)getInputParameter("GFCFRequiredStableMonths"); // e.g., 12
	double waitMonthsAfterBothConditions = getInputParameter("AssistedMonthsAfterGFCFStability"); // e.g., 60
	
	// Only start checking after the start month
	if (currMonth() < stabilityCheckStartMonth)
		return;
	
	// Check if GFCF is stable (within tolerance of previous value)
	if (!_bGFCFStabilized && _prevGFCF > 0 && currentGFCF > 0)
	{
		double relativeChange = abs(currentGFCF - _prevGFCF) / _prevGFCF;
		
		if (relativeChange <= gfcfStabilityTolerance)
		{
			_consecutiveStableGFCFMonths++;
			
			// Check if we've had enough consecutive stable months
			if (_consecutiveStableGFCFMonths >= requiredStableMonths)
			{
				_bGFCFStabilized = true;
				_monthGFCFStabilized = currMonth() - requiredStableMonths + 1; // first stable month
			}
		}
		else
		{
			// Reset stability counter if GFCF changed significantly
			_consecutiveStableGFCFMonths = 0;
		}
	}
	
	_prevGFCF = currentGFCF;
	
	// Only update AssistedProductionUpto when BOTH conditions are met:
	// 1. IndivsWealth has reached target (_bIndivsWealthTargetReached)
	// 2. GFCF has stabilized (_bGFCFStabilized)
	if (_bIndivsWealthTargetReached && _bGFCFStabilized)
	{
		// Use the later of the two events as the reference point
		long referenceMonth = max(_monthIndivsWealthTargetReached, _monthGFCFStabilized);
		
		// Set FinishCalibrationAt and extend NYears ONLY ONCE when conditions are first met
		if (!_bCalibrationEndpointSet)
		{
			_bCalibrationEndpointSet = true;
			
			// Calculate the actual calibration endpoint:
			// referenceMonth + AssistedMonthsAfterWealthTarget + StabilizationMonthsAfterAssisted
			double assistedMonths = getInputParameter("AssistedMonthsAfterWealthTarget");
			double StabilizationMonths = getInputParameter("StabilizationMonthsAfterAssisted");
			double actualFinishCalibrationAt = referenceMonth + assistedMonths + StabilizationMonths;
			
			// Only update if earlier than the default upper bound (240)
			double currentFinishCalibAt = getInputParameter("FinishCalibrationAt");
			if (actualFinishCalibrationAt < currentFinishCalibAt)
			{
				DEPData().mInputParameters()["FinishCalibrationAt"] = actualFinishCalibrationAt;
				DEPData().FinishCalibrationAt() = (long)actualFinishCalibrationAt;  // update cached value

				// Extend NYears to include SimulateNYearsAfterCalibration
				double simulateYearsAfter = getInputParameter("SimulateNYearsAfterCalibration");
				long monthsPerYear = getDEPData().getMonthsPerYear();
				double newNYears = ceil((actualFinishCalibrationAt + simulateYearsAfter * monthsPerYear) / monthsPerYear);
				
				DEPData().mInputParameters()["NYears"] = newNYears;
				DEPData().NYears() = newNYears;
				DEPData().NMonths() = monthsPerYear * newNYears;
				
				// Resize plot data vectors to accommodate new NYears
				DEPData().ResizePlotsDataForNewNYears(newNYears);
				
				// Update plot window X-axis scales to reflect new NYears
#ifdef DEPLOYERS_GRAPHICS
				UpdateAllPlotScales(newNYears);
#endif
			}
			
			// Convert SaveMonthsAfterCalibration and SaveMonthsFromMonth0 to absolute SaveMonths
			// The first save point will be at FinishCalibrationAt (calibration end), not now
			long finishCalibAt = getDEPData().getFinishCalibrationAt();
			DEPData().SaveMonths().clear();
			DEPData().SaveMonths().push_back(finishCalibAt);  // Always save at calibration end
			for (const auto& relativeMonth : getDEPData().getSaveMonthsAfterCalibration())
			{
				int absoluteMonth = finishCalibAt + relativeMonth;
				DEPData().SaveMonths().push_back(absoluteMonth);
			}
			// Also add absolute months from SaveMonthsFromMonth0
			for (const auto& absoluteMonth : getDEPData().getSaveMonthsFromMonth0())
			{
				DEPData().SaveMonths().push_back(absoluteMonth);
			}

			// Reinitialize pandemic timing now that FinishCalibrationAt is finalized
			// Also extend NYears if pandemic end would exceed current simulation length
			if (_pandemicState.isPandemicActive())
			{
				_pandemicState.reinitializeTiming();

				// Extend NYears if needed to accommodate full pandemic duration + recovery observation
				long pandemicEnd = _pandemicState.getPandemicEndMonth();
				long monthsPerYear = getDEPData().getMonthsPerYear();
				double simulateYearsAfter = getInputParameter("SimulateNYearsAfterCalibration");
				long requiredEndMonth = pandemicEnd + (long)(simulateYearsAfter * monthsPerYear);  // 5 years after pandemic ends
				double requiredNYears = ceil((double)requiredEndMonth / monthsPerYear);
				
				double currentNYears = getDEPData().getNYears();
				if (requiredNYears > currentNYears)
				{
					DEPData().mInputParameters()["NYears"] = requiredNYears;
					DEPData().NYears() = requiredNYears;
					DEPData().NMonths() = monthsPerYear * (long)requiredNYears;
					DEPData().ResizePlotsDataForNewNYears(requiredNYears);
					
					LogFile() << "[PANDEMIC] Extended NYears from " << currentNYears
						<< " to " << requiredNYears
						<< " to accommodate pandemic end month " << pandemicEnd
						<< " + " << simulateYearsAfter << " years recovery observation\n";
#ifdef DEPLOYERS_GRAPHICS
					UpdateAllPlotScales(requiredNYears);
#endif
				}
			}
		}
		
		double newAssistedUpto = referenceMonth + waitMonthsAfterBothConditions;
		double maxUpto = getDEPData().getFinishCalibrationAt();
		newAssistedUpto = min(maxUpto, newAssistedUpto);
		
		double currentUpto = getInputParameter("AssistedProductionUpto");
		if (abs(newAssistedUpto - currentUpto) > 1)
		{
			DEPData().mInputParameters()["AssistedProductionUpto"] = newAssistedUpto;
		}
	}
	// If only one condition is met, keep AssistedProductionUpto at a high value
	// to ensure producers get at least 50% assistance
	else if (_bIndivsWealthTargetReached || _bGFCFStabilized)
	{
		// Ensure AssistedProductionUpto is at least current month + 60
		// This guarantees continued assistance while waiting for the other condition
		double minAssistedUpto = currMonth() + waitMonthsAfterBothConditions;
		double maxUpto = getDEPData().getFinishCalibrationAt();
		minAssistedUpto = min(maxUpto, minAssistedUpto);
		
		double currentUpto = getInputParameter("AssistedProductionUpto");
		if (currentUpto < minAssistedUpto)
		{
			DEPData().mInputParameters()["AssistedProductionUpto"] = minAssistedUpto;
		}
	}
}

void CWorld::AdjustSAMParameters()
{
	// Dynamically update AssistedProductionUpto based on GFCF stability
	updateAssistedProductionUptoFromGFCFStability();
	
	// Adjust workers wealth to target SAM value
	if (isCalibrationPhase())
	{
		double IndivsWealthTarget = getInputParameter("IndivsWealthTarget");

		double toTotalPopulationYear = getDEPData().getUpscaleSimulationFactor();

		double currIndivsShares = 0;
		for (auto& pair : this->Workers())
		{
			auto& indiv = *pair;
			currIndivsShares += indiv.getmyTotSharesValue();
		}
		currIndivsShares *= toTotalPopulationYear;

		// Check if total household wealth (Cash + Shares + GFCF) has reached target
		// This triggers transition from assisted to normal calibration
		if (!_bIndivsWealthTargetReached && getDEPData().getIndivsWealth() >= IndivsWealthTarget)
		{
			_bIndivsWealthTargetReached = true;
			_monthIndivsWealthTargetReached = currMonth();
		}

		if (currIndivsShares > IndivsWealthTarget)
		{
			for (auto& pair : this->Workers())
			{
				auto& indiv = *pair;

				DEPData().TotalInitialCash() -= indiv.Cash(); // remove indiv.Cash() from TotalInitialCash
				indiv.Cash() = 0;
				indiv.myGFCF() = 0;
			}
		}
		else
		{
			double currIndivsWealth = min(IndivsWealthTarget, getDEPData().getIndivsWealth());

			double wealthCorrectionFactor = 1.;
			if (getDEPData().getIndivsWealth() > IndivsWealthTarget)
				wealthCorrectionFactor = IndivsWealthTarget / getDEPData().getIndivsWealth(); // wealthCorrectionFactor <= 1

			// Wealth = Cash + Shares + GFCF = (1 + IndivsCashToGFCFShares) + (Shares + GFCF), with IndivsCashToGFCFShares = Cash/(Shares+GFCF)
			double IndivsCashToGFCFShares = getInputParameter("IndivsCashToGFCFShares"); // 0.30 for Austria 2010
			for (auto& pair : this->Workers())
			{
				auto& indiv = *pair;

				double newWealth = indiv.getWealth() * wealthCorrectionFactor;
				indiv.myGFCF() = max(0., newWealth / (1. + IndivsCashToGFCFShares) - indiv.getmyTotSharesValue());

				auto currCash = indiv.Cash();
				indiv.Cash() = max(0., newWealth - indiv.getmyTotSharesValue() - indiv.myGFCF());
				auto deltaCash = indiv.Cash() - currCash;
				DEPData().TotalInitialCash() += deltaCash; // add deltaCash to TotalInitialCash
			}
		}
	}

	// Dynamic transition: use assisted calibration until IndivsWealthTarget is reached,
	// continue assisted for AssistedMonthsAfterWealthTarget months, then smoothly transition
	// to normal ChangeFraction over StabilizationMonthsAfterAssisted months
	double AssistedMonthsAfter = getInputParameter("AssistedMonthsAfterWealthTarget"); // e.g., 24 months
	double StabilizationMonths = getInputParameter("StabilizationMonthsAfterAssisted"); // e.g., 24 months
	double ChangeFraction;
	double householdChangeFraction;
	
	if (!_bIndivsWealthTargetReached)
	{
		// Still in assisted calibration phase (before reaching wealth target)
		ChangeFraction = getInputParameter("ChangeFractionAssisted");
		householdChangeFraction = getInputParameter("ChangeFractionAssisted");
	}
	else
	{
		// Wealth target reached
		long monthsSinceTarget = currMonth() - _monthIndivsWealthTargetReached;
		
		if (monthsSinceTarget < AssistedMonthsAfter)
		{
			// Still in assisted phase (first 24 months after reaching target)
			ChangeFraction = getInputParameter("ChangeFractionAssisted");
			householdChangeFraction = getInputParameter("ChangeFractionAssisted");
		}
		else if (monthsSinceTarget < AssistedMonthsAfter + StabilizationMonths)
		{
			// Gradual transition phase
			double transitionProgress = (monthsSinceTarget - AssistedMonthsAfter) / StabilizationMonths;
			ChangeFraction = getInputParameter("ChangeFractionAssisted")
				+ transitionProgress * (getInputParameter("ChangeFraction") - getInputParameter("ChangeFractionAssisted"));
			householdChangeFraction = ChangeFraction;
		}
		else
		{
			// Fully transitioned to normal calibration
			ChangeFraction = getInputParameter("ChangeFraction");
			householdChangeFraction = getInputParameter("ChangeFraction");
		}
	}

	// Adjust ConsumPXFactor of Indivs to match SAM's Household PX Consumption
	if (getInputParameter("IndivConsum") > 0
		&& !isRealMarketSimulation())
	{

		double MaxConsumPXFactor = getInputParameter("MaxConsumPXFactor");

		for (long gType = 0; gType < getSAM().getnPXproducerTypes(); ++gType)
		{
			// Adjust ConsumPXFactor of Indivs as a function of their totWorth

			if (getRefTotalHouseholdPXConsum().at(gType) == 0)
				continue;

			double IndivsErr =
				((double)getDEPData().getTotalHouseholdPXConsum().at(gType)
					- getRefTotalHouseholdPXConsum().at(gType)) / getRefTotalHouseholdPXConsum().at(gType);

			// Clamp the error to prevent aggressive overshooting
			// Limit correction to at most 20% per step in either direction
			double clampedErr = max(-0.2, min(0.2, IndivsErr));

			double newVal = min(MaxConsumPXFactor,
				max(0., getConsumPXFactor().at(gType) * (1. - clampedErr))); // range [0, MaxConsumPXFactor]
			if (newVal > 0.95 * MaxConsumPXFactor)
				ERRORmsg("ConsumPXFactor too high, > MaxConsumPXFactor = " + to_string(MaxConsumPXFactor), true);

			ConsumPXFactor()[gType] = getConsumPXFactor().at(gType)
				* (1.0 - householdChangeFraction) + newVal * householdChangeFraction;
		}
	}

	// Adjust GovConsumPXFactor of Government to match SAM's Government PX Consumption
	if (getInputParameter("GovConsum") > 0
		&& !isRealMarketSimulation())
	{
		double upscaleFactor = getDEPData().getUpscaleSimulationFactor();
		double toTotalPopulationYear = 12. * upscaleFactor;

		for (long gType = 0; gType < getSAM().getnPXproducerTypes(); ++gType)
		{
			// Adjust GovConsumPXFactor of Gov

			if (RefGovConsumPX().at(gType) == 0)
				continue;

			double GovConsum_gType = getGovernment().getGoodsIhave(gType) * toTotalPopulationYear;

			double GovErr = (GovConsum_gType - RefGovConsumPX().at(gType))
				/ RefGovConsumPX().at(gType);

			double newVal = min(1.0, max(0., GovConsumPXFactor().at(gType) * (1. - GovErr))); // range [0, 1]

			GovConsumPXFactor()[gType] = GovConsumPXFactor().at(gType)
				* (1.0 - ChangeFraction) + newVal * ChangeFraction;
		}
	}

	// Adust InitSalaryCalibFactor() to get the SAM's InitUnemployment
	if (currMonth() >= getInputParameter("StartUnemploymentCalibrationAt")
		&& currMonth() <= getDEPData().getFinishCalibrationAt())
	{
		// Use the same dynamic ChangeFraction computed above
		double adjustFraction = ChangeFraction;
		double Unemploymentpercent = getSAM().getInitUnemploymentPercent();
		double currUnemp = DEPData().getUnemployment();
		//MJ		double currUnemp = DEPData().getAvgUnemployment();
		double minUnemploymentpercent = 1.0;
		double smoothUnemploypercent = max(minUnemploymentpercent, 100. * currUnemp);

		if (abs(Unemploymentpercent - smoothUnemploypercent) > 1.0)
		{
			// proportional filter
			double prevval = getInitSalaryCalibFactor();
			double newval = getInitSalaryCalibFactor() * (Unemploymentpercent / smoothUnemploypercent);
			InitSalaryCalibFactor() = prevval + adjustFraction * (newval - prevval);
			double maxFactor = 10.0;
			InitSalaryCalibFactor() = min(maxFactor, max(1 / maxFactor, InitSalaryCalibFactor()));
		}
	}

	// Adjust myGOSfactor of Producers to match SAM's GOS ratios
	if (isCalibrationPhase())
	{
		// Adjust GOSfactor to match SAM GOS ratios
		double adjustFraction = 0.01; // Slower than salary adjustment

		for (auto& pProducer : *pProducers())
		{
			if (!pProducer || pProducer->getAgentType() < 0) continue;

			GoodType prodType = pProducer->getAgentType();
			double targetGOS = pProducer->getmyGrossOpSurplus();

			// Calculate actual GOS from last month's operations
			double actualRevenue = pProducer->getproducedUnitsOf(prodType)
				* pProducer->getmyPriceOf(prodType);
			double baseCost = pProducer->getproducedUnitsOf(prodType)
				* pProducer->getproductionPrice();

			if (baseCost > 0 && targetGOS > 0 && pProducer->getproducedUnitsOf(prodType) > 0)
			{
				double actualGOS = (actualRevenue - baseCost) / pProducer->getproducedUnitsOf(prodType);

				// Guard against division by zero or very small values
				const double MIN_GOS = 1e-10;
				if (actualGOS > MIN_GOS && targetGOS > MIN_GOS)
				{
					double errorRatio = actualGOS / targetGOS;

					// Clamp errorRatio to reasonable bounds (e.g., 0.1 to 10.0)
					errorRatio = max(0.1, min(10.0, errorRatio));

					// Proportional adjustment
					double newFactor = 1.0 / errorRatio;
					pProducer->myGOSfactor() += adjustFraction * (newFactor - pProducer->myGOSfactor());

					// Clamp GOSfactor to reasonable bounds
					pProducer->myGOSfactor() = max(0.1, min(10.0, pProducer->myGOSfactor()));
				}
			}
		}
	}

	// Reset prices to one at specified month
	if (currMonth() == getInputParameter("ResetPricesToOneAt"))
	{
		for (auto& pair : CAgent::PriceCalibFactor())
		{
			if (pair.first == getSAM().GFCFtype()) // this is 1 + loan interestRate
				continue;

			CAgent::PriceCalibFactor()[pair.first] = DEPData().MarketPrices()[pair.first];
			DEPData().MarketPrices()[pair.first] = 1.0;
		}

		SetPricesToOne();
	}
};

// Log simulation stage milestones to the log file
void CWorld::logSimulationMilestones()
{
	long month = currMonth();
	long startCalibAt = (long)getInputParameter("StartCalibrationAt");
	long finishCalibAt = getDEPData().getFinishCalibrationAt();
	long assistedUpto = (long)getInputParameter("AssistedProductionUpto");
	long startUnempCalibAt = (long)getInputParameter("StartUnemploymentCalibrationAt");

	// Get current stage for logging
	SimulationStage stage = currSimulationStage();
	const char* stageName = getSimulationStageName(stage);

	// Helper to write timestamp
	auto writeTimestamp = [this]() {
		LogFile() << "  - Timestamp: ";
		writeTimeAndDate(LogFile());
	};

	// Stage 1: Initialization (month 0)
	if (month == 0)
	{
		LogFile() << "\n\n" << string(80, '=');
		LogFile() << "\n[MILESTONE] Month " << month << ": SIMULATION INITIALIZATION";
		LogFile() << "\n  - Stage: " << stageName;
		writeTimestamp();
		LogFile() << "  - SAM initialized, agents created";
		LogFile() << "\n  - StartCalibrationAt: " << startCalibAt;
		LogFile() << "\n  - FinishCalibrationAt (dynamic): " << finishCalibAt;
		LogFile() << "\n  - AssistedProductionUpto (dynamic): " << assistedUpto;
		LogFile() << "\n" << string(80, '=') << "\n";
	}

	// Stage 2: Pre-Calibration ends → Calibration begins
	if (month == startCalibAt)
	{
		LogFile() << "\n\n" << string(80, '=');
		LogFile() << "\n[MILESTONE] Month " << month << ": PRE-CALIBRATION ENDS → ASSISTED CALIBRATION BEGINS";
		LogFile() << "\n  - Stage: " << stageName;
		writeTimestamp();
		LogFile() << "  - Producers now run in parallel (not sequentially by work day)";
		LogFile() << "\n  - Salary adjustments enabled (market-driven)";
		LogFile() << "\n  - GDP/CPI tracking begins";
		LogFile() << "\n" << string(80, '=') << "\n";
	}

	// Stage 3: Unemployment calibration begins
	if (month == startUnempCalibAt)
	{
		LogFile() << "\n\n" << string(80, '-');
		LogFile() << "\n[MILESTONE] Month " << month << ": UNEMPLOYMENT CALIBRATION BEGINS";
		LogFile() << "\n  - Stage: " << stageName;
		writeTimestamp();
		LogFile() << "  - InitSalaryCalibFactor adjustments to match SAM unemployment target";
		LogFile() << "\n" << string(80, '-') << "\n";
	}

	// Stage 4: IndivsWealth target reached (dynamic milestone)
	if (_bIndivsWealthTargetReached && month == _monthIndivsWealthTargetReached)
	{
		LogFile() << "\n\n" << string(80, '*');
		LogFile() << "\n[MILESTONE] Month " << month << ": HOUSEHOLD WEALTH TARGET REACHED";
		LogFile() << "\n  - Stage: " << stageName;
		writeTimestamp();
		LogFile() << "  - IndivsWealth >= IndivsWealthTarget (" << getInputParameter("IndivsWealthTarget") << ")";
		LogFile() << "\n  - Assisted calibration will continue for " << getInputParameter("AssistedMonthsAfterWealthTarget") << " more months";
		LogFile() << "\n" << string(80, '*') << "\n";
	}

	// Stage 5: GFCF stabilized (dynamic milestone)
	if (_bGFCFStabilized && month == _monthGFCFStabilized)
	{
		LogFile() << "\n\n" << string(80, '*');
		LogFile() << "\n[MILESTONE] Month " << month << ": GFCF STABILIZED";
		LogFile() << "\n  - Stage: " << stageName;
		writeTimestamp();
		LogFile() << "  - Household GFCF stable for " << getInputParameter("GFCFRequiredStableMonths") << " consecutive months";
		LogFile() << "\n  - Stability tolerance: " << (getInputParameter("GFCFStabilityTolerance") * 100) << "%";
		LogFile() << "\n" << string(80, '*') << "\n";
	}

	// Stage 6: Both conditions met - calibration endpoint set (dynamic milestone)
	if (_bCalibrationEndpointSet && _bIndivsWealthTargetReached && _bGFCFStabilized)
	{
		long referenceMonth = max(_monthIndivsWealthTargetReached, _monthGFCFStabilized);
		long transitionStart = referenceMonth + (long)getInputParameter("AssistedMonthsAfterWealthTarget");
		
		if (month == referenceMonth)
		{
			LogFile() << "\n\n" << string(80, '#');
			LogFile() << "\n[MILESTONE] Month " << month << ": CALIBRATION ENDPOINT DETERMINED";
			LogFile() << "\n  - Stage: " << stageName;
			writeTimestamp();
			LogFile() << "  - Both wealth target and GFCF stability conditions met";
			LogFile() << "\n  - AssistedProductionUpto NOW SET TO: " << assistedUpto;
			LogFile() << "\n  - FinishCalibrationAt NOW SET TO: " << finishCalibAt;
			LogFile() << "\n  - Transition will begin at month: " << transitionStart;
			LogFile() << "\n" << string(80, '#') << "\n";
		}
	}

	// Stage 7: Assisted production ends → Transition begins
	if (month == assistedUpto && month > 0)
	{
		LogFile() << "\n\n" << string(80, '=');
		LogFile() << "\n[MILESTONE] Month " << month << ": ASSISTED PRODUCTION ENDS → TRANSITION BEGINS";
		LogFile() << "\n  - Stage: " << stageName;
		writeTimestamp();
		LogFile() << "  - Production subsidies end";
		LogFile() << "\n  - ChangeFraction transitions from " << getInputParameter("ChangeFractionAssisted")
			<< " to " << getInputParameter("ChangeFraction");
		LogFile() << "\n  - Transition duration: " << (finishCalibAt - assistedUpto) << " months";
		LogFile() << "\n" << string(80, '=') << "\n";
	}

	// Stage 8: Calibration finishes → Post-calibration (free market)
	if (month == finishCalibAt)
	{
		LogFile() << "\n\n" << string(80, '=');
		LogFile() << "\n[MILESTONE] Month " << month << ": CALIBRATION COMPLETE → FREE MARKET DYNAMICS";
		LogFile() << "\n  - Stage: " << stageName;
		writeTimestamp();
		LogFile() << "  - All SAM parameter adjustments cease";
		LogFile() << "\n  - Full market-driven behavior";
		LogFile() << "\n  - Snapshot saved with '_Calibrated' suffix";
		LogFile() << "\n" << string(80, '=') << "\n";
	}

	// Policy shock windows (if configured)
	long percentDemandFrom = (long)getInputParameter("PercentDemandFrom");
	long percentDemandUpto = (long)getInputParameter("PercentDemandUpto");
	if (month == percentDemandFrom && percentDemandFrom < 999000)
	{
		LogFile() << "\n\n" << string(80, '!');
		LogFile() << "\n[MILESTONE] Month " << month << ": DEMAND SHOCK BEGINS";
		writeTimestamp();
		LogFile() << "  - PercentDemand factor: " << getInputParameter("PercentDemand");
		LogFile() << "\n  - Duration: until month " << percentDemandUpto;
		LogFile() << "\n" << string(80, '!') << "\n";
	}
	if (month == percentDemandUpto && percentDemandFrom < 999000)
	{
		LogFile() << "\n[MILESTONE] Month " << month << ": DEMAND SHOCK ENDS ";
		writeTimestamp();
	}

	long fiscalShockFrom = (long)getInputParameter("FiscalPolicyShockFrom");
	long fiscalShockUpto = (long)getInputParameter("FiscalPolicyShockUpto");
	if (month == fiscalShockFrom && fiscalShockFrom < 999000)
	{
		LogFile() << "\n\n" << string(80, '!');
		LogFile() << "\n[MILESTONE] Month " << month << ": FISCAL POLICY SHOCK BEGINS";
		writeTimestamp();
		LogFile() << "  - FiscalPolicyShockFactor: " << getInputParameter("FiscalPolicyShockFactor");
		LogFile() << "\n  - Duration: until month " << fiscalShockUpto;
		LogFile() << "\n" << string(80, '!') << "\n";
	}
	if (month == fiscalShockUpto && fiscalShockFrom < 999000)
	{
		LogFile() << "\n[MILESTONE] Month " << month << ": FISCAL POLICY SHOCK ENDS ";
		writeTimestamp();
	}

	long taxChangeFrom = (long)getInputParameter("TaxChangeFrom");
	long taxChangeUpto = (long)getInputParameter("TaxChangeUpto");
	if (month == taxChangeFrom && taxChangeFrom < 999000)
	{
		LogFile() << "\n\n" << string(80, '!');
		LogFile() << "\n[MILESTONE] Month " << month << ": TAX CHANGE BEGINS";
		writeTimestamp();
		LogFile() << "  - TaxChangeFactor: " << getInputParameter("TaxChangeFactor");
		LogFile() << "\n  - Duration: until month " << taxChangeUpto;
		LogFile() << "\n" << string(80, '!') << "\n";
	}
	if (month == taxChangeUpto && taxChangeFrom < 999000)
	{
		LogFile() << "\n[MILESTONE] Month " << month << ": TAX CHANGE ENDS ";
		writeTimestamp();
	}

	// Reset prices milestone
	long resetPricesAt = (long)getInputParameter("ResetPricesToOneAt");
	if (month == resetPricesAt && resetPricesAt < 999000)
	{
		LogFile() << "\n\n" << string(80, '!');
		LogFile() << "\n[MILESTONE] Month " << month << ": PRICES RESET TO ONE ";
		writeTimestamp();
		LogFile() << "\n" << string(80, '!') << "\n";
	}

	// Flush log file to ensure milestones are written immediately
	LogFile().flush();
}

//  Initialize the world for a new month

void CWorld::initializeMonth()
{
	// Remove unused Producers, startup new ones

	DismantleAndStartNewProducers();

	// -----------------------------------------------------------------------------------

	AdjustSAMParameters();

	// -----------------------------------------------------------------------------------

	DEPData().TotSupply().clear();
	DEPData().TotProduced().clear();
	DEPData().TotalProduction() = 0;
	DEPData().TotDemand().clear();

	// make a randomized copy of Workers and Producers

	static vector<CAgent*> rndAgents;
	rndAgents.clear();
	rndAgents.insert(rndAgents.end(), pWorkers()->begin(), pWorkers()->end());
	rndAgents.insert(rndAgents.end(), pProducers()->begin(), pProducers()->end());
	std::shuffle(rndAgents.begin(), rndAgents.end(), myRandomEngine());

	for (auto& pAgent : rndAgents) // Workers & Producers
		if (pAgent != nullptr)
			pAgent->monthInitialize();

	pGovernment()->monthInitialize();

	for (auto& gPair : ExtSectors())
		pExtSect(gPair.first)->monthInitialize();

	DEPData().monthInitialize();
}

void CWorld::runOneMonth()
{
	// Log simulation stage milestones
	logSimulationMilestones();

	// Update pandemic state for current month (phase transitions, shock factors)
	updatePandemicState();

	// Update BLE expectations for current month (if enabled)
	updateBLEExpectations();

	if (currMonth() == 0)
	{
		InitializeSimulation();
		getSAM().writeInputSAM();
		SAM().saveInputSAM(SimulationName() + "_" + getSAM().CountryCode() + "_SAM.csv");
	}

	pDEPData()->AnalizeLastMonth();// review

	// Log key metrics for monitoring - MONTHLY for curve comparison
	// Curves: TotalProduction (Production[64]), AvgUnempl (Employment[1]), realGDP (GovValues[7]), Firms0-9 (FirmsPerFirmsize[0])
	{
		double totalFirms = 0;
		for (const auto& pair : getDEPData().getnFirmsPerFirmsize()) {
			totalFirms += pair.second;
		}
		double firms0to9Pct = (totalFirms > 0) ? 
			100.0 * getDEPData().getnFirmsPerFirmsize().at("0-9") / totalFirms : 0.0;
		
		// Labeled output for readability
		double cpiValue = getDEPData().getCPItracker().getCPI().size() > 0
			? getDEPData().getCPItracker().getCPI().back() : 100.0;
		LogFile() << "\n[CURVES] Month " << std::setw(3) << currMonth()
			<< ", Production=" << std::fixed << std::setprecision(2) << (getDEPData().getTotalProduction() * 1.e-9)
			<< ", Unempl%=" << std::setprecision(2) << (getDEPData().getUnemployment() * 100)
			<< ", realGDP=" << std::setprecision(2) << (getDEPData().getGDPtracker().getreal_gdp() * 1.e-9)
			<< ", CPI=" << std::setprecision(2) << cpiValue
			<< ", size0-9%=" << std::setprecision(1) << firms0to9Pct;
		LogFile().flush();
	}

	// Modified condition to handle reloads properly
	if (currMonth() > InitMonth() + 1)
	{
		// Normal operation: read external sectors I/O (use v2 parser)
		readMyExternalSectorsIO_v2();
	}
	else if (getInputParameter("LoadMonthN") > 0 && currMonth() == InitMonth() + 1)
	{
		// First month after reload: force I/O synchronization
		writeExternalSectorsIO();
	}

	initializeMonth();

	// 1. Make a randomized copy of individuals  ----------------------------------------------

	static vector<CAgent*> rndIndivs;
	rndIndivs.clear();
	rndIndivs.insert(rndIndivs.end(), pWorkers()->begin(), pWorkers()->end()); // sorted by increasing ID
	std::shuffle(rndIndivs.begin(), rndIndivs.end(), myRandomEngine());

	// 2. Randomize Producers and sort them by their MonthlyActivityMonth *only*  ----------------------------------------

	static vector<AgentSortedByFirst> sortedProducersByWorkDayOnly; // sort only by .first

	sortedProducersByWorkDayOnly.clear();
	for (auto pProd : (*pProducers())) // sorted by increasing ID
		if (pProd != nullptr)
			sortedProducersByWorkDayOnly.push_back(AgentSortedByFirst(pProd->getMonthlyActivityMonth(), pProd));

	// shuffle the ID-sorted vector
	std::shuffle(sortedProducersByWorkDayOnly.begin(), sortedProducersByWorkDayOnly.end(), myRandomEngine());

	// sort by .first only
	// warning: don't use pair<x, y>, it sorts by pair's first and then by second
	// AgentSortedByFirst sorts by pair's first only


	std::sort(sortedProducersByWorkDayOnly.begin(),
		sortedProducersByWorkDayOnly.end(), myless<AgentSortedByFirst>()); // by increasing date

	// don't shuffle Producers again, preserve activity sequence to have one month activity period

// =============  Monthly activity of Indivs, Producers, Gov and ExtSects  ===================

	GoodQtty nWorkers = pWorkers()->size();
	// Each activity group is like a workDay of month (RunProducer)
	GoodQtty WorkDaysPerMonth = min(nWorkers, (GoodQtty)DEPData().InputParameter("WorkDaysPerMonth"));
	long nIndivsPerWorkDay = (GoodQtty)(ceil((double)rndIndivs.size() / WorkDaysPerMonth));
	GoodQtty nProducers = sortedProducersByWorkDayOnly.size();

	//  >>>>>>>>>>>>>>>>>>>>>>>  MAIN LOOP <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

	Government().GoodsToBuy() = Government().getmyGoodsIwish();

	for (auto& pair : ExtSectors())
	{
		auto& ExtSector = *pair.second;
		ExtSector.GoodsToBuy() = ExtSector.getmyGoodsIwish();
	}

	long indivN = 0;
	long producerN = 0;
	for (long workDayN = 0; workDayN < WorkDaysPerMonth; ++workDayN)
	{
		// Final consumers buy from producers, by groups of nIndivsPerWorkDay, randomized list every month
		do
		{
			CWorker* pIndiv = (CWorker*)rndIndivs.at(indivN);

			if (DEPData().InputParameter("IndivConsum"))
			{
				if (currMonth() >= 9999) DEPData().CheckAccountingBalance(); // && pIndiv->_ID == 21
				pIndiv->monthActivity();
				if (currMonth() >= 9999) DEPData().CheckAccountingBalance();
			}

			if (DEPData().InputParameter("GovConsum"))
			{
				Government().myProxyBuyerID() = pIndiv->getID();

				Government().BuyGoods();
				if (currMonth() >= 9999) DEPData().CheckAccountingBalance();
			}

			if (DEPData().InputParameter("ExtSectConsum"))
			{
				for (auto& pair : ExtSectors())
				{
					auto& ExtSector = *pair.second;

					ExtSector.myProxyBuyerID() = pIndiv->getID();

					ExtSector.BuyGoods();
					if (currMonth() >= 9999) DEPData().CheckAccountingBalance();
				}
			}

		} while (++indivN % nIndivsPerWorkDay != 0 && indivN < nWorkers);

		if (isPreCalibration())
		{
			// Sequential producer activity by work day during pre-calibration
			while (producerN < nProducers)
			{
				auto& sortedAgent = sortedProducersByWorkDayOnly.at(producerN);
				CProducer* pProd = (CProducer*)(sortedAgent.second);
				if (sortedAgent.first != workDayN)
					break;

				pProd->monthActivity();
				if (currMonth() >= 9999) DEPData().CheckAccountingBalance();
				++producerN;
			}
		}
		else
			for (auto& pair : sortedProducersByWorkDayOnly)
			{
				CProducer* pProd = (CProducer*)(pair.second);
				if (currMonth() >= 9999) DEPData().CheckAccountingBalance();

				if (pProd->isProductionDay(workDayN) == true)
					pProd->monthActivity(); // full month activity if Nproduction_days=1
			}
	}

	//  ==============================================================================================

	// Remaining tasks at the end of month: FinancialMarket, Taxes, Subsidies, Interests...

	Government().monthlyActivity();

	for (auto& pair : ExtSectors())
		pair.second->monthActivity();

	FinancialMarket().monthActivity();

	getSAM().writeSAMmonth();

	writeFirmBirthDeathHistogram();

	writeExternalSectorsIO();

	DEPData().CheckAccountingBalance();
};

///////////////////////////////////////////////////////////////////////////////////////
