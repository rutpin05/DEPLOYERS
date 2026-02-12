
// ExtSectors.cpp

#include "./pch.h"

//======================  CExtSect  ===================================
CExtSect::CExtSect(AgentID ID, AgentType aType)
	: CProducer(ID, aType, nullptr)
{
	pImports() = nullptr;
	pExports() = nullptr;
	pExports_Init() = nullptr;
	initialize();
}
CExtSect::~CExtSect() { }

ofstream& operator<<(ofstream& ofstrm, const CExtSect& extSector)
{
	ofstrm << (CAgent&)extSector; // call base class

	ofstrm << endl << " Imports {";
	for (const auto& nn : extSector.getImports())
		ofstrm << " " << nn;
	ofstrm << " }";

	ofstrm << endl << " Exports {";
	for (const auto& nn : extSector.getExports())
		ofstrm << " " << nn;
	ofstrm << " }";

	ofstrm << endl << " prevExports {";
	for (const auto& nn : extSector.getprevExports())
		ofstrm << " " << nn;
	ofstrm << " }";

	return ofstrm;
}
ifstream& operator>>(ifstream& ifstrm, CExtSect& extSector)
{
	string name;
	GoodQtty qtty;
	ifstrm >> (CAgent&)extSector; // call base class

	extSector.Imports().resize(0);
	ifstrm >> name >> name; // " Imports {"
	while (ifstrm >> name, name != "}")
	{
		auto x = atof(name.c_str());
		qtty = (GoodQtty)x;
		extSector.Imports().push_back(qtty);
	}

	extSector.Exports().resize(0);
	ifstrm >> name >> name; // " Exports {"
	while (ifstrm >> name, name != "}")
	{
		auto x = atof(name.c_str());
		qtty = (GoodQtty)x;
		extSector.Exports().push_back(qtty);
	}

	extSector.prevExports().resize(0);
	ifstrm >> name >> name; // " Exports {"
	while (ifstrm >> name, name != "}")
	{
		auto x = atof(name.c_str());
		qtty = (GoodQtty)x;
		extSector.prevExports().push_back(qtty);
	}

	return ifstrm;
}

void CExtSect::initialize()
{
	name() = getSAM().getAccNameOfN(getAgentType());
	CProducer::initialize();

	GoodType GFCFtype = getSAM().GFCFtype();
	pImports() = nullptr;
	pImports() = new vector<GoodQtty>(getSAM().getnPXproducerTypes() + 1, 0); // +1: GFCFtype
	pExports() = nullptr;
	pExports() = new vector<GoodQtty>(getSAM().getnPXproducerTypes() + 1, 0); // +1: GFCFtype
	pprevExports() = nullptr;
	pprevExports() = new vector<GoodQtty>(getSAM().getnPXproducerTypes() + 1, 0); // +1: GFCFtype
	pExports_Init() = nullptr;
	pExports_Init() = new vector<GoodQtty>(getSAM().getnPXproducerTypes() + 1, 0); // +1: GFCFtype

	InitCash() = 5 * getWorld().getnWorkers() * getSAM().getInitSalary()
		* getInputParameter("MonetaryBasePerActiveSalaries");
	Cash() = InitCash();
	ConsumptionBudget() = 0.0;
	ToSell(getAgentType()) = InitCash(); // Imports

	pUsedBank() = nullptr;

	// Products that can be bought from this ExtSect (IMPORTS)
	double productionPriceInit = 1.0;
	productionPrice() = productionPriceInit;
	myPrice()[getAgentType()] = productionPriceInit;
	ToSell(getAgentType());// get ready to offer it to clients

	RefExports().clear();
	totalRefExports() = 0;
	for (long gType = 0; gType <= getSAM().getnPXproducerTypes(); ++gType)
	{
		RefExports()[gType] = getSAM().getRowCol(gType, getAgentType());
		totalRefExports() += getSAM().getRowCol(gType, getAgentType());
		ExportFraction()[gType] = 1.0;
		ExportConsumPXFactor()[gType] = 1.0;
	}

	for (long gType = 0; gType <= getSAM().getnPXproducerTypes(); ++gType)
		ExportFraction().at(gType) = RefExports()[gType] / totalRefExports();

	//ExportFraction().at(GFCFtype) = RefExports()[GFCFtype] / totalRefExports();
}

