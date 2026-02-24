
//  Producer.cpp

//   DEPLOYERS v2

#include "./pch.h"

vector<vector<CProducerSpecs*>>* CProducer::_pProducerFigSpecs;
map< GoodType, CProducerSpecs* > CProducer::_ProducerSpecs;
map< GoodType, CProductSpecs* >  CProducer::_ProductSpecs;
vector<string> CProducer::_ProducerLabelOfType;
map< string, GoodType > CProducer::_ProducerTypeOfLabel;

//======================  CProducerSpecs  ===================================

CProducerSpecs::CProducerSpecs(GoodType producerTy)
	: _producerType(producerTy)
{
	// Producer definition parameters, they are not modified during the simulation
};
CProducerSpecs::~CProducerSpecs() {};

ofstream& operator<<(ofstream& ofstrm, const CProducerSpecs& producerSpecs)
{
	string tab1 = "\t", tab2 = "\t\t", tab3 = "\t\t\t";

	ofstrm << tab1 << CProducer::ProducerLabelOfType()[producerSpecs.getproducerType()]
		<< " {" << endl;

	ofstrm << tab1 << "}" << endl;

	return ofstrm;
};  
ifstream& operator>>(std::ifstream& infile, CProducerSpecs& producerSpecs)
{
	std::string productName, word, name, bracket;
	double value = 0;

	infile >> bracket;// bracket"{"

	while (infile >> word, word != "}")
	{
		CWorld::ERRORmsg(
			"Unexpected word reading PRODUCER_DEFINITIONS " + word, true);
	}

	return infile;
}

GoodType& CProducerSpecs::producerType() { return _producerType; };
const GoodType& CProducerSpecs::getproducerType() const { return _producerType; };
const CProductSpecs& CProducerSpecs::getSpecsOfProductType(GoodType productType)
{
	return CProducer::getProductSpecs(productType);
};

//======================  CHistory  ===================================

// Constructor now takes HISTORY_LENGTH as parameter
CHistory::CHistory(int historyLength = 0)
	: _HISTORY_LENGTH(historyLength),
	_historyIndex(0),
	_filledCount(0),
	_average(0.0)
{
	resize(_HISTORY_LENGTH, 0.0);
}

// Copy constructor
CHistory::CHistory(const CHistory& other)
	: vector<double>(other),
	_HISTORY_LENGTH(other._HISTORY_LENGTH),
	_historyIndex(other._historyIndex),
	_filledCount(other._filledCount),
	_average(other._average)
{
}

// Copy assignment operator
CHistory& CHistory::operator=(const CHistory& other)
{
	if (this != &other) {
		// Check if HISTORY_LENGTH matches
		if (_HISTORY_LENGTH != other._HISTORY_LENGTH) {
			CWorld::ERRORmsg("CHistory: Cannot assign histories with different lengths", true);
		}

		// Copy base class
		vector<double>::operator=(other);

		// Copy members (except const _HISTORY_LENGTH)
		_historyIndex = other._historyIndex;
		_filledCount = other._filledCount;
		_average = other._average;
	}
	return *this;
}

// Move constructor
CHistory::CHistory(CHistory&& other) noexcept
	: vector<double>(std::move(other)),
	_HISTORY_LENGTH(other._HISTORY_LENGTH),
	_historyIndex(other._historyIndex),
	_filledCount(other._filledCount),
	_average(other._average)
{
}

// Move assignment operator
CHistory& CHistory::operator=(CHistory&& other)
{
	if (this != &other) {
		// Check if HISTORY_LENGTH matches
		if (_HISTORY_LENGTH != other._HISTORY_LENGTH) {
			CWorld::ERRORmsg("CHistory: Cannot assign histories with different lengths", true);
		}

		// Move base class
		vector<double>::operator=(std::move(other));

		// Copy members (except const _HISTORY_LENGTH)
		_historyIndex = other._historyIndex;
		_filledCount = other._filledCount;
		_average = other._average;
	}
	return *this;
}

ofstream& operator<<(ofstream& ofstrm, const CHistory& history)
{
	ofstrm << " { "
		<< history._HISTORY_LENGTH << " " << history._historyIndex << " "
		<< history._filledCount << " " << history._average;

	ofstrm << " {";
	for (const auto& value : history)
		ofstrm << " " << value;
	ofstrm << " }";

	ofstrm << " }";
	return ofstrm;
}

ifstream& operator>>(ifstream& ifstrm, CHistory& history)
{
	string bracket;
	int historyLength;

	ifstrm >> bracket; // " {"
	ifstrm >> historyLength >> history._historyIndex
		>> history._filledCount >> history._average;

	// Verify the loaded history length matches
	if (historyLength != history._HISTORY_LENGTH) {
		CWorld::ERRORmsg("CHistory: loaded HISTORY_LENGTH (" + to_string(historyLength) +
			") doesn't match expected (" + to_string(history._HISTORY_LENGTH) + ")", true);
	}

	ifstrm >> bracket; // " {"
	history.clear();
	history.resize(history._HISTORY_LENGTH, 0.0);

	int count = 0;
	while (ifstrm >> bracket, bracket != "}")
	{
		if (count >= history._HISTORY_LENGTH) {
			CWorld::ERRORmsg("CHistory: too many values in saved data", true);
			break;
		}
		double value = atof(bracket.c_str());
		history[count++] = value;
	}

	ifstrm >> bracket; // " }"
	return ifstrm;
}

double CHistory::updateAndReturnAvg() {
	// Use the full HISTORY_LENGTH for calculation, not just filled count
	if (_filledCount == 0) {
		_average = 0.0;
		return _average;
	}

	double total = 0.0;
	int countToUse = min(_filledCount, _HISTORY_LENGTH);
	for (int i = 0; i < countToUse; i++) {
		total += (*this)[i];
	}
	_average = total / countToUse;
	return _average;
}

