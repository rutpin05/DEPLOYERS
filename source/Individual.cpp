
//  Worker.cpp

//   DEPLOYERS v2

#include "./pch.h"

//======================  CWorker  ===================================

double CWorker::_PropToConsume;

CWorker::CWorker(AgentID id)
	: CAgent(id, (AgentType)WorkerType)
{
	initialize();
}
CWorker::~CWorker()
{
}

ofstream& operator<<(ofstream& ofstrm, const CWorker& indiv)
{
	ofstrm << "\n Indiv {";
	ofstrm << " ID " << (long)indiv.getID();

	operator<<(ofstrm, (CAgent&)indiv); // call base class

	ofstrm << endl << " bPartTimeWorker " << indiv.getbPartTimeWorker();
	ofstrm << endl << " myAvailableTime " << indiv.getmyAvailableTime();

	ofstrm << endl << " LgroupIndices {";
	for (const auto& nn : indiv.getLgroupIndices())
		ofstrm << " " << nn;
	ofstrm << " }";

	ofstrm << endl << " myLgroupN " << indiv._myLgroupN;

	ofstrm << endl << " HgroupIndices {";
	for (const auto& nn : indiv.getHgroupIndices())
		ofstrm << " " << nn;
	ofstrm << " }";

	ofstrm << endl << " myHgroupN " << indiv._myHgroupN;

	ofstrm << endl << " BondToSharesRatio " << indiv._BondToSharesRatio;
	ofstrm << " myBond ";
	ofstrm << indiv._myBond;

	ofstrm << " Employer ";
	if (indiv.getpEmployer() != nullptr)
		ofstrm << "" << indiv.getpEmployer()->getID();
	else
		ofstrm << "" << UndefAgentID;

	ofstrm << " OwnedBank ";
	if (indiv.getpOwnedBank() != nullptr)
		ofstrm << "" << indiv.getpOwnedBank()->getID();
	else
		ofstrm << "" << UndefAgentID;

	ofstrm << " }" << endl;

	return ofstrm;
}
ifstream& operator>>(ifstream& ifstrm, CWorker& indiv)
{
	string name;
	AgentID id;
	GoodType gType;

	operator>>(ifstrm, (CAgent&)indiv); // call base class

	ifstrm >> name >> indiv.bPartTimeWorker(); // " bPartTimeWorker "
	ifstrm >> name >> indiv.myAvailableTime(); // " myAvailableTime "

	ifstrm >> name >> name; // " LgroupIndices {"
	while (ifstrm >> name, name != "}")
	{
		gType = atoi(name.c_str());
		indiv.LgroupIndices().push_back(gType);
	}

	ifstrm >> name >> indiv._myLgroupN; // " myLgroupN "

	ifstrm >> name >> name; // " HgroupIndices {"
	while (ifstrm >> name, name != "}")
	{
		gType = atoi(name.c_str());
		indiv.HgroupIndices().push_back(gType);
	}

	ifstrm >> name >> indiv._myHgroupN; // " myHgroupN "

	ifstrm >> name >> indiv._BondToSharesRatio;
	ifstrm >> name;
	ifstrm >> indiv._myBond;

	// my employer
	ifstrm >> name >> id; // employers are allways Producers
	if (id >= 0)
		assert(indiv.pEmployer()->getID() == id);

	// my OwnedBank
	ifstrm >> name >> id; // will be assigned in CWorld::assignPointersToBanks

	return ifstrm;
}

void CWorker::initialize() // virtual
{
	CAgent::initialize(); // call base class

	myAvailableTime() = 1.0;
	bPartTimeWorker() = false;
	pLgroupIndices() = new vector<long>;
	pHgroupIndices() = new vector<long>;
	pEmployer() = nullptr;
	pOwnedBank() = nullptr;
	if (getInputParameter("MaxNBanks") > 0)
		pUsedBank() = pCentralBank();// initially
	else
		pUsedBank() = nullptr;

	gammaC() = getInputParameter("gammaC");

	monthInitialize();
}
bool CWorker::getHasWorkedCurrentMonth() const { return getmyAvailableTime() == 1.; };

