
//  Agent.cpp

//   DEPLOYERS v2

#include "./pch.h"

double CAgent::_gammaC;
double CAgent::_PriceAdaptFactor = 1.005;
CTypeDoubleMap CAgent::_PriceCalibFactor;
vector<vector<double>> CAgent::_BuyFraction;

ofstream& operator<<(ofstream& ofstrm, const CTypeDoubleMap& mapTyDou)
{
	ofstrm << "{";
	for (const auto& pair : mapTyDou)
	{
		ofstrm << " " << getSAM().getAccNameOfN(pair.first);
		ofstrm << " " << pair.second; // name qtty
	}
	ofstrm << " }";

	return ofstrm;
}
ifstream& operator>>(ifstream& ifstrm, CTypeDoubleMap& mapTyDou)
{
	string name, bracket;
	double dQtty = -1.;
	ifstrm >> bracket; // "{"

	CGood good;
	while (ifstrm >> name, name != "}")
	{
		ifstrm >> dQtty;
		mapTyDou[getSAM().getAccNofName(name)] = dQtty;
	}

	return ifstrm;
}

//======================  CStringDoubleMap  ==================================

ofstream& operator<<(ofstream& ofstrm, const CStringDoubleMap& mapStrDou)
{
	ofstrm << "{";
	for (const auto& pair : mapStrDou)
	{
		ofstrm << " " << pair.first;
		ofstrm << " " << pair.second; // name qtty
	}
	ofstrm << " }";

	return ofstrm;
}
ifstream& operator>>(ifstream& ifstrm, CStringDoubleMap& mapStrDou)
{
	string name, bracket;
	double dQtty = -1.;
	ifstrm >> bracket; // "{"

	while (ifstrm >> name, name != "}")
	{
		ifstrm >> dQtty;
		mapStrDou[name] = dQtty;
	}

	return ifstrm;
}

//======================  CAgent  ===================================

double CAgent::mylogitProb(GoodType gType)
{
	double sum = 0., price = 0., logitProb = 0.;
	auto nGoodsIwish = getmyGoodsIwish().size();
	for (const auto& gPair : getmyGoodsIwish())
	{
		price = getmyPriceOf(gPair.first);
		if (price <= 0)
			getWorld().ERRORmsg("price <= 0 in mylogitProb");

		sum += exp(-getgammaC() * log(price));
	}

	if (sum <= 0)
		getWorld().ERRORmsg("sum <= 0 in mylogitProb");

	logitProb = nGoodsIwish * exp(-getgammaC() * log(getmyPriceOf(gType))) / sum;

	return logitProb;
}

CAgent::CAgent(AgentID id, AgentType agentTy)
	: _ID(id), _myProxyBuyerID(id), _agentType(agentTy), _initialMonth(currMonth())
{
	pmyProducers() = nullptr;
	initialize(); // variables
}
CAgent::~CAgent() {}

