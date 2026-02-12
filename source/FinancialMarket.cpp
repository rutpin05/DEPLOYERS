
// FinancialMarket.cpp

#include "./pch.h"

/*
	CBond {
		GoodQtty _faceValue; // "valor nominal" $1000
		double _couponRate; // "tasa" 6%
		GoodQtty _couponDate; // months, every 6 months
		GoodQtty _maturityDate; // as absolutemonths, "vencimiento", = emissionDate + 10 years * 12 months/year
		GoodQtty _price; // current price $1.077,10 if the discount rate is 5% (see below)
	}

	Here are some things to know about how bonds work:
	A bond is a unit of debt that can be traded like an asset such as a stock.
	With a bond, a buyer purchases a chunk of debt from an issuer, acting like a lender.
	In exchange, the issuer pays a small interest fee regularly.
	Those who buy bonds are loaning money to the issuer for a fixed period of time.
	At the end of that period, the value of the bond is repaid.
	Investors also receive a pre-determined interest rate (the coupon) - usually paid annually.
	Bonds are a fixed-income investment, which is a broad asset class.
	Bond issuers, or "debtors," pay regular fixed interest payments to bondholders,
	or "creditors," and return the original amount borrowed at an agreed-upon time when a bond matures.
	Bonds are a key ingredient in a balanced portfolio

	Suppose IBM issues a $1,000 6% bond that matures in 10 years.
	This means that IBM pays annual interest of $60 ($1,000 x 6%) and repays the $1,000
	at the end of 10 years. The present value of the bond is the sum of the present values
	of the interest and principal. To calculate the present value, we need a discount rate,
	which is the yield that the market would demand to buy the bond. Assume that the discount rate
	is 5% per year. Then, the present value of the interest is:

	$60 x (1 - 1/(1 + 0,05)^10) / 0,05 = $463,19

	And the present value of the principal is:

	$1.000 / (1 + 0,05)^10 = $613,91

	Therefore, the present value of the bond is:

	$463,19 + $613,91 = $1.077,10

	This means that the bond is trading above par, since its value is greater than its face value.

	read://https_economipedia.com/

*/

CBond::CBond()
{
	_faceValue = 0;
	_couponRate = 0;
	_purchaseDate = 0;
	_couponDate = 0;
	_maturityDate = 0;

	_price = 0;
}
CBond::CBond(GoodQtty faceValue, double couponRate, GoodQtty emissionDate,
	GoodQtty couponDate, GoodQtty maturityDate)
{
	_faceValue = faceValue;
	_couponRate = couponRate;
	_purchaseDate = emissionDate;
	_couponDate = couponDate;
	_maturityDate = maturityDate;
	_price = 0;
};

ofstream& operator<<(ofstream& ofstrm, const CBond& b)
{
	ofstrm << " faceValue " << b._faceValue << " couponRate " << b._couponRate
		<< " emissionDate " << b._purchaseDate << " couponDate " << b._couponDate
		<< " maturityDate " << b._maturityDate << " price " << b._price << endl;
	return ofstrm;
}
ifstream& operator>>(ifstream& ifstrm, CBond& b)
{
	string word;
	ifstrm >> word >> b._faceValue >> word >> b._couponRate
		>> word >> b._purchaseDate >> word >> b._couponDate
		>> word >> b._maturityDate >> word >> b._price;
	return ifstrm;
}

double CBond::updatePrice(double discountRate)
{
	double coupon = _faceValue * _couponRate;
	GoodQtty nMonths = 1 + (GoodQtty)round((_couponDate - currMonth()) / 12.);
	double presentValOfInterest =
		coupon * (1. - 1. / pow(1. + discountRate, nMonths)) / discountRate;
	double presentValueOfPrincipal = _faceValue / pow(1. + discountRate, nMonths);
	_price = presentValueOfPrincipal + presentValOfInterest;
	return _price;
};

//==================  CFinancialMarket  ==========================

// At emission all shares represent the same equity value (e.g. 1000 mu)
// Producers start the Financial Market through the emission of new shares
// Every month Households set their NSharesToTrade value [>0:buy, <0:sell]