GoodType CWorker::getmyLindex() const { return getLgroupIndices().at(SAM().nLaccounts()); }
void CWorker::assignHandLgroups()
{
	int firstHaccN = getSAM().getAccountGroups().at("H").at(0)->accN();
	int firstLaccN = getSAM().getAccountGroups().at("L").at(0)->accN();
	if (getSAM().nHaccounts() == 1)
		_myHgroupN = firstHaccN;

	if (SAM().nLaccounts() == 1)
		_myLgroupN = firstLaccN;

	if (getSAM().SAMname() == "SAMEXT90desag")
	{
		GoodType nHattribs = 4;
		long agric = 0, more65 = 1, rural = 2, quint = 3; // attributes

		GoodType nLattribs = 1;
		pLgroupIndices()->clear();
		pLgroupIndices()->resize(nLattribs, 0); // agric

		pHgroupIndices()->clear();
		pHgroupIndices()->resize(nHattribs, 0);

		HgroupIndices()[more65] = getRandom01() < 0.1 ? 1 : 0;
		HgroupIndices()[agric] = getRandom01() < 0.24 ? 1 : 0;
		HgroupIndices()[rural] = getRandom01() < 0.3 ? 1 : 0;

		HgroupIndices()[quint] = 1 + floor(5.0 * getRandom01());

		if (!HgroupIndices()[more65]) // Men65
		{
			if (HgroupIndices()[agric]) // Men65Agr
			{
				if (HgroupIndices()[quint] < 4)
					_myHgroupN = firstHaccN; // Men65AgrBaj
				else
					_myHgroupN = firstHaccN + 1; // Men65AgrAlt
			}
			else // Men65NoAgr
				_myHgroupN = firstHaccN + 1 + HgroupIndices()[quint]; // Men65NoAgr12345
		}
		else // Mas65
		{
			if (HgroupIndices()[rural]) // Mas65Rur
			{
				if (HgroupIndices()[quint] < 4)
					_myHgroupN = firstHaccN + 7; // Mas65RurBaj
				else
					_myHgroupN = firstHaccN + 8; // Mas65RurAlt
			}
			else // Mas65Urb
			{
				if (HgroupIndices()[quint] < 4)
					_myHgroupN = firstHaccN + 9; // Mas65UrbBaj
				else
					_myHgroupN = firstHaccN + 10; // Mas65UrbAlt
			}
		}

		bool agricWorker =
			getHgroupIndices().at(more65) == 0 && getHgroupIndices().at(agric) == 1
			|| getHgroupIndices().at(more65) == 1 && getHgroupIndices().at(rural) == 1;
		if (agricWorker)
		{
			LgroupIndices()[agric] = 1;
			_myLgroupN = getSAM().getAccountGroups().at("L").at(0)->accN();
		}
		else
		{
			LgroupIndices()[agric] = 0;
			_myLgroupN = getSAM().getAccountGroups().at("L").at(0)->accN();
		}
	}
	else if (getSAM().nLaccounts() != 1 || getSAM().nHaccounts() != 1)
		getWorld().ERRORmsg("error in CWorker::assignHandLgroups()", true);
}
GoodType CWorker::getmyHindex() const { return getHgroupIndices().at(SAM().nHaccounts()); }
double& CWorker::PropToConsume() { return CWorker::_PropToConsume; }
double CWorker::getPropToConsume() { return CWorker::_PropToConsume; }
CProducer*& CWorker::pEmployer()
{
	return _pEmployer;
}
CBank*& CWorker::pOwnedBank()
{
	return _pOwnedBank;
}
const CProducer* CWorker::getpEmployer() const { return _pEmployer; };
const CBank* CWorker::getpOwnedBank() const
{
	return _pOwnedBank;
}
double CWorker::getmyTotSharesValue() const
{
	double totSharesValue = 0;
	for (const auto& pair : getmyShares()) //M J
		totSharesValue += pair.second * getWorld().getpProducers()->at(pair.first._id)->getmyCurrShareVal();

	return totSharesValue;
}
GoodQtty CWorker::getGoodsToBuyOf(GoodType gType) { return GoodsToBuy()[gType]; };
GoodQtty CWorker::getWealth() const
{
	GoodQtty Wealth =
		getMoneyHoldings()
		+ getmyTotSharesValue()
		+ getmyTotBondsValue()
		+ getmyGFCF();

	return Wealth;
}

///////////////////////////////////////////////////////////////////