ofstream& operator<<(ofstream& ofstrm, const CAgent& agent)
{
	ofstrm
		<< "\n initialMonth " << agent.getinitialMonth()
		<< "\n MonthlyActivityMonth " << agent.getMonthlyActivityMonth()
		<< " InitCash " << agent.getInitCash()
		<< " Cash " << agent.getCash()
		<< " ConsumptionBudget " << agent.getConsumptionBudget() << endl;

	ofstrm << " myProxyBuyerID " << agent.getmyProxyBuyerID();

	ofstrm << " Goods_I_wish ";
	ofstrm << agent.getmyGoodsIwish();

	ofstrm << endl << " ProducerIDs {";
	if (agent.getpmyProducers() != nullptr)
		for (const auto& pair : *(agent.getpmyProducers()))
			ofstrm << pair.first;
	else
		ofstrm << " -1";
	ofstrm << " }";

	ofstrm << " myProviderID ";
	ofstrm << agent.getmyProviderID();

	ofstrm << endl << " GoodsIhave ";
	ofstrm << agent.getGoodsIhave();

	ofstrm << " myFixCapital " << agent.getmyFixCapital();
	ofstrm << " purchasedFixCapital " << agent.getpurchasedFixCapital();
	ofstrm << " myGFCF " << agent.getmyGFCF();

	ofstrm << endl << " WorkTimeToBuy " << agent.getWorkTimeToBuy();

	ofstrm << endl << " GoodsToBuy ";
	ofstrm << agent.getGoodsToBuy();

	ofstrm << endl << " ToSell ";
	ofstrm << agent.getToSell();

	ofstrm << endl << " myPriceOfSalary " << agent.getmyPriceOfSalary();
	ofstrm << endl << " myPrice { ";
	for (const auto& pair : agent._myPrice)
		ofstrm << "" << getSAM().getAccNameOfN(pair.first)
		// << " " << std::setprecision(17) << pair.second << " ";
		<< " " << pair.second << " ";
	ofstrm << "}";

	ofstrm << endl << " myShares ";
	ofstrm << agent.getmyShares();
	ofstrm << endl << " SharesToTrade "; ofstrm << agent.getSharesToTrade();
	ofstrm << endl << " myValueToBuyShares " << agent.getmyValueToBuyShares();
	ofstrm << " myDividendYield " << agent.getmyDividendYield();

	ofstrm << endl << " myBondsValueToTrade " << agent._myBondsValueToTrade;
	ofstrm << endl << " NBonds " << agent.getNBonds();
	ofstrm << endl << " myTotBondsValue " << agent.getmyTotBondsValue();

	if (agent.getmyUsedBank())
	{
		ofstrm << "\n UsedBank bTy " << agent.getmyUsedBank()->getAgentType()
			<< " bID " << agent.getmyUsedBank()->getID();
	}
	else
		ofstrm << "\n UsedBank bTy " << UndefAgentType << " bID " << UndefAgentID;

	ofstrm << " myBankAccountStatus ";
	ofstrm << agent.getmyBankAccountStatus();

	ofstrm << "\n WealthPrevMonth " << agent.getWealthPrevMonth();
	ofstrm << " IncomePrevMonth " << agent.getIncomePrevMonth();
	ofstrm << " NetTaxesPrevMonth " << agent.getNetTaxesPrevMonth();
	ofstrm << " IncomeCurr " << agent.getIncomeCurr();
	ofstrm << " NetTaxesCurr " << agent.getNetTaxesCurr();

	return ofstrm;
}
ifstream& operator>>(ifstream& ifstrm, CAgent& agent)
{
	string name, word, word1, bracket;
	double doub = 0;

	ifstrm
		>> name >> agent.initialMonth()
		>> name >> agent.MonthlyActivityMonth()
		>> name >> agent.InitCash()
		>> name >> agent.Cash()
		>> name >> agent.ConsumptionBudget();

	ifstrm >> word >> agent.myProxyBuyerID();

	ifstrm >> name; // " myGoodsIwish "
	ifstrm >> agent.myGoodsIwish();

	ifstrm >> name >> bracket; // " ProducerIDs {"
	while (ifstrm >> word, word != "}")
	{
		auto ty = atoi(word.c_str());
		ifstrm >> word;
		auto id = atoi(word.c_str());
		(*agent.pmyProducers())[CAgentFID(ty, id)] = World().pProducers()->at(id);
	}

	ifstrm >> name; // " myProviderID "
	ifstrm >> agent.myProviderID();

	ifstrm >> word; // " GoodsIhave "
	ifstrm >> agent.GoodsIhave();

	ifstrm >> word >> agent.myFixCapital();
	ifstrm >> word >> agent.purchasedFixCapital();
	ifstrm >> word >> agent.myGFCF();

	ifstrm >> word >> agent.WorkTimeToBuy();

	ifstrm >> word;
	ifstrm >> agent.GoodsToBuy();

	ifstrm >> word;
	ifstrm >> agent.ToSell();

	ifstrm >> word >> agent._myPriceOfSalary;
	ifstrm >> word >> bracket; // " myPrice { "
	while (ifstrm >> name, name != "}")
	{
		GoodType gTy = getSAM().getAccNofName(name);
		ifstrm >> doub;
		agent.myPrice()[gTy] = doub;
	}

	ifstrm >> word;
	ifstrm >> agent.myShares();
	ifstrm >> word; ifstrm >> agent.SharesToTrade();
	ifstrm >> word >> agent.myValueToBuyShares();
	ifstrm >> word >> agent.myDividendYield();

	ifstrm >> name >> agent._myBondsValueToTrade;
	ifstrm >> word >> agent.NBonds();
	ifstrm >> word >> agent.myTotBondsValue();

	ifstrm >> word; // "UsedBank"
	long bTy, bID;
	ifstrm >> word >> bTy >> word >> bID;
	if (bTy == UndefAgentType)
	{
		agent.pUsedBank() = nullptr; // e.g. ExtSect
	}
	else if (bTy == getCentralBankType())
		agent.pUsedBank() = (CBank*)getpCentralBank();
	else
	{
		auto result = getCentralBank().getBanks().find(bID);
		if (result == getCentralBank().getBanks().end())
			pCentralBank()->Banks()[bID] = new CBank(bTy);

		agent.pUsedBank() = pCentralBank()->Banks()[bID];
	}

	ifstrm >> word >> word1; // myBankAccountStatus {
	ifstrm >> agent.myBankAccountStatus();

	ifstrm >> word >> agent.WealthPrevMonth();
	ifstrm >> word >> agent.IncomePrevMonth();
	ifstrm >> word >> agent.NetTaxesPrevMonth();
	ifstrm >> word >> agent.IncomeCurr();
	ifstrm >> word >> agent.NetTaxesCurr();

	return ifstrm;
}

