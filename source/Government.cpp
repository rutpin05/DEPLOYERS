
// Government.cpp

#include "./pch.h"

//==================  CGovernment  ==========================

CGovernment::CGovernment(AgentID ID, AgentType ty) : CAgent(ID, ty)
{
	initialize();
};
CGovernment::~CGovernment() {};

ofstream& operator<<(ofstream& ofstrm, const CGovernment& theState)
{
	ofstrm << " ID " << (long)theState.getID();
	ofstrm << (CAgent&)theState; // call base class

	ofstrm << " MaxDebt " << theState._MaxDebt;
	ofstrm << " nBondsEmission " << theState._nBondsEmission;

	ofstrm << " myBondTemplate ";
	ofstrm << theState._myBondTemplate;

	ofstrm << " AnnUnemplQtty " << theState._AnnUnemplQtty;

	ofstrm << " ProducerIDs {";
	for (const auto& id : theState.getProducerIDs())
		ofstrm << " " << id;
	ofstrm << " }";

	ofstrm << "\n TaxGFCF " << theState.getTaxGFCF() << endl;
	ofstrm << "\n TaxHouseholdProducts "; ofstrm << theState.getTaxHouseholdProducts() << endl;
	ofstrm << "\n TaxSectExtProducts "; ofstrm << theState.getTaxSectExtProducts() << endl;

	return ofstrm;
};
ifstream& operator>>(ifstream& ifstrm, CGovernment& theState)
{
	string word, word1;
	AgentID ID;

	ifstrm >> word >> ID; // as TheStateID
	operator>>(ifstrm, (CAgent&)theState); // call base class

	ifstrm >> word >> theState._MaxDebt >> word >> theState.nBondsEmission();

	ifstrm >> word;
	ifstrm >> theState._myBondTemplate;

	ifstrm >> word >> theState._AnnUnemplQtty;

	ifstrm >> word >> word1; // " ProducerIDs {"
	while (ifstrm >> word, word != "}") // id
	{
		ifstrm >> ID; // id
		theState.ProducerIDs().insert(ID);
	}

	ifstrm >> word >> theState.TaxGFCF();
	ifstrm >> word; ifstrm >> theState.TaxHouseholdProducts();
	ifstrm >> word; ifstrm >> theState.TaxSectExtProducts();

	return ifstrm;
};

void CGovernment::initialize()
{
	pUsedBank() = pCentralBank();

	_MaxDebt = 2 * getInputParameter("InitialGovDebt");

	nBondsEmission() = 0;
	myBondTemplate()._faceValue = 0;
	myBondTemplate()._couponRate = 0.06;
	myBondTemplate()._couponDate = 6;
	// every _couponDate months: (currMonth - emission) % coupondate == 0

	myBondTemplate()._purchaseDate = 999999; // currMonth
	myBondTemplate()._maturityDate = 24; // 24 months: 2 years
	myBondTemplate()._price = 0;

	InitCash() = 0;
	if (getInputParameter("MaxNBanks") == 0)
		InitCash() = 2 * getWorld().getnWorkers()
		* getInputParameter("MonetaryBasePerActiveSalaries")
		* getSAM().getInitSalary();

	Cash() = InitCash();
	AnnUnemplQtty() = 0;
	GoodsIhave() -= GoodsIhave();// GoodsIhave(GFCF) is flow, _myGFCF is the stock

	// =================  TAX rates  ============================

	// Producers already include their tax rates in their definitions (

	// 1. -------------------- "TaxProducts" ----------------------

	// 1.1 ExtSectors cols Products, payed in allocateBuyersQttyAndMoney at purchase time

	double total = 0;
	for (const auto& acc : getSAM().getAccountGroups().at("X"))
	{
		GoodType XcolN = acc->accN();
		TaxSectExtProducts()[XcolN] = 0;

		total = 0;
		for (long pType = 0; pType < getSAM().getnPProducerTypes(); ++pType)
			total += getSAM().getRowCol(pType, XcolN);

		if (total > 0)
			TaxSectExtProducts()[XcolN] = getSAM().getRowCol("TaxProducts", XcolN) / total;
	}

	// 1.2 GFCF col Products, payed in allocateBuyersQttyAndMoney at purchase time

	GoodType GFCFcol = getSAM().getAccNofName("GFCF");
	TaxGFCF() = 0;

	total = 0;
	for (long pType = 0; pType < getSAM().getnPProducerTypes(); ++pType)
		total += getSAM().getRowCol(pType, GFCFcol);

	if (total > 0)
		TaxGFCF() = getSAM().getRowCol("TaxProducts", GFCFcol) / total;

	// 1.3 Government col Products, payed in allocateBuyersQttyAndMoney at purchase time

	GoodType Gcol = getSAM().getAccNofName("Government");
	TaxGovernmentProducts() = 0;

	total = 0;
	for (long pType = 0; pType < getSAM().getnPProducerTypes(); ++pType)
		total += getSAM().getRowCol(pType, Gcol);

	if (total > 0)
		TaxGovernmentProducts() = getSAM().getRowCol("TaxProducts", Gcol) / total;

	// 1.4 Households cols Products, payed in allocateBuyersQttyAndMoney at purchase time

	for (const auto& acc : getSAM().getAccountGroups().at("H"))
	{
		GoodType Hcol = acc->accN();
		TaxHouseholdProducts()[Hcol] = 0;

		total = 0;
		for (long pType = 0; pType < getSAM().getnPProducerTypes(); ++pType)
			total += getSAM().getRowCol(pType, Hcol);

		if (total > 0)
			TaxHouseholdProducts()[Hcol] = getSAM().getRowCol("TaxProducts", Hcol) / total;
	}
};