void CWorker::monthInitialize()
{
	CAgent::monthInitialize();

	// Labor market friction: in tight labor markets, workers may quit to seek better wages
	// This creates natural unemployment (frictional/structural) and prevents unemployment from hitting zero
	// Only applies after calibration starts and when unemployment is below NAIRU
	if (getpEmployer() != nullptr && !isPreCalibration()) {
		
		double unemploymentRate = getDEPData().getUnemployment();
		double nairu = getInputParameter("NAIRU");
		
		// Only apply when labor market is tight (unemployment below NAIRU)
		if (unemploymentRate < nairu) {
			double frictionSensitivity = getInputParameter("LaborFrictionSensitivity");
			
			// How tight is the labor market? 0 at NAIRU, 1 at zero unemployment
			double laborMarketTightness = (nairu - unemploymentRate) / nairu;
			
			// Quit probability increases as labor market tightens
			// Workers are more willing to quit when jobs are plentiful
			double quitProb = frictionSensitivity * laborMarketTightness * laborMarketTightness * 0.1; // Scale down
			quitProb = min(0.05, quitProb); // Cap at 5% monthly quit rate
			
			if (getRandom01() < quitProb) {
				// Worker quits to seek better opportunities
				// Raise wage expectations (bargaining power in tight market)
				double wageBoost = 1.0 + 0.1 * laborMarketTightness;
				setmyPriceOfSalary(getmyPriceOfSalary() * wageBoost);
				
				// Release from current employer
				pEmployer()->ReleaseThisEmployee(*this);
			}
		}
	}

	SharesToTrade().second = 0;
	myValueToBuyShares() = 0;
	myBondsValueToTrade() = 0;

	IncomePrevMonth() = IncomeCurr();
	IncomeCurr() = 0;

	NetTaxesPrevMonth() = NetTaxesCurr();
	NetTaxesCurr() = 0;

	myAvailableTime() = 1.0;
	
	// === COVID-19 Labor Confinement ===
	// During lockdown, workers in affected sectors can work fewer hours
	// This creates a SUPPLY shock - producers see reduced labor availability
	// and must respond naturally (Darwinian ABM - no intervention on producer logic)
	if (getWorld().getPandemicState().isPandemicActive() && 
		getWorld().getPandemicState().isInPandemicPeriod()) {
		// Get worker's employment sector (use employer's sector if employed)
		GoodType workerSector = UndefGoodType;
		if (getpEmployer() != nullptr) {
			workerSector = getpEmployer()->getAgentType();
		}
		if (workerSector != UndefGoodType) {
			double laborShock = getWorld().getPandemicState().getLaborShock(workerSector);
			if (laborShock < 1.0) {
				// Worker can only offer (laborShock * 100)% of their time
				myAvailableTime() = laborShock;
			}
		}
	}
	
	bPartTimeWorker() = false;
	RemoveServiceGoods(); // These are FINAL consumption except for Producers (Intermed.)
}
void CWorker::monthActivity() // virtual
{
	if (DebugLevel() > 2 && currMonth() >= getInputParameter("DebugFromMonth"))
	{
		assert(DEPData().CheckAccountingBalance());
		LogFile()
			<< "\n month " << currMonth()
			<< " Indiv_" << getID()
			<< " Cash " << getCash();
		LogFile().flush();
	}

	GoodQtty buyGoodsBudget = MakeListOfGoodsToBuy();
	if (buyGoodsBudget > 0)
	{
		GoodQtty additionalNeededFunding = 2 * buyGoodsBudget - (getCash() + getmyBankBalance());

		// Indivs are not allowed to apply for a credit (yet), try to sell myShares to get cash

		if (additionalNeededFunding > 0
			&& getInputParameter("UseSecondaryMarket")) // money may not be available within this time month
		{
			SharesToTrade() = 0;
			myValueToBuyShares() = 0;
			for (const auto& sh : getmyShares())
			{
				auto pType = sh.first._type;
				//				assert(pType >= 0); // is a producer
				auto id = sh.first._id;
				auto nShares = sh.second;

				const CProducer& producer = *World().pProducers()->at(id);
				if (producer.getInFinancialMarket()) // this indiv may be just first owner of a small producer
				{
					GoodQtty nShs = min<GoodQtty>(nShares, ceil(additionalNeededFunding / producer.getmyCurrShareVal()));
					// minus: sell
					SharesToTrade() = CShare(sh.first, -nShs); //first:prodFID, minus: sell

					break;
				}
			}
		}

		BuyGoods();
	}

	myGFCF() = myGFCF() - myGFCF()
		* getDEPData().getDepreciationRate().at(0) / 12.0; // per month

	// Bank activities

	ReturnLoansToMyBank();// only loans, keep surplus Cash

	// ============  split into liquidity and assets to buy (Financial market shares)  ======================

	if (getInputParameter("FinancialMarket"))
	{
		GoodQtty liquidity = max<GoodQtty>(4 * getConsumptionBudget(),
			getInputParameter("nBufferSalaries") * getmySalary_mu());

		if (getCash() > liquidity
			|| getmyBankBalance() > liquidity)
		{
			GoodQtty plannedAssetBudget = getCash() + getmyBankBalance() - liquidity;
			GoodQtty bondsBudget = max(0., plannedAssetBudget * _BondToSharesRatio);
			myBondsValueToTrade() = bondsBudget;
			myValueToBuyShares() = plannedAssetBudget - bondsBudget;
			SharesToTrade() = 0;
			myDividendYield() = 1.0;
		}
	}

	if (DebugLevel() > 1 && currMonth() >= getInputParameter("DebugFromMonth"))
		assert(DEPData().CheckAccountingBalance());
};