void CAgent::initialize()
{
	_PriceAdaptFactor = getInputParameter("PriceAdaptFactor");
	MonthlyActivityMonth() = 999999999; // getRandom01()* getInputParameter("WorkDaysPerMonth");
	GoodsIhave().clear();
	myFixCapital() = 0;
	myFixCapToBuy() = 0;
	purchasedFixCapital() = 0;
	myGFCF() = 0;

	WorkTimeToBuy() = 0;
	GoodsToBuy().clear();
	ToSell().clear(); // Workers: goods from dismantled Producers

	myPrice().clear();
	_myPriceOfSalary = 1.0;
	myDividendYield() = 1.0;
	myShares().clear();
	SharesToTrade() = CShare();
	myValueToBuyShares() = 0;

	NBonds() = 0;
	myBondsValueToTrade() = 0;
	myTotBondsValue() = 0;

	ConsumptionBudget() = 1 * 12 * getSAM().getInitSalary(); // keep some years salary
	InitCash() = 0;
	_Cash = 0;

	pUsedBank() = nullptr;
	delete pmyProducers();
	pmyProducers() = new map<CAgentFID, CProducer*>;

	IncomePrevMonth() = 0;
	IncomeCurr() = 0;
	NetTaxesPrevMonth() = 0;
	NetTaxesCurr() = 0;

	WealthPrevMonth() = 0;

	for (long gType = 0; gType < getSAM().getnPProducerTypes(); ++gType)
		myPrice()[gType] = 1.;
}

bool CAgent::DeleteMyMoney(GoodQtty deleteMyMoney)
{
	if (getInputParameter("MaxNBanks") > 0
		&& (this->IsCentralBank() || this->IsPrivateBank()))
		CWorld::ERRORmsg("DeleteMyMoney called from a Bank", true);

	bool done = true;
	if (deleteMyMoney == 0)
		return done;

	if (getCash() < deleteMyMoney)
	{
		GoodQtty fromBank = deleteMyMoney - getCash();
		done = GetCashFromBank(fromBank);
	}

	if (!done)
		getWorld().ERRORmsg("GetCashFromBank(deleteMyMoney) failed", true);
	else
	{
		Cash() -= deleteMyMoney;
		DEPData().TotalInitialCash() -= deleteMyMoney;
	}

	return done;
};

GoodQtty CAgent::getMoneyHoldings() const { return getCash() + getmyBankBalance(); };
GoodQtty CAgent::getWealth() const { return -1; }

//-------------------------  Accessors  -------------------------
AgentID CAgent::getID() const { return _ID; };
CAgentFID CAgent::getFID() const { return CAgentFID(_agentType, _ID); };
AgentType CAgent::getAgentType() const { return _agentType; }
string CAgent::getAgentName() const {
	string name;

	if (IsWorker())
		name = "Indiv_" + to_string(getID());
	else if (IsProducer())
		name = CProducer::getProducerLabelOfType(getAgentType()) + "_" + to_string(getID());
	else if (IsGovernment())
		name = "Government";
	else if (IsCentralBank())
		name = "CentralBank";
	else if (IsPrivateBank())
		name = "Bank_" + to_string(getID());
	else
		name = getSAM().getAccNameOfN(getAgentType());

	return name;
}
bool CAgent::IsWorker() const { return (getAgentType() == WorkerType); }
bool CAgent::IsGovernment() const { return (getAgentType() == GovernmentType); }
bool CAgent::IsCentralBank() const { return (getAgentType() == getCentralBankType()); }
bool CAgent::IsPrivateBank() const { return (getAgentType() == PrivateBankType()); }
bool CAgent::IsExtSect() const { return getSAM().IsExtSectType(getAgentType()); }
bool CAgent::IsProducer() const { return getAgentType() >= 0; }
long& CAgent::initialMonth() { return _initialMonth; };
long CAgent::getinitialMonth() const { return _initialMonth; }