CFinancialMarket::CFinancialMarket()
{
	_pSellerSHolders = new vector<AgentSortedByFirst>;
	_pBuyerSHolders = new vector<AgentSortedByFirst>;

	_pSellerBHolders = new vector<AgentSortedByFirst>;
	_pBuyerBHolders = new vector<AgentSortedByFirst>;
	_pBHolderNbonds = new map<AgentID, GoodQtty>;

	_pSortedProducersByDividend = new vector<AgentSortedByFirst>;
	initialize();
};
CFinancialMarket::~CFinancialMarket() {};

ofstream& operator<<(ofstream& ofstrm, const CFinancialMarket& financialMarket)
{
	ofstrm << " _InitShareValue " << financialMarket._InitShareValue
		<< " _RateOfPrice " << financialMarket._RateOfPrice
		<< " _PriceAdjustSpeed " << financialMarket._PriceAdjustSpeed
		<< " _Price " << financialMarket._Price
		<< " _NextPrice " << financialMarket._NextPrice
		<< " _UpRateLimit " << financialMarket._UpRateLimit
		<< " _DownRateLimit " << financialMarket._DownRateLimit;

	return ofstrm;
}
ifstream& operator>>(ifstream& ifstrm, CFinancialMarket& financialMarket)
{
	string word;
	ifstrm >> word >> financialMarket._InitShareValue
		>> word >> financialMarket._RateOfPrice
		>> word >> financialMarket._PriceAdjustSpeed
		>> word >> financialMarket._Price
		>> word >> financialMarket._NextPrice
		>> word >> financialMarket._UpRateLimit
		>> word >> financialMarket._DownRateLimit;

	return ifstrm;
}

void CFinancialMarket::initialize()
{
	InitShareValue() = 1000.;
	RateOfPrice() = 1.0;
	PriceAdjustSpeed() = 1.0;
	Price() = 1000.0;
	NextPrice() = 1.0;
	UpRateLimit() = 1.10;
	DownRateLimit() = 0.90;

	SellerSHolders().clear();
	BuyerSHolders().clear();
	SortedProducersByDividend().clear();
}

// --------------------   SHARES   -------------------

void CFinancialMarket::CollectSharesSellersAndBuyers()
{
	SellerSHolders().clear();
	BuyerSHolders().clear();

	// Producers selling NEW Equity shares to finance production   ---------------

	for (auto pProd : *World().pProducers()) // sorted by increasing ID
	{
		// sort seller Producers starting by highest dividend profitability

		if (pProd != nullptr && pProd->getWealth() > 0 && pProd->getSharesToTrade().second < 0) // second < 0: to sell
		{
			auto SHolder = AgentSortedByFirst(pProd->getmyDividendYield(), pProd);

			SellerSHolders().push_back(SHolder); // seller Producers sorted by increasing ID
		}
	}

	// Household Sellers   ------------------------------------------------

	if (getInputParameter("UseSecondaryMarket"))
	{
		for (auto pIndiv : *World().pWorkers()) // sorted by increasing ID
		{
			auto myValueToBuyShares = pIndiv->getmyValueToBuyShares();
			if (myValueToBuyShares >= 0)
				continue; // not selling

			// SellerIndiv

			if (pIndiv->getSharesToTrade().second >= 0)
				continue; // not selling

			const CProducer producer = *getWorld().getpProducers()->at(pIndiv->getSharesToTrade().first._id);
			auto SHolder = AgentSortedByFirst(producer.getmyDividendYield(), pIndiv);
			SellerSHolders().push_back(SHolder);
		}
	}

	// Seller Producers followed by seller Households, by increasing ID. Shuffle and sort by decreasing PriceOf(GFCFtype)

	std::shuffle(SellerSHolders().begin(), SellerSHolders().end(), myRandomEngine());
	std::sort(SellerSHolders().begin(), SellerSHolders().end(), mygreater<AgentSortedByFirst>());

	// Household Buyers   ------------------------------------------------

	for (auto pIndiv : *World().pWorkers()) // sorted by increasing ID
	{
		auto myValueToBuyShares = pIndiv->getmyValueToBuyShares();
		if (myValueToBuyShares <= 0)
			continue; // not buying

		auto SHolder = AgentSortedByFirst(pIndiv->getmyDividendYield(), pIndiv);
		BuyerSHolders().push_back(SHolder);
	}

	// ExtSect Buyers   ------------------------------------------------

	for (auto& pair : ExtSectors())
	{
		auto& extSect = *pair.second;
		auto myValueToBuyShares = extSect.getmyValueToBuyShares();
		if (myValueToBuyShares > 0)
		{
			auto SHolder = AgentSortedByFirst(extSect.getmyDividendYield(), &extSect);
			BuyerSHolders().push_back(SHolder);
		}
	}

	// Government Buyer   ------------------------------------------------

	auto myValueToBuyShares = getGovernment().getmyValueToBuyShares();
	if (myValueToBuyShares > 0)
	{
		auto SHolder = AgentSortedByFirst(getGovernment().getmyDividendYield(), pGovernment());
		BuyerSHolders().push_back(SHolder);
	}

	// Buyers. Shuffle and sort by decreasing PriceOf(GFCFtype)

	std::shuffle(BuyerSHolders().begin(), BuyerSHolders().end(), myRandomEngine());
	std::sort(BuyerSHolders().begin(), BuyerSHolders().end(), mygreater<AgentSortedByFirst>());
}