GoodQtty CWorker::MakeListOfGoodsToBuy()
{
	GoodType HCol = getmyHgroupN();

	GoodsToBuy() -= GoodsToBuy(); // set to 0's

	double disposableIncome = max<double>(0, getIncomePrevMonth() - getNetTaxesPrevMonth());
	double liquidity = getCash() + getmyBankBalance();
	double nBufferMonths = getInputParameter("nBufferSalaries");
	double disposable = max(0.,
		disposableIncome + getPropToConsume() * (getWealth() - nBufferMonths * getmySalary_mu()));
	disposable = min(liquidity, disposable);

	// BLE MODE: Adjust consumption based on expected inflation
	// When UseBLE=1 and BLE is in ACTIVE phase, workers adjust consumption propensity
	// based on inflation expectations learned from free-market dynamics
	if (getInputParameter("UseBLE") == 1.0) {
		const CBLEExpectations& ble = getWorld().getBLEExpectations();
		// Only apply BLE influence when in ACTIVE phase (after training completes)
		if (ble.canInfluenceAgents()) {
			double expectedInflation = ble.getExpectedInflation();
			double bleWeight = getInputParameter("BLEWeight");

			// Inflation adjustment: reduce consumption if expecting high inflation
			// This reflects precautionary behavior when real purchasing power is uncertain
			// Mild effect: 10% inflation expectation reduces consumption by ~1%
			double inflationAdjustment = 1.0 - 0.10 * expectedInflation;
			inflationAdjustment = max(0.8, min(1.2, inflationAdjustment)); // Clamp to reasonable bounds

			// Blend with original disposable based on BLEWeight
			disposable = (1.0 - bleWeight) * disposable + bleWeight * (disposable * inflationAdjustment);
		}
	}

	double totmyGoodsIwish = 0;
	// myGoodsIwish(gType) already includes its fraction of GFCF given by SAM().GFCFfractionOf(gType)
	for (const auto& gPair : getmyGoodsIwish())
		totmyGoodsIwish += gPair.second;

	double purchaseBudget = 0;
	if (totmyGoodsIwish == 0)
		return 0;

	for (const auto& gPair : getmyGoodsIwish())
	{
		// myGoodsIwish(gType) already includes the gType fraction of the total GFCF
		auto gType = gPair.first;
		GoodsToBuy()[gType] = 0;
		double myGoods = gPair.second;
		double toBuy = 0;
		auto ConsumPXFactor = getWorld().getConsumPXFactor().at(gType);

		toBuy = ConsumPXFactor
			* (disposable / getmyGoodsIwish().size()) // toBuy(gType) depends on the number of goods
			* (getmyGoodsIwish()(gType) / totmyGoodsIwish); // idiosyncratic preferences/needs given by the SAM
		if (getgammaC() > 0) // logit model
			toBuy *= mylogitProb(gType);

		toBuy = max(0., toBuy - getGoodsIhave(gType));

		if (currMonth() >= getInputParameter("PercentDemandFrom")
			&& currMonth() <= getInputParameter("PercentDemandUpto"))
			toBuy *= getInputParameter("PercentDemand");

		// === COVID-19 Physical Confinement ===
		// During lockdown, sectors are PHYSICALLY closed (not preference-based)
		// Workers CAN'T spend on closed sectors; unspent money becomes forced savings
		double confinementFactor = getWorld().getPandemicState().getConfinementFactor(gType);
		if (confinementFactor < 1.0) {
			double originalToBuy = toBuy;
			toBuy *= confinementFactor;  // Can only buy what's physically available
			double forcedSaving = (originalToBuy - toBuy) * getmyPriceOf(gType);
			// Note: forcedSaving stays in liquidity (Cash/Bank), tracked for statistics
			World().PandemicState().addForcedSavings(forcedSaving);
		}

		toBuy = ceil(toBuy);
		GoodsToBuy()[gType] = toBuy;

		purchaseBudget += toBuy * getmyPriceOf(gType); // to be payed by this Indiv
	}

	return ceil(purchaseBudget);
}