// -----------------------  Goods  -----------------------
const CGoods& CAgent::getGoodsIhave() const { return _GoodsIhave; }
const CGoods& CAgent::getGoodsToBuy() const { return _GoodsToBuy; }
CGoods& CAgent::myGoodsIwish() { return _myGoodsIwish; };
const CGoods& CAgent::getmyGoodsIwish() const { return _myGoodsIwish; }
CGoods& CAgent::GoodsIhave() { return _GoodsIhave; }
CGoods& CAgent::ToSell(void) { return _ToSell; };
const CGoods& CAgent::getToSell() const { return _ToSell; }
double& CAgent::WorkTimeToBuy() { return _WorkTimeToBuy; };
double CAgent::getWorkTimeToBuy() const { return _WorkTimeToBuy; };
CGoods& CAgent::GoodsToBuy() { return _GoodsToBuy; };
GoodQtty CAgent::GoodsIhave(GoodType gType)
{
	if (_GoodsIhave.find(gType) == _GoodsIhave.end())
		return 0;

	assert(GoodsIhave()[gType] >= 0);
	return GoodsIhave().at(gType);
}
GoodQtty& CAgent::ToSell(GoodType gType)
{
	assert(ToSell()[gType] >= 0);
	return ToSell()[gType];
}
void CAgent::setGoodsToBuyOf(GoodType gType, GoodQtty qtty)
{
	assert(qtty >= 0);
	_GoodsToBuy[gType] = qtty;
}
const GoodQtty CAgent::getGoodsToBuyOf(GoodType gType) const
{
	GoodQtty qtty = _GoodsToBuy(gType);
	assert(qtty >= 0);
	return qtty;
}
const GoodQtty CAgent::getGoodsIhave(GoodType gType) const
{
	if (_GoodsIhave.find(gType) == _GoodsIhave.end())
		return 0;

	GoodQtty qtty = _GoodsIhave.at(gType);
	if (qtty < 0)
		getWorld().ERRORmsg("getGoodsIhave <0", true);
	return qtty;
}
const GoodQtty CAgent::getToSell(GoodType gType) const
{
	GoodQtty qtty = _ToSell(gType);
	assert(qtty >= 0);
	return qtty;
}
CGoods CAgent::getInventory()
{
	// mu this month

	CGoods inventory;
	auto CurrentTimeMonth = currMonth();
	if (CurrentTimeMonth == 0)
		return inventory;

	const auto& marketPrices = getDEPData().getMarketPrices();

	for (const auto& gPair : GoodsIhave())
		inventory[gPair.first] += gPair.second * marketPrices.at(gPair.first);

	for (const auto& gPair : ToSell())
		inventory[gPair.first] += gPair.second * marketPrices.at(gPair.first);

	return inventory;
}
GoodQtty CAgent::valGoodsIhave() const
{
	const auto& marketPrices = getDEPData().getMarketPrices();
	GoodQtty gValue = 0;

	for (const auto& gPair : getGoodsIhave())
		gValue += gPair.second * marketPrices.at(gPair.first);

	return gValue;
};
GoodQtty CAgent::valToSell() const
{
	const auto& marketPrices = getDEPData().getMarketPrices();
	GoodQtty gValue = 0;

	for (const auto& gPair : getToSell())
		gValue += gPair.second * marketPrices.at(gPair.first);

	return gValue;
};
CShare CAgent::getSharesToTrade() const
{
	return _SharesToTrade;
};
GoodQtty CAgent::getmyProviderIDof(GoodType gType) const
{
	if (_myProviderID.find(gType) == _myProviderID.end())
		return undefinedGoodType;

	return _myProviderID.at(gType);
};
// -------------------------- Prices --------------------------
CTypeDoubleMap& CAgent::myPrice() { return _myPrice; };
double& CAgent::PriceAdaptFactor() { return _PriceAdaptFactor; }
double CAgent::getPriceAdaptFactor() { return _PriceAdaptFactor; }
CTypeDoubleMap& CAgent::PriceCalibFactor() { return _PriceCalibFactor; }
CTypeDoubleMap CAgent::getPriceCalibFactor() { return _PriceCalibFactor; }
double CAgent::getPriceCalibFactorOfType(GoodType gType)
{
	return CAgent::getPriceCalibFactor().at(gType);
}

void CAgent::setmyPriceOf(const GoodType gType, const double price)
{
	if (price <= 0)
		getWorld().ERRORmsg("price <= 0 in CAgent::setmyPriceOf", true);

	myPrice()[gType] = price;
};
const double CAgent::getmyPriceOf(GoodType gType) const
{
	if (_myPrice.find(gType) == _myPrice.end())
		return 0;

	return _myPrice.at(gType);
};

double CAgent::myBuyerPriceOf(const GoodType gType, const double sellerPrice)
{
	if (getmyPriceOf(gType) == 0)
	{
		assert(sellerPrice > 0);
		myPrice()[gType] = sellerPrice; // first time buyer, learns gType price

		return sellerPrice;
	}

	return getmyPriceOf(gType);
};
const double CAgent::getmySalary_mu() const
{
	return getmyPriceOfSalary() * getWorld().getInitSalaryCalibFactor() * getSAM().getInitSalary();
};

// ------------------- Cash & Bank -------------------
GoodQtty CAgent::getmyValueToBuyShares() const { return min(getMoneyHoldings(), _myValueToBuyShares); };
GoodQtty CAgent::getmyFixCapital() const { return _myFixCapital; };
GoodQtty CAgent::getpurchasedFixCapital() const { return _purchasedFixCapital; };
GoodQtty CAgent::getmyGFCF() const { return _myGFCF; };
void CAgent::WriteTransaction(CAgent* pFromAgent, CAgent* pToAgent, string txt = "")
{
	if (txt == "")
		return;
	ofstream& Outf = LogFile();

	// auto rnd = getRandom01();
	Outf << "\n month " << currMonth()
		// << " rnd " << rnd
		<< " " << pFromAgent->getAgentName()
		<< txt
		<< " " << pToAgent->getAgentName();

	Outf.flush();
};
GoodQtty& CAgent::Cash()
{
	assert(_Cash >= -9999999999 || currMonth() < 1);
	return _Cash;
};
GoodQtty& CAgent::ConsumptionBudget() { return _ConsumptionBudget; };
GoodQtty CAgent::getCash() const { return _Cash; }
GoodQtty CAgent::getConsumptionBudget() const { return _ConsumptionBudget; }
GoodQtty CAgent::getmyBankBalance() const
{
	if (getmyUsedBank() == nullptr
		|| getInputParameter("MaxNBanks") == 0)
		return 0;

	const CAgent* _This = (CAgent*)this;
	return getmyUsedBank()->getClientCurrentBalance(_This);
}
bool CAgent::GetCashFromBank(GoodQtty& loanQtty)
{
	if (loanQtty == 0)
		return true;

	if (loanQtty < 0)
		CWorld::ERRORmsg("loanQtty < 0 in  CAgent::GetCashFromBank", true);

	if (getmyUsedBank() == nullptr && getInputParameter("MaxNBanks") > 0) {
		if (pCentralBank()->getBanks().size() > 0) {
			pUsedBank() = pCentralBank()->getRandomBank();
		}
		else {
			pUsedBank() = pCentralBank();
		}
	}

	if (getmyUsedBank() != nullptr && getmyUsedBank()->IsCentralBank() && pCentralBank()->getBanks().size() > 0) {
		if (!IsPrivateBank() && !IsGovernment()) {
			pUsedBank() = pCentralBank()->getRandomBank();
		}
	}

	if (getmyUsedBank() == nullptr || getInputParameter("MaxNBanks") == 0) // includes ExtSects
		return false;

	auto granted = getmyUsedBank()->GetCashFromBank(*this, loanQtty);
	return granted;
}