void CFinancialMarket::SharesTradeActivity()
{
	if (BuyerSHolders().size() == 0 || SellerSHolders().size() == 0)
		return;

	// Prices are equal to the dividend yield of the producer

	double toTotalPopulationYear = getDEPData().toTotalPopulationYear();
	GoodType GFCFtype = getSAM().GFCFtype();
	for (auto& buyerpair : BuyerSHolders()) // start from high buy price (DividendYield)
	{
		auto& buyerPrice = buyerpair.first;
		auto pBuyer = buyerpair.second;
		auto buyerAgType = pBuyer->getAgentType();

		if (pBuyer->getmyValueToBuyShares() == 0)
			continue;

		for (auto& sellerpair : SellerSHolders()) // start from high sellerPrice
		{
			auto& sellerPrice = sellerpair.first;
			auto pSeller = sellerpair.second;
			auto sellerAType = pSeller->getAgentType();
			//double sellValue = -pSeller->getSharesToTrade().second * pSeller->getmyCurrShareVal(); // minus because it comes as negative to sell
			CShare sellShares = pSeller->getSharesToTrade(); // share: <FID, nShares>
			auto sellNShares = -sellShares.second; // sell: second is negative

			if (sellNShares <= 0)
				continue; // next seller

			if (buyerPrice == 1 // ==1: initial purchase
				|| buyerPrice >= sellerPrice)
			{
				// PrimaryMarket: Producer seller, emission of new shares, increase FixCapital
				if (pSeller->IsProducer())
				{
					auto pSellerProducer = (CProducer*)pSeller;

					double availabletoBuyShares = min(pBuyer->getmyValueToBuyShares(),
						(pBuyer->getCash() + pBuyer->getmyBankBalance()) - 4 * pBuyer->getConsumptionBudget());//M J 4?
					GoodQtty buyNshares = floor(availabletoBuyShares / pSellerProducer->getmyCurrShareVal());
					if (buyNshares <= 0)
						continue;

					GoodQtty nTradedShares = min(buyNshares, sellNShares);
					double Tradedvalue = nTradedShares * pSellerProducer->getmyCurrShareVal();
					bool ok = pBuyer->PayTo(pSellerProducer, Tradedvalue, "");
					if (ok)
					{
						// -------------   Buyer   -----------------------------------

						pBuyer->myShares(pSellerProducer->getFID()) = nTradedShares;
						pBuyer->myValueToBuyShares() = max((GoodQtty)0, pBuyer->myValueToBuyShares() - (GoodQtty)Tradedvalue);
						pBuyer->pmyProducer(pSellerProducer->getFID()) = pSellerProducer;
						pBuyer->myDividendYield() = pSeller->myDividendYield(); //M J

						if (pBuyer->IsWorker())
							SAM().SAMmonthAt("GFCF", ((CWorker*)pBuyer)->getmyHgroupN()) += Tradedvalue * toTotalPopulationYear;
						else if (pBuyer->IsGovernment())
							SAM().SAMmonthAt("GFCF", "Government") += Tradedvalue * toTotalPopulationYear;
						else if (pBuyer->IsExtSect())
						{
							((CExtSect*)pBuyer)->Exports()[getSAM().GFCFtype()] += Tradedvalue * toTotalPopulationYear;
							SAM().SAMmonthAt("GFCF", buyerAgType) += Tradedvalue * toTotalPopulationYear;
						}

						// -------------   SellerProducer   -----------------------------------

						pSellerProducer->myShares(pSellerProducer->getFID()) -= nTradedShares; // < 0 for producers
						pSellerProducer->SharesToTrade() += nTradedShares; // sell: SharesToTrade qtty is negative
						(*pSellerProducer->pOwners())[pBuyer->getFID()] = pBuyer;
						pSellerProducer->InFinancialMarket() = true;
					}
					else
						CWorld::ERRORmsg("pBuyer->PayTo(pSellerProducer, Sharesvalue failed", true);
				}
				else if ((bool)getInputParameter("UseSecondaryMarket"))
				{
					assert(pSeller->IsWorker()); // for the time being...
					CWorker* pSellerIndiv = (CWorker*)pSeller;
					auto producerFID = pSellerIndiv->SharesToTrade().first;
					CProducer& producer = *World().pProducers()->at(producerFID._id);

					auto sellValue = -producer.getmyCurrShareVal() * pSellerIndiv->SharesToTrade().second; // second is negative for selling
					GoodQtty TradedValue = min(pBuyer->getmyValueToBuyShares(), (GoodQtty)sellValue);
					GoodQtty nTradedShares = floor((double)TradedValue / producer.getmyCurrShareVal());
					TradedValue = nTradedShares * producer.getmyCurrShareVal();
					bool ok = pBuyer->PayTo(pSellerIndiv, TradedValue, "");
					if (ok)
					{
						// -------------   Buyer   -----------------------------------

						pBuyer->myShares(producerFID) = nTradedShares;
						pBuyer->SharesToTrade() -= nTradedShares; // buyer: SharesToTrade qtty is positive

						pBuyer->pmyProducers()->at(producerFID) = &producer;
						(*producer.pOwners())[pBuyer->getFID()] = pBuyer;

						// -------------   SellerIndiv   -----------------------------------

						pSellerIndiv->myShares(producerFID) -= nTradedShares;
						assert(pSellerIndiv->myShares(producerFID) >= 0);
						if (pSellerIndiv->myShares(producerFID) == 0)
						{
							pSellerIndiv->myShares().erase(producerFID);
							producer.Owners().erase(pSellerIndiv->getFID());
						}

						pSellerIndiv->SharesToTrade() += nTradedShares; // seller: NSharesToTrade qtty is negative

						pSellerIndiv->myDividendYield() *= CAgent::getPriceAdaptFactor(); //M J
						pBuyer->myDividendYield() /= CAgent::getPriceAdaptFactor();
					}
					else
						CWorld::ERRORmsg("pBuyer->PayTo(pSellerIndiv, Sharesvalue failed", true);
				}
			}
			else // buyerPrice < sellerPrice
			{
				if (pSeller->IsWorker())
				{
					CWorker* pSellerIndiv = (CWorker*)pSeller;
					if (pSellerIndiv->getpmyProducers() == pBuyer->getpmyProducers())
						// two owners of the same producer
					{
						pBuyer->myDividendYield() *= CAgent::getPriceAdaptFactor(); //M J
						pSellerIndiv->myDividendYield() /= CAgent::getPriceAdaptFactor();
					}
				}
				else // SellerProducer and IndivBuyer
				{
					pBuyer->myDividendYield() *= CAgent::getPriceAdaptFactor(); //M J
					// myDividendYield() is computed in producer::PayDividends
				}
			}
		}
	}
}