void CWorker::TryToStartupNewProducer()
{
	if (getpmyProducers()->size() >= getInputParameter("MaxOwnedProducers"))
		return; //M J Producers can only be started by a non-owner Indiv

	// MinStartupOwnerWealth check with gradual ramp-up during calibration
	// Factor goes from 0.0 at GFCFStabilityCheckStartMonth to 1.0 at AssistedProductionUpto
	double minWealth = getInputParameter("MinStartupOwnerWealth");
	if (minWealth > 0) {
		double factor = 0.0;
		
		if (isRealMarketSimulation()) {
			factor = 1.0;  // Full wealth requirement after calibration
		} else if (isTransitionCalibration()) {
			factor = 1.0;  // Full requirement during transition
		} else if (isAssistedCalibration()) {
			// Gradual ramp-up during assisted calibration
			double startMonth = getInputParameter("GFCFStabilityCheckStartMonth");
			double endMonth = getInputParameter("AssistedProductionUpto");
			if (currMonth() > startMonth && endMonth > startMonth) {
				factor = (currMonth() - startMonth) / (endMonth - startMonth);
			}
		}
		// factor = 0 during PreCalibration (no wealth requirement)
		
		if (getWealth() < factor * minWealth)
			return;
	}

	if (getWealth() < getInputParameter("MinSalariesToStartProducer") * getmySalary_mu())
		return;

	if (getRandom01() > getInputParameter("ProducerStartupProbab"))
		return;

	AgentType producerType = UndefAgentType;

	// During calibration phases, ensure at least one producer per sector with SAM output
	if (isCalibrationPhase())
	{
		for (const auto& pair : getDEPData().getnCurrentPProducers())
		{
			auto gType = pair.first;

			if (pair.second == 0 && getSAM().getSAMGrossOutput_mu(gType) > 0)
			{
				producerType = gType;
				break;
			}
		}
	}

	if (producerType == UndefAgentType)
	{
		// Suggest a producerType based on demand in the local ID neighborhood
		producerType = World().getRandomGoodDemandedInNeighborhood(this->getID());
	}

	if (producerType == UndefAgentType)
		return;

	// ---------------   Start new Producer  --------------------------

	CAgent* pOwner = this;
	// World().newProducer will do this: if (getSAM().IsPublicSector(producerType))		pOwner = pGovernment();

	CProducer* pnewProd = World().newProducer(producerType, pOwner);
/* //MJ
	// Transfer startup capital from owner to new producer
	// Only transfer if producer was successfully created and is owned by this worker (not public sector)
	if (pnewProd != nullptr && pnewProd->getpOwners() != nullptr 
		&& pnewProd->getpOwners()->count(this->getFID()) > 0)
	{
		double startupFraction = getInputParameter("StartupCapitalFraction");
		if (startupFraction > 0 && getCash() > 0)
		{
			// Transfer a fraction of owner's cash to the new producer
			GoodQtty startupCapital = (GoodQtty)(getCash() * startupFraction);
			if (startupCapital > 0)
			{
				Cash() -= startupCapital;
				pnewProd->Cash() += startupCapital;
				
				// Log the capital transfer
				try {
					LogFile() << " StartupCap:" << startupCapital;
					LogFile().flush();
				}
				catch (...) {  } // logging must never break simulation
			}
		}
	}
*/
};
CBank* CWorker::TryToFoundCommercialBank()
{
	GoodQtty initCash = 0;

	if (pOwnedBank() == nullptr)
		//	pOwnedBank() = pCentralBank()->BankFoundationRequest(this, initCash);
		pOwnedBank() = pCentralBank()->BankFoundationRequest(nullptr, initCash);

	return pOwnedBank();
}

///////////////////////////////////////////////////////////////////////////////////////