bool CAgent::PayTo(CAgent* pToAgent, GoodQtty value, string txt)
{
	if (DebugLevel() > 1 && currMonth() >= getInputParameter("DebugFromMonth"))
	{
		assert(DEPData().CheckAccountingBalance()
			&& Cash() >= 0 && pToAgent->Cash() >= 0
		);
	}

	bool done = true;
	if (value > 0 && getCash() <= value)
		done = GetCashFromBank(value);
	else if (value < 0 && pToAgent->getCash() <= -value) // charged to ToAgent
	{
		auto draw = -value;
		done = pToAgent->PayTo(this, draw, txt);
		return done;
	}

	if (getCash() >= value)
	{
		Cash() -= value;
		if (IsCentralBank() || IsPrivateBank())
			((CBank*)this)->Cash_I_Own() -= value;

		pToAgent->Cash() += value;
		if (pToAgent->IsCentralBank() || pToAgent->IsPrivateBank())
			((CBank*)pToAgent)->Cash_I_Own() += value;
		return true;
	}
	else
	{
		getWorld().ERRORmsg("PayTo error", true);
		return false;
	}
}

void CAgent::monthInitialize()
{
}
void CAgent::RemoveServiceGoods()
{
	for (auto& pair : GoodsIhave())
		pair.second = 0;
};
void CAgent::monthActivity()
{
}
void CAgent::ReturnLoansToMyBank()
{
	if (getmyUsedBank() == nullptr)
		return;

	GoodQtty currentDebt = -getmyBankBalance();

	if (currentDebt <= 0 || Cash() <= 2 * getConsumptionBudget())
		return;

	GoodQtty payed = min(currentDebt, Cash() - getConsumptionBudget());
	if (payed < currentDebt / 120.0)
		return;

	getmyUsedBank()->ClientCashOperation(this, payed);
	if (getmyBankBalance() == 0) // change Bank
		pUsedBank() = pCentralBank()->getRandomBank();
};

// ----------------------- Activity -----------------------

void CAgent::BuyGoods()
{
	// 1. Workers

	if (IsProducer() && WorkTimeToBuy() > 0) // Only Producers can hire workers
	{
		vector<CAgent*> myNeighborWorkers;
		World().getNeighborsWorkersOf(this, myNeighborWorkers);
		if (currMonth() >= 9999) DEPData().CheckAccountingBalance();

		for (auto pNei : myNeighborWorkers)
		{
			CWorker& neiWorker = *(CWorker*)pNei;

			((CProducer*)this)->TryToHireNeighbor(neiWorker); // includes salary readjustments
			if (currMonth() >= 9999) DEPData().CheckAccountingBalance();

			if (WorkTimeToBuy() == 0)
				break; // skip rest of neighbors
		}
	}

	// 2. Goods

	// make a writable copy of the GoodsToBuy and shuffle it
	double IwishSpreadFraction = getInputParameter("IwishSpreadFraction");
	vector< pair<GoodType, GoodQtty> > GoodsToBuy;
	for (const auto& gPair : getGoodsToBuy())
	{
		GoodType gType = gPair.first;
		double buyQtty = gPair.second;
		if (this->IsGovernment() || this->IsExtSect())
		{
			buyQtty = buyQtty / (12.0 * getSAM().getActive());
			buyQtty *= ((1.0 - 0.5 * IwishSpreadFraction) + IwishSpreadFraction * getRandom01());
			buyQtty = doubleToGQtty(buyQtty);
		}

		GoodsToBuy.push_back(pair<GoodType, GoodQtty>(gType, buyQtty));
	}

	std::shuffle(GoodsToBuy.begin(), GoodsToBuy.end(), myRandomEngine());

	// buy each gType
	vector<CAgent*> myNeighborProducers;
	World().getNeighborsProducersOf(this, myNeighborProducers);

	for (const auto& gPair : GoodsToBuy)
	{
		GoodType gType = gPair.first;
		double gQtty = gPair.second;
		if (gQtty == 0)
			continue;

		if ((!IsPrivateBankType(gType) || getInputParameter("MaxNBanks") > 1)
			&& (gType != getCentralBankType() || getInputParameter("MaxNBanks") > 0))
			DEPData().TotDemand()[gType] += gQtty;

		if (getSAM().IsExtSectType(gType)) //IMPORTS: this producer is purchasing imports from ExtSect (gType row)
			BuyFromAgent(ExtSect(gType), gType, gQtty);
		else
		{
			// used in TryToStartupNewProducer
			// Record the demand for this good type at the world level, associated with this agent
			World().addDemandEvent(this->getID(), gType);

			if (currMonth() > getInputParameter("UsePrevProvidersFrom")
				&& getmyProviderIDof(gType) >= 0)
			{
				auto pNei = getWorld().getpProducers()->at(getmyProviderIDof(gType));
				if (pNei != nullptr)
					BuyFromAgent(*pNei, gType, gQtty);
			}

			for (auto pNei : myNeighborProducers)
			{
				if (gQtty == 0)
					break; // done with this gType, skip neighbors, go to next gType

				if (pNei == this)
					continue;

				// buy from neighbor

				BuyFromAgent(*pNei, gType, gQtty);
			}
		}
	}
	if (currMonth() >= 9999) DEPData().CheckAccountingBalance();
};