void CExtSect::monthInitialize()
{
	IncomePrevMonth() = IncomeCurr();
	IncomeCurr() = 0;

	NetTaxesPrevMonth() = NetTaxesCurr();
	NetTaxesCurr() = 0;

	myDemand() = 0;
	ConsumptionBudget() = 0;
	assistedQtties().clear();
	SharesToTrade() = CShare(getFID(), 0);

	// ============  set myGoodsIwish and ToSell  ============================================

	GoodType myExtSectType = getAgentType();
	GoodType GFCFtype = getSAM().GFCFtype();

	bool _IsDisaggExtCountry = false;
	string extCountryCode = name().substr(1, 2);
	if (getWorld().getpFigaro() != nullptr
		&& getWorld().getFigaro()._pDisaggExtSectCountries->find(extCountryCode)
		!= getWorld().getFigaro()._pDisaggExtSectCountries->end())
		_IsDisaggExtCountry = true;

	// 1. EXPORTS to this ExtSect interface  --------------------------------------------------

	myGoodsIwish().clear();
	// Set ExtSect.myGoodsIwish to the Imports values of
	// extCountry's IO file, see CWorld::readMyExternalSectorsIO()
	// Suppose this country is ES reading InitialInput_PT_IO.dep: Exports_Init()[SectorN] is
	// the value IMPORTED from this ExtSect interface by SectorN of PorTugal.
	// Distribute the total among sectors here,
	// in the ratios given by the SAM (weighted by price).

	double totalToExport = 0, totalToExportSAM = 0;
	for (long gType = 0; gType < getSAM().getnPProducerTypes(); ++gType)
	{
		if (getSAM().getSAMGrossOutput_mu(gType) == 0) // skip sectors with no production
			continue;

		Exports().at(gType) = getSAM().getRowCol(gType, getAgentType());
		Exports_Init().at(gType) = getSAM().getRowCol(gType, getAgentType());

		// Smooth inter country changes

		if (getWorld().getpFigaro() != nullptr && name() != "RW")
			Exports_Init().at(gType) = getprevExports(gType)
			+ ceil((getExports_Init().at(gType) - getprevExports(gType)) * getInputParameter("ExportFraction"));

		totalToExport += getExports_Init().at(gType);
		totalToExportSAM += getSAM().getRowCol(gType, getAgentType());
	}

	double buyQtty = 0;

	for (long gType = 0; gType < getSAM().getnPProducerTypes(); ++gType)
	{
		if (getSAM().getSAMGrossOutput_mu(gType) == 0) // skip sectors with no production
			continue;

		if (gType == getSAM().GFCFtype())
			continue; // this gType fraction of the GFCF qtty already included in its buyQtty

		if (getWorld().getpFigaro() == nullptr
			|| name() == "RW") // Use SAM values
		{
			buyQtty = getSAM().getRowCol(gType, getAgentType()); // per year

			// Add this gType fraction of the GFCF qtty
			buyQtty += getSAM().getRowCol("GFCF", getAgentType()) * SAM().GFCFfractionOf(gType);
		}
		else // external country is being simulated, use read IO values
		{
			buyQtty = totalToExport * ExportFraction(gType);

			// Add this gType fraction of the GFCF qtty
			buyQtty += totalToExport * ExportFraction(GFCFtype) * SAM().GFCFfractionOf(gType);
		}

		if (buyQtty > 0)
			myGoodsIwish()[gType] = doubleToGQtty(buyQtty);
	}

	for (auto& gPair : myGoodsIwish())
	{
		if (getgammaC() > 0) // logit model
		{
			buyQtty = gPair.second;

			if (currMonth() >= getInputParameter("PercentDemandFrom")
				&& currMonth() <= getInputParameter("PercentDemandUpto"))
				buyQtty *= getInputParameter("PercentDemand");

			buyQtty *= mylogitProb(gPair.first);
			gPair.second = doubleToGQtty(buyQtty);
		}
	}

	// 2. IMPORTS from this ExtSect: initialize its ToSell availability  ------------------

	ToSell(getAgentType()) = 0;

	if (getWorld().getpFigaro() == nullptr || name() == "RW") // Use SAM values
	{
		for (long gType = 0; gType < getSAM().getnPProducerTypes(); ++gType)
			ToSell(getAgentType()) += getSAM().getRowCol(getAgentType(), gType);
	}
	else // external country is being simulated, use read IO values
	{
		if (_IsDisaggExtCountry)
			ToSell(getAgentType()) = Imports().at(getAgentType());
		else
		{
			for (long gType = 0; gType < getSAM().getnPProducerTypes(); ++gType)
			{
				double sellQtty = Imports().at(gType);

				if (currMonth() >= getInputParameter("PercentDemandFrom")
					&& currMonth() <= getInputParameter("PercentDemandUpto"))
					sellQtty *= getInputParameter("PercentDemand");

				ToSell(getAgentType()) += sellQtty;
			}
		}

		// Help to start production: supplement non-produced qtty  -----------

		double defaultToSell = 2.0 * InitCash();
		double uptoMonth = getInputParameter("AssistedProductionUpto");
		double assistedRange = uptoMonth; // Assisted always from 0

		if (currMonth() <= uptoMonth)
		{
			GoodQtty delta = max((GoodQtty)0, (GoodQtty)defaultToSell - ToSell(getAgentType()))
				* max(0., uptoMonth - (double)currMonth()) / assistedRange; // fade out
			if (delta > 0)
			{
				ToSell(getAgentType()) += delta;
				assistedQttiesOf(getAgentType()) += delta;
			}
		}
	}

	prevExports() = getExports_Init();
	Exports().clear();
	Exports().resize(getSAM().getnPProducerTypes() + 1, 0); // +1: GFCFtype
	// Exports_Init is used for normalization in allocateBuyersQttyAndMoney

	// In CExtSect::monthActivity(), protect RW's import values:

	if (name() != "RW")  // Only clear for simulated external countries
	{
		Imports().clear();
		Imports().resize(getSAM().getnPProducerTypes() + 1, 0);
	}

	// In CExtSect::monthInitialize(), after Imports().resize():

	if (name() == "RW")
	{
		// For RW, use SAM values for imports since RW is not simulated
		for (long gType = 0; gType < getSAM().getnPProducerTypes(); ++gType)
		{
			// RW imports from domestic sectors = domestic sectors' exports to RW
			// This is the column in SAM for RW (what each sector exports to RW)
			Imports()[gType] = getSAM().getRowCol(gType, getAgentType());
		}

		// Also handle GFCF component if needed
		GoodType GFCFtype = getSAM().GFCFtype();
		if (GFCFtype < getSAM().getnPProducerTypes())
		{
			Imports()[GFCFtype] = getSAM().getRowCol(GFCFtype, getAgentType());
		}
	}
}
void CExtSect::monthActivity()
{
	if (DebugLevel() > 2 && currMonth() >= getInputParameter("DebugFromMonth"))
	{
		assert(DEPData().CheckAccountingBalance());
		LogFile()
			<< "\n month " << currMonth()
			<< " " << getAgentName()
			<< " Cash " << getCash();
		LogFile().flush();
	}

	if (getInputParameter("ExtSectConsum"))
	{
		GoodType GFCFtype = getSAM().GFCFtype();
		double buyQtty = 0;
		// 1. This ExtSector's transfers with each Household
		AgentType ExtSectCol = this->getAgentType();

		// Salaries/transfers from this ExtSect to Households from members abroad
		map<GoodType, GoodQtty> nHIndivs;  // Hrow, nIndivs
		for (auto pIndiv : *World().pWorkers())
			nHIndivs[pIndiv->getmyHgroupN()]++;

		map<GoodType, GoodQtty> ExtSectToHousehold;
		for (const auto& pair : nHIndivs)
		{
			auto Hrow = pair.first;
			auto nIndivsH = pair.second;

			if (nHIndivs[Hrow] == 0)
				ExtSectToHousehold[Hrow] = 0;
			else
				ExtSectToHousehold[Hrow] = getSAM().getRowCol(Hrow, ExtSectCol)
				/ (12. * DEPData().getUpscaleSimulationFactor() * nIndivsH);
		}

		for (const auto& pIndiv : *getWorld().getpWorkers())
		{
			bool done = false;
			auto myHrow = pIndiv->getmyHgroupN();
			auto myQtty = ExtSectToHousehold[myHrow];

			done = PayTo(pIndiv, myQtty);

			if (done)
			{
				SAM().SAMmonth()[myHrow][ExtSectCol] += myQtty * 12. // mu
					* getSAM().getActive() / getWorld().getNWorkers();
			}
		}
	}
}

//=========================================================