void CGovernment::GovTransfersToHouseholds()
{
	bool done = false;
	double UnempSubsidFactor = getInputParameter("UnempSubsidFactor"); //  use 0.80 del IPREM instead?
	double totalToUnemployed = UnempSubsidFactor * getSAM().getRowCol("Households", "CompEmployees")
		* getSAM().getInitUnemploymentPercent() * 0.01;
	double totalSubsidies = max<GoodQtty>(0, getSAM().getRowCol("Households", "Government") - totalToUnemployed);
	double subsidiesPerActiveMonth = totalSubsidies / (12. * getSAM().getActive());

	// Kurzarbeit parameters for pandemic support
	auto& pandemicState = World().PandemicState();
	bool isPandemicActive = pandemicState.isPandemicActive() && pandemicState.isInPandemicPeriod();
	double furloughSubsidyRate = pandemicState.getFurloughSubsidyRate();   // e.g., 0.80
	double furloughCoverageRate = pandemicState.getFurloughCoverageRate(); // e.g., 0.50

	for (auto pIndiv : *World().pWorkers())
	{
		double myUnemployment = UnempSubsidFactor * pIndiv->getmyAvailableTime() * pIndiv->getmySalary_mu();
		double CPIinflation = getDEPData().getCPItracker().getlastCPI() / 100.;
		GoodQtty total = myUnemployment;
		if (CPIinflation > 0.)
			total += subsidiesPerActiveMonth * (1. + CPIinflation); //M J (?)
		else
			total += subsidiesPerActiveMonth;

		// === COVID-19 Kurzarbeit (Short-Time Work) Subsidy ===
		// Government pays workers for lockdown-induced lost hours
		// This keeps workers employed while compensating for reduced work time
		if (isPandemicActive && pIndiv->getmyAvailableTime() < 1.0) {
			// Check if worker is employed and participating in Kurzarbeit
			if (pIndiv->getpEmployer() != nullptr) {
				// Worker is employed but has reduced hours due to lockdown
				// Kurzarbeit subsidy covers a fraction of lost wages
				double lostHours = 1.0 - pIndiv->getmyAvailableTime();
				double lostWages = lostHours * pIndiv->getmySalary_mu();
				
				// Only some firms participate in Kurzarbeit (coverage rate)
				// and subsidy covers partial wages (subsidy rate)
				if (getRandom01() < furloughCoverageRate) {
					double kurzarbeitPayment = furloughSubsidyRate * lostWages;
					total += kurzarbeitPayment;
					pandemicState.addKurzarbeitPayment(kurzarbeitPayment);
				}
			}
		}

		done = PayTo(pIndiv, total);
		if (!done)
			CWorld::ERRORmsg(getAgentName() + ": not enough money to pay Subsidies", true);

		SAM().SAMmonth()[pIndiv->getmyHgroupN()][getSAM().getAccNofName("Government")] +=
			total * 12 * DEPData().getUpscaleSimulationFactor();
	}
}

void CGovernment::GovPayBondCoupons()
{
	if (Government().NBonds() == 0) // no active bonds
		return;

	auto& bondTemplate = myBondTemplate();

	for (const auto& pIndiv : *getWorld().getpWorkers())
	{
		auto& bond = pIndiv->myBond();
		if (bond._faceValue == 0)
			continue;

		assert(bondTemplate._faceValue = bond._faceValue);

		// every _couponDate months: (currMonth - emission) % coupondate == 0
		if ((currMonth() - (long)bond._purchaseDate) % (long)bond._couponDate != 0)
			continue;

		bool done = false;
		GoodQtty coupon = (bond._faceValue * bond._couponRate * bond._couponDate / 12.);
		if (currMonth() == bond._purchaseDate + bond._maturityDate)
			coupon += bond._faceValue; // last coupon, return also faceValue

		coupon *= pIndiv->NBonds();
		done = PayTo(pIndiv, coupon);
		if (!done)
			CWorld::ERRORmsg(getAgentName() + ": not enough money to pay BondCoupons", true);

		if (currMonth() == bond._purchaseDate + bond._maturityDate)
		{
			Government().NBonds() -= pIndiv->NBonds(); // decrease N active bonds

			if (Government().NBonds() == 0) // erase Gov bond
				bondTemplate._faceValue = 0;

			bond._faceValue = 0; // erase bond
			bond._couponRate = 0;
			bond._couponDate = 0;
			bond._maturityDate = 0;
			bond._purchaseDate = 0;
			bond._price = 0;

			pIndiv->NBonds() = 0;
		}
	}
}
void CGovernment::ReturnExcessCashToCB()
{
	// Try to return excess loan

	GoodQtty currentDebt = -getmyBankBalance();

	if (Cash() > 2 * _ConsumptionBudget && currentDebt > 0)
	{
		GoodQtty payed = Cash() - _ConsumptionBudget;

		CBankEntry accStatus = pUsedBank()->ClientCashOperation(this, payed);
		//myBankAccountStatus() = accStatus;
	}
}
GoodQtty CGovernment::getWealth() const
{
	GoodQtty Wealth = getCash() + getmyBankBalance();

	return Wealth;
}