bool CAgent::HireWorkTime(CAgent& neighbor, double& qtty)
{
	// Hiring workers is a special trade
	if (qtty == 0 || !neighbor.IsWorker())
		return false;

	CWorker& neiIndiv = *(CWorker*)(&neighbor);

	CProducer* pEmployer = (CProducer*)this;
	pEmployer->TryToHireNeighbor(neiIndiv); // includes salary readjustments

	return true;
}

double CAgent::BuyFromAgent(CAgent& neighbor, GoodType gType, double& qtty)
{
	if (currMonth() >= 9999) DEPData().CheckAccountingBalance();
	const GoodQtty NOTRADE = 0;

	CAgent* pBuyer = this;
	CAgent* pSeller = &neighbor;

	//  Producers buy IMPORTS from ExtSect seller (SAM row: seller ExtSect, col: buyer Producer)

	double priceAgreed = 0;
	auto priceSeller = pSeller->getmyPriceOf(gType);
	auto priceBuyer = pBuyer->myBuyerPriceOf(gType, priceSeller);

	double SellerproductionPrice = 0;
	if (pSeller->IsProducer())
		SellerproductionPrice = ((CProducer*)pSeller)->getproductionPrice();

	double AdaptFactor = getPriceAdaptFactor(); // 1.005
	double AdaptFraction = AdaptFactor - 1.0;

	GoodQtty gSellMax = pSeller->getToSell(gType);

	if (pBuyer->IsGovernment() || pBuyer->IsExtSect() //M J can be improved...?
		|| pSeller->IsGovernment() || pSeller->IsExtSect()) // no choice for buyers
		priceAgreed = priceSeller;
	else if (priceBuyer >= priceSeller)
	{
		priceAgreed = priceSeller; // usually the price is set by the seller

		if (pSeller->IsProducer() && pSeller->getAgentType() == gType)
			((CProducer*)pSeller)->myDemand() += qtty;

		if (gSellMax == 0)
		{
			return NOTRADE;
		}

		// If Seller is Producer both traders learn from this SUCCESSFUL Market interaction:
		if (pSeller->IsProducer()) // && !pBuyer->IsGovernment() && !pBuyer->IsExtSect())
		{
			// Seller: increase the price up to productionPrice * MaxPriceFactor
			pSeller->setmyPriceOf(gType,
				min(pSeller->getmyPriceOf(gType) * AdaptFactor,
					SellerproductionPrice * getInputParameter("MaxPriceFactor")));

			// Buyer (firm): lower the salary offered
			pBuyer->setmyPriceOf(gType, pBuyer->getmyPriceOf(gType) / AdaptFactor);
		}
	}
	else
	{
		priceAgreed = 0;

		// If Seller is Producer both traders learn from this FAILED Market interaction:
		if (pSeller->IsProducer())// && !pBuyer->IsGovernment() && !pBuyer->IsExtSect())
		{
			// Seller: lower the price down to productionPrice
//				rndAdaptFactor = (1. + AdaptFraction * World().getRandom01()); // force call to tRandom01()
			pSeller->setmyPriceOf(gType,
				max(pSeller->getmyPriceOf(gType) / AdaptFactor,
					SellerproductionPrice));

			// Buyer: increase the price (up to productionPrice * MaxPriceFactor ?)
//				rndAdaptFactor = (1. + AdaptFraction * World().getRandom01()); // force call to tRandom01()
			pBuyer->setmyPriceOf(gType,
				min(pBuyer->getmyPriceOf(gType) * AdaptFactor,
					SellerproductionPrice * getInputParameter("MaxPriceFactor")));
		}
	}

	if (priceAgreed == 0)
		return NOTRADE;

	// Regular goods. Settle traded quantity

	GoodQtty tradedQuantity = min(qtty, (double)gSellMax);

	if (tradedQuantity == 0)
		return NOTRADE;

	if (tradedQuantity * priceAgreed > (pBuyer->getCash() + pBuyer->getmyBankBalance())
		&& !pBuyer->IsGovernment() && !pBuyer->IsExtSect())
	{
		if (pBuyer->IsWorker())
			return NOTRADE;
		else
		{
			assert(pBuyer->IsProducer());
			GoodQtty fromBank = max(0, tradedQuantity * priceAgreed - 0.9 * pBuyer->getCash());
			bool ok = ((CProducer*)pBuyer)->GetCashFromBank(fromBank);
			if (!ok)
				return NOTRADE;
		}
	}

	// Carry out the trade

	if (pSeller->IsProducer())
	{
		((CProducer*)pSeller)->LastUsedMonth() = currMonth();
		pBuyer->myProviderIDof(gType) = pSeller->getID();
	}

	// 4. Allocate this tradedQuantity and tradedMoney

	GoodQtty tradedQtty =
		allocateBuyersQttyAndMoney(tradedQuantity, priceAgreed, gType, pSeller, pBuyer);
	// services are also stored as goods to avoid re-purchasing, but will be removed at next CWorld::initializeMonth

	qtty -= tradedQtty;

	assert(pBuyer->getGoodsToBuyOf(gType) >= 0);
	if (currMonth() >= 9999) DEPData().CheckAccountingBalance();

	return tradedQuantity;
};