void CHistory::updateHistory(double newVal) {
	(*this)[_historyIndex] = newVal;
	_historyIndex = (_historyIndex + 1) % _HISTORY_LENGTH;
	if (_filledCount < _HISTORY_LENGTH) {
		_filledCount++;
	}

	// Update average after adding new value
	if (_filledCount == 0) {
		_average = 0.0;
	}
	else {
		double total = 0.0;
		int countToUse = min(_filledCount, _HISTORY_LENGTH);
		for (int i = 0; i < countToUse; i++) {
			total += (*this)[i];
		}
		_average = total / countToUse;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

//======================  CProducer  ===================================

CProducer::CProducer(AgentID id, GoodType producerType, CAgent* powner)
	: CAgent(id, producerType),
	_ProductionHistory((int)getInputParameter("HISTORY_LENGTH")),
	_mySupplyHistory((int)getInputParameter("HISTORY_LENGTH")),
	_LeftToSellHistory((int)getInputParameter("HISTORY_LENGTH")),
	_myDemandHistory((int)getInputParameter("HISTORY_LENGTH"))
{
	pOwners() = new map<CAgentFID, CAgent*>;
	if (powner != nullptr)
	{
		(*pOwners())[powner->getFID()] = powner;
		powner->myProducers()[this->getFID()] = this;
	}

	pEmployees() = new vector<CWorker*>;

	auto gType = getAgentType();
	if (gType >= 0 && gType < getSAM().getnPProducerTypes())
		++DEPData().nCurrentPProducers()[gType];

	initialize();
}

CProducer::~CProducer() {};

ofstream& operator<<(ofstream& ofstrm, const CProducer& producer)
{
	ofstrm << " " << CProducer::getProducerLabelOfType(producer.getAgentType()) << " {";
	ofstrm << " ID " << (long)producer.getID();

	ofstrm << (CAgent&)producer; // call base class

	ofstrm << "\n timeWorked() " << producer.gettimeWorked();

	ofstrm << endl << " EmployeesID { ";
	for (const auto& employee : producer.getEmployees())
		ofstrm << "" << employee->getID() << " ";
	ofstrm << " }";

	ofstrm << endl << "ProductionHistory "; ofstrm << producer.getProductionHistory();
	ofstrm << endl << "mySupplyHistory "; ofstrm << producer.getmySupplyHistory();
	ofstrm << endl << "LeftToSellHistory "; ofstrm << producer.getLeftToSellHistory();
	ofstrm << endl << "myDemandHistory "; ofstrm << producer.getmyDemandHistory();

	ofstrm << endl
		<< " LastUsedMonth " << producer.getLastUsedMonth()
		<< " myFirstProductionDay " << producer.getmyFirstProductionDay()
		<< " ToBeProduced " << producer.getToBeProduced()
		<< " currDepositInterests_mu " << producer.getcurrDepositInterests_mu();

	ofstrm << " assistedQtties "; ofstrm << producer.getassistedQtties();

	ofstrm << endl << " OwnerIDs {";
	if (producer.getpOwners() != nullptr)
		for (const auto& pair : *(producer.getpOwners()))
			ofstrm << pair.first;
	else
		ofstrm << " -1";
	ofstrm << " }";

	ofstrm << "\n ProductionPrice " << producer.getproductionPrice();
	ofstrm << "\n myMarkupFactor " << producer.getmyMarkupFactor();
	ofstrm << "\n InFinancialMarket " << producer.getInFinancialMarket();
	ofstrm << "\n NoCompEmployees " << producer.getNoCompEmployees();

	ofstrm << "\n myGrossOpSurplus " << producer.getmyGrossOpSurplus();
	ofstrm << "\n myLabourProductivity " << producer.getmyLabourProductivity();

	ofstrm << endl << " producedUnits "; ofstrm << producer.getproducedUnits();
	ofstrm << endl << " StockReference " << producer.getStockReference();
	ofstrm << " prevmyDemand " << producer.getprevmyDemand();
	ofstrm << " myDemand " << producer.getmyDemand();
	ofstrm << " prevavgmyDemand " << producer.getprevavgmyDemand();
	ofstrm << " avgmyDemand " << producer.getavgmyDemand();
	ofstrm << " myGOSfactor " << producer.getmyGOSfactor();
	ofstrm << " averageProduction " << producer.getaverageProduction();
	ofstrm << " mySupply " << producer.getmySupply();
	ofstrm << " LeftToSell " << producer.getLeftToSell();
	ofstrm << " prevToBeProduced " << producer.getprevToBeProduced();

	ofstrm << endl << "}";

	return ofstrm;
}
ifstream& operator>>(ifstream& ifstrm, CProducer& producer)
{
	operator>>(ifstrm, (CAgent&)producer); // call base class

	string name, word, bracket;
	long nn;

	ifstrm >> word >> producer.timeWorked();

	ifstrm >> word >> bracket; // " EmployeesID { "
	while (ifstrm >> word, word != "}")
	{
		nn = atoi(word.c_str());
		CWorker* pIndiv = getWorld().getpWorkers()->at(nn);
		producer.Employees().push_back(pIndiv);
	}

	producer.ProductionHistory().clear();
	ifstrm >> word; ifstrm >> producer.ProductionHistory();
	producer.mySupplyHistory().clear();
	ifstrm >> word; ifstrm >> producer.mySupplyHistory();
	producer.LeftToSellHistory().clear();
	ifstrm >> word; ifstrm >> producer.LeftToSellHistory();
	producer.myDemandHistory().clear();
	ifstrm >> word; ifstrm >> producer.myDemandHistory();

	ifstrm
		>> name >> producer.LastUsedMonth()
		>> name >> producer.myFirstProductionDay()
		>> name >> producer.ToBeProduced()
		>> name >> producer.currDepositInterests_mu();

	ifstrm >> name;  ifstrm >> producer.assistedQtties();

	ifstrm >> name >> bracket; // " OwnerIDs {"
	while (ifstrm >> word, word != "}")
	{
		auto ty = atoi(word.c_str());
		ifstrm >> word;
		auto id = atoi(word.c_str());
		if (ty == -1)
		{
			CWorker* pIndiv = getWorld().getpWorkers()->at(id);
			(*producer.pOwners())[CAgentFID(ty, id)] = pIndiv;
			//pIndiv->pmyProducers() = &producer;
			pIndiv->pmyProducer(producer.getFID()) = &producer;
		}
		else if (ty == GovernmentType)
		{
			(*producer.pOwners())[CAgentFID(ty, id)] = pGovernment();
			//Government().pmyProducers() = &producer;
			pGovernment()->pmyProducer(producer.getFID()) = &producer;
		}
		else if (getSAM().IsExtSectType(ty))
		{
			(*producer.pOwners())[CAgentFID(ty, id)] = pExtSect(id);
			//ExtSect(id).pmyProducers() = &producer;
			pExtSect(id)->pmyProducer(producer.getFID()) = &producer;
		}
	}

	ifstrm >> word >> producer.productionPrice();
	ifstrm >> word >> producer.myMarkupFactor();
	ifstrm >> word >> producer.InFinancialMarket();
	ifstrm >> word >> producer.NoCompEmployees();

	ifstrm >> word >> producer.myGrossOpSurplus();
	ifstrm >> word >> producer.myLabourProductivity();

	ifstrm >> word; ifstrm >> producer.producedUnits();
	ifstrm >> word >> producer.StockReference();
	ifstrm >> word >> producer.prevmyDemand();
	ifstrm >> word >> producer.myDemand();
	ifstrm >> word >> producer.prevavgmyDemand();
	ifstrm >> word >> producer.avgmyDemand();
	ifstrm >> word >> producer.myGOSfactor();
	ifstrm >> word >> producer.averageProduction();
	ifstrm >> word >> producer.mySupply();
	ifstrm >> word >> producer.LeftToSell();
	ifstrm >> word >> producer.prevToBeProduced();

	ifstrm >> word; // "}" 

	//  ------------------------

	for (auto pEmployee : producer.Employees())
		pEmployee->pEmployer() = &producer;

	return ifstrm;
}

void CProducer::initialize()
{
	if (getProductSpecs().size() == 0)
		return; // not ready yet

	GoodType producerType = getAgentType();

	_consecutiveMonthsNegativeWealth = 0;

	NoCompEmployees() = false;
	if (!IsNonProducerBankType(producerType))
		if (getProductSpecs(getAgentType()).getCompEmployees() == 0)
			NoCompEmployees() = true;

	CAgent::initialize(); // base class

	// Initialize producer variables

	myGrossOpSurplus() = 0;
	myGOSfactor() = 1.0;  // Start at 1.0, will be adjusted during calibration
	myMarkupFactor() = 1.0;
	myLabourProductivity() = 0;

	auto& productSpecs = *CProducer::ProductSpecs()[getAgentType()];
	if (!IsExtSect()
		&& !IsNonProducerBankType(producerType))
	{
		if (getSAM().getRowCol("CompEmployees", getAgentType()) > 0)
			myLabourProductivity() = (double)productSpecs.getGrossOutput_mu()
			/ getSAM().getRowCol("CompEmployees", getAgentType());

		if (getSAM().getRowCol("GrossOpSurplus", getAgentType()) > 0)
		{
			myGrossOpSurplus() = getSAM().getRowCol("GrossOpSurplus", getAgentType())
				/ (double)productSpecs.getGrossOutput_mu();
			myGOSfactor() = 1.0;  // Start at 1.0, will be adjusted during calibration
		}
	}

	Employees().clear();

	currIC_mu() = 0;
	currCompEmployees_mu() = 0;
	currNetTaxes_mu() = 0;
	currDepositInterests_mu() = 0;
	currLoanPaymentsAndInterests_mu() = 0;
	LastUsedMonth() = currMonth();
	myFirstProductionDay() = -1;
	timeWorked() = 0;

	LeftToSell() = 0;
	ToBeProduced() = 0;
	averageProduction() = 0;
	mySupply() = 0;
	prevToBeProduced() = 0;
	producedUnits().clear();
	assistedQtties().clear();

	StockReference() = max(0.5 * getavgmyDemand(), 10.0 * getmySalary_mu());
	ProductionHistory().average() = 0;
	mySupplyHistory().average() = 0;
	LeftToSellHistory().average() = 0;
	prevmyDemand() = 0;
	myDemand() = 0;
	prevavgmyDemand() = 0;
	avgmyDemand() = 0;

	NewSharesIssued() = 0; // Only one initial owner
	if (pOwners()->size() > 1) // keep first owner only
	{
		CAgent* pOwner = (*pOwners()).begin()->second;
		assert(pOwners()->size() > 0);
		pOwners()->clear();
		(*pOwners())[pOwner->getFID()] = pOwner;
		pOwner->pmyProducer(this->getFID()) = this;
	}

	WorkTimeToBuy() = 0;

	if (!IsNonProducerBankType(producerType))
	{
		const auto& NeededPerUnit = productSpecs.getNeededPerUnit();
		for (const auto& inPair : NeededPerUnit)
		{
			auto gType = inPair.first;
			GoodsIhave()[gType] = 0;
			GoodsToBuy()[gType] = 0;
			myPrice()[gType] = 1.0;
		}
	}

	productionPrice() = 1.0;
	if (producerType >= 0)
	{
		myPrice()[producerType] = productionPrice();
		ToSell(producerType) = 0;// get ready to offer it to clients
	}

	InFinancialMarket() = false;
}

bool CProducer::getHasWorkedCurrentMonth() const { return gettimeWorked() > 0.; };

map<CAgentFID, CAgent*>*& CProducer::pOwners() { return _pOwners; };
map<CAgentFID, CAgent*>& CProducer::Owners() { return *_pOwners; };
vector<CWorker*>*& CProducer::pEmployees() { return _pEmployees; };
vector<CWorker*>* CProducer::getpEmployees() const { return _pEmployees; };
vector<CWorker*>& CProducer::Employees() { return *_pEmployees; };
GoodQtty& CProducer::ToBeProduced() { return _ToBeProduced; };
long& CProducer::LastUsedMonth() { return _LastUsedMonth; };
const map<CAgentFID, CAgent*>* CProducer::getpOwners() const { return _pOwners; };
const map<CAgentFID, CAgent*>& CProducer::getOwners() const { return *_pOwners; };
const vector<CWorker*>& CProducer::getEmployees() const { return *_pEmployees; };
GoodQtty CProducer::getToBeProduced() const { return _ToBeProduced; };
const long CProducer::getLastUsedMonth() const { return _LastUsedMonth; };
void CProducer::releaseEmployees()
{
	for (auto pair : Employees())
		pair->pEmployer() = nullptr;
}
CGoods CProducer::getInventory()
{
	return CAgent::getInventory();
};

//--------------------  Producer Specs  ----------------------------

const string CProducer::getProducerName() const { return getAgentName(); }
map<GoodType, CProducerSpecs*>& CProducer::mProducerSpecs() { return CProducer::_ProducerSpecs; };
const map<GoodType, CProducerSpecs*>& CProducer::getProducerSpecs() { return CProducer::_ProducerSpecs; };
const CProducerSpecs& CProducer::getSpecsOfProducerType(AgentType producerType) { return *getProducerSpecs().at(producerType); };
const CProducerSpecs& CProducer::Specs() { return CProducer::getSpecsOfProducerType(this->getAgentType()); };
const CProducerSpecs& CProducer::getSpecs() const { return CProducer::getSpecsOfProducerType(this->getAgentType()); };

//--------------------  This Producer Status & Specs  ----------------------------

const vector<string>& CProducer::getProducerLabelOfType() { return _ProducerLabelOfType; };
const string CProducer::getProducerLabelOfType(GoodType gType)
{
	if (gType == getCentralBankType())
		return "CentralBank";
	else if (gType == getPrivateBankType())
		return "PrivateBank";
	else
		return getAccount(gType).label();
};
const AgentType CProducer::getProducerTypeOfLabel(string label) { return getSAM().getAccNofLabel(label); };
vector<string>& CProducer::ProducerLabelOfType() { return _ProducerLabelOfType; };
map<string, AgentType>& CProducer::ProducerTypeOfLabel() { return _ProducerTypeOfLabel; };
const map<string, AgentType>& CProducer::getProducerTypeOfLabel() { return _ProducerTypeOfLabel; };
const CProductSpecs& CProducer::getSpecsOfProductType(GoodType productType)
{
	return CProducer::getProductSpecs(productType);
};
void CProducer::readProducerSpecs(ifstream& ifstrm) // static
{
	string producerName, bracket;

	ProducerTypeOfLabel().clear();

	ifstrm >> bracket; // " {"
	mProducerSpecs().clear();
	AgentType producerType = UndefAgentType;
	while (ifstrm >> producerName, producerName != "}")
	{
		producerType = getSAM().getAccNofName(producerName);

		CProducerSpecs& producerSpecs = *new CProducerSpecs(producerType);

		ifstrm >> producerSpecs;

		mProducerSpecs()[producerSpecs.getproducerType()]
			= &producerSpecs;
	}
};
void CProducer::writeProducerSpecs(ofstream& ofstrm) // static
{
	ofstrm
		<< "PRODUCER_DEFINITIONS(Specs) {" << endl;
	for (const auto& pair : getProducerSpecs())
	{
		ofstrm << *pair.second;
	}
	ofstrm << "}" << endl << endl;
};
void CProducer::readProductSpecs(ifstream& ifstrm) // static
{
	string productName, bracket;

	ifstrm >> bracket; // " {"
	CProducer::ProductSpecs().clear();
	while (ifstrm >> productName, productName != "}")
	{
		CProductSpecs& productDef =
			*new CProductSpecs(CGoods::GoodNameToType(productName));

		ifstrm >> productDef;

		CProducer::ProductSpecs()[productDef.getproductType()]
			= &productDef;
	}
};
void CProducer::writeProductSpecs(ofstream& ofstrm) // static
{
	ofstrm << "PRODUCT_DEFINITIONS(Specs) {" << endl;
	for (const auto& pair : CProducer::ProductSpecs())
	{
		ofstrm << *pair.second;
	}
	ofstrm << "}" << endl << endl;

	ofstrm << endl;
};
map<GoodType, CProductSpecs*>& CProducer::ProductSpecs() { return CProducer::_ProductSpecs; };
const map<GoodType, CProductSpecs*>& CProducer::getProductSpecs() { return CProducer::_ProductSpecs; };
GoodQtty& CProducer::StockReference() { return _StockReference; };
const GoodQtty& CProducer::getStockReference() const { return _StockReference; }
GoodQtty CProducer::getNewSharesIssued() const { return _NewSharesIssued; };
double CProducer::getmyCurrShareVal() const
{
	double currShareVal = 0;
	GoodQtty myShs = getmySharesOf(getFID());
	if (myShs != 0)
		currShareVal = (double)getWealth() / (-myShs); // -: n shares sold

	return currShareVal;
};
double CProducer::getmyTotSharesValue() const
{
	return (double)getWealth(); // -: n shares sold
}
const CProductSpecs& CProducer::getProductSpecs(GoodType productType)
{
	return *CProducer::getProductSpecs().at(productType);
};
const GoodQtty CProducer::MinimumProductUnits(const GoodType productType) const
{
	const auto& Specs = CProducer::getProductSpecs(productType);
	if (Specs.getImportPrice() > 0)
		return 1;

	GoodQtty minQtty = 1;
	double minEfficiency = 1.0; // 0.1 avoid too expensive production

	return 1;
};
GoodQtty CProducer::releaseCash()
{
	GoodQtty cash = Cash();
	Cash() = 0;
	return cash;
};
CGoods CProducer::transferGoodsIhave()
{
	CGoods Ihave = GoodsIhave();
	GoodsIhave() -= GoodsIhave(); // reset to 0
	return Ihave;
};
CGoods CProducer::transferToSell()
{
	CGoods toSell = ToSell();
	ToSell() -= ToSell(); // reset to 0
	return toSell;
};
const CWorker* CProducer::ReleaseLastEmployee()
{
	CWorker* pEmployee = Employees().back();
	assert(pEmployee->getpEmployer() == this);

	pEmployee->pEmployer() = nullptr;

	Employees().pop_back();

	return pEmployee;
}
const bool CProducer::ReleaseThisEmployee(CWorker& employee)
{
	assert(employee.getpEmployer() == this);

	bool ok = false;
	for (long nEmployee = 0; nEmployee < getEmployees().size(); ++nEmployee)
	{
		if (Employees()[nEmployee]->getID() == employee.getID())
		{
			swap(Employees().at(nEmployee), Employees().back());
			ReleaseLastEmployee();
			ok = true;
			break;
		}
	}
	return ok;
}

GoodQtty CProducer::getWealth() const
{
	GoodQtty Wealth = getCash() + getmyBankBalance() + getmyFixCapital();

	return Wealth;
}
double CProducer::getavailableWorkTime() const
{
	double availableWorkTime = 0;
	for (int n = 0; n < getEmployees().size(); ++n)
		availableWorkTime += getEmployees().at(n)->getmyAvailableTime();

	return availableWorkTime;
}

double CProducer::updateAndReturnAvgProduction() {
	return ProductionHistory().updateAndReturnAvg(); // see RunProducer for last data
}
double CProducer::updateAndReturnAvgmySupply() {
	return mySupplyHistory().updateAndReturnAvg();
}
double CProducer::updateAndReturnAvgLeftToSell() {
	return LeftToSellHistory().updateAndReturnAvg();
}

bool CProducer::isProductionDay(int Ntoday, int daysInMonth,
	int productionDays, int myFirstProductionDay) const
{
	if (productionDays <= 0 || daysInMonth <= 0 || Ntoday <= 0 || Ntoday > daysInMonth
		|| myFirstProductionDay <= 0 || myFirstProductionDay > daysInMonth) {
		return false;
	}

	int interval = daysInMonth / productionDays;
	int remainder = daysInMonth % productionDays;

	for (int i = 0; i < productionDays; ++i) {
		int day = myFirstProductionDay + i * interval + min(i, remainder);
		if (day > daysInMonth) {
			break;
		}
		if (day == Ntoday) {
			return true;
		}
	}

	return false;
}

bool CProducer::isProductionDay(int workDayN) const
{
	int WorkDaysPerMonth = getInputParameter("WorkDaysPerMonth"); // 20 labor days
	int Nproduction_days = getInputParameter("Nproduction_days"); // 4 once a week
	int myFirstProductionDay = getmyFirstProductionDay();

	if (Nproduction_days <= 0 || WorkDaysPerMonth <= 0
		|| myFirstProductionDay < 0 || myFirstProductionDay >= WorkDaysPerMonth)
		return false;

	int Ntoday = workDayN % WorkDaysPerMonth;
	int interval = WorkDaysPerMonth / Nproduction_days;
	int remainder = WorkDaysPerMonth % Nproduction_days;

	for (int i = 0; i < Nproduction_days; ++i) {
		int day = myFirstProductionDay + i * interval + min(i, remainder);
		if (day > WorkDaysPerMonth) {
			break;
		}
		if (day == Ntoday) {
			return true;
		}
	}

	return false;
}

/////////////////////////   monthActivity   ////////////////////////////////

void CProducer::monthInitialize()
{
	if (getSAM().getSAMGrossOutput_mu(getAgentType()) == 0)
		return;

	CAgent::monthInitialize();

	IncomePrevMonth() = IncomeCurr();
	IncomeCurr() = 0;
	GoodType productType = getAgentType();

	NetTaxesPrevMonth() = NetTaxesCurr();
	NetTaxesCurr() = 0;

	ConsumptionBudget() = 0;
	timeWorked() = 0;
	SharesToTrade() = CShare(getFID(), 0);
};

void CProducer::monthActivity()
{
	if (getSAM().getSAMGrossOutput_mu(getAgentType()) == 0)
		return;

	GoodType productType = getAgentType();

	// Setup diagnostic logging if needed
	bool isTargetProducer = IsTargetProducerForDiagnostics();

	LogDiagnostic(isTargetProducer, "CASH_FLOW_START");

	if (currMonth() >= 9999)
		DEPData().CheckAccountingBalance();

	// Phase 1: Initial setup and capitalization
	InitializeProducerCash();
	ApplyCapitalDepreciation();

	// Phase 2: Planning and procurement
	GoodQtty buyGoodsBudget = PlanProductionAndProcurement();

	LogDiagnostic(isTargetProducer, "CASH_FLOW_BUDGET", buyGoodsBudget);

	// Phase 3: Procurement execution
	double cashBeforeBuyGoods = getCash();
	double balanceBeforeBuyGoods = getmyBankBalance();

	ExecuteProcurement();

	LogDiagnostic(isTargetProducer, "CASH_FLOW_AFTER_BUY",
		cashBeforeBuyGoods, balanceBeforeBuyGoods);

	// Phase 4: Production
	double cashBeforeProduction = getCash();
	double balanceBeforeProduction = getmyBankBalance();

	GoodQtty producedUnits = ExecuteProduction();

	LogDiagnostic(isTargetProducer, "CASH_FLOW_PRODUCTION",
		producedUnits, cashBeforeProduction, balanceBeforeProduction);

	// Phase 5: Fixed capital investment
	ExecuteFixedCapitalInvestment();

	// Phase 6: Financial management
	double cashBeforeLoanManagement = getCash();
	double balanceBeforeLoanManagement = getmyBankBalance();

	ManageFinancing(producedUnits, isTargetProducer);

	LogDiagnostic(isTargetProducer, "CREDIT_MANAGEMENT_RESULT",
		cashBeforeLoanManagement, balanceBeforeLoanManagement);

	LogDiagnostic(isTargetProducer, "CASH_FLOW_END",
		cashBeforeBuyGoods, balanceBeforeBuyGoods);

	// Phase 7: Period cleanup
	FinalizeMonthActivity();
}

bool CProducer::IsTargetProducerForDiagnostics() const
{
	long targetProducerID = getInputParameter("PlotProducerID");
	return (targetProducerID == AllAgentsID || targetProducerID == getID());
}

void CProducer::LogDiagnostic(bool isTargetProducer, const string& tag)
{
	if (!isTargetProducer || currMonth() < getInputParameter("DebugFromMonth"))
		return;

	GoodType productType = getAgentType();
	LogFile() << "\n[" << tag << "] Month:" << currMonth()
		<< " ID:" << getID()
		<< " Cash:" << getCash()
		<< " BankBalance:" << getmyBankBalance()
		<< " Wealth:" << getWealth()
		<< " FixCapital:" << getmyFixCapital()
		<< " LeftToSell:" << getToSell(productType);
	LogFile().flush();
}

void CProducer::LogDiagnostic(bool isTargetProducer, const string& tag,
	GoodQtty buyGoodsBudget)
{
	if (!isTargetProducer || currMonth() < getInputParameter("DebugFromMonth"))
		return;

	LogFile() << "\n[" << tag << "] Month:" << currMonth()
		<< " ID:" << getID()
		<< " BuyGoodsBudget:" << buyGoodsBudget
		<< " ConsumptionBudget:" << getConsumptionBudget()
		<< " ToBeProduced:" << getToBeProduced()
		<< " AvgProduction:" << getAvgProduction()
		<< " Cash:" << getCash();
	LogFile().flush();
}

void CProducer::LogDiagnostic(bool isTargetProducer, const string& tag,
	double cashBefore, double balanceBefore)
{
	if (!isTargetProducer || currMonth() < getInputParameter("DebugFromMonth"))
		return;

	if (tag == "CASH_FLOW_AFTER_BUY")
	{
		LogFile() << "\n[" << tag << "] Month:" << currMonth()
			<< " ID:" << getID()
			<< " CashSpent:" << (cashBefore - getCash())
			<< " CreditUsed:" << (balanceBefore - getmyBankBalance())
			<< " Cash:" << getCash()
			<< " BankBalance:" << getmyBankBalance()
			<< " WorkTimeToBuy:" << WorkTimeToBuy()
			<< " Employees:" << Employees().size();
	}
	else if (tag == "CREDIT_MANAGEMENT_RESULT")
	{
		GoodType productType = getAgentType();
		double avgProduction = getAvgProduction();
		double targetCreditLine = avgProduction * getmyPriceOf(productType);

		LogFile() << "\n[" << tag << "] Month:" << currMonth()
			<< " ID:" << getID()
			<< " CashChange:" << (getCash() - cashBefore)
			<< " BalanceChange:" << (getmyBankBalance() - balanceBefore)
			<< " FinalCash:" << getCash()
			<< " FinalBalance:" << getmyBankBalance()
			<< " TargetCredit:" << targetCreditLine;
	}
	else if (tag == "CASH_FLOW_END")
	{
		LogFile() << "\n[" << tag << "] Month:" << currMonth()
			<< " ID:" << getID()
			<< " FinalCash:" << getCash()
			<< " FinalBalance:" << getmyBankBalance()
			<< " FinalWealth:" << getWealth()
			<< " NetCashChange:" << (getCash() - cashBefore)
			<< " NetBalanceChange:" << (getmyBankBalance() - balanceBefore);
	}

	LogFile().flush();
}

void CProducer::LogDiagnostic(bool isTargetProducer, const string& tag,
	GoodQtty producedUnits, double cashBefore, double balanceBefore)
{
	if (!isTargetProducer || currMonth() < getInputParameter("DebugFromMonth"))
		return;

	GoodType productType = getAgentType();
	double productionCost = producedUnits * getmyPriceOf(productType);

	LogFile() << "\n[" << tag << "] Month:" << currMonth()
		<< " ID:" << getID()
		<< " ProducedUnits:" << producedUnits
		<< " ProductionCost:" << productionCost
		<< " ProductionPrice:" << getmyPriceOf(productType)
		<< " CashChange:" << (getCash() - cashBefore)
		<< " CreditChange:" << (getmyBankBalance() - balanceBefore)
		<< " Cash:" << getCash()
		<< " BankBalance:" << getmyBankBalance();
	LogFile().flush();
}

void CProducer::InitializeProducerCash()
{
	// Producer initCash is taken from owner
	if (getCash() == 0 && getmyBankAccountStatus()._pAgent == nullptr)
	{
		GoodQtty initCash = 1.0 * max<GoodQtty>(getmySalary_mu(),
			getWorld().getFinancialMarket().getInitShareValue());
		auto& owner = *Owners().begin()->second;

		if (owner.Cash() > 2. * initCash)
		{
			owner.Cash() -= initCash;
			Cash() += initCash;
		}
		else
		{
			bool done = GetCashFromBank(initCash);
			if (!done)
				CWorld::ERRORmsg(getAgentName() + " cannot get initCash from Bank", true);
		}

		GoodQtty nShares = ceil(initCash / getWorld().getFinancialMarket().getInitShareValue());
		myShares(getFID()) += -nShares;
		SharesToTrade() = CShare(getFID(), 0);

		owner.myShares(getFID()) += nShares;

		if (currMonth() >= 9999)
			DEPData().CheckAccountingBalance();
	}
}

/*
void CProducer::InitializeProducerCash()
{
	// Check if shares have already been initialized for this producer
	// myShares(getFID()) will be 0 (or positive) if uninitialized, negative if initialized
	bool sharesAlreadyInitialized = (myShares(getFID()) < 0);
	
	if (sharesAlreadyInitialized)
		return; // Already fully initialized
	
	if (Owners().empty())
		return; // No owner to assign shares to
		
	auto& owner = *Owners().begin()->second;
	
	// Determine what cash value to use for share calculation
	// If cash was already transferred (from startup capital), use that
	// Otherwise, initialize cash first
	GoodQtty cashForShares = getCash();
	
	if (cashForShares == 0 && getmyBankAccountStatus()._pAgent == nullptr)
	{
		// No cash yet - transfer from owner or get from bank
		GoodQtty initCash = 1.0 * max<GoodQtty>(getmySalary_mu(),
			getWorld().getFinancialMarket().getInitShareValue());

		if (owner.Cash() > 2. * initCash)
		{
			owner.Cash() -= initCash;
			Cash() += initCash;
			cashForShares = initCash;
		}
		else
		{
			bool done = GetCashFromBank(initCash);
			if (!done)
				CWorld::ERRORmsg(getAgentName() + " cannot get initCash from Bank", true);
			cashForShares = initCash;
		}
	}

	// Always initialize shares based on current cash (whether pre-transferred or just obtained)
	if (cashForShares > 0)
	{
		GoodQtty nShares = ceil(cashForShares / getWorld().getFinancialMarket().getInitShareValue());
		myShares(getFID()) += -nShares;  // Producer's liability (negative shares = shares outstanding)
		SharesToTrade() = CShare(getFID(), 0);

		owner.myShares(getFID()) += nShares;  // Owner's asset

		if (currMonth() >= 9999)
			DEPData().CheckAccountingBalance();
	}
}
*/

void CProducer::ApplyCapitalDepreciation()
{
	GoodType productType = getAgentType();
	double deprecRatePerMonth = getDEPData().getDepreciationRate().at(productType) / 12.0;
	myFixCapital() = myFixCapital() - myFixCapital() * deprecRatePerMonth;
}

GoodQtty CProducer::PlanProductionAndProcurement()
{
	GoodQtty buyGoodsBudget = MakeListOfGoodsToBuy();
	ConsumptionBudget() = 3 * buyGoodsBudget;
	return buyGoodsBudget;
}

bool CProducer::TryToHireNeighbor(CWorker& neighborIndiv)
{
	// Early rejection conditions
	if (WorkTimeToBuy() <= 0 ||
		neighborIndiv.getpEmployer() != nullptr ||
		neighborIndiv.getmyAvailableTime() <= 0)
		return false;

	// Price negotiation
	double priceAgreed = 0.0;

	// Both agree if Employer offers same or higher price than worker's asking price
	if (getmyPriceOfSalary() >= neighborIndiv.getmyPriceOfSalary())
		priceAgreed = getmyPriceOfSalary();

	// Handle hiring result
	if (priceAgreed > 0) {
		WorkTimeToBuy() = max<double>(0, WorkTimeToBuy() - neighborIndiv.getmyAvailableTime());
		neighborIndiv.pEmployer() = this;
		Employees().push_back(&neighborIndiv);
	}

	// Update SALARY based on negotiation outcome
	if (isPreCalibration()) {
		// Phase 1: Frozen salaries during pre-calibration
		return (priceAgreed > 0);
	}

	// Phase 2 & 3: Market-driven salary adjustments
	double priceAdaptFactor = CAgent::getPriceAdaptFactor();

	// Calculate MONTHLY inflation adjustment from year-over-year rate
	double monthlyInflationMultiplier = 1.0;

	if (!isPreCalibration()) {
		// Get the CPI values to calculate actual year-over-year inflation
		const auto& cpiVector = getDEPData().getCPItracker().getCPI();

		// Need at least 13 months of data for year-over-year comparison
		if (cpiVector.size() >= 13) {
			double currentCPI = cpiVector.back();
			double yearAgoCPI = cpiVector[cpiVector.size() - 13]; // 12 months ago

			if (yearAgoCPI > 0) {
				// Calculate actual year-over-year inflation rate
				double yearOverYearInflationRate = (currentCPI - yearAgoCPI) / yearAgoCPI;

				// Convert annual inflation rate to monthly: (1 + r_annual)^(1/12) - 1
				double monthlyInflationRate = pow(1.0 + yearOverYearInflationRate, 1.0 / 12.0) - 1.0;

				// Apply the CPItoSalaryFactor to the monthly rate
				double CPItoSalaryFactor = getInputParameter("CPItoSalaryFactor");
				double adjustedMonthlyRate = monthlyInflationRate * CPItoSalaryFactor;

				// Convert back to multiplier
				monthlyInflationMultiplier = 1.0 + adjustedMonthlyRate;
			}
		}
	}

	if (priceAgreed > 0) {
		// SUCCESSFUL HIRE: Worker increases asking price
		double marketAdjusted = neighborIndiv.getmyPriceOfSalary() * priceAdaptFactor;

		// Apply inflation adjustment additively to the market adjustment
		double inflationAdjustment = 0.0;
		if (monthlyInflationMultiplier > 1.0) {
			inflationAdjustment = neighborIndiv.getmyPriceOfSalary() * (monthlyInflationMultiplier - 1.0);
		}

		double newSalary = marketAdjusted + inflationAdjustment;

		neighborIndiv.setmyPriceOfSalary(newSalary);

		// SUCCESSFUL HIRE: Employer slightly reduces offer
		double employerMarketAdjusted = getmyPriceOfSalary() / priceAdaptFactor;

		// Apply same inflation adjustment
		double employerInflationAdjustment = 0.0;
		if (monthlyInflationMultiplier > 1.0) {
			employerInflationAdjustment = getmyPriceOfSalary() * (monthlyInflationMultiplier - 1.0);
		}

		double employerNewSalary = employerMarketAdjusted + employerInflationAdjustment;

		setmyPriceOfSalary(employerNewSalary);

		return true;
	}
	else {
		// FAILED HIRE: Worker lowers asking price
		double marketAdjusted = neighborIndiv.getmyPriceOfSalary() / priceAdaptFactor;

		// Apply inflation adjustment additively
		double inflationAdjustment = 0.0;
		if (monthlyInflationMultiplier > 1.0) {
			inflationAdjustment = neighborIndiv.getmyPriceOfSalary() * (monthlyInflationMultiplier - 1.0);
		}

		double newSalary = marketAdjusted + inflationAdjustment;

		neighborIndiv.setmyPriceOfSalary(newSalary);

		// FAILED HIRE: Employer increases offered salary
		double employerMarketAdjusted = getmyPriceOfSalary() * priceAdaptFactor;

		// Apply inflation adjustment
		double employerInflationAdjustment = 0.0;
		if (monthlyInflationMultiplier > 1.0) {
			employerInflationAdjustment = getmyPriceOfSalary() * (monthlyInflationMultiplier - 1.0);
		}

		double employerNewSalary = employerMarketAdjusted + employerInflationAdjustment;

		setmyPriceOfSalary(employerNewSalary);

		return false;
	}
}

void CProducer::ExecuteProcurement()
{
	BuyGoods();
	BuyGoods(); // Two buy rounds for ExtSect and Gov
}

GoodQtty CProducer::ExecuteProduction()
{
	GoodQtty producedUnits = RunProducer();

	if (producedUnits > 0)
		LastUsedMonth() = currMonth();

	GoodType productType = getAgentType();
	mySupply() = getToSell(productType);
	DEPData().TotSupply()[productType] += getToSell(getAgentType());

	return producedUnits;
}

void CProducer::ExecuteFixedCapitalInvestment()
{
	if (getmyFixCapToBuy() <= 0)
		return;

	GoodType productType = getAgentType();
	vector<double> GFCFfractionOf = getSAM().getGFCFfractions();
	double toTotalPopulationYear = getDEPData().toTotalPopulationYear();
	double FixCapIncrease = getmyFixCapToBuy();

	// Determine actual achievable fixed capital increase
	CTypeDoubleMap maxFixCapfromGoodsIhave;
	CalculateMaxFixedCapitalFromInventory(FixCapIncrease, GFCFfractionOf,
		maxFixCapfromGoodsIhave);

	// Convert goods to fixed capital
	if (FixCapIncrease > 0)
		ConvertGoodsToFixedCapital(FixCapIncrease, GFCFfractionOf);
}

void CProducer::CalculateMaxFixedCapitalFromInventory(
	double& FixCapIncrease,
	vector<double>& GFCFfractionOf,
	CTypeDoubleMap& maxFixCapfromGoodsIhave)
{
	for (GoodType gType = 0; gType < getSAM().getnPProducerTypes(); ++gType)
	{
		if (getSAM().GFCFfractionOf(gType) > 0)
		{
			maxFixCapfromGoodsIhave[gType] = getGoodsIhave(gType) / getSAM().GFCFfractionOf(gType);

			if (maxFixCapfromGoodsIhave[gType] < FixCapIncrease)
			{
				if (maxFixCapfromGoodsIhave[gType] > 0)
					FixCapIncrease = maxFixCapfromGoodsIhave[gType];
				else
				{
					if (getRandom01() < getSAM().GFCFfractionOf(gType))
						FixCapIncrease = 0;
					else
						GFCFfractionOf[gType] = 0;
				}
			}
		}
	}
}

void CProducer::ConvertGoodsToFixedCapital(
	double FixCapIncrease,
	const vector<double>& GFCFfractionOf)
{
	GoodType productType = getAgentType();

	for (GoodType gType = 0; gType < getSAM().getnPProducerTypes(); ++gType)
	{
		double gFixCapcomponent = FixCapIncrease * GFCFfractionOf[gType];
		auto toTotalPopulationYear = 12. * ((double)getSAM().getActive()
			* (1.0 - getDEPData().getUnemployment()) / getWorld().getNWorkers());
//			* (1.0 - getDEPData().getAvgUnemployment()) / getWorld().getNWorkers());
		GoodsIhave()[gType] = max<double>(0, getGoodsIhave(gType) - gFixCapcomponent);
		myFixCapToBuy() = max<double>((GoodQtty)0, myFixCapToBuy() - gFixCapcomponent);

		myFixCapital() += gFixCapcomponent;
		purchasedFixCapital() += gFixCapcomponent;

		if (currMonth() > getInputParameter("AssistedProductionUpto"))
		{
			SAM().SAMmonthAt(gType, getAgentType()) -= gFixCapcomponent * toTotalPopulationYear;

			// Update both GDP measures for the reclassification from IC to GFCF
			double gfcfValue = gFixCapcomponent * getmyPriceOf(gType);

			// Expenditure approach: add investment
			DEPData().GDPnominal() += gfcfValue * toTotalPopulationYear;
			DEPData().TotalUnits() += gFixCapcomponent * toTotalPopulationYear;
			DEPData().GDPtracker().addToGDPComponent(gType, gFixCapcomponent * toTotalPopulationYear);

			// Value Added approach: IC reduction increases VA
			//DEPData().GDPVAnominal() += gfcfValue * toTotalPopulationYear;
		}
	}
}

void CProducer::ManageFinancing(GoodQtty producedUnits, bool isTargetProducer)
{
	auto balance = getmyBankBalance();

	if (currMonth() > getInputParameter("AssistedProductionUpto"))
	{
		ManageOperationalFinancing(producedUnits, balance, isTargetProducer);
	}
	else
	{
		ManageBootstrapFinancing(producedUnits, balance, isTargetProducer);
	}
}

void CProducer::ManageOperationalFinancing(
	GoodQtty producedUnits,
	double balance,
	bool isTargetProducer)
{
	GoodType productType = getAgentType();

	// Calculate target credit line based on production scale
	double avgProduction = getAvgProduction();
	double targetCreditLine = avgProduction * getmyPriceOf(productType);

	if (targetCreditLine < getmySalary_mu())
		targetCreditLine = max(getmySalary_mu(), producedUnits * getmyPriceOf(productType));

	// Desired operating cash: 2x average production (not 0.1x target credit!)
	double desiredOperatingCash = 2.0 * avgProduction * getmyPriceOf(productType);
	if (desiredOperatingCash < getmySalary_mu())
		desiredOperatingCash = getmySalary_mu();

	LogCreditManagementCalculation(isTargetProducer, avgProduction,
		targetCreditLine, balance, desiredOperatingCash);

	// Execute financing strategy
	if (getCash() > desiredOperatingCash * 1.2)  // More than 20% excess
	{
		ManageExcessCash(balance, targetCreditLine, desiredOperatingCash);
	}
	else if (balance < 0 && -balance < targetCreditLine * 0.8)
	{
		EstablishCreditLine(targetCreditLine, isTargetProducer);
	}
	else if (getCash() < desiredOperatingCash * 0.8 && balance >= 0)  // Cash too low
	{
		EstablishCreditLine(targetCreditLine, isTargetProducer);
	}
}

void CProducer::LogCreditManagementCalculation(
	bool isTargetProducer,
	double avgProduction,
	double targetCreditLine,
	double balance,
	double minOperatingCash)
{
	if (!isTargetProducer || currMonth() < getInputParameter("DebugFromMonth"))
		return;

	LogFile() << "\n[CREDIT_MANAGEMENT_CALC] Month:" << currMonth()
		<< " ID:" << getID()
		<< " AvgProduction:" << avgProduction
		<< " TargetCreditLine:" << targetCreditLine
		<< " CurrentBalance:" << balance
		<< " MinOperatingCash:" << minOperatingCash
		<< " Cash:" << getCash();
	LogFile().flush();
}

void CProducer::ManageExcessCash(double balance, double targetCreditLine, double minOperatingCash)
{
	GoodType productType = getAgentType();
	double avgProduction = getAvgProduction();
	bool postCalibration = (currMonth() > getInputParameter("AssistedProductionUpto"));

	// Desired operating cash: 2x average production value
	double desiredOperatingCash = 2.0 * avgProduction * getmyPriceOf(productType);
	if (desiredOperatingCash < getmySalary_mu())
		desiredOperatingCash = getmySalary_mu();

	// Calculate excess cash beyond desired operating level
	GoodQtty excessCash = getCash() - desiredOperatingCash;

	if (balance >= 0) {
		// Has deposits -> convert to credit line structure
		if (postCalibration) {
			// Post-calibration: establish credit line and distribute excess
			GoodQtty neededCredit = targetCreditLine;
			bool granted = GetCashFromBank(neededCredit);

			if (granted) {
				// Recalculate excess after establishing credit
				excessCash = getCash() - desiredOperatingCash;

				// Distribute excess to owners, keeping desired operating cash
				if (excessCash > desiredOperatingCash * 0.1) {  // Only if meaningful excess
					PayDividends(excessCash * 0.9);  // Keep 10% safety buffer
				}
			}
		}
		else {
			// During calibration: just convert deposits to credit line
			GoodQtty withdrawAmount = targetCreditLine + balance;
			GetCashFromBank(withdrawAmount);
		}
	}
	else if (-balance > targetCreditLine * 1.5) {
		// Excessive credit -> repay excess to target level
		GoodQtty excessCredit = -balance - targetCreditLine;
		GoodQtty availableForRepayment = getCash() - desiredOperatingCash;
		GoodQtty repayment = min(availableForRepayment, excessCredit);

		if (repayment > targetCreditLine * 0.05 && repayment > 0) {
			pUsedBank()->ClientCashOperation(this, repayment);
		}
	}
	else if (-balance < targetCreditLine * 0.5) {
		// Credit line too small -> establish proper working capital
		GoodQtty neededCredit = targetCreditLine * 0.7 - (-balance);
		if (neededCredit > 0) {
			GetCashFromBank(neededCredit);
		}

		// After establishing credit, check for excess cash
		if (postCalibration) {
			excessCash = getCash() - desiredOperatingCash;
			if (excessCash > desiredOperatingCash * 0.1) {
				PayDividends(excessCash * 0.9);
			}
		}
	}
	else {
		// Credit line is adequate - just manage excess cash
		if (postCalibration && excessCash > desiredOperatingCash * 0.1) {
			PayDividends(excessCash * 0.9);
		}
	}
}

void CProducer::EstablishCreditLine(double targetCreditLine, bool isTargetProducer)
{
	GoodQtty neededCredit = targetCreditLine * 0.9 - (-getmyBankBalance());
	if (neededCredit > 0)
	{
		bool granted = GetCashFromBank(neededCredit);
		if (!granted && isTargetProducer)
		{
			LogFile() << "\n[CREDIT_DENIED] Month:" << currMonth()
				<< " ID:" << getID()
				<< " RequestedCredit:" << neededCredit
				<< " Wealth:" << getWealth();
			LogFile().flush();
		}
	}
}

void CProducer::ManageBootstrapFinancing(
	GoodQtty producedUnits,
	double balance,
	bool isTargetProducer)
{
	GoodType productType = getAgentType();

	// During assisted production period: original logic for bootstrap stability
	if (balance < 0)
		pUsedBank()->ClientCashOperation(this, min(-balance, Cash()));

	double productionExpenses = producedUnits * getmyPriceOf(productType);
	double surplusDividends = max(0, getCash() - 2.0 * productionExpenses);

	if (isTargetProducer && currMonth() >= getInputParameter("DebugFromMonth"))
	{
		LogFile() << "\n[BOOTSTRAP_DIVIDENDS] Month:" << currMonth()
			<< " ID:" << getID()
			<< " ProductionExpenses:" << productionExpenses
			<< " SurplusDividends:" << surplusDividends
			<< " Cash:" << getCash();
		LogFile().flush();
	}

	PayDividends(surplusDividends);
}

void CProducer::FinalizeMonthActivity()
{
	IncomePrevMonth() = IncomeCurr();
	IncomeCurr() = 0;

	NetTaxesPrevMonth() = NetTaxesCurr();
	NetTaxesCurr() = 0;

	currIC_mu() = 0;
	currCompEmployees_mu() = 0;
	currNetTaxes_mu() = 0;
	currDepositInterests_mu() = 0;
	currLoanPaymentsAndInterests_mu() = 0;

	if (DebugLevel() > 1 && currMonth() >= getInputParameter("DebugFromMonth"))
		assert(DEPData().CheckAccountingBalance());
}

// ================   MakeListOfGoodsToBuy  ========================

GoodQtty CProducer::MakeListOfGoodsToBuy() // returns productionCost
{
	// -------------- update stockReference, make list to buy and estimate the purchaseBudget -------------

	WorkTimeToBuy() = 0;
	myFixCapToBuy() = 0;
	GoodsToBuy() -= GoodsToBuy(); // set to 0's

	updateStockReferenceAndToBeProduced();

	GoodQtty desiredProductUnits = getToBeProduced();
	if (desiredProductUnits == 0)
		return 0;

	// Query returns extraNeededInputGoods to prepare the _GoodsToBuy list

	GoodType productType = getAgentType();
	const CProductSpecs& productSpecs = CProducer::getProductSpecs(productType);
	GoodQtty producedUnits = 0;
	CTypeDoubleMap deltaInputGoods;
	CTypeDoubleMap extraNeededInputGoods;
	double extraNeededWorkTime = 0;
	double extraNeededK = 0;

	double productionCost = -1; // as a flag from MakeListOfGoodsToBuy instead of from RunProducer
	//  ----------------------------------------------
	double timeworked = Query(desiredProductUnits, producedUnits, productionCost,
		deltaInputGoods, extraNeededInputGoods, extraNeededWorkTime, extraNeededK);

	//  ----------------------------------------------

	// extra needed WorkTime

	if (extraNeededWorkTime < 0)
	{
		// dismiss (-)extraQtty employees
		int tobedismissed = floor(-extraNeededWorkTime);
		for (int n = 0; n < tobedismissed; ++n)
			ReleaseLastEmployee();
	}
	else
		WorkTimeToBuy() = extraNeededWorkTime;

	// extra needed FixCap

	myFixCapToBuy() = extraNeededK * getInputParameter("KtoFixCapitalFactor");

	productionCost += extraNeededK * (getInputParameter("KtoFixCapitalFactor") - 1.);

	//  ------------   make list of IC goods to buy   ----------------------------------

	for (auto& gPair : extraNeededInputGoods)
	{
		auto ICtype = gPair.first;

		double myICtoBuyOf = extraNeededInputGoods.at(ICtype);// best
		double myFixCapToBuyOf = myFixCapToBuy() * getSAM().GFCFfractionOf(ICtype);

		double toBuyOf = myICtoBuyOf + myFixCapToBuyOf;

		setGoodsToBuyOf(ICtype, toBuyOf);
	}

	return productionCost;
};

void CProducer::updateStockReferenceAndToBeProduced()
{
	GoodType productType = getAgentType();

	// 1. Update average values

	// Smooth transition from ChangeFractionAssisted to ChangeFraction
	// (same pattern as ApplyAssistedProduction to avoid glitches)
	double transitionStart = getInputParameter("AssistedProductionUpto");
	double transitionEnd = getDEPData().getFinishCalibrationAt();
	double ChangeFraction;
	if (currMonth() <= transitionStart) {
		ChangeFraction = getInputParameter("ChangeFractionAssisted");
	} else if (currMonth() >= transitionEnd) {
		ChangeFraction = getInputParameter("ChangeFraction");
	} else {
		double transitionProgress = (currMonth() - transitionStart) / (transitionEnd - transitionStart);
		ChangeFraction = getInputParameter("ChangeFractionAssisted")
			+ transitionProgress * (getInputParameter("ChangeFraction") - getInputParameter("ChangeFractionAssisted"));
	}

	avgmyDemand() = getavgmyDemand()
		+ ChangeFraction * (getmyDemand() - getavgmyDemand());

	LeftToSellHistory().updateHistory(getToSell(productType));
	double avgLeftToSell = updateAndReturnAvgLeftToSell();

	mySupplyHistory().updateHistory(mySupply()); // mySupply is updated in monthActivity
	double avgmySupply = updateAndReturnAvgmySupply();

	double prevProducedUnits = producedUnitsOf(productType); // producedUnitsOf set in RunProducer
	ProductionHistory().updateHistory(prevProducedUnits);
	double avgProduction = updateAndReturnAvgProduction();

	// 2. Decide desired Supply level (StockReference)

	double prevStockReference = StockReference();
	double newStockRef = ceil(getavgmyDemand() * getInputParameter("DesiredStockLevelFactor"));

	// BLE MODE: Adjust stock reference based on expected GDP growth
	// When UseBLE=1 and BLE is in ACTIVE phase, producers anticipate future demand
	// based on learned expectations from free-market dynamics
	if (getInputParameter("UseBLE") == 1.0) {
		const CBLEExpectations& ble = getWorld().getBLEExpectations();
		// Only apply BLE influence when in ACTIVE phase (after training completes)
		if (ble.canInfluenceAgents()) {
			// Get expected growth factor (converts log growth to level multiplier)
			double expectedGrowthFactor = ble.getExpectedGrowthFactor();

			// Blend backward-looking and forward-looking targets based on BLEWeight
			// BLEWeight=0: pure backward-looking (traditional DEPLOYERS)
			// BLEWeight=1: pure BLE (Poledna-style forward-looking)
			double bleWeight = getInputParameter("BLEWeight");

			double backwardTarget = newStockRef;
			double bleTarget = newStockRef * expectedGrowthFactor;

			newStockRef = (1.0 - bleWeight) * backwardTarget + bleWeight * bleTarget;
		}
	}

	double deltaStockReference = newStockRef - prevStockReference;

	double maxdeltaStockReference = StockReference() * ChangeFraction;
	deltaStockReference = min(maxdeltaStockReference,
		max(-maxdeltaStockReference, deltaStockReference));

	StockReference() = StockReference() + deltaStockReference;

	// Calculate goods to be produced

	ToBeProduced() = assistedQttiesOf(productType) + max(0.0, StockReference() - getToSell(productType));

	// INFLATION model, phase 1: increase price if product was almost sold out and there is high demand

	if (currMonth() > getInputParameter("StartCalibrationAt") &&
		getToSell(productType) < getInputParameter("LowUnsoldFraction") * prevStockReference
		&& deltaStockReference >= 0
		&& getRandom01() < getInputParameter("InflationProbability"))
		myMarkupFactor() *= getInputParameter("InflationFactor"); // myMarkupFactor is used in phase 2 (RunProducer)

	// Keep a copy of these values for next iteration
	prevToBeProduced() = ToBeProduced();
	prevavgmyDemand() = getavgmyDemand();
	prevmyDemand() = getmyDemand();
	myDemand() = 0;
}

// ================   Query  ========================

double CProducer::Query(GoodQtty& desiredProductUnits, GoodQtty& producedUnits,
	double& productionCost, CTypeDoubleMap& deltaInputGoods, CTypeDoubleMap& extraNeededInputGoods,
	double& extraNeededWorkTime, double& extraNeededK) const
{
	if (desiredProductUnits == 0)
		return 0;

	// Initialize query context
	GoodType productType = getAgentType();
	const CProductSpecs& productSpecs = CProducer::getProductSpecs(productType);
	const CGoods availableInputGoods = getGoodsIhave();

	deltaInputGoods.clear();
	extraNeededInputGoods.clear();

	// Calculate production constraints from labor and capital
	double usedWorkTime = 0;
	CTypeDoubleMap MaxFromGoodType;
	double MaxFromWorkTime = 0;
	double MaxFromFixCap = 0;

	// Use Leontief production function
	CalculateLeontiefConstraints(desiredProductUnits, productSpecs,
		usedWorkTime, extraNeededWorkTime, extraNeededK,
		MaxFromWorkTime, MaxFromFixCap);

	// Calculate constraints from intermediate goods
	CTypeDoubleMap NeededPerUnit = productSpecs.getNeededPerUnit();
	CalculateIntermediateGoodsConstraints(desiredProductUnits, availableInputGoods,
		extraNeededK, NeededPerUnit, MaxFromGoodType, extraNeededInputGoods);

	// Determine actual producible units given all constraints
	producedUnits = DetermineProducibleUnits(desiredProductUnits,
		MaxFromWorkTime, MaxFromFixCap, MaxFromGoodType, NeededPerUnit);

	// Calculate production cost and input consumption
	CalculateProductionCostAndInputs(producedUnits, desiredProductUnits, productSpecs,
		productionCost, NeededPerUnit, deltaInputGoods, productType);

	// Final work time calculation
	usedWorkTime = (producedUnits * productSpecs.getCompEmployees()) / getmySalary_mu();

	return usedWorkTime;
}

void CProducer::CalculateLeontiefConstraints(
	GoodQtty desiredProductUnits,
	const CProductSpecs& productSpecs,
	double& usedWorkTime,
	double& extraNeededWorkTime,
	double& extraNeededK,
	double& MaxFromWorkTime,
	double& MaxFromFixCap) const
{
	double currentK = getmyFixCapital() / getInputParameter("KtoFixCapitalFactor");

	// Labor constraints
	if (productSpecs.getCompEmployees() > 0)
	{
		double availableWorkTime = getavailableWorkTime();
		usedWorkTime = (desiredProductUnits * productSpecs.getCompEmployees()) / getmySalary_mu();
		MaxFromWorkTime = floor(availableWorkTime * getmySalary_mu() / productSpecs.getCompEmployees());
		extraNeededWorkTime = usedWorkTime - availableWorkTime;
	}
	else // No workers constraint
	{
		MaxFromWorkTime = desiredProductUnits;
		extraNeededWorkTime = 0;
	}

	// Fixed capital constraints
	if (getmyGrossOpSurplus() > 0)
	{
		MaxFromFixCap = floor(currentK / getmyGrossOpSurplus());
		double newK = desiredProductUnits * getmyGrossOpSurplus();
		extraNeededK = max<double>(0, newK - currentK);

		// Smooth transition from ChangeFractionAssisted to ChangeFraction
		double transitionStart = getInputParameter("AssistedProductionUpto");
		double transitionEnd = getDEPData().getFinishCalibrationAt();
		double ChangeFraction;
		if (currMonth() <= transitionStart) {
			ChangeFraction = getInputParameter("ChangeFractionAssisted");
		} else if (currMonth() >= transitionEnd) {
			ChangeFraction = getInputParameter("ChangeFraction");
		} else {
			double transitionProgress = (currMonth() - transitionStart) / (transitionEnd - transitionStart);
			ChangeFraction = getInputParameter("ChangeFractionAssisted")
				+ transitionProgress * (getInputParameter("ChangeFraction") - getInputParameter("ChangeFractionAssisted"));
		}

		// Limit capital expansion rate to realistic levels
		// In real economies, annual GFCF/Capital is typically 5-15%
		// Only apply the limit AFTER assisted production phase
		if (currentK == 0) // initialisation
			extraNeededK = min(extraNeededK, getmySalary_mu() * ChangeFraction);
		else if (currMonth() > getInputParameter("AssistedProductionUpto"))
			extraNeededK = min(extraNeededK, currentK * getInputParameter("KchangeFraction"));
		else
			extraNeededK = min(extraNeededK, currentK); // Original behavior during assisted phase
	}
	else // No FixCapital constraint
	{
		MaxFromFixCap = desiredProductUnits;
		extraNeededK = 0;
	}
}

void CProducer::CalculateIntermediateGoodsConstraints(
	GoodQtty desiredProductUnits,
	const CGoods& availableInputGoods,
	double extraNeededK,
	const CTypeDoubleMap& NeededPerUnit,
	CTypeDoubleMap& MaxFromGoodType,
	CTypeDoubleMap& extraNeededInputGoods) const
{
	for (const auto& ICpair : availableInputGoods)
	{
		GoodType ICtype = ICpair.first;

		// Reserve goods needed for fixed capital formation
		GoodQtty neededForGFCF = ceil(extraNeededK * getSAM().GFCFfractionOf(ICtype));
		GoodQtty availableOfICtype = max<GoodQtty>(0, availableInputGoods(ICtype) - neededForGFCF);

		// Calculate maximum production from this input
		GoodQtty maxFromThisICtype = desiredProductUnits;
		double neededPerUnit = NeededPerUnit.at(ICtype);

		if (neededPerUnit > 0)
			maxFromThisICtype = floor(availableOfICtype / neededPerUnit);

		// Calculate extra needed quantity
		if (maxFromThisICtype < desiredProductUnits)
			extraNeededInputGoods[ICtype] =
			max<double>(0., ceil(desiredProductUnits * neededPerUnit - availableOfICtype));

		// Track this constraint if it limits production
		if (neededPerUnit > 0)
			MaxFromGoodType[ICtype] = maxFromThisICtype;
	}
}

GoodQtty CProducer::DetermineProducibleUnits(
	GoodQtty desiredProductUnits,
	double MaxFromWorkTime,
	double MaxFromFixCap,
	const CTypeDoubleMap& MaxFromGoodType,
	CTypeDoubleMap& NeededPerUnit) const
{
	GoodQtty producedUnits = desiredProductUnits;

	// Apply labor constraint
	if (MaxFromWorkTime < producedUnits)
		producedUnits = MaxFromWorkTime;

	// Apply capital constraint
	if (MaxFromFixCap < producedUnits)
	{
		if (MaxFromFixCap > 0)
			producedUnits = MaxFromFixCap;
		else
		{
			double NeededKPerUnit = 1. * getmyGrossOpSurplus();
			if (getRandom01() < NeededKPerUnit)
				producedUnits = 0;
		}
	}

	// Apply intermediate goods constraints
	for (const auto& ICpair : MaxFromGoodType)
	{
		GoodType ICtype = ICpair.first;

		if (MaxFromGoodType.at(ICtype) < producedUnits)
		{
			if (MaxFromGoodType.at(ICtype) > 0)
				producedUnits = MaxFromGoodType.at(ICtype);
			else
			{
				if (getRandom01() < NeededPerUnit.at(ICtype))
					producedUnits = 0;
				else
					NeededPerUnit.at(ICtype) = 0; // using a local copy that can be modified
			}
		}
	}

	return producedUnits;
}

void CProducer::CalculateProductionCostAndInputs(
	GoodQtty producedUnits,
	GoodQtty desiredProductUnits,
	const CProductSpecs& productSpecs,
	double& productionCost,
	CTypeDoubleMap& NeededPerUnit,
	CTypeDoubleMap& deltaInputGoods,
	GoodType productType) const
{
	GoodQtty costUnits = max(producedUnits, desiredProductUnits);

	// Restore real NeededPerUnit if called from MakeListOfGoodsToBuy
	if (productionCost == -1)
		NeededPerUnit = productSpecs.getNeededPerUnit();

	// Initialize production cost with labor and capital
	productionCost = 0;
	productionCost += costUnits * productSpecs.getCompEmployees() * getmyPriceOfSalary();
	productionCost += costUnits * getmyGrossOpSurplus();

	// Add intermediate consumption costs
	for (const auto& ICpair : NeededPerUnit)
	{
		const GoodType ICtype = ICpair.first;
		double neededPerUnit = NeededPerUnit.at(ICtype);

		if (neededPerUnit == 0)
			continue;

		productionCost += costUnits * neededPerUnit * getmyPriceOf(ICtype);
		double neededOfType = producedUnits * neededPerUnit;
		deltaInputGoods[ICtype] = -neededOfType;
	}

	// Add production taxes
	AddProductionTaxesToCost(productionCost, costUnits, productType);

	assert(productionCost >= 0);
}

void CProducer::AddProductionTaxesToCost(
	double& productionCost,
	GoodQtty costUnits,
	GoodType productType) const
{
	if (getSAM().getAccountGroups().find("T") == getSAM().getAccountGroups().end())
		return;

	for (const auto& Tpair : getSAM().getAccountGroups().at("T"))
	{
		auto TRow = Tpair->accN();
		double taxPerUnit = ((double)getSAM().getRowCol(TRow, productType)
			/ CProducer::getProductSpecs(getAgentType()).getGrossOutput_mu());

		// Apply tax policy changes if in effect
		if (currMonth() >= getInputParameter("TaxChangeFrom")
			&& currMonth() <= getInputParameter("TaxChangeUpto")
			&& productType == getInputParameter("ProducerAccN")
			&& TRow == getInputParameter("TaxAccN"))
			taxPerUnit *= getInputParameter("TaxChangeFactor");

		productionCost += costUnits * taxPerUnit;
	}
}

// ================   RunProducer  ========================

GoodQtty CProducer::RunProducer()
{
	GoodQtty producedUnits = 0;
	GoodQtty desiredProductUnits = getToBeProduced();
	if (desiredProductUnits == 0)
		return producedUnits;

	GoodType productType = getAgentType();

	// Handle external sectors specially - they produce without constraints
	if (getSAM().IsExtSectType(productType))
	{
		ToSell(productType) += desiredProductUnits;
		ToBeProduced() = 0;
		return desiredProductUnits;
	}

	LeftToSell() = getToSell(productType);

	// Query production feasibility and calculate resource requirements
	double productionCost = 0;
	CTypeDoubleMap deltaInputGoods;
	CTypeDoubleMap extraNeededInputGoods;
	double extraNeededWorkTime = 0;
	double extraNeededK = 0;

	timeWorked() = Query(desiredProductUnits, producedUnits, productionCost,
		deltaInputGoods, extraNeededInputGoods, extraNeededWorkTime, extraNeededK);

	producedUnitsOf(productType) = producedUnits;

	DEPData().TotProduced()[productType] += producedUnits * 12. * getDEPData().getUpscaleSimulationFactor();
	DEPData().TotalProduction() += producedUnits * 12. * getDEPData().getUpscaleSimulationFactor();

	if (producedUnits == 0)
		return 0;

	// Secure financing for production
	if (!SecureProductionFinancing(productionCost))
		return 0;

	double finalProductionCost = 0;

	// Execute production activities
	ReleaseExcessEmployees();
	finalProductionCost += PayEmployeeSalaries(producedUnits, deltaInputGoods);
	ConsumeIntermediateInputs(deltaInputGoods);
	finalProductionCost += CalculateIntermediateConsumptionCost(deltaInputGoods);
	finalProductionCost += PayProductionTaxes(producedUnits);

	// Update pricing based on actual production costs
	UpdateProductionPricing(producedUnits, finalProductionCost, productType);

	// Handle assisted production for startup period
	GoodQtty finalProducedUnits = ApplyAssistedProduction(producedUnits, productType);

	ToSell(productType) += finalProducedUnits;

	return finalProducedUnits;
}

bool CProducer::SecureProductionFinancing(double productionCost)
{
	GoodQtty neededFromBank = max(0., 2 * productionCost - getCash());
	if (neededFromBank > 0)
	{
		bool granted = GetCashFromBank(neededFromBank);
		if (!granted)
			return false;
	}
	return true;
}

void CProducer::ReleaseExcessEmployees()
{
	// Dismiss surplus employees based on actual time worked vs available
	double totAvailableTime = 0;
	double timeToBePayed = gettimeWorked();

	for (auto& pWorker : Employees())
	{
		double thisWorkerTime = min(timeToBePayed, pWorker->getmyAvailableTime());
		timeToBePayed -= thisWorkerTime;
		totAvailableTime += thisWorkerTime;

		// Dismiss surplus employees if we've covered all needed work time
		if (totAvailableTime >= gettimeWorked())
		{
			while (true)
			{
				if (pWorker != Employees().back())
					ReleaseLastEmployee();
				else
					break;
			}
		}

		if (pWorker == Employees().back())
			break;
	}
}

double CProducer::PayEmployeeSalaries(GoodQtty producedUnits, const CTypeDoubleMap& deltaInputGoods)
{
	GoodType productType = getAgentType();
	const CProductSpecs& productSpecs = CProducer::getProductSpecs(productType);
	double compEmployees = producedUnits * productSpecs.getCompEmployees();

	// Calculate total available work time for salary distribution
	double totAvailableTime = 0;
	double timeToBePayed = gettimeWorked();
	for (auto& pWorker : Employees())
	{
		double thisWorkerTime = min(timeToBePayed, pWorker->getmyAvailableTime());
		timeToBePayed -= thisWorkerTime;
		totAvailableTime += thisWorkerTime;
		if (pWorker == Employees().back())
			break;
	}

	// Pay each employee proportionally to their contribution
	string txt = "";
	auto firstLrow = getSAM().getAccountGroups().at("L").at(0)->accN();
	auto perYear = 12. * ((double)getSAM().getActive()
		* (1.0 - getDEPData().getUnemployment()) / getWorld().getNWorkers());
	//			* (1.0 - getDEPData().getAvgUnemployment()) / getWorld().getNWorkers());

	double salariesPayed = 0;
	timeToBePayed = gettimeWorked();

	if (timeToBePayed > 0)
	{
		for (auto& pWorker : Employees())
		{
			double thisWorkerTime = min(timeToBePayed, pWorker->getmyAvailableTime());
			double mySalary = compEmployees * thisWorkerTime / totAvailableTime;

			timeToBePayed -= thisWorkerTime;
			auto LgroupN = pWorker->getmyLgroupN();
			auto HgroupN = pWorker->getmyHgroupN();

			bool done = PayTo(pWorker, mySalary, txt);

			if (!done)
				CWorld::ERRORmsg("not enough money to pay salary", true);
			else
			{
				currCompEmployees_mu() += mySalary;
				DEPData().TotalSalaries() +=
					mySalary / (getWorld().getInitSalaryCalibFactor() * getSAM().getInitSalary());
				DEPData().TotalWorkedTime() += thisWorkerTime;
				pWorker->IncomeCurr() += mySalary;
				SAM().SAMmonth()[LgroupN][productType] += mySalary * perYear;
				SAM().SAMmonth()[HgroupN][LgroupN] += mySalary * perYear;
			}

			// Handle external sector salary payments
			PayExternalSectorSalaries(pWorker, firstLrow);

			salariesPayed += mySalary;
			pWorker->myAvailableTime() = max(0., pWorker->getmyAvailableTime() - thisWorkerTime);
			if (pWorker->myAvailableTime() > 0 && pWorker->myAvailableTime() < 1.0)
				pWorker->bPartTimeWorker() = true;

			// Release remaining surplus employees
			if (salariesPayed >= compEmployees)
			{
				while (true)
				{
					if (pWorker == Employees().back())
						break;
					else
						ReleaseLastEmployee();
				}
			}

			if (pWorker == Employees().back())
				break;
		}
	}

	// Release part-time employees so they can seek other employment
	for (int n = 0; n < Employees().size(); ++n)
	{
		auto pWorker = Employees().at(n);
		if (pWorker->getmyAvailableTime() > 0)
			ReleaseThisEmployee(*pWorker);
	}

	return salariesPayed;
}

void CProducer::PayExternalSectorSalaries(CWorker* pWorker, int firstLrow)
{
	auto myLix = pWorker->getmyLgroupN() - firstLrow;
	long ixSect = -1;
	for (auto& pairExtSect : getExtSectors())
	{
		++ixSect;
		auto qtty = getWorld().getExtSectorsToLabor().at(ixSect).at(myLix);
		if (qtty > 0)
		{
			CWorld::ERRORmsg("Is this correct?", true);
			string ExtSectconcept = "";

			bool done = pairExtSect.second->PayTo(pWorker, qtty, ExtSectconcept);
			if (!done)
				CWorld::ERRORmsg("not enough money to pay ExtSectSalary", true);

			pWorker->IncomeCurr() += qtty;
		}
	}
}

void CProducer::ConsumeIntermediateInputs(const CTypeDoubleMap& deltaInputGoods)
{
	for (const auto& pair : deltaInputGoods)
	{
		auto ICtype = pair.first;
		GoodsIhave()[ICtype] += deltaInputGoods.at(ICtype);
		assert(getGoodsIhave(ICtype) >= 0);
	}
}

double CProducer::CalculateIntermediateConsumptionCost(const CTypeDoubleMap& deltaInputGoods)
{
	double icCost = 0;
	for (const auto& pair : deltaInputGoods)
	{
		auto ICtype = pair.first;
		auto ICqtty = pair.second;
		double cost = -ICqtty * getmyPriceOf(ICtype);
		icCost += cost;
		currIC_mu() += cost;
	}
	return icCost;
}

double CProducer::PayProductionTaxes(GoodQtty producedUnits)
{
	GoodType productType = getAgentType();
	double TotalNetTax = 0;
	double toTotalPopulationYear = getDEPData().toTotalPopulationYear();

	if (getSAM().getAccountGroups().find("T") != getSAM().getAccountGroups().end())
	{
		for (const auto& Tpair : getSAM().getAccountGroups().at("T"))
		{
			auto TRow = Tpair->accN();
			double taxPerUnit = ((double)getSAM().getRowCol(TRow, productType)
				/ CProducer::getProductSpecs(getAgentType()).getGrossOutput_mu());

			// Apply tax policy changes if in effect
			if (currMonth() >= getInputParameter("TaxChangeFrom")
				&& currMonth() <= getInputParameter("TaxChangeUpto")
				&& productType == getInputParameter("ProducerAccN")
				&& TRow == getInputParameter("TaxAccN"))
				taxPerUnit *= getInputParameter("TaxChangeFactor");

			SAM().SAMmonth()[TRow][productType]
				+= taxPerUnit * producedUnits * toTotalPopulationYear;
			SAM().SAMmonth()[getSAM().getAccNofName("Government")][TRow]
				+= taxPerUnit * producedUnits * toTotalPopulationYear;

			TotalNetTax += taxPerUnit * producedUnits * getmyPriceOf(productType);
		}
	}

	if (TotalNetTax != 0)
	{
		string txt = "";
		bool done = PayTo(pGovernment(), TotalNetTax, txt);
		if (!done)
			CWorld::ERRORmsg("not enough money to pay TotalNetTax", true);

		NetTaxesCurr() += TotalNetTax;
		currNetTaxes_mu() += TotalNetTax;
	}

	auto perYear = 12. * ((double)getSAM().getActive()
		* (1.0 - getDEPData().getUnemployment()) / getWorld().getNWorkers());
	//			* (1.0 - getDEPData().getAvgUnemployment()) / getWorld().getNWorkers());

	return TotalNetTax;
}

void CProducer::PayDividends(GoodQtty dividends)
{
	if (dividends <= 0 || pOwners()->empty())
		return;

	// Calculate total shares to determine distribution
	GoodQtty totalShares = 0;
	for (const auto& ownerPair : *pOwners()) {
		totalShares += ownerPair.second->getmySharesOf(getFID());
	}

	if (totalShares == 0) {
		// No shares issued yet - give all to first owner
		auto& firstOwner = *pOwners()->begin()->second;
		bool done = PayTo(&firstOwner, dividends, "dividends");
		if (done) {
			firstOwner.IncomeCurr() += dividends;
		}
		return;
	}

	// Distribute proportionally to share ownership
	for (const auto& ownerPair : *pOwners()) {
		auto& owner = *ownerPair.second;
		GoodQtty ownerShares = owner.getmySharesOf(getFID());
		GoodQtty ownerDividend = (dividends * ownerShares) / totalShares;

		if (ownerDividend > 0) {
			bool done = PayTo(&owner, ownerDividend, "dividends");
			if (done) {
				owner.IncomeCurr() += ownerDividend;
			}
		}
	}
}

void CProducer::UpdateProductionPricing(GoodQtty producedUnits, double finalProductionCost, GoodType productType)
{
	if (currMonth() > getInputParameter("StartCalibrationAt") && producedUnits > 0)
	{
		productionPrice() = finalProductionCost / producedUnits;
		productionPrice() *= myMarkupFactor();
		setmyPriceOf(productType, max(productionPrice(), getmyPriceOf(productType)));

		if (productionPrice() > 14)
			CWorld::ERRORmsg("productionPrice() > 14", true);
	}

	if (currMonth() > getInputParameter("StartCalibrationAt") && producedUnits > 0)
		setmyPriceOf(productType, max(productionPrice(), getmyPriceOf(productType)));
}

GoodQtty CProducer::ApplyAssistedProduction(GoodQtty producedUnits, GoodType productType)
{
	assistedQtties().clear();
	double assistedUptoMonth = getInputParameter("AssistedProductionUpto");
	double assistedRange = assistedUptoMonth;
	GoodQtty delta = 0;

	if (currMonth() <= assistedUptoMonth)
	{
		delta = max<GoodQtty>(0, ToBeProduced() - producedUnits)
			* max<double>(0, assistedUptoMonth - currMonth()) / assistedRange;
		if (delta > 0)
		{
			producedUnits += delta;
			ToSell(productType) += delta;
			assistedQttiesOf(productType) += delta;
		}
	}

	return producedUnits;
}