void CGovernment::monthInitialize()
{
	IncomePrevMonth() = IncomeCurr();
	IncomeCurr() = 0;

	NetTaxesPrevMonth() = NetTaxesCurr();
	NetTaxesCurr() = 0;

	RemoveServiceGoods(); // These are FINAL consumption except for Producers (Intermed.)

	// Keep Public Debt constant during calibration

	if (getInputParameter("MaxNBanks") > 0
		&& getInputParameter("InitialGovDebt") != 0
		&& !isRealMarketSimulation())
	{
		GoodQtty initGovDebt = getInputParameter("InitialGovDebt")
			* getWorld().getNWorkers() / getSAM().getActive();
		GoodQtty currDebt = getCash() + getmyBankBalance();

		GoodQtty shift = initGovDebt - currDebt;

		Cash() += shift;
		CentralBank().Cash() -= shift;
		CentralBank().InitCash() -= shift;
		auto status = getmyUsedBank()->ClientCashOperation(this, shift);
	}

	// -------- Do we need to emit Bonds? -----------------

	GoodQtty gdp = getDEPData().getGDPnominal(); // 1100000 * 1e6;
	if (gdp > 0)
	{
		_MaxDebt = gdp / getDEPData().getUpscaleSimulationFactor();
		GoodQtty currDebt = getCash() + getmyBankBalance();

		if (currDebt < -_MaxDebt // _MaxDebt is positive
			&& myBondTemplate()._faceValue == 0) // no payable bonds active
		{
			myBondTemplate()._faceValue = 10000;
			myBondTemplate()._couponRate = 0.06;
			myBondTemplate()._couponDate = 6; // every _couponDate months since emission
			myBondTemplate()._maturityDate = 24; // 24 months: 2 years after purchaseDate
			// (currMonth - purchaseDate) % coupondate == 0

			myBondTemplate()._purchaseDate = 999999; // currMonth at purchase time

			myBondTemplate()._price = myBondTemplate()._faceValue; // emission price

			// try to rise balance to 50% before MaxDebt
			GoodQtty bondEmissionValue = -currDebt - _MaxDebt + 0.5 * _MaxDebt;

			nBondsEmission() = 1 + bondEmissionValue / myBondTemplate()._faceValue;
			bondEmissionValue = nBondsEmission() * myBondTemplate()._faceValue;
		}
	}

	// ============  set myGoodsIwish  ============================================

	if (getInputParameter("GovConsum"))
	{
		GoodType GFCFtype = getSAM().GFCFtype();

		myGoodsIwish().clear();

		for (long gType = 0; gType < getSAM().getnPXproducerTypes(); ++gType)
		{
			double buyQtty = getSAM().getRowCol(gType, "Government");

			// Add this gType fraction of the GFCF qtty
			buyQtty += getSAM().getRowCol("GFCF", "Government") * SAM().GFCFfractionOf(gType);

			if (buyQtty > 0)
			{
				// --------------------------------------------------------------------
				if (currMonth() >= getInputParameter("FiscalPolicyShockFrom")
					&& currMonth() <= getInputParameter("FiscalPolicyShockUpto"))
					buyQtty *= getInputParameter("FiscalPolicyShockFactor");
				// --------------------------------------------------------------------

				if (currMonth() >= getInputParameter("PercentDemandFrom")
					&& currMonth() <= getInputParameter("PercentDemandUpto"))
					buyQtty *= getInputParameter("PercentDemand");

			}
			myGoodsIwish()[gType] = doubleToGQtty(buyQtty); // all sectors
			setmyPriceOf(gType, 1.0);
		}

		for (auto& gPair : myGoodsIwish())
		{
			auto gType = gPair.first;
			auto& buyQtty = gPair.second;
			if (getgammaC() > 0) // logit model
				buyQtty = doubleToGQtty(buyQtty * mylogitProb(gType));

			auto GovConsumPXFactor = getWorld().getGovConsumPXFactor().at(gType);
			buyQtty *= GovConsumPXFactor; // ConsumPXFactor aprox. 1.0
		}
	}
}
void CGovernment::monthActivity() {};
void CGovernment::monthlyActivity()
{
	if (!(bool)getInputParameter("IndivConsum"))
		return;

	GovTransfersToHouseholds();

	GovPayBondCoupons();

	ReturnExcessCashToCB();
}