GoodQtty CAgent::allocateBuyersQttyAndMoney(
	GoodQtty tradedQuantity, double priceAgreed, GoodType gType, CAgent* pSeller, CAgent* pBuyer)
{
	assert(pSeller->IsProducer());
	double toTotalPopulationYear = getDEPData().toTotalPopulationYear();
	bool ok = false;
	GoodType GFCFtype = getSAM().GFCFtype();

	auto processTransaction = [&](GoodQtty& IhaveQ, GoodQtty& GFCFQ, double taxProducts, double taxGFCF) {
		GoodQtty totalMoney = round((IhaveQ + GFCFQ) * priceAgreed);
		ok = pBuyer->PayTo(pSeller, totalMoney);
		if (!ok) LogFile() << " ***WARNING: PayTo failed in allocateBuyersQttyAndMoney***, ";

		ok = pBuyer->PayTo(pGovernment(), taxProducts + taxGFCF);
		if (!ok) LogFile() << " ***WARNING: PayTo failed in allocateBuyersQttyAndMoney***, ";

		pSeller->ToSell()[gType] -= tradedQuantity;
		pBuyer->setGoodsToBuyOf(gType, max((GoodQtty)0, pBuyer->getGoodsToBuyOf(gType) - tradedQuantity));
		pBuyer->GoodsIhave()[gType] += IhaveQ;
		pBuyer->GoodsIhave()[GFCFtype] += GFCFQ; // GFCF flow
		pBuyer->myGFCF() += GFCFQ; // GFCF stock

		GoodType buyerCol = pBuyer->getAgentType(); // if PProducer, it is the same as gType
		if (pBuyer->IsWorker())
			buyerCol = ((CWorker*)pBuyer)->getmyHgroupN();
		else if (pBuyer->IsGovernment())
			buyerCol = getSAM().getAccNofName("Government");
		else {
			assert(pBuyer->IsProducer()); // includes ExtSectors as producers
			buyerCol = pBuyer->getAgentType();
		}

		SAM().SAMmonthAt(gType, buyerCol) += IhaveQ * toTotalPopulationYear;
		SAM().SAMmonthAt("GFCF", buyerCol) += GFCFQ * toTotalPopulationYear;
		SAM().SAMmonthAt(gType, "GFCF") += GFCFQ * toTotalPopulationYear;
		SAM().SAMmonthAt("TaxProducts", buyerCol) += taxProducts * toTotalPopulationYear;
		SAM().SAMmonthAt("TaxProducts", "GFCF") += taxGFCF * toTotalPopulationYear;
		SAM().SAMmonthAt("Government", "TaxProducts") += (taxGFCF + taxProducts) * toTotalPopulationYear;

		DEPData().DataGrossOutput_mu(gType) += IhaveQ * priceAgreed * toTotalPopulationYear;
		DEPData().DataGrossOutput_mu(GFCFtype) += GFCFQ * priceAgreed * toTotalPopulationYear;
		DEPData().TotalDataGrossOutput_mu() += tradedQuantity * priceAgreed * toTotalPopulationYear;

		DEPData().GDPnominal() += tradedQuantity * priceAgreed * toTotalPopulationYear;
		DEPData().TotalUnits() += tradedQuantity * toTotalPopulationYear;
		DEPData().GDPtracker().addToGDPComponent(gType, tradedQuantity * toTotalPopulationYear);
		};

	if (pBuyer->IsWorker()) {
		auto HgroupN = ((CWorker*)pBuyer)->getmyHgroupN();
		GoodQtty IhaveQ = tradedQuantity;
		GoodQtty GFCFQ = 0;
		GoodQtty gTotQtty = getSAM().getRowCol(gType, HgroupN);
		if (gTotQtty > 0)
			IhaveQ = doubleToGQtty((double)tradedQuantity
				/ (1.0 + ((double)getSAM().getRowCol("GFCF", HgroupN) * SAM().GFCFfractionOf(gType)) / gTotQtty));

		GFCFQ = max((GoodQtty)0, tradedQuantity - IhaveQ);

		double taxProducts = IhaveQ * priceAgreed * getGovernment().getTaxHouseholdProducts().at(HgroupN);
		double taxGFCF = GFCFQ * priceAgreed * getGovernment().getTaxGFCF();
		processTransaction(IhaveQ, GFCFQ, taxProducts, taxGFCF);
		DEPData().CPItracker().addToCPIComponents(gType, tradedQuantity * toTotalPopulationYear);
	}
	else if (pBuyer->IsGovernment()) {
		GoodQtty IhaveQ = tradedQuantity;
		GoodQtty GFCFQ = 0;
		GoodQtty gTotQtty = getSAM().getRowCol(gType, "Government");
		if (gTotQtty > 0)
			IhaveQ = doubleToGQtty((double)tradedQuantity
				/ (1.0 + ((double)getSAM().getRowCol("GFCF", "Government") * SAM().GFCFfractionOf(gType)) / gTotQtty));

		GFCFQ = max((GoodQtty)0, tradedQuantity - IhaveQ);

		double taxProducts = IhaveQ * priceAgreed * getGovernment().getTaxGovernmentProducts();
		double taxGFCF = GFCFQ * priceAgreed * getGovernment().getTaxGFCF();
		processTransaction(IhaveQ, GFCFQ, taxProducts, taxGFCF);
	}
	// EXPORTS: this producer (SAM row) is selling to ExtSect buyer (col)
	else if (pBuyer->IsExtSect()) {
		auto ExtSectType = pBuyer->getAgentType();
		GoodQtty IhaveQ = tradedQuantity;
		GoodQtty GFCFQ = 0;
		GoodQtty gTotQtty = (getWorld().getpFigaro() == nullptr
			|| ((CExtSect*)pBuyer)->name() == "RW"
			|| currMonth() <= getInputParameter("ReadIOfilesFromMonth"))
			? getSAM().getRowCol(gType, ExtSectType)
			: ((CExtSect*)pBuyer)->Exports_Init()[gType];
		if (gTotQtty > 0)
			IhaveQ = doubleToGQtty((double)tradedQuantity
				/ (1.0 + ((double)getSAM().getRowCol("GFCF", ExtSectType) * SAM().GFCFfractionOf(gType)) / gTotQtty));

		GFCFQ = max((GoodQtty)0, tradedQuantity - IhaveQ);

		double taxProducts = IhaveQ * priceAgreed * getGovernment().getTaxSectExtProducts().at(ExtSectType);
		double taxGFCF = GFCFQ * priceAgreed * getGovernment().getTaxGFCF();
		processTransaction(IhaveQ, GFCFQ, taxProducts, taxGFCF);
		((CExtSect*)pBuyer)->Exports()[gType] += priceAgreed * IhaveQ * toTotalPopulationYear;
	}
	// Interm. consumption & IMPORTS
	else {
		GoodQtty tradedMoney = round(priceAgreed * tradedQuantity);
		ok = pBuyer->PayTo(pSeller, tradedMoney);
		if (!ok && DebugLevel() > 0 && currMonth() >= getInputParameter("DebugFromMonth")) {
			CWorld::ERRORmsg("pBuyer->PayTo(pSeller failed", true);
		}
		else {
			pBuyer->GoodsIhave()[gType] += tradedQuantity;
			pBuyer->setGoodsToBuyOf(gType, max((GoodQtty)0, pBuyer->getGoodsToBuyOf(gType) - tradedQuantity));
			pSeller->ToSell()[gType] -= tradedQuantity;
			if (getSAM().IsExtSectType(gType))
				// IMPORTS: this Producer buyer (col) is purchasing imports from ExtSect (row)
			{
				((CExtSect*)pSeller)->Imports()[pBuyer->getAgentType()] += priceAgreed * tradedQuantity * toTotalPopulationYear;
				DEPData().GDPnominal() -= tradedQuantity * priceAgreed * toTotalPopulationYear;
				DEPData().TotalUnits() -= tradedQuantity * toTotalPopulationYear;
				DEPData().GDPtracker().addToGDPComponent(gType, -tradedQuantity * toTotalPopulationYear);

			}
			// Interm. consumption between two domestic PProducers
			else {
				DEPData().DataGrossOutput_mu(gType) += priceAgreed * tradedQuantity * toTotalPopulationYear;
				DEPData().TotalDataGrossOutput_mu() += priceAgreed * tradedQuantity * toTotalPopulationYear;
			}
			SAM().SAMmonthAt(gType, pBuyer->getAgentType()) += tradedQuantity * toTotalPopulationYear;
			assert(pSeller->ToSell()[gType] >= 0);
		}
	}

	return tradedQuantity;
}