// --------------------   BONDS   -------------------

void CFinancialMarket::CollectBondsSellersAndBuyers()
{
	SellerBHolders().clear();
	BuyerBHolders().clear();

	// Government issuing new bonds
	if (Government().nBondsEmission() > 0)
	{
		auto price = getGovernment().getmyBondTemplate()._price;
		auto BHolder = AgentSortedByFirst(price, (CAgent*)getpGovernment());

		SellerBHolders().push_back(BHolder); // sellers sorted by increasing ID
	}

	for (auto pIndiv : *World().pWorkers()) // sorted by increasing ID
	{
		auto myBondsValueToTrade = pIndiv->getmyBondsValueToTrade();
		if (myBondsValueToTrade <= 0)
			continue; // not buying

		auto BHolder = AgentSortedByFirst(pIndiv->myPriceOfBond(), pIndiv);
		BuyerBHolders().push_back(BHolder);
	}

	std::shuffle(BuyerSHolders().begin(), BuyerSHolders().end(), myRandomEngine());
	//M J don't sort, all have the same price
	//std::sort(BuyerSHolders().begin(), BuyerSHolders().end(), mygreater<AgentSortedByFirst>());
}
void CFinancialMarket::BondsTradeActivity()
{
	if (BuyerBHolders().size() == 0)
		return;

	if (Government().nBondsEmission() == 0)
		return;

	CBond& bondTemplate = Government().myBondTemplate();
	auto& sellerPrice = bondTemplate._price;

	for (auto& buyerpair : BuyerBHolders())
	{
		if (Government().nBondsEmission() == 0)
			break; // sold out

		auto buyerAgType = buyerpair.second->getAgentType();
		assert(buyerAgType == WorkerType);
		auto pIndivBuyer = (CWorker*)(buyerpair.second);
		auto& buyerPrice = pIndivBuyer->myBond()._price;
		assert(buyerPrice == buyerpair.first);

		double buyValue = pIndivBuyer->getmyBondsValueToTrade();
		if (buyValue == 0)
			continue;

		if (buyerPrice == 0 // first time buyer
			|| buyerPrice >= sellerPrice)
		{
			GoodQtty buyNbonds = floor(buyValue / Government().getmyBondTemplate()._price);
			GoodQtty nTradedBonds = min(buyNbonds, Government().nBondsEmission());
			double Tradedvalue = nTradedBonds * bondTemplate._price;
			bool ok = pIndivBuyer->PayTo(pGovernment(), Tradedvalue, "");
			if (ok)
			{
				// IndivBuyer: keep a copy in myBond
				pIndivBuyer->myBond()._faceValue = bondTemplate._faceValue;
				pIndivBuyer->myBond()._couponRate = bondTemplate._couponRate;
				pIndivBuyer->myBond()._couponDate = bondTemplate._couponDate; // every n months
				pIndivBuyer->myBond()._maturityDate = bondTemplate._maturityDate;

				pIndivBuyer->myBond()._purchaseDate = currMonth();
				pIndivBuyer->myBond()._price = 0; // update it at sell time

				pIndivBuyer->myBondsValueToTrade() -= Tradedvalue;
				pIndivBuyer->myTotBondsValue() += Tradedvalue;
				pIndivBuyer->NBonds() += nTradedBonds;

				Government().myTotBondsValue() += Tradedvalue;
				Government().nBondsEmission() -= nTradedBonds; // bonds for sale
				Government().NBonds() += nTradedBonds; // N active bonds
			}
			else
				CWorld::ERRORmsg("pIndivBuyer->PayTo(pSellerGov, Bondsvalue failed", true);
		}
		else // buyerPrice < sellerPrice
		{
			CWorld::ERRORmsg("FinancialMarket::BondsTradeActivity: buyerPrice < sellerPrice", true);
			/*
			if (pSeller->IsWorker())
			{
				CWorker* pSellerIndiv = (CWorker*)pSeller;
				if (pSellerIndiv->getpmyProducers() == pIndivBuyer->getpmyProducers())
					// two owners of the same producer
				{
					pIndivBuyer->setmyPriceOf(GFCFtype, pIndivBuyer->getmyPriceOf(GFCFtype) * CAgent::getPriceAdaptFactor());
					pSellerIndiv->setmyPriceOf(GFCFtype, pSellerIndiv->getmyPriceOf(GFCFtype) / CAgent::getPriceAdaptFactor());
				}
			}
			else // SellerProducer and IndivBuyer
			*/
			{
				pIndivBuyer->myBond()._price *= CAgent::getPriceAdaptFactor();
				// PriceOf(GFCFtype) is computed in producer::PayDividends (dividendYield)
			}
		}
	}
}

// --------------------------------------------------------

void CFinancialMarket::monthActivity()
{
	if (!getInputParameter("FinancialMarket"))
		return;

	CollectSharesSellersAndBuyers(); // Households' buy to Invest & sell to Consume
	SharesTradeActivity();

	CollectBondsSellersAndBuyers(); // Households' buy to Invest & sell to Consume
	BondsTradeActivity();
}

//======================  CAgentFID  ===================================

ofstream& operator<<(ofstream& ofstrm, const CAgentFID& fullID)
{
	ofstrm << " " << fullID._type << " " << fullID._id;
	return ofstrm;
};
ifstream& operator>>(ifstream& ifstrm, CAgentFID& fullID)
{
	ifstrm >> fullID._type >> fullID._id;
	return ifstrm;
};

//======================  CShare  ===================================

ofstream& operator<<(ofstream& ofstrm, const CShare& sh)
{
	ofstrm << sh.first << " " << sh.second; // FID nshares

	return ofstrm;
}
ifstream& operator>>(ifstream& ifstrm, CShare& sh)
{
	ifstrm >> sh.first >> sh.second; // FID nshares

	return ifstrm;
}

//======================  CShares  ===================================

CShares::CShares() { }
CShares::~CShares() { }

ofstream& operator<<(ofstream& ofstrm, const CShares& shs)
{
	ofstrm << " {";
	for (const auto& pair : shs)
	{
		ofstrm << " { ";
		ofstrm << pair.first << " " << pair.second << " }";
	}
	ofstrm << " }";

	return ofstrm;
}
ifstream& operator>>(ifstream& ifstrm, CShares& shs)
{
	string bracket;
	CShare sh;

	ifstrm >> bracket; // shares "{"
	while (ifstrm >> bracket, bracket != "}") // share "{"
	{
		ifstrm >> sh >> bracket;
		shs[sh.first] = sh.second;
	}

	return ifstrm;
}

GoodQtty& CShares::operator[](CAgentFID fid)
{
	auto result = find(fid);
	if (result == end())
		insert(CShare(fid, 0));

	return at(fid);
}
const GoodQtty CShares::operator()(CAgentFID fid) const
{
	auto result = find(fid);
	if (result != end())
	{
		assert(result->second > 0);
		return result->second;
	}
	else
		return 0;
}

CShares& CShares::operator=(const CShares& shs)
{
	clear();
	for (const auto& cgd : shs)
		(*this)[cgd.first] = cgd.second;

	return *this;
}
CShares& CShares::operator+(const CShares& shs) const
{
	CShares* ret = new CShares();
	for (const auto& cgd : shs)
		(*ret)[cgd.first] = (*this).at(cgd.first) + cgd.second;

	return *ret;
}
CShares& CShares::operator+=(const CShares& shs)
{
	for (const auto& cgd : shs)
		(*this)[cgd.first] += cgd.second;

	return *this;
}
CShares& CShares::operator-(const CShares& shs) const
{
	CShares* ret = new CShares();
	for (const auto& cgd : shs)
		(*ret)[cgd.first] = (*this).at(cgd.first) - cgd.second;

	return *ret;
}
CShares& CShares::operator-=(const CShares& shs)
{
	for (const auto& cgd : shs)
		(*this)[cgd.first] -= cgd.second;

	return *this;
}

//============================================
