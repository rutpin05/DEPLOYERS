
// pch.h
// Editor Font, plain text: Lucida console
//   DEPLOYERS v2

//  Self-deployment of Complex Economic Systems

//  time g++ -o DEPLOYERS -std=c++11 -O3 *.cpp *.h
// find /mnt/c/Users/marti/AppData/Local/Microsoft/Linux/HeaderCache/1.0/Ubuntu-22.04/ -name whateveryouwant 2>/dev/null
//  find / -name wait.h 2>/dev/null

/*
Ludwig von Mises "For the purpose of [human] science we must start
from the action of the individual because this is the only thing of
which we can have direct cognition. The idea of a society that could
operate or manifest itself apart from the action of individuals is
absurd. Everything social must in some way be recognizable in the
action of the individual.... Every form of society is operative in
the actions of individuals aiming at definite ends...."

"Almost always the men who achieve these fundamental inventions of
a new paradigm have been either very young or very new to the field
whose paradigm they change."
Thomas S. Kuhn The structure of scientific revolutions (1962)
*/

#ifndef PCH_H
#define PCH_H

#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS

#ifdef _WINDOWS
#include <windows.h>
#endif

// add headers that you want to pre-compile here

#ifdef _WIN32
#define WINDOWS_VERSION
#define DEPLOYERS_GRAPHICS
// #undef DEPLOYERS_GRAPHICS
#else
#define LINUX_VERSION
#undef DEPLOYERS_GRAPHICS
#endif

#include <cassert>
#include <iostream>
#include <array>
#include <set>
#include <unordered_set>
#include <map>
#include <string>
#include <vector>
#include <deque>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <random>
#include <cstdio>
#include <math.h>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <sstream> // For stringstream
#include <thread>
#include <chrono>

#include <numeric>

#ifdef DEPLOYERS_GRAPHICS
#include "GlgClass.h"
#endif

// Use preprocessor directives to include platform-specific headers
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace std;

#undef GOODQTTY_IS_DOUBLE
//#define GOODQTTY_IS_DOUBLE

// see also:CBank::CheckBalance()
#ifdef GOODQTTY_IS_DOUBLE
typedef double GoodQtty;
#else
typedef long long GoodQtty;
#endif

typedef long GoodType;

class CGood;
class CGoods;
const GoodType UndefGoodType = -999;
class CTypeQttyVector;

class CAgent;
typedef long AgentID;
typedef long  AgentType; // -1: Indiv, >=0: associated to P_name
typedef pair<AgentType, AgentID> AgentTypeID;

// Trader types

const AgentType UndefAgentType = -999;
const AgentType WorkerType = -1;
const AgentType GovernmentType = -2;

static AgentType _CentralBankType = -3;
static AgentType _PrivateBankType = -4;
static bool _BanksAreProducers;

bool& BanksAreProducers();
bool getBanksAreProducers();

AgentType& CentralBankType();
const AgentType getCentralBankType();
AgentType& PrivateBankType();
const AgentType getPrivateBankType();
bool IsCentralBankType(GoodType gType);
bool IsPrivateBankType(GoodType gType);
bool IsBankType(GoodType gTy);
bool IsProducerBankType(GoodType gTy);
bool IsNonProducerBankType(GoodType gTy);

// Workers, Producers and Banks IDs: >=0
const AgentID UndefAgentID = UndefAgentType;
const AgentID AllAgentsID = -9;

class CProductSpecs;
class CWorker;
class CProducerSpecs;
class CProducer;
class CGovernment;
class CExtSect;
class CBankEntry;
class CBank;
class CCentralBank;
class CFinancialMarket;
class CWorld;
class CDeployers;
class CAgentFID;
class CShare;
class CShares;
class CSimulatedCountry;
class DataPoint;
class CPlotDefinition;
class CGDPtracker;
class CMovingAverage;
class CAgent;
class CBLEExpectations;

#ifdef DEPLOYERS_GRAPHICS
class CDeployersGUI;
class CColor;
class CPlotVars;
typedef vector<CPlotVars> CPlotsVars; // long: plot index
class CPlotsXYBase;
class CDEPplotWindow;

GlgAppContext& AppContext();

// Global function to update plot scales from World.cpp
void UpdateAllPlotScales(double newNYears);

//======================  CColorDefinitions  ===================================

class CColor
{
public:
	double dR, dG, dB;
	CColor() : dR(0.), dG(0.), dB(0.) {};
	CColor(double r, double g, double b) : dR(r), dG(g), dB(b) {};
};

typedef  map<long, CColor> CColorDefinitions; // long: color index

void defineColors();
CColorDefinitions& ColorDefinitions();

#endif

class CFigaro;
class CAccount;
class CSAM;
class CSHolder;
class CBond;
class CBHolder;

class CData;
class CTypeDoubleMap;
class CStringDoubleMap;
class  CVectorDoubles;
class  C2DVectorDoubles;

mt19937*& pmyRandomEngine();
mt19937& myRandomEngine();
// An array of pointers to plot data points.
typedef std::vector<DataPoint*> DataArrayType;

class SortedAgent
{
public:
	GoodQtty _qtty;
	AgentType _aTy;
	AgentID _ID;

	SortedAgent() { _qtty = 0;  _aTy = UndefAgentType; _ID = UndefAgentID; };
	SortedAgent(GoodQtty qtty, AgentID ID, AgentType aTy)
		: _qtty(qtty), _ID(ID), _aTy(aTy) {
	};
	friend ofstream& operator<<(ofstream& ofstrm, const SortedAgent& sortedAgnt);
	friend ifstream& operator>>(ifstream& ifstrm, SortedAgent& sortedAgnt);
};
ofstream& operator<<(ofstream& ofstrm, const SortedAgent& sortedAgnt);
ifstream& operator>>(ifstream& ifstrm, SortedAgent& sortedAgnt);
bool operator> (const SortedAgent& ag1, const SortedAgent& ag2);
bool operator< (const SortedAgent& ag1, const SortedAgent& ag2);

class ShareHolder
{
public:
	double _val;
	AgentID _ID;
	AgentType _aTy;

	ShareHolder() { _val = 0; _ID = UndefAgentID;  _aTy = UndefAgentType; };
	ShareHolder(double val, AgentID ID, AgentType aTy)
		: _val(val), _ID(ID), _aTy(aTy) {
	};
};
bool operator> (const ShareHolder& ag1, const ShareHolder& ag2);
bool operator< (const ShareHolder& ag1, const ShareHolder& ag2);

typedef pair<double, CAgent*> AgentSortedByFirst; // sort only by .first
bool operator> (const AgentSortedByFirst& p1, const AgentSortedByFirst& p2);
bool operator< (const AgentSortedByFirst& p1, const AgentSortedByFirst& p2);

template <class T> struct mygreater {
	bool operator() (const T& x, const T& y) const
	{
		return x > y;
	}
	typedef T first_argument_type;
	typedef T second_argument_type;
	typedef bool result_type;
};
template <class T> struct myless {
	bool operator() (const T& x, const T& y) const
	{
		return x < y;
	}
	typedef T first_argument_type;
	typedef T second_argument_type;
	typedef bool result_type;
};

const double getInputParameter(string param);

static CWorld* _pWorld = nullptr; // the only one
CWorld*& pWorld();
const CWorld* getpWorld();
CWorld& World();
const CWorld& getWorld();

static CCentralBank* _pCentralBank = nullptr; // the only one
CCentralBank*& pCentralBank();
const CCentralBank* getpCentralBank();
CCentralBank& CentralBank();
const CCentralBank& getCentralBank();

long& InitMonth();
long getInitMonth();
long currMonth();

// Simulation stage enumeration for runtime stage tracking
enum class SimulationStage {
	Initialization,          // Month 0: Setting up agents and SAM
	PreCalibration,          // Before StartCalibrationAt: Frozen salaries, sequential producers
	AssistedCalibration,     // StartCalibrationAt to AssistedProductionUpto: Production subsidized
	TransitionCalibration,   // AssistedProductionUpto to FinishCalibrationAt: Smooth transition
	RealMarketSimulation     // After FinishCalibrationAt: Free market dynamics
};

SimulationStage currSimulationStage();
const char* getSimulationStageName(SimulationStage stage);
bool isPreCalibration();
bool isAssistedCalibration();
bool isTransitionCalibration();
bool isRealMarketSimulation();  // Renamed from isPostCalibration
bool isCalibrationPhase();  // True during any calibration phase (Assisted or Transition)

const double getRandom01();
GoodQtty doubleToGQtty(double d);

CSAM*& pSAM();
const CSAM* getpSAM();
CSAM& SAM();
const CSAM& getSAM();

map< string, vector<CAccount*> >& AccountGroups();
const map< string, vector<CAccount*> >& getAccountGroups();
void addToAccountToGroup(CAccount* pESect, string str);

CAccount* pAccount(GoodType gType);
const CAccount* getpAccount(GoodType gType);
CAccount& Account(GoodType gType);
const CAccount& getAccount(GoodType gType);

CGovernment*& pGovernment();
const CGovernment* getpGovernment();
CGovernment& Government();
const CGovernment& getGovernment();

map<GoodType, CExtSect*>& ExtSectors();
const map<GoodType, CExtSect*>& getExtSectors();
void addExtSect(GoodType gType, CExtSect* pESect);

CExtSect*& pExtSect(GoodType gType);
const CExtSect* getpExtSect(GoodType gType);
CExtSect& ExtSect(GoodType gType);
const CExtSect& getExtSect(GoodType gType);

static CData* _pCData = nullptr;
CData*& pDEPData();
const CData* getpDEPData();
CData& DEPData();
const CData& getDEPData();

static ofstream* _pLogf;
ofstream*& pLogFile();
ofstream& LogFile();

static ofstream* _pExternalSectorsIO;
ofstream& ExternalSectorsIO();

static long _DebugLevel = 0;
long& DebugLevel();

// Use a cross-platform sleep function
static void crossPlatformSleep(double seconds) {
#ifdef _WIN32
	Sleep((DWORD)(seconds * 1000)); // Windows API sleep function expects milliseconds
#else
	//	sleep(seconds); // POSIX sleep function expects seconds
	std::this_thread::sleep_for(std::chrono::milliseconds((int)(seconds * 1000.)));
#endif
}

typedef enum
{
	BUTTON_PRESS = 0,
	RESIZE,
	MOUSE_MOVE
} EventType;

typedef vector< CAgent* > CNeighbors;
typedef vector< CWorker* > CNeiIndivs;
typedef vector< CProducer* > CNeiProducers;

//======================  CMovingAverage  ===================================

class CMovingAverage {
private:
	deque<double> window;
	size_t windowSize;
	double sum;

public:
	CMovingAverage();
	CMovingAverage(size_t size);


	// Clear all values from the window
	void clear();

	// Add a new value and return the new average
	double update(double newValue);
	double getCurrentAverage() const;
};

//======================  CTypeQttyVector  ===================================

class CTypeQttyVector : public vector<GoodQtty>
{
public:
	CTypeQttyVector() {};
	CTypeQttyVector(int sz, int def = 0)
	{
		resize(sz, def);
	};
	~CTypeQttyVector() {};

	friend ofstream& operator<<(ofstream& ofstrm, const CTypeQttyVector& entry);
	friend ifstream& operator>>(ifstream& ifstrm, CTypeQttyVector& entry);
};

//======================  CVectorDoubles  ===================================

class  CVectorDoubles : public vector<double>
{
	friend ofstream& operator<<(ofstream& ofstrm, const CVectorDoubles& curve);
	friend ifstream& operator>>(ifstream& ifstrm, CVectorDoubles& curve);
public:

	CVectorDoubles() = default;
};

ofstream& operator<<(ofstream& ofstrm, const CVectorDoubles& curve);
ifstream& operator>>(ifstream& ifstrm, CVectorDoubles& curve);

//======================  C2DVectorDoubles  ===================================

class  C2DVectorDoubles : public vector<CVectorDoubles>
{
	friend ofstream& operator<<(ofstream& ofstrm, const C2DVectorDoubles& curves);
	friend ifstream& operator>>(ifstream& ifstrm, C2DVectorDoubles& curves);
public:

	C2DVectorDoubles() = default;
};

ofstream& operator<<(ofstream& ofstrm, const C2DVectorDoubles& curves);
ifstream& operator>>(ifstream& ifstrm, C2DVectorDoubles& curves);

//======================  CTypeDoubleMap  ===================================

class CTypeDoubleMap : public map<GoodType, double>
{
public:

	CTypeDoubleMap() = default;
};

ofstream& operator<<(ofstream& ofstrm, const CTypeDoubleMap& mapTyDou);
ifstream& operator>>(ifstream& ifstrm, CTypeDoubleMap& mapTyDou);

//======================  CStringDoubleMap  ===================================

class CStringDoubleMap : public map<string, double>
{
public:

	CStringDoubleMap() = default;
};

ofstream& operator<<(ofstream& ofstrm, const CStringDoubleMap& mapStrDou);
ifstream& operator>>(ifstream& ifstrm, CStringDoubleMap& mapStrDou);

//////////////////////// 1. GOODS  //////////////////////////////////////

//======================  CGood  ===================================

const GoodType undefinedGoodType = -1;
class CGood
{
private:
	GoodType _type;
	GoodQtty _quantity;


public:
	CGood();
	CGood(GoodType gTy, GoodQtty q);
	CGood(const CGood& cg);

	GoodType& type();
	GoodQtty& quantity();

	GoodType getType() const;
	GoodQtty getQuantity() const;

	CGood& operator=(const CGood& g);
	CGood& operator+=(const CGood& g);
	CGood& operator-=(const CGood& g);

	CGood operator+(const CGood& g) const;
	CGood operator-(const CGood& g) const;

	bool operator==(const CGood& g) const;
	bool operator!=(const CGood& g) const;
	bool operator>(const CGood& g) const;
	bool operator<(const CGood& g) const;
	bool operator>=(const CGood& g) const;
	bool operator<=(const CGood& g) const;

	CGood& operator=(GoodQtty q);
	CGood& operator+=(GoodQtty q);
	CGood& operator-=(GoodQtty q);
	CGood& operator*=(double d);
	CGood& operator/=(double d);

	CGood operator*(double d) const;
	CGood operator/(double d) const;
	CGood operator+(GoodQtty q) const;
	CGood operator-(GoodQtty q) const;

	bool operator==(GoodQtty q) const;
	bool operator!=(GoodQtty q) const;
	bool operator>(GoodQtty q) const;
	bool operator<(GoodQtty q) const;
	bool operator>=(GoodQtty q) const;
	bool operator<=(GoodQtty q) const;

	friend CGoods& operator+=(CGoods& thisGoods, const CGood& cgood);
	friend CGoods& operator-=(CGoods& thisGoods, const CGood& cgood);

	friend ofstream& operator<<(ofstream& ofstrm, const CGood& cgood);
	friend ifstream& operator>>(ifstream& ifstrm, CGood& good);
};

ofstream& operator<<(ofstream& ofstrm, const CGood& cgood);
ifstream& operator>>(ifstream& ifstrm, CGood& good);

CGoods& operator+=(CGoods& thisGoods, const CGood& cgd);
CGoods& operator-=(CGoods& thisGoods, const CGood& cgd);
CGood operator*(double d, const CGood& g);
//======================  CGoods  ===================================

class CGoods : public map< GoodType, GoodQtty >
{
private:
	static vector<string> _GoodTypeToName;
	static map< string, GoodType > _GoodNameToType;

public:
	CGoods();
	~CGoods();

	static void clearGoodsDefinitions();
	static void defineNewGoodType(const string& name);

	//   accessors
	static vector<string>& vGoodTypeToName();
	static const vector<string>& GoodTypeToName();
	static const string& GoodTypeToName(GoodType gTy);
	static map< string, GoodType >& mGoodNameToType();
	static const map< string, GoodType >& GoodNameToType();
	static GoodType GoodNameToType(const string& gname);

	GoodQtty& operator[](GoodType gType);
	GoodQtty operator()(GoodType gType) const;
	CGoods& operator=(const CGoods& cgds);
	CGoods operator+(const CGoods& cgds) const;
	CGoods& operator+=(const CGoods& cgds);
	CGoods operator-(const CGoods& cgds) const;
	CGoods& operator-=(const CGoods& cgds);

	friend ofstream& operator<<(ofstream& ofstrm, const CGoods& cgoods);
	friend ifstream& operator>>(ifstream& ifstrm, CGoods& goods);
};

ofstream& operator<<(ofstream& ofstrm, const CGoods& cgoods);
ifstream& operator>>(ifstream& ifstrm, CGoods& goods);

//======================  CAgentFID  ===================================

class CAgentFID
{
public:
	AgentType _type;
	AgentID _id;

	CAgentFID() : _type(UndefAgentType), _id(UndefAgentID) {};
	CAgentFID(AgentType type, AgentID id) : _type(type), _id(id) {};

	const CAgentFID& operator=(const CAgentFID& fullID)
	{
		_id = fullID._id;
		_type = fullID._type;
		return *this;
	};

	bool operator==(const CAgentFID& fullID) const
	{
		return (_id == fullID._id) && (_type == fullID._type);
	};

	bool operator!=(const CAgentFID& fullID) const
	{
		return (_id != fullID._id) || (_type != fullID._type);
	};

	bool operator<(const CAgentFID& fullID) const
	{
		bool ok = (_id < fullID._id) || (_id == fullID._id && _type < fullID._type);
		return ok;
	};

	bool operator>(const CAgentFID& fullID) const
	{
		bool ok = (_id > fullID._id) || (_id == fullID._id && _type > fullID._type);
		return ok;
	};

	friend ofstream& operator<<(ofstream& ofstrm, const CAgentFID& fullID);
};

ofstream& operator<<(ofstream& ofstrm, const CAgentFID& fullID);
ofstream& operator>>(ifstream& ifstrm, const CAgentFID& fullID);

//======================  CShare  ===================================

class CShare : public pair<CAgentFID, GoodQtty> // producerFID, nShares
{
public:

	CShare() {
		first = CAgentFID(UndefAgentType, UndefAgentID);
		second = 0;
	};
	CShare(CAgentFID fid, GoodQtty nsh) {
		first = fid;
		second = nsh;
	};

	CShare& operator=(const CShare& sh)
	{
		assert(first._type == UndefAgentType || first._type == sh.first._type);
		first = sh.first;
		second = sh.second;
		return *this;
	}

	CShare& operator=(GoodQtty nSh)
	{
		second = nSh;
		return *this;
	}

	CShare& operator+=(GoodQtty nSh)
	{
		second += nSh;
		return *this;
	}

	CShare& operator-=(GoodQtty nSh)
	{
		second -= nSh;
		return *this;
	}

	friend ofstream& operator<<(ofstream& ofstrm, const CShare& sh);
	friend ifstream& operator>>(ifstream& ifstrm, CShare& sh);
};

ofstream& operator<<(ofstream& ofstrm, const CShare& sh);
ifstream& operator>>(ifstream& ifstrm, CShare& sh);

//======================  CShares  ===================================

class CShares : public map<CAgentFID, GoodQtty> // producerFID, nShares
{
public:
	CShares();
	~CShares();

	GoodQtty& operator[](CAgentFID fid);
	const GoodQtty operator()(CAgentFID fid) const;
	CShares& operator=(const CShares& shs);
	CShares& operator+(const CShares& shs) const;
	CShares& operator+=(const CShares& shs);
	CShares& operator-(const CShares& shs) const;
	CShares& operator-=(const CShares& shs);

	friend ofstream& operator<<(ofstream& ofstrm, const CShares& shs);
	friend ifstream& operator>>(ifstream& ifstrm, CShares& shs);
};

ofstream& operator<<(ofstream& ofstrm, const CShares& shs);
ifstream& operator>>(ifstream& ifstrm, CShares& shs);

//======================  CBond  ===================================

class CBond
{
public:
	GoodQtty _faceValue; // "valor nominal" $10000
	double _couponRate; // "tasa" 6%
	GoodQtty _purchaseDate; // absolutemonths
	GoodQtty _couponDate; // every 6 months
	GoodQtty _maturityDate; // absolutemonths, "vencimiento" (6months * 2) * 2years
	double _price; // a function of depreciation and remaining time

	CBond();
	CBond(GoodQtty faceValue, double couponRate, GoodQtty couponDate, GoodQtty emissionDate, GoodQtty maturityDate);
	~CBond() {};
	double updatePrice(double discountRate); // current price $1.077,10 if the discount rate is 5%

	friend ifstream& operator>>(ifstream& ifstrm, CBond& v);
	friend ofstream& operator<<(ofstream& ofstrm, const CBond& v);
};

ofstream& operator<<(ofstream& ofstrm, const CBond& b);
ifstream& operator>>(ifstream& ifstrm, CBond& b);

//////////////////////// 2. AGENTS  //////////////////////////////////////

class CBankEntry
{
public:
	CAgent* _pAgent;
	GoodQtty _value;
	double _rate;
	long _initialMonth;
	long _endMonth;
	bool _operationOK;

	CBankEntry(CAgent* pAgent, GoodQtty value, double rate,
		long initialMonth, long endMonth, bool ok);
	CBankEntry();
	~CBankEntry();

	void clear();

	friend ofstream& operator<<(ofstream& ofstrm, const CBankEntry& entry);
	friend ifstream& operator>>(ifstream& ifstrm, CBankEntry& entry);
};

ofstream& operator<<(ofstream& ofstrm, const CBankEntry& entry);
ifstream& operator>>(ifstream& ifstrm, CBankEntry& entry);

ofstream& operator<<(ofstream& ofstrm, const CTypeQttyVector& vect);
ifstream& operator>>(ifstream& ifstrm, CTypeQttyVector& vect);

//======================  CAgent  ===================================

class CAgent
{
protected:

	AgentID _myProxyBuyerID;
	AgentID _ID;
	const AgentType _agentType; // -1: Indiv, >=0: assigned to P_name...
	long _initialMonth;
	long _MonthlyActivityMonth;
	CGoods _myGoodsIwish;
	CGoods _GoodsIhave;
	GoodQtty _myFixCapital;
	GoodQtty _purchasedFixCapital;
	GoodQtty _myGFCF;
	CGoods _ToSell;
	double _WorkTimeToBuy;
	GoodQtty _myFixCapToBuy;
	CGoods _GoodsToBuy;
	CGoods _FixCap_GoodsToBuy_ratios;
	GoodQtty _Cash;
	GoodQtty _ConsumptionBudget;
	GoodQtty _InitCash;
	CTypeDoubleMap _myPrice;
	double _myPriceOfSalary;
	CGoods _myProviderID; // map<GoodType, AgentID>: AgentID passed as a GoodQtty

	CShares _myShares;
	GoodQtty _myValueToBuyShares;
	CShare _SharesToTrade; // producerFID, nShares
	double _myDividendYield;

	GoodQtty _myBondsValueToTrade;
	GoodQtty _NBonds;
	double _myTotBondsValue;

	static double _PriceAdaptFactor;
	static CTypeDoubleMap _PriceCalibFactor;
	static vector<vector<double>> _BuyFraction;

	map<CAgentFID, CProducer*>* _pmyProducers; // map<CAgentFID, CAgent*>*, like _pOwners;

	CBank* _pUsedBank;
	CBankEntry _myBankAccountStatus;
	GoodQtty _WealthPrevMonth;
	GoodQtty _IncomePrevMonth, _IncomeCurr;
	GoodQtty _NetTaxesPrevMonth, _NetTaxesCurr;

	static double _gammaC;
	static double& gammaC() { return _gammaC; };
	static double getgammaC() { return _gammaC; };
	virtual double mylogitProb(GoodType gType);

	CAgent(AgentID id, AgentType agentTy);
	~CAgent();

	bool DeleteMyMoney(GoodQtty deleteMyMoney);
	CGoods& myProviderID() { return _myProviderID; };
	GoodQtty& myProviderIDof(GoodType gType) { return _myProviderID[gType]; };
	CShare& SharesToTrade() { return _SharesToTrade; };
	GoodQtty& myValueToBuyShares() { return _myValueToBuyShares; };

	GoodQtty& myBondsValueToTrade() { return _myBondsValueToTrade; };
	long& MonthlyActivityMonth() { return _MonthlyActivityMonth; };
	long& initialMonth();
	virtual void initialize();

	GoodQtty& myGFCF() { return _myGFCF; };
	double& WorkTimeToBuy();

	GoodQtty& myFixCapital() { return _myFixCapital; };
	GoodQtty& purchasedFixCapital() { return _purchasedFixCapital; };
	GoodQtty& myFixCapToBuy() { return _myFixCapToBuy; };
	CGoods& GoodsToBuy();
	CGoods& FixCap_GoodsToBuy_ratios() { return _FixCap_GoodsToBuy_ratios; };
	virtual GoodQtty MakeListOfGoodsToBuy() { return 0; };

	bool HireWorkTime(CAgent& neighbor, double& qtty);
	double BuyFromAgent(CAgent& neighbor, GoodType gType, double& qtty);
	GoodQtty allocateBuyersQttyAndMoney(GoodQtty tradedQtty, double priceAgreed,
		GoodType gType, CAgent* pSeller, CAgent* pBuyer);

public:

	virtual bool getHasWorkedCurrentMonth() const = 0;
	virtual GoodQtty getMoneyHoldings() const;
	virtual GoodQtty getWealth() const;

	GoodQtty getmyValueToBuyShares() const;
	GoodQtty getmyBondsValueToTrade() const { return _myBondsValueToTrade; };
	double getWorkTimeToBuy() const;
	const CGoods& getFixCap_GoodsToBuy_ratios() const { return _FixCap_GoodsToBuy_ratios; };
	GoodQtty getmyFixCapToBuy() const { return _myFixCapToBuy; };
	AgentID& myProxyBuyerID() { return _myProxyBuyerID; };
	AgentID getmyProxyBuyerID() const { return _myProxyBuyerID; };
	map<CAgentFID, CProducer*>*& pmyProducers() { return _pmyProducers; };
	map<CAgentFID, CProducer*>& myProducers() { return *_pmyProducers; };
	map<CAgentFID, CProducer*>* getpmyProducers() const { return _pmyProducers; };
	CProducer*& pmyProducer(CAgentFID fid) { return myProducers()[fid]; };
	CProducer& myProducer(CAgentFID fid) { return *(myProducers()[fid]); };
	double& myTotBondsValue() { return _myTotBondsValue; }
	double getmyTotBondsValue() const { return _myTotBondsValue; };
	const CGoods& getmyProviderID() const { return _myProviderID; };
	GoodQtty getmyProviderIDof(GoodType gType) const;

	CShares& myShares() { return _myShares; };
	GoodQtty& myShares(CAgentFID fid) { return _myShares[fid]; };
	const CShares& getmyShares() const { return _myShares; };
	GoodQtty getmySharesOf(CAgentFID fid) const { return _myShares.at(fid); };

	GoodQtty& NBonds() { return _NBonds; };
	GoodQtty getNBonds() const { return _NBonds; };
	double& myDividendYield() { return _myDividendYield; };
	double getmyDividendYield() const { return _myDividendYield; };

	GoodQtty valGoodsIhave() const;
	GoodQtty valToSell() const;
	virtual double getmyTotSharesValue() const { return 0; };
	GoodQtty getmyBankBalance() const;


	virtual void monthInitialize();
	virtual void monthActivity();

	virtual CShare getSharesToTrade() const;
	virtual CGoods getInventory();

	GoodQtty& InitCash() { return _InitCash; };
	GoodQtty getmyFixCapital() const;
	GoodQtty getpurchasedFixCapital() const;
	GoodQtty getmyGFCF() const;
	long getMonthlyActivityMonth() const { return _MonthlyActivityMonth; };
	const GoodQtty getInitCash() const { return _InitCash; };
	GoodQtty& ConsumptionBudget();
	GoodQtty& ToSell(GoodType gType);
	CGoods& GoodsIhave();
	CGoods& ToSell();
	void BuyGoods();
	void ReturnLoansToMyBank();
	CTypeDoubleMap& myPrice();
	GoodQtty& WealthPrevMonth() { return _WealthPrevMonth; };
	GoodQtty& IncomePrevMonth() { return _IncomePrevMonth; };
	GoodQtty& IncomeCurr() { return _IncomeCurr; };
	GoodQtty& NetTaxesPrevMonth() { return _NetTaxesPrevMonth; };
	GoodQtty& NetTaxesCurr() { return _NetTaxesCurr; };
	GoodQtty GoodsIhave(GoodType gType);
	CGoods& myGoodsIwish();
	const CGoods& getmyGoodsIwish() const;

	static double& PriceAdaptFactor();
	static CTypeDoubleMap& PriceCalibFactor();
	static CTypeDoubleMap getPriceCalibFactor();
	static double getPriceCalibFactorOfType(GoodType gType);
	void setID(AgentID id) { _ID = id; };
	CBankEntry& myBankAccountStatus() { return _myBankAccountStatus; };
	const CBankEntry getmyBankAccountStatus() const { return _myBankAccountStatus; };
	CBank*& pUsedBank() { return _pUsedBank; };
	CBank* getmyUsedBank() const { return _pUsedBank; };
	void setmyPriceOf(const GoodType gID, const double price);
	double myBuyerPriceOf(const GoodType gType, const double sellerPrice = 0);
	GoodQtty& Cash();

	const CGoods& getGoodsIhave() const;
	const GoodQtty getGoodsIhave(GoodType gType) const;
	void setGoodsToBuyOf(GoodType gType, GoodQtty qtty);
	const GoodQtty getGoodsToBuyOf(GoodType gType) const;
	void WriteTransaction(CAgent* pFromAgent, CAgent* pToAgent, string txt);
	const CGoods& getToSell() const;
	const GoodQtty getToSell(GoodType gType) const;
	const GoodQtty getWealthPrevMonth() const { return _WealthPrevMonth; };
	const GoodQtty getIncomePrevMonth() const { return _IncomePrevMonth; };
	const GoodQtty getNetTaxesPrevMonth() const { return _NetTaxesPrevMonth; };
	const GoodQtty getIncomeCurr() const { return _IncomeCurr; };
	const GoodQtty getNetTaxesCurr() const { return _NetTaxesCurr; };
	AgentID getID() const;
	CAgentFID getFID() const;
	bool IsWorker() const;
	bool IsGovernment() const;
	bool IsCentralBank() const;
	bool IsPrivateBank() const;
	bool IsExtSect() const;
	bool IsProducer() const;
	AgentType getAgentType() const;
	string getAgentName() const;
	long getinitialMonth() const;
	GoodQtty getCash() const;
	GoodQtty getConsumptionBudget() const;
	static double getPriceAdaptFactor();
	const double getmyPriceOf(GoodType gType) const;
	void setmyPriceOfSalary(double priceOfsal) {
		_myPriceOfSalary = priceOfsal;
	};
	const double getmyPriceOfSalary() const { return _myPriceOfSalary; };
	const double getmySalary_mu() const;
	bool GetCashFromBank(GoodQtty& loanQtty);
	const CGoods& getGoodsToBuy() const;
	bool PayTo(CAgent* pToAgent, GoodQtty value, string txt = "");

	void RemoveServiceGoods();

	friend CWorker;
	friend CProducer;
	friend CWorld;
	friend CFinancialMarket;

	friend ofstream& operator<<(ofstream& ofstrm, const CAgent& agent);
	friend ifstream& operator>>(ifstream& ifstrm, CAgent& agent);
};

ofstream& operator<<(ofstream& ofstrm, const CAgent& agent);
ifstream& operator>>(ifstream& ifstrm, CAgent& agent);

//======================  CWorker  ===================================

class CWorker : public CAgent
{
private:
	static double _PropToConsume;
	//static CTypeDoubleMap _PropToConsume;
	//static double _totalPropToConsume;

	double _myAvailableTime;
	bool _bPartTimeWorker;
	CProducer* _pEmployer;
	CBank* _pOwnedBank;

	vector<long>* _pLgroupIndices;
	GoodType _myLgroupN;
	vector<long>* _pHgroupIndices;
	GoodType _myHgroupN;

	CBond _myBond;

	//------------------  Accessors  ----------------------

protected:
	vector<CProducer*>* _myNeighboringProducers = nullptr;
	vector<CProducer*>*& myNeighboringProducers() { return _myNeighboringProducers; };

	double _BondToSharesRatio;
	double& myPriceOfBond() { return _myBond._price; };

	vector<long>*& pLgroupIndices() { return _pLgroupIndices; }
	vector<long>& LgroupIndices() { return *_pLgroupIndices; }
	vector<long>*& pHgroupIndices() { return _pHgroupIndices; }
	vector<long>& HgroupIndices() { return *_pHgroupIndices; }
	GoodQtty MakeListOfGoodsToBuy(); // virtual

	virtual void initialize(void);

	bool getHasWorkedCurrentMonth() const;

public:
	CWorker(AgentID id);
	~CWorker();

	virtual void monthInitialize();

	const vector<CProducer*>* getmyNeighboringProducers() const { return _myNeighboringProducers; };
	bool& bPartTimeWorker() { return _bPartTimeWorker; };
	bool getbPartTimeWorker() const { return _bPartTimeWorker; };
	double& myAvailableTime() { return _myAvailableTime; };
	double getmyAvailableTime() const { return _myAvailableTime; };
	GoodType getmyLindex() const;
	GoodType getmyLgroupN() const { return _myLgroupN; };

	void assignHandLgroups();

	virtual double getmyTotSharesValue() const;
	CBond& myBond() { return _myBond; };
	const CBond& getmyBond() const { return _myBond; };
	virtual GoodQtty getWealth() const;

	GoodType getmyHindex() const;
	GoodType getmyHgroupN() const { return _myHgroupN; };

	static double& PropToConsume();
	static double getPropToConsume();
	//static CTypeDoubleMap& PropToConsume();
	//static CTypeDoubleMap getPropToConsume();
	//static double& totalPropToConsume();
	//static double gettotalPropToConsume();

	const vector<long>& getLgroupIndices() const { return *_pLgroupIndices; }
	const vector<long>& getHgroupIndices() const { return *_pHgroupIndices; }
	CProducer*& pEmployer();
	CBank*& pOwnedBank();
	const CBank* getpOwnedBank() const;
	GoodQtty getGoodsToBuyOf(GoodType gType);
	virtual void monthActivity();
	CBank* TryToFoundCommercialBank();
	void TryToStartupNewProducer();

	const CProducer* getpEmployer() const;

	friend class CWorld;
	friend class CFinancialMarket;
	friend ofstream& operator<<(ofstream& ofstrm, const CWorker& indiv);
	friend ifstream& operator>>(ifstream& ifstrm, CWorker& indiv);
};

ofstream& operator<<(ofstream& ofstrm, const CWorker& indiv);
ifstream& operator>>(ifstream& ifstrm, CWorker& indiv);

//====================== CProduct & CProducer specs  ==============

class CProductSpecs  // static object of class CProducer
	// Product fabrication parameters, constant throughout the simulation
{
protected:
	GoodType _productType;
	double _TotalIC;
	CTypeDoubleMap _NeededPerUnit;
	CTypeDoubleMap _Needed_Init;
	vector< vector<double> >* _pNeededPerFigUnit;
	long _initMonth;
	bool _bImported;
	double _ImportPrice;
	bool _bIsService;

	double _TaxAlcohTab;
	double _TransfNetas;
	double _PrestDesemp;
	double _PrestPension;
	double _IRPF;
	double _TaxImportCE;
	double _TaxImportRW;

	double _Men65AgrBaj;
	double _Men65AgrAlt;
	double _Men65NoAgr1q;
	double _Men65NoAgr2q;
	double _Men65NoAgr3q;
	double _Men65NoAgr4q;
	double _Men65NoAgr5q;
	double _Mas65RurBaj;
	double _Mas65RurAlt;
	double _Mas65UrbBaj;
	double _Mas65UrbAlt;

	double _CompEmployees;
	double _GrossOpSurplus;
	double _Households;
	double _Government;
	double _TaxProducts;
	double _TaxProduction;
	double _GrossOutput_mu;

	double _CuentaDeCapital;
	double _TransfersToHouseholds;
	double _TransfersToGovernment;
	double _TransfersToCapitalAcc;

	// specs accessors

	GoodType& productType();
	CTypeDoubleMap& NeededPerUnit();
	CTypeDoubleMap& Needed_Init();
	const CTypeDoubleMap& getNeeded_Init() const;
	double& NeededPerUnit(GoodType gType);
	double& Needed_Init(GoodType gType);
	vector< vector<double> >*& pNeededPerFigUnit() { return _pNeededPerFigUnit; };
	vector< vector<double> >& NeededPerFigUnit() { return *_pNeededPerFigUnit; };
	const vector<vector<double>>& getNeededPerFigUnit() const { return *_pNeededPerFigUnit; };
	double& NeededPerFigUnit(GoodType country, GoodType sector);
	const double getNeededPerFigUnitOf(GoodType country, GoodType sector) const;
	long& initMonth();
	double& ImportPrice();
	bool& bImported();
	bool& bIsService();

	double& CompEmployees();
	double& GrossOpSurplus();
	double& GrossOutput_mu();

	double& TaxProduction();
	double& TaxProducts();
	double& TaxImportCE();
	double& TaxImportRW();
	double& TransfersToHouseholds();
	double& TransfersToGovernment();
	double& TransfersToCapitalAcc();
	double& TaxAlcohTab() { return _TaxAlcohTab; };
	double& TransfNetas() { return _TransfNetas; };
	double& PrestDesemp() { return _PrestDesemp; };
	double& PrestPension() { return _PrestPension; };
	double& IRPF() { return _IRPF; };
	double& Men65AgrBaj() { return _Men65AgrBaj; };
	double& Men65AgrAlt() { return _Men65AgrAlt; };
	double& Men65NoAgr1q() { return _Men65NoAgr1q; };
	double& Men65NoAgr2q() { return _Men65NoAgr2q; };
	double& Men65NoAgr3q() { return _Men65NoAgr3q; };
	double& Men65NoAgr4q() { return _Men65NoAgr4q; };
	double& Men65NoAgr5q() { return _Men65NoAgr5q; };
	double& Mas65RurBaj() { return _Mas65RurBaj; };
	double& Mas65RurAlt() { return _Mas65RurAlt; };
	double& Mas65UrbBaj() { return _Mas65UrbBaj; };
	double& Mas65UrbAlt() { return _Mas65UrbAlt; };

	double& CuentaDeCapital();
	double& Households();
	double& Government();

	void initialize();

public:
	CProductSpecs(GoodType _productType);
	~CProductSpecs();

	double getTotalIC() const;
	const GoodType& getproductType() const;

	const double getGrossOpSurplus() const { return _GrossOpSurplus; };
	const double HouseholdsGoods() const { return _Households; };
	const double getGovernment() const { return _Government; };
	const double getTaxProducts() const { return _TaxProducts; }
	const double getTaxProduction() const;
	const double getGrossOutput_mu() const { return _GrossOutput_mu; }
	const double getCompEmployees() const;
	const double getPrestDesemp() const { return _PrestDesemp; };
	const double getTransfNetas() const { return _TransfNetas; };
	const double getTaxAlcohTab() const { return _TaxAlcohTab; };
	const double getPrestPension() const { return _PrestPension; };
	const double getIRPF() const { return _IRPF; };
	const double getMen65AgrBaj() const { return _Men65AgrBaj; };
	const double getMen65AgrAlt() const { return _Men65AgrAlt; };
	const double getMen65NoAgr1q() const { return _Men65NoAgr1q; };
	const double getMen65NoAgr2q() const { return _Men65NoAgr2q; };
	const double getMen65NoAgr3q() const { return _Men65NoAgr3q; };
	const double getMen65NoAgr4q() const { return _Men65NoAgr4q; };
	const double getMen65NoAgr5q() const { return _Men65NoAgr5q; };
	const double getMas65RurBaj() const { return _Mas65RurBaj; };
	const double getMas65RurAlt() const { return _Mas65RurAlt; };
	const double getMas65UrbBaj() const { return _Mas65UrbBaj; };
	const double getMas65UrbAlt() const { return _Mas65UrbAlt; };

	double getTaxImportCE() const;
	double getTaxImportRW() const;
	double getTransfersToHouseholds() const;
	double getTransfersToGovernment() const;
	double getTransfersToCapitalAcc() const;
	const CTypeDoubleMap& getNeededPerUnit() const;
	const double getNeededPerUnitOf(GoodType gType) const;
	const long getinitMonth() const;
	const double getImportPrice() const;
	const bool getbImported() const;
	const bool& getbIsService() const;

	friend CWorld;
	friend ofstream& operator<<(ofstream& ofstrm, const CProductSpecs& productDefinition);
	friend ifstream& operator>>(ifstream& ifstrm, CProductSpecs& productDefinition);
};

ifstream& operator>>(ifstream& ifstrm, CProductSpecs& specs);
ofstream& operator<<(ofstream& ofstrm, const CProductSpecs& specs);

class CProducerSpecs
	// Producer definition parameters
{
protected:
	GoodType _producerType; // >=0 because -1:Indiv

	GoodType& producerType(); // >=0 because -1:Indiv, associated to Producer name

	//-----------------------------------------------------

public:
	CProducerSpecs(GoodType producerTy);
	~CProducerSpecs();

	static const CProductSpecs& getSpecsOfProductType(GoodType productType);

	const GoodType& getproducerType() const;

	friend class CWorld;
	friend ofstream& operator<<(ofstream& ofstrm, const CProducerSpecs& specs);
	friend ifstream& operator>>(ifstream& ifstrm, CProducerSpecs& specs);
};

ofstream& operator<<(ofstream& ofstrm, const CProducerSpecs& specs);
ifstream& operator>>(ifstream& ifstrm, CProducerSpecs& specs);

//======================  CHistory  ===================================

class CHistory : public vector<double>
{
protected:
	const int _HISTORY_LENGTH;  // Make it const since it's set in constructor
	int _historyIndex = 0;
	int _filledCount = 0;
	double _average = 0;

public:
	// Constructor now takes HISTORY_LENGTH as parameter
	CHistory(int historyLength);

	// Copy constructor
	CHistory(const CHistory& other);

	// Copy assignment operator
	CHistory& operator=(const CHistory& other);

	// Move constructor
	CHistory(CHistory&& other) noexcept;

	// Move assignment operator
	CHistory& operator=(CHistory&& other);

	void updateHistory(double newVal);

	double updateAndReturnAvg();

	double& average() { return _average; }
	const double getaverage() const { return _average; }
	int getHistoryLength() const { return _HISTORY_LENGTH; }

	friend ofstream& operator<<(ofstream& ofstrm, const CHistory& history);
	friend ifstream& operator>>(ifstream& ifstrm, CHistory& history);
};

ofstream& operator<<(ofstream& ofstrm, const CHistory& history);
ifstream& operator>>(ifstream& ifstrm, CHistory& history);

//======================  CProducer  ===================================

class CProducer : public CAgent
{
protected:

	vector<CWorker*>* _myNeighboringWorkers = nullptr;
	vector<CWorker*>*& myNeighboringWorkers() { return _myNeighboringWorkers; };
	vector<CProducer*>* _myNeighboringProducers = nullptr;
	vector<CProducer*>*& myNeighboringProducers() { return _myNeighboringProducers; };

	CHistory _ProductionHistory;
	CHistory& ProductionHistory() { return _ProductionHistory; }

	CHistory _mySupplyHistory;
	CHistory& mySupplyHistory() { return _mySupplyHistory; }

	CHistory _LeftToSellHistory;
	CHistory& LeftToSellHistory() { return _LeftToSellHistory; }

	CHistory _myDemandHistory;
	CHistory& myDemandHistory() { return _myDemandHistory; }

	static vector<vector<CProducerSpecs*>>* _pProducerFigSpecs;

	static map< GoodType, CProducerSpecs* > _ProducerSpecs;
	static map< GoodType, CProductSpecs* > _ProductSpecs;
	static vector<string> _ProducerLabelOfType; // [NProducersTypes]
	static map< string, GoodType > _ProducerTypeOfLabel;

	map<CAgentFID, CAgent*>* _pOwners;
	vector<CWorker*>* _pEmployees;
	int _consecutiveMonthsNegativeWealth;

	long _LastUsedMonth;
	int _myFirstProductionDay;
	int& myFirstProductionDay() { return  _myFirstProductionDay; };

	double _productionPrice;
	double _myMarkupFactor;
	double& myMarkupFactor() { return _myMarkupFactor; };

	double _myGrossOpSurplus;
	double& myGrossOpSurplus() { return _myGrossOpSurplus; };
	double getmyGrossOpSurplus() const { return _myGrossOpSurplus; };

	double _myGOSfactor;
	double& myGOSfactor() { return _myGOSfactor; };

	double _myLabourProductivity;
	double& myLabourProductivity() { return _myLabourProductivity; };
	double getmyLabourProductivity() const { return _myLabourProductivity; };

	double _averageProduction;
	GoodQtty _mySupply;
	GoodQtty _myDemand;
	GoodQtty _prevmyDemand;
	GoodQtty _prevavgmyDemand;
	GoodQtty _avgmyDemand;
	GoodQtty _StockReference;
	GoodQtty _ToBeProduced;
	GoodQtty _LeftToSell;
	GoodQtty _prevToBeProduced;
	GoodQtty _currIC_mu;
	GoodQtty _currCompEmployees_mu;
	GoodQtty _currNetTaxes_mu;
	GoodQtty _currDepositInterests_mu;
	GoodQtty _currLoanPaymentsAndInterests_mu;
	CGoods _producedUnits;
	CGoods _assistedQtties;
	double _timeWorked;
	double& timeWorked() { return _timeWorked; };

	bool _InFinancialMarket;
	bool _NoCompEmployees;
	GoodQtty _NewSharesIssued;

	double& averageProduction() { return _averageProduction; };
	GoodQtty& mySupply() { return _mySupply; };
	GoodQtty& prevToBeProduced() { return _prevToBeProduced; };

	double updateAndReturnAvgProduction();
	double updateAndReturnAvgmySupply();
	double updateAndReturnAvgLeftToSell();

public:

	CProducer(AgentID id, GoodType producerType, CAgent* owner);
	~CProducer();

	virtual void initialize(void);

	int getConsecutiveMonthsNegativeWealth() const { return _consecutiveMonthsNegativeWealth; }
	void incrementConsecutiveMonthsNegativeWealth() { ++_consecutiveMonthsNegativeWealth; }
	void resetConsecutiveMonthsNegativeWealth() { _consecutiveMonthsNegativeWealth = 0; }

	const vector<CWorker*>* getmyNeighboringWorkers() const { return _myNeighboringWorkers; };
	const vector<CProducer*>* getmyNeighboringProducers() const { return _myNeighboringProducers; };
	virtual CGoods getInventory();

	bool getHasWorkedCurrentMonth() const;

	static void readProducerSpecs(ifstream& ifstrm);
	static void writeProducerSpecs(ofstream& ofstrm);
	static void readProductSpecs(ifstream& ifstrm);
	static void writeProductSpecs(ofstream& ofstrm);

	static vector<vector<CProducerSpecs*>>*& pProducerFigSpecs() { return _pProducerFigSpecs; };
	static vector<vector<CProducerSpecs*>>& ProducerFigSpecs() { return *_pProducerFigSpecs; };
	static const vector<vector<CProducerSpecs*>>& getProducerFigSpecs() { return *_pProducerFigSpecs; };

	static map< GoodType, CProducerSpecs* >& mProducerSpecs();
	static const map< GoodType, CProducerSpecs* >& getProducerSpecs();
	static map< GoodType, CProductSpecs* >& ProductSpecs();
	static const map< GoodType, CProductSpecs* >& getProductSpecs();

	static const CProductSpecs& getProductSpecs(GoodType productType);

	static const CProducerSpecs& getSpecsOfProducerType(AgentType producerType);
	static const CProductSpecs& getSpecsOfProductType(GoodType productType);

	static vector<string>& ProducerLabelOfType(); // [NProducersTypes]
	static map<string, AgentType>& ProducerTypeOfLabel();

	static const vector<string>& getProducerLabelOfType();
	static const string getProducerLabelOfType(GoodType gType);

	static const map<string, AgentType>& getProducerTypeOfLabel();
	static const AgentType getProducerTypeOfLabel(string gname);

	double gettimeWorked() const { return _timeWorked; };
	double getmyGOSfactor() const { return _myGOSfactor; };
	double getaverageProduction() const { return _averageProduction; };
	GoodQtty getmySupply() const { return _mySupply; };
	GoodQtty getLeftToSell() const { return _LeftToSell; };
	const string getProducerName() const;
	const CProducerSpecs& Specs();
	void releaseEmployees();
	const double getproductionPrice() const { return _productionPrice; };
	bool& InFinancialMarket() { return _InFinancialMarket; };
	bool getInFinancialMarket() const { return _InFinancialMarket; };
	bool& NoCompEmployees() { return _NoCompEmployees; };
	bool getNoCompEmployees() const { return _NoCompEmployees; };
	double& productionPrice() { return _productionPrice; };
	double getmyMarkupFactor() const { return _myMarkupFactor; };

	map<CAgentFID, CAgent*>*& pOwners();
	map<CAgentFID, CAgent*>& Owners();
	vector<CWorker*>*& pEmployees();
	vector<CWorker*>* getpEmployees() const;
	vector<CWorker*>& Employees();
	GoodQtty& ToBeProduced();

	GoodQtty& currIC_mu() { return _currIC_mu; };
	GoodQtty& currCompEmployees_mu() { return _currCompEmployees_mu; };
	GoodQtty& currNetTaxes_mu() { return _currNetTaxes_mu; };
	GoodQtty& currDepositInterests_mu() { return _currDepositInterests_mu; };
	GoodQtty& currLoanPaymentsAndInterests_mu() { return _currLoanPaymentsAndInterests_mu; };

	CGoods& producedUnits() { return _producedUnits; };
	const CGoods& getproducedUnits() const { return _producedUnits; };
	CGoods& assistedQtties() { return _assistedQtties; };
	long& LastUsedMonth();
	const map<CAgentFID, CAgent*>* getpOwners() const;
	const map<CAgentFID, CAgent*>& getOwners() const;
	const vector<CWorker*>& getEmployees() const;
	GoodQtty getToBeProduced() const;
	const GoodQtty getcurrIC_mu() const { return _currIC_mu; };
	const GoodQtty getcurrCompEmployees_mu() const { return _currCompEmployees_mu; };
	const GoodQtty getcurrNetTaxes_mu() const { return _currNetTaxes_mu; };
	const GoodQtty getcurrDepositInterests_mu() const { return _currDepositInterests_mu; };
	const GoodQtty getcurrLoanPaymentsAndInterests_mu() const { return _currLoanPaymentsAndInterests_mu; };
	GoodQtty& assistedQttiesOf(GoodType gType) { return _assistedQtties[gType]; };
	const CGoods& getassistedQtties() const { return _assistedQtties; };
	GoodQtty& producedUnitsOf(GoodType gType) { return _producedUnits[gType]; };
	GoodQtty getproducedUnitsOf(GoodType gType) { return _producedUnits[gType]; };
	const long getLastUsedMonth() const;

	GoodQtty& StockReference();
	const GoodQtty& getStockReference() const;
	const CHistory& getProductionHistory() const { return _ProductionHistory; };
	double getAvgProduction() const { return _ProductionHistory.getaverage(); };
	const CHistory& getmySupplyHistory() const { return _mySupplyHistory; };
	double getAvgmySupply() const { return _mySupplyHistory.getaverage(); };
	const CHistory& getLeftToSellHistory() const { return _LeftToSellHistory; };
	double getAvgLeftToSell() const { return _LeftToSellHistory.getaverage(); };
	const CHistory& getmyDemandHistory() const { return _myDemandHistory; }
	double getAvgmyDemand() const { return _myDemandHistory.getaverage(); }

	GoodQtty getprevToBeProduced() const { return _prevToBeProduced; };
	GoodQtty& myDemand() { return _myDemand; };
	GoodQtty getmyDemand() const { return _myDemand; };
	GoodQtty& prevmyDemand() { return _prevmyDemand; };
	GoodQtty getprevmyDemand() const { return _prevmyDemand; };
	GoodQtty& prevavgmyDemand() { return _prevavgmyDemand; };
	GoodQtty getprevavgmyDemand() const { return _prevavgmyDemand; };
	GoodQtty& avgmyDemand() { return _avgmyDemand; };
	GoodQtty getavgmyDemand() const { return _avgmyDemand; };
	GoodQtty& LeftToSell() { return _LeftToSell; };
	virtual GoodQtty getWealth() const;
	GoodQtty& NewSharesIssued() { return _NewSharesIssued; };
	GoodQtty getNewSharesIssued() const;

	double getmyCurrShareVal() const;
	double getmyTotSharesValue() const;

	GoodQtty releaseCash();
	CGoods transferGoodsIhave(); // Indivs: goods from dismantled Producers
	CGoods transferToSell(); // Indivs: goods from dismantled Producers

	const CProducerSpecs& getSpecs() const;
	const GoodQtty MinimumProductUnits(const GoodType productType) const;
	bool TryToHireNeighbor(CWorker& neighborIndiv);
	int getmyFirstProductionDay() const { return  _myFirstProductionDay; };
	bool isProductionDay(int Ntoday, int daysInMonth, int productionDays, int myFirstProductionDay) const;
	bool isProductionDay(int workDayN) const;
	const CWorker* ReleaseLastEmployee();
	const bool ReleaseThisEmployee(CWorker& employee);

	// activity

	virtual void monthInitialize();
	virtual void monthActivity();

	bool IsTargetProducerForDiagnostics() const;
	void LogDiagnostic(bool isTargetProducer, const string& tag);
	void LogDiagnostic(bool isTargetProducer, const string& tag, GoodQtty buyGoodsBudget);
	void LogDiagnostic(bool isTargetProducer, const string& tag, double cashBefore, double balanceBefore);
	void LogDiagnostic(bool isTargetProducer, const string& tag, GoodQtty producedUnits, double cashBefore, double balanceBefore);
	void InitializeProducerCash();
	void ApplyCapitalDepreciation();
	GoodQtty PlanProductionAndProcurement();
	void ExecuteProcurement();
	GoodQtty ExecuteProduction();
	void ExecuteFixedCapitalInvestment();
	void CalculateMaxFixedCapitalFromInventory(double& FixCapIncrease, vector<double>& GFCFfractionOf, CTypeDoubleMap& maxFixCapfromGoodsIhave);
	void ConvertGoodsToFixedCapital(double FixCapIncrease, const vector<double>& GFCFfractionOf);
	void ManageFinancing(GoodQtty producedUnits, bool isTargetProducer);
	void ManageOperationalFinancing(GoodQtty producedUnits, double balance, bool isTargetProducer);
	void LogCreditManagementCalculation(bool isTargetProducer, double avgProduction, double targetCreditLine, double balance, double minOperatingCash);
	void ManageExcessCash(double balance, double targetCreditLine, double minOperatingCash);
	void EstablishCreditLine(double targetCreditLine, bool isTargetProducer);
	void ManageBootstrapFinancing(GoodQtty producedUnits, double balance, bool isTargetProducer);
	void FinalizeMonthActivity();

	GoodQtty MakeListOfGoodsToBuy();
	void updateStockReferenceAndToBeProduced();

	void PayDividends(GoodQtty dividends);

	GoodQtty RunProducer();
	bool SecureProductionFinancing(double productionCost);
	void ReleaseExcessEmployees();
	double PayEmployeeSalaries(GoodQtty producedUnits, const CTypeDoubleMap& deltaInputGoods);
	void PayExternalSectorSalaries(CWorker* pWorker, int firstLrow);
	void ConsumeIntermediateInputs(const CTypeDoubleMap& deltaInputGoods);
	double CalculateIntermediateConsumptionCost(const CTypeDoubleMap& deltaInputGoods);
	double PayProductionTaxes(GoodQtty producedUnits);
	void UpdateProductionPricing(GoodQtty producedUnits, double finalProductionCost, GoodType productType);
	GoodQtty ApplyAssistedProduction(GoodQtty producedUnits, GoodType productType);

	double getavailableWorkTime() const;
	double Query(GoodQtty& desiredProductUnits, GoodQtty& producedUnits,
		double& productionCost, CTypeDoubleMap& deltaInputGoods, CTypeDoubleMap& extraNeededInputGoods,
		double& extraNeededWorkTime, double& extraNeededK) const;

	void CalculateLeontiefConstraints(GoodQtty desiredProductUnits, const CProductSpecs& productSpecs, double& usedWorkTime, double& extraNeededWorkTime, double& extraNeededK, double& MaxFromWorkTime, double& MaxFromFixCap) const;

	void CalculateIntermediateGoodsConstraints(GoodQtty desiredProductUnits, const CGoods& availableInputGoods, double extraNeededK, const CTypeDoubleMap& NeededPerUnit, CTypeDoubleMap& MaxFromGoodType, CTypeDoubleMap& extraNeededInputGoods) const;

	GoodQtty DetermineProducibleUnits(GoodQtty desiredProductUnits, double MaxFromWorkTime, double MaxFromFixCap, const CTypeDoubleMap& MaxFromGoodType, CTypeDoubleMap& NeededPerUnit) const;

	void CalculateProductionCostAndInputs(GoodQtty producedUnits, GoodQtty desiredProductUnits, const CProductSpecs& productSpecs, double& productionCost, CTypeDoubleMap& NeededPerUnit, CTypeDoubleMap& deltaInputGoods, GoodType productType) const;

	void AddProductionTaxesToCost(double& productionCost, GoodQtty costUnits, GoodType productType) const;

	friend class CWorld;

	friend ofstream& operator<<(ofstream& ofstrm, const CProducer& producer);
	friend ifstream& operator>>(ifstream& ifstrm, CProducer& producer);
};

ofstream& operator<<(ofstream& ofstrm, const CProducer& producer);
ifstream& operator>>(ifstream& ifstrm, CProducer& producer);

//======================  CExtSect  ===================================

class CExtSect : public CProducer
{
protected:
	string _name;
	vector<GoodQtty>* _pImports;
	vector<GoodQtty>* _pExports;
	vector<GoodQtty>* _pprevExports;
	vector<GoodQtty>* _pExports_Init;// Monthly update

	CTypeDoubleMap _RefExports;
	double _totalRefExports;

	CTypeDoubleMap _ExportFraction;
	CTypeDoubleMap _ExportConsumPXFactor;

public:

	CExtSect(AgentID ID, AgentType aType);
	~CExtSect();

	vector<GoodQtty>*& pprevExports() { return _pprevExports; };
	vector<GoodQtty>& prevExports() { return *_pprevExports; };
	GoodQtty& prevExports(GoodType gType) { return _pprevExports->at(gType); };
	const vector<GoodQtty>& getprevExports() const { return *_pprevExports; };
	GoodQtty getprevExports(GoodType gType) const { return _pprevExports->at(gType); };
	CTypeDoubleMap& RefExports() { return _RefExports; };
	double& totalRefExports() { return _totalRefExports; };
	CTypeDoubleMap& ExportFraction() { return _ExportFraction; };
	double ExportFraction(GoodType gType) { return _ExportFraction.at(gType); };
	CTypeDoubleMap& ExportConsumPXFactor() { return _ExportConsumPXFactor; };
	const CTypeDoubleMap& getExportConsumPXFactor() const { return _ExportConsumPXFactor; };
	double getExportConsumPXFactor(GoodType gType) const { return _ExportConsumPXFactor.at(gType); };
	double gettotalRefExports() const { return _totalRefExports; };
	const CTypeDoubleMap& getRefExports() const { return _RefExports; };
	double getRefExports(GoodType gType) const { return _RefExports.at(gType); };

	string& name() { return _name; };
	vector<GoodQtty>*& pImports() { return _pImports; };
	vector<GoodQtty>& Imports() { return *_pImports; };
	const vector<GoodQtty>& getImports() const { return *_pImports; };
	vector<GoodQtty>*& pExports() { return _pExports; };
	vector<GoodQtty>& Exports() { return *_pExports; };
	const vector<GoodQtty>& getExports() const { return *_pExports; };
	vector<GoodQtty>*& pExports_Init() { return _pExports_Init; };
	vector<GoodQtty>& Exports_Init() { return *_pExports_Init; };
	const vector<GoodQtty>& getExports_Init() const { return *_pExports_Init; };

	void initialize();
	virtual void monthInitialize();
	virtual void monthActivity();

	friend class CWorld;
	friend ofstream& operator<<(ofstream& ofstrm, const CExtSect& extSect);
	friend ifstream& operator>>(ifstream& ifstrm, CExtSect& extSect);
};

ofstream& operator<<(ofstream& ofstrm, const CExtSect& extSect);
ifstream& operator>>(ifstream& ifstrm, CExtSect& extSect);

//======================  CBank  ===================================

// In the class declarations section, update CBank class:

//======================  CBank  ===================================

class CBank : public CAgent  // CHANGED: was public CProducer
{
protected:
	long _MaxNBanks;
	double _ReserveRatio; // beta in Dawid's Reserve Requirement Ratio (RRR)

	GoodQtty _monthInflow;
	GoodQtty _monthOutflow;
	GoodQtty _TotalBullion;
	GoodQtty _ClientsLoans;
	GoodQtty _ClientsDeposits;
	GoodQtty _Cash_I_Own;
	GoodQtty _prevCash_I_Own;

	GoodQtty _RiskExposureAmount;

	map<AgentID, CBankEntry> _ClientsAccounts;// <AgentOrBankOwner*, entry>

public:

	double _rc; // Central Bank policy rate
	double _markup, _markdown;
	double _lambdaB;
	map<AgentID, CBank*> _Banks; // BankID, pBank

	virtual GoodQtty getWealth(void) const;
	GoodQtty& monthInflow() { return _monthInflow; };
	GoodQtty getmonthInflow() const { return _monthInflow; };
	GoodQtty& monthOutflow() { return _monthOutflow; };
	GoodQtty getmonthOutflow() const { return _monthOutflow; };
	GoodQtty& TotalBullion() { return _TotalBullion; };
	double& ReserveRatio() { return _ReserveRatio; };
	GoodQtty& Cash_I_Own() { return _Cash_I_Own; };
	GoodQtty& prevCash_I_Own() { return _prevCash_I_Own; };
	GoodQtty& ClientsDeposits() { return _ClientsDeposits; };
	GoodQtty& ClientsLoans() { return _ClientsLoans; };
	long& MaxNBanks() { return _MaxNBanks; };
	map<AgentID, CBankEntry>& ClientsAccounts()
	{
		return _ClientsAccounts;
	};
	map<AgentID, CBank*>& Banks() { return _Banks; };
	GoodQtty& RiskExposureAmount() { return _RiskExposureAmount; };

	const GoodQtty getTotalBullion() const { return _TotalBullion; };
	const double getReserveRatio() const { return _ReserveRatio; };
	const GoodQtty getCash_I_Own() const { return _Cash_I_Own; };
	const GoodQtty getprevCash_I_Own() const { return _prevCash_I_Own; };
	const GoodQtty getClientsDeposits() const { return _ClientsDeposits; };
	const GoodQtty getClientsLoans() const { return _ClientsLoans; };
	const long getMaxNBanks() const { return _MaxNBanks; };
	const map<AgentID, CBankEntry>& getClientsAccounts()
		const {
		return _ClientsAccounts;
	};
	const map<AgentID, CBank*>& getBanks() const { return _Banks; };
	const GoodQtty getRiskExposureAmount() const { return _RiskExposureAmount; };
	const GoodQtty getRiskExposureBudget() const;

	CBank() = default; // default constructor
	CBank(
		AgentType bankType // CentralBankType or PrivateBankType
	);
	~CBank();

	void initialize();
	void monthInitialize();
	void updateRiskExposureAmount();
	void monthActivity();

	double CalculateInflationAdjustedRate(double baseRate) const;

	double getCurrInterests(CAgent& agent);

	virtual void CollectInterests();
	void ReturnExcessCashToCB();
	double getDepositsRate() const;
	double getCurrentInterestRateOf(CAgent* pClient) const;
	const GoodQtty getClientCurrentBalance(const CAgent* pClient) const;

	CBankEntry ClientDefaultClose(CAgent* pClient);

	CBankEntry ClientCashOperation(CAgent* pClient, GoodQtty deposit, long nMonths = 0);
	void AddValToAccount(CBankEntry& account, GoodQtty value);
	bool GetEnoughReserveFromCB(GoodQtty withdraw);

	GoodQtty ClientsLoansMax();

	GoodQtty ExcessLiquidityBudget() const;

	GoodQtty Equity() const;

	bool GetCashFromBank(CAgent& agent, GoodQtty& loanQtty);

	bool CheckBalance();

	virtual bool getHasWorkedCurrentMonth() const { return false; }; // Banks don't "work" in the traditional sense

	friend ofstream& operator<<(ofstream& ofstrm, const CBank& bank);
	friend ifstream& operator>>(ifstream& ifstrm, CBank& bank);
};

ofstream& operator<<(ofstream& ofstrm, const CBank& bank);
ifstream& operator>>(ifstream& ifstrm, CBank& bank);

//======================  CCentralBank  ===================================

class CCentralBank : public CBank
{
public:

	CCentralBank();
	~CCentralBank();

	void initialize();
	double getDepositsRate() const;

	GoodQtty Equity() const;

	const GoodQtty getRiskExposureBudget() const;

	void monthInitialize();
	void monthActivity();

	// CentralBank only:

	CBank* BankFoundationRequest(CWorker* pIndivOwner, GoodQtty initialCash);
	CBank* getRandomBank();

	friend ofstream& operator<<(ofstream& ofstrm, const CCentralBank& cbank);
	friend ifstream& operator>>(ifstream& ifstrm, CCentralBank& cbank);
};

ofstream& operator<<(ofstream& ofstrm, const CCentralBank& cbank);
ifstream& operator>>(ifstream& ifstrm, CCentralBank& cbank);

//==================  FinancialMarket  ==========================

	// see FinancialMarket.cpp

class CBHolder
{
public:
	CAgent* _pBHolder;
	GoodQtty _myNBonds;
	//double _myPrice;

	CBHolder(CAgent* pBHolder, GoodQtty myNBonds) //, double _myPri)
	{
		_pBHolder = pBHolder;
		_myNBonds = myNBonds;
		//_myPrice = _myPri;
	};
	~CBHolder() {};
};

//--------------------------------

class CSHolder
{
public:
	CAgent* _pSHolder;
	GoodQtty _myNShares;
	double _myPrice;

	CSHolder(CAgent* _pSH, GoodQtty _myNS, double _myPri)
	{
		_pSHolder = _pSH;
		_myNShares = _myNS;
		_myPrice = _myPri;
	};
	~CSHolder() {};
};

//--------------------------------

class CFinancialMarket
{
	double _InitShareValue; // 1000. mu
	double _RateOfPrice;
	double _PriceAdjustSpeed;
	double _Price;
	double _NextPrice;
	double _UpRateLimit;
	double _DownRateLimit;

	// --------------------   SHARES   -------------------

	vector<AgentSortedByFirst>* _pSellerSHolders;
	vector<AgentSortedByFirst>*& pSellerSHolders() { return _pSellerSHolders; };
	vector<AgentSortedByFirst>* _pBuyerSHolders;

	vector<AgentSortedByFirst>* _pSortedProducersByDividend;

	double& InitShareValue() { return _InitShareValue; };
	double& RateOfPrice() { return _RateOfPrice; };
	double& PriceAdjustSpeed() { return _PriceAdjustSpeed; };
	double& Price() { return _Price; };
	double& NextPrice() { return _NextPrice; };
	double& UpRateLimit() { return _UpRateLimit; };
	double& DownRateLimit() { return _DownRateLimit; };

	vector<AgentSortedByFirst>& SellerSHolders() { return *_pSellerSHolders; };
	vector<AgentSortedByFirst>& BuyerSHolders() { return *_pBuyerSHolders; };

	vector<AgentSortedByFirst>& SortedProducersByDividend() { return *_pSortedProducersByDividend; };

	// --------------------   BONDS   -------------------

	vector<AgentSortedByFirst>* _pSellerBHolders;
	vector<AgentSortedByFirst>*& pSellerBHolders() { return _pSellerBHolders; };
	vector<AgentSortedByFirst>* _pBuyerBHolders;

	map<AgentID, GoodQtty>* _pBHolderNbonds; // indivID, Nbonds

	vector<AgentSortedByFirst>& SellerBHolders() { return *_pSellerBHolders; };
	vector<AgentSortedByFirst>& BuyerBHolders() { return *_pBuyerBHolders; };

	map<AgentID, GoodQtty>& BHolderNbonds() { return *_pBHolderNbonds; };

public:
	CFinancialMarket();
	~CFinancialMarket();

	void initialize();

	void CollectSharesSellersAndBuyers();
	void SharesTradeActivity();
	double getInitShareValue() const { return _InitShareValue; };

	void CollectBondsSellersAndBuyers();
	void BondsTradeActivity();

	void monthActivity();

	friend ofstream& operator<<(ofstream& ofstrm, const CFinancialMarket& financialMarket);
	friend ifstream& operator>>(ifstream& ifstrm, CFinancialMarket& financialMarket);
};

ofstream& operator<<(ofstream& ofstrm, const CFinancialMarket& financialMarket);
ifstream& operator>>(ifstream& ifstrm, CFinancialMarket& financialMarket);

//==================  Government  ==========================

class CGovernment : public CAgent
{
protected:

	// Bonds
	GoodQtty _MaxDebt;

	GoodQtty _nBondsEmission;
	CBond _myBondTemplate;
	CBond& myBondTemplate() { return _myBondTemplate; };

	GoodQtty _AnnUnemplQtty;
	set<AgentID> _ProducerIDs;
	set<AgentID>& ProducerIDs() { return _ProducerIDs; };

	double _TaxGFCF; // SAM.at(TaxProducts, GFCF), payed in allocateBuyersQttyAndMoney
	double _TaxGovernmentProducts; // SAM.at(TaxProducts, Gov col)
	CTypeDoubleMap _TaxHouseholdProducts; // HgroupsSize, SAM.at(TaxProducts, Hcol)
	CTypeDoubleMap _TaxSectExtProducts; // XgroupsSize, SAM.at(TaxProducts, Xcol)

public:

	bool getHasWorkedCurrentMonth() const { return false; };
	double& TaxGFCF() { return _TaxGFCF; };
	double& TaxGovernmentProducts() { return _TaxGovernmentProducts; };
	CTypeDoubleMap& TaxHouseholdProducts() { return _TaxHouseholdProducts; };
	CTypeDoubleMap& TaxSectExtProducts() { return _TaxSectExtProducts; };
	double getTaxGFCF() const { return _TaxGFCF; };
	double getTaxGovernmentProducts() const { return _TaxGovernmentProducts; };
	const CTypeDoubleMap& getTaxHouseholdProducts() const { return _TaxHouseholdProducts; };
	const CTypeDoubleMap& getTaxSectExtProducts() const { return _TaxSectExtProducts; };

	// Bonds

	GoodQtty& nBondsEmission() { return _nBondsEmission; };
	const CBond& getmyBondTemplate() const { return _myBondTemplate; };

	// Input variables
	const set<AgentID>& getProducerIDs() const { return _ProducerIDs; };

	// Output variables
	GoodQtty& AnnUnemplQtty() { return _AnnUnemplQtty; };

	CGovernment(AgentID ID, AgentType ty);
	~CGovernment();

	void initialize();
	virtual void monthInitialize();
	virtual void monthActivity();
	void monthlyActivity();

	void GovTransfersToHouseholds();
	void GovPayBondCoupons();
	void ReturnExcessCashToCB();
	virtual GoodQtty getWealth() const;

	friend class CWorld;
	friend class CProducer;
	friend class CFinancialMarket;
	friend ofstream& operator<<(ofstream& ofstrm, const CGovernment& theState);
	friend ifstream& operator>>(ifstream& ifstrm, CGovernment& theState);
};

ofstream& operator<<(ofstream& ofstrm, const CGovernment& theState);
ifstream& operator>>(ifstream& ifstrm, CGovernment& theState);

///////////////////////    CFigaro    ////////////////////////////////

class CFigProducer
{
public:

	AgentType _countryType;
	AgentType _sectorType;

	string _label;
	vector< vector<double> >* _pIC;
	double _L, _K, _Tproduction, _Tproducts;

	string& label() { return _label; };
	const string& getlabel() const { return _label; };
	vector< vector<double> >*& pIC() { return _pIC; };
	vector< vector<double> >& IC() { return *_pIC; };
	const vector< vector<double> >& getIC() const { return *_pIC; };

	CFigProducer(int Ncountries, int Nsectors);
	~CFigProducer();
};

class CInstiTransfers
{
public:
	double ToL = 0, ToK = 0, ToTproduction = 0, ToTproducts = 0;
};

class CFigaro
{
public:
	int _Ncountries;
	int _Nsectors;
	vector<string>* _pCountryTypeToCode;
	map<string, AgentType>* _pCountryCodeToType;
	vector<string>* _pSectorTypeToCode;
	map<string, AgentType>* _pSectorCodeToType;

	string _thisCountryName;
	string _thisCountryCode;
	int _thisCountryType;
	long _year, _active, _units;
	double _InitUnemploymentPercent;
	set<string>* _pDisaggExtSectCountries;
	set<string>* _pAggExtSectCountries;
	set<string>* _pAllExtSectCountries;

	vector< vector<CFigProducer> >* _pFigProducers;

	// Final comsumers

	vector< vector< vector<double> > >* _pFC_Government;
	vector< vector< vector<double> > >* _pFC_Households;
	vector< vector< vector<double> > >* _pFC_GFCF;
	vector< vector< vector<double> > >* _pFC_NPISH;
	vector< vector< vector<double> > >* _pFC_ChgInvent;

	vector<CInstiTransfers>* _pTransfers_Government; // indices: countryCol
	vector<CInstiTransfers>* _pTransfers_Households;
	vector<CInstiTransfers>* _pTransfers_GFCF;
	vector<CInstiTransfers>* _pTransfers_NPISH;
	vector<CInstiTransfers>* _pTransfers_ChgInvent;

public:
	CFigaro();
	~CFigaro();

	void initialize();
	const int getNcountries() const { return _Ncountries; };
	const int getNsectors() const { return _Nsectors; };

	vector< vector<CFigProducer> >*& pFigProducers() { return _pFigProducers; };
	vector< vector<CFigProducer> >& FigProducers() { return *_pFigProducers; };
	const vector<vector< CFigProducer> >& getFigProducers() const { return *_pFigProducers; };

	vector< vector< vector<double> > >*& pFC_Government() { return _pFC_Government; };
	vector< vector< vector<double> > >& FC_Government() { return *_pFC_Government; };
	const vector < vector< vector<double> > >& getFC_Government() const { return *_pFC_Government; };

	vector< vector< vector<double> > >*& pFC_Households() { return _pFC_Households; };
	vector< vector< vector<double> > >& FC_Households() { return *_pFC_Households; };
	const vector < vector< vector<double> > >& getFC_Households() const { return *_pFC_Households; };

	vector< vector< vector<double> > >*& pFC_NPISH() { return _pFC_NPISH; };
	vector< vector< vector<double> > >& FC_NPISH() { return *_pFC_NPISH; };
	const vector < vector< vector<double> > >& getFC_NPISH() const { return *_pFC_NPISH; };

	vector< vector< vector<double> > >*& pFC_ChgInvent() { return _pFC_ChgInvent; };
	vector< vector< vector<double> > >& FC_ChgInvent() { return *_pFC_ChgInvent; };
	const vector < vector< vector<double> > >& getFC_ChgInvent() const { return *_pFC_ChgInvent; };

	vector< vector< vector<double> > >*& pFC_GFCF() { return _pFC_GFCF; };
	vector< vector< vector<double> > >& FC_GFCF() { return *_pFC_GFCF; };
	const vector < vector< vector<double> > >& getFC_GFCF() const { return *_pFC_GFCF; };

	double readCSVField(ifstream& ifstrm, char separator, bool isLastColumn) const;

	bool readFIGAROmatrix();
};


///////////////////////    CSAM    ////////////////////////////////

class CAccount
{
public:
	string _label;
	string _accName;
	long _accN;
	long _accGroupIndex;
	CTypeQttyVector _rowQtties;

	const string& label() const { return _label; };
	const string& accName() const { return _accName; };
	long accN() const { return _accN; };
	long& accGroupIndex() { return _accGroupIndex; };
	long getaccGroupIndex() const { return _accGroupIndex; };
	const CTypeQttyVector& rowQtties() const { return _rowQtties; };
	GoodQtty rowQtty(GoodQtty gType) const { return _rowQtties.at(gType); }

	CAccount(int nAccounts);
	~CAccount();
};

class CSimulatedCountry
{
public:
	string _code, _name;
	GoodQtty _ActivePop;
	double _InitUnemp;
	set<string> _DisaggExtSectCountries;
	set<string> _AggExtSectCountries;
	set<string> _AllExtSectCountries;

	double _X0, _Y0;

	CSimulatedCountry() {
		_ActivePop = 0; _InitUnemp = 0; _X0 = 10; _Y0 = 10;
	};
	~CSimulatedCountry() {};
};

// ----------   CSAM    ----------

class CSAM
{
protected:

	string _SAMname;

	vector<string>* _pSAMTXT;
	vector<string>& SAMTXT() { return *_pSAMTXT; };

	vector<vector<double>>* _pSAMmonth;

	vector<double>* _pGFCFfractions;
	vector<double>* _pRowSum;
	vector<double>* _pColSum;

	long _units;
	long _year;
	double _InitUnemploymentPercent;
	double _InitSalary;

	int _nAccounts, _nPXproducerTypes;
	long _nPProducerTypes;
	GoodType _GFCFtype;

	vector<double> _SAMGrossOutput_mu; // column total of Producers
	vector<double>& SAMGrossOutput_mu() { return _SAMGrossOutput_mu; }; // column total of Producers

	map<string, GoodType> _accNofLabel;
	map<string, GoodType> _accNofName;
	map<GoodType, string> _accNameOfN;
	vector<CAccount*>* _pAccounts;
	map< string, vector<CAccount*> > _AccountGroups;

	map<GoodType, map<GoodType, double>> _CtoPfraction;
	map<GoodType, map<GoodType, double>>& CtoPfraction() { return _CtoPfraction; }
	long& nPProducerTypes() { return _nPProducerTypes; }
	void setupGFCFfractions();

public:

	CSAM();
	~CSAM();

	long _active;
	void readCountryParameters(const string& countryCode, CSimulatedCountry& country);
	void readSimulatedCountries(ifstream& ifstrm);

	bool writeFigaroInputFiles();

	const string& CountryCode() const { return _SAMname; };
	vector<vector<double>>*& pSAMmonth() { return _pSAMmonth; };
	const vector<vector<double>>* getpSAMmonth() const { return _pSAMmonth; };
	vector<vector<double>>& SAMmonth() { return *_pSAMmonth; };

	double& SAMmonthAt(GoodType row, GoodType col)
	{
		return SAMmonth()[row][col];
	}
	double& SAMmonthAt(GoodType row, string colLabel)
	{
		return SAMmonth()[row][getSAM().getAccNofName(colLabel)];
	}
	double& SAMmonthAt(string rowLabel, GoodType col)
	{
		return SAMmonth()[getSAM().getAccNofName(rowLabel)][col];
	}

	double& SAMmonthAt(string rowLabel, string colLabel)
	{
		return SAMmonth()[getSAM().getAccNofName(rowLabel)][getSAM().getAccNofName(colLabel)];
	}

	const vector<vector<double>>& getSAMmonth() const { return *_pSAMmonth; };

	vector<double>*& pGFCFfractions() { return _pGFCFfractions; };
	vector<double>& GFCFfractions() { return *_pGFCFfractions; };
	const vector<double>& getGFCFfractions() const { return *_pGFCFfractions; };
	double const GFCFfractionOf(GoodType gType) const;
	vector<double>*& pRowSum() { return _pRowSum; };
	vector<double>*& pColSum() { return _pColSum; };
	const vector<double>* getpRowSum() const { return _pRowSum; };
	const vector<double>* getpColSum() const { return _pColSum; };
	long nLaccounts() const;
	long nHaccounts() const;
	static bool IsPublicSector(GoodType producerType);
	const string& SAMname() const { return _SAMname; };
	vector<CAccount*>*& pAccounts() { return _pAccounts; };
	vector<CAccount*>& Accounts() { return *_pAccounts; };
	const vector<CAccount*>& getAccounts() const { return *_pAccounts; };
	GoodType addAccount(CAccount* pSect) {
		_pAccounts->push_back(pSect); return GoodType(_pAccounts->size() - 1);
	};
	CAccount& Account(GoodType gType) { return *(*_pAccounts)[gType]; };
	const CAccount& getAccount(GoodType gType) const { return *(*_pAccounts)[gType]; };
	map< string, vector<CAccount*> >& AccountGroups() { return _AccountGroups; };

	GoodQtty& setRowCol(GoodType row, GoodType col);
	GoodQtty& setRowCol(GoodType row, string colLabel);
	GoodQtty& setRowCol(string rowLabel, string colLabel);
	GoodQtty& setRowCol(string rowLabel, GoodType col);
	GoodQtty getRowCol(GoodType row, GoodType col) const;
	GoodQtty getRowCol(GoodType row, string colLabel) const;
	GoodQtty getRowCol(string rowLabel, string colLabel) const;
	GoodQtty getRowCol(string rowLabel, GoodType col) const;
	map<string, GoodType>& accNofLabel() { return _accNofLabel; };
	GoodType getAccNofLabel(string label) const
	{
		auto result = _accNofLabel.find(label);
		if (result != _accNofLabel.end())
			return result->second;
		else
			return UndefGoodType;
	};
	map<string, GoodType>& accNofName() { return _accNofName; };
	GoodType getAccNofName(string name) const
	{
		auto result = _accNofName.find(name);
		if (result != _accNofName.end())
			return result->second;
		else
			return UndefGoodType;
	};
	map<GoodType, string>& accNameOfN() { return _accNameOfN; };

	string getAccNameOfN(GoodType gType) const
	{
		if (gType == getCentralBankType())
			return "CentralBank";
		else if (gType == getPrivateBankType())
			return "PrivateBank";
		else
			return _accNameOfN.at(gType);
	};
	bool IsExtSectType(GoodType gType) const;
	bool IsGFCFType(GoodType gType) const;

	GoodType GFCFtype() const;
	double InitUnemploymentPercent() const { return _InitUnemploymentPercent; };
	long getUnits() const { return _units; };
	int& nPXproducerTypes() { return _nPXproducerTypes; }
	int getnPXproducerTypes() const { return _nPXproducerTypes; }
	long getnPProducerTypes() const { return _nPProducerTypes; }

	int nAccounts() const { return _nAccounts; }
	int& nAccounts() { return _nAccounts; }
	int getnAccounts() const { return _nAccounts; }
	const CAccount& getAccount(string name) const { return *_pAccounts->at(getAccNofName(name)); };
	const map< string, vector<CAccount*> >& getAccountGroups() const { return _AccountGroups; };

	double& SAMGrossOutput_mu(GoodType gType) { return _SAMGrossOutput_mu[gType]; };
	double getSAMGrossOutput_mu(GoodType gType) const { return _SAMGrossOutput_mu[gType]; }; // column total of Producers

	double& InitSalary() { return _InitSalary; };
	double getInitSalary() const { return _InitSalary; };
	double getInitUnemploymentPercent() const { return _InitUnemploymentPercent; }
	long getActive() const { return _active; }

	void fillCtoPfractions();
	void splitC_goodsIntoTheirP_goodsComponents();

	bool BuildSAM();

	void initializeFigaroWithCountryData(CSimulatedCountry& country);
	void initializeSAMProperties(const CFigaro& FIGARO);
	void buildCountrySAMAccounts(const CFigaro& FIGARO);
	set<GoodType> createCountryTypeSet(const set<string>& countryCodes, const CFigaro& FIGARO);
	void initializeCentralDataStructures();
	int buildProducerAccounts(const CFigaro& FIGARO);
	void registerAccountInMaps(const CAccount& account, int accountNumber);
	void processIntermediateConsumption(CAccount& account, const CFigaro& FIGARO, int SAMaccRowN, int FIGsectorN);
	void processExportsToDisaggCountries(CAccount& account, const CFigaro& FIGARO, int SAMaccRowN);
	void processExportsToAggCountries(CAccount& account, const CFigaro& FIGARO, int SAMaccRowN);
	void processRegularAggCountryExports(CAccount& account, const CFigaro& FIGARO, int SAMaccRowN, GoodType extCountryN, int colN);
	void processRestOfWorldExports(CAccount& account, const CFigaro& FIGARO, int SAMaccRowN, int colN);
	void processFinalConsumption(CAccount& account, const CFigaro& FIGARO, int FIGsectorN);
	int buildExternalSectorAccounts(const CFigaro& FIGARO, int SAMaccRowN, const set<GoodType>& allMyCountriesTypes);
	int buildDisaggregatedExtSectorAccounts(const CFigaro& FIGARO, int SAMaccRowN);
	int buildAggregatedExtSectorAccounts(const CFigaro& FIGARO, int SAMaccRowN, const set<GoodType>& allMyCountriesTypes);
	int buildRegularAggExtSectorAccount(const CFigaro& FIGARO, int SAMaccRowN, const string& aggExtCountryCode, GoodType extCountryN);
	int buildRestOfWorldAccount(const CFigaro& FIGARO, int SAMaccRowN, const string& aggExtCountryCode, const set<GoodType>& allMyCountriesTypes);
	int buildSectorExtSectorAccounts(const CFigaro& FIGARO, int SAMaccRowN);
	void processExportsToSectorExtSectors(CAccount& account, const CFigaro& FIGARO, int SAMaccRowN);
	int buildGFCFAccount(const CFigaro& FIGARO, int SAMaccRowN);
	int buildValueAddedAccounts(const CFigaro& FIGARO, int SAMaccRowN);
	int buildInstitutionalAccounts(int SAMaccRowN);
	int buildGovernmentAccount(int SAMaccRowN);
	int buildHouseholdAccount(int SAMaccRowN);
	void finalizeAccountSetup();
	void computeEconomicIndicators();
	void readSAM(string fileNameExt);

	void readSAMTXT(string fileNameExt);
	void writeInputSAM() const;
	void writeSAMTXT(ofstream& ofstrm) const;
	const vector<string>& getSAMTXT() const { return *_pSAMTXT; };

	void writeSAMmonth() const;

	void saveInputSAM(const std::string& filename, const std::string& separator = ";");

	friend class CWorld;
};

// ===============    Neighbors  ==================

class CNeighborsVectorManager {
private:
	std::vector<CAgent*> _AgentsPtrs;
	
	// Cache optimization: store sorted IDs to avoid repeated sorting
	mutable std::vector<AgentID> _sortedIDs;
	mutable bool _isDirty = true;  // Flag indicating cache needs rebuild

public:
	long _MaxNeighboringWorkers = 0;
	long _MaxNeighboringProducers = 0;
	std::mt19937 rng;

	// Constructor to initialize the random number generator
	CNeighborsVectorManager() : rng(std::random_device{}()) {}

	std::vector<CAgent*>& AgentsPtrs() { return _AgentsPtrs; };
	const std::vector<CAgent*>& getAgentsPtrs() const { return _AgentsPtrs; };
	const std::mt19937& getRNG() const { return rng; }  // NEW: Add const accessor
	std::mt19937& getRNG() { return rng; }  // NEW: Add non-const accessor

	// Get sorted IDs (with caching for performance)
	const std::vector<AgentID>& getSortedIDs() const;

	bool saveState(const std::string& filename) const;
	bool loadState(const std::string& filename);
	bool saveRNGState(const std::string& filename);
	bool loadRNGState(const std::string& filename);
	void clear();
	void randomInsert(CAgent* ptr);
	bool removeNeighbor(CAgentFID fid);
	CAgent* getAgentPtr(CAgentFID fid);
};

//======================  Behavioral Learning Equilibrium (BLE)  ===============
// Implementation of Poledna et al. (2023) BLE mechanism for fair comparison
// Agents use AR(1) rules to form expectations about GDP growth and inflation
// Parameters are learned via constant-gain recursive least squares
//
// THREE-PHASE OPERATION:
// - OFF: During calibration (economy is artificial, don't learn)
// - TRAINING: After calibration (learn from free-market dynamics, don't influence)
// - ACTIVE: After training period (learn AND influence agent decisions)

class CBLEExpectations
{
public:
	// Phase enumeration
	enum class BLEPhase {
		OFF = 0,        // Not running (during calibration)
		TRAINING = 1,   // Learning only (no influence on agents)
		ACTIVE = 2      // Full operation (learning + influence)
	};

protected:
	// AR(1) parameters for output expectations: E[Y(t)] = alpha_Y * Y(t-1) + beta_Y
	double _alpha_Y = 0.99;    // Persistence parameter (near unit root)
	double _beta_Y = 0.0;      // Intercept

	// AR(1) parameters for inflation expectations: E[pi(t)] = alpha_pi * pi(t-1) + beta_pi
	double _alpha_pi = 0.95;   // Persistence parameter
	double _beta_pi = 0.0;     // Intercept

	// Historical data for learning
	std::deque<double> _outputHistory;     // Log real GDP history
	std::deque<double> _inflationHistory;  // CPI inflation history
	int _learningWindow = 20;              // Months of data for parameter estimation

	// Current expectations (updated each month)
	double _expectedGrowth = 0.0;          // Expected log GDP growth rate
	double _expectedInflation = 0.0;       // Expected inflation rate

	// Learning parameters
	double _learningGain = 0.02;           // Constant gain learning rate

	// State tracking
	bool _isInitialized = false;
	long _lastUpdateMonth = -1;

	// Phase tracking (NEW)
	BLEPhase _phase = BLEPhase::OFF;
	long _trainingStartMonth = -1;
	long _trainingEndMonth = -1;
	int _minTrainingMonths = 24;           // Minimum months before going active

public:
	CBLEExpectations() {}
	~CBLEExpectations() {}

	// Core methods
	void initialize();
	void updateExpectations(double currentLogGDP, double currentInflation, long currentMonth);
	void learnParameters();

	// Phase management (NEW)
	void startTraining(long month);
	void activateIfReady(long month);
	void resetForTraining();

	// Accessors
	double getExpectedGrowth() const { return _expectedGrowth; }
	double getExpectedInflation() const { return _expectedInflation; }
	double getAlphaY() const { return _alpha_Y; }
	double getBetaY() const { return _beta_Y; }
	double getAlphaPi() const { return _alpha_pi; }
	double getBetaPi() const { return _beta_pi; }
	bool isInitialized() const { return _isInitialized; }
	int getLearningWindow() const { return _learningWindow; }
	double getLearningGain() const { return _learningGain; }

	// Phase accessors (NEW)
	BLEPhase getPhase() const { return _phase; }
	bool isOff() const { return _phase == BLEPhase::OFF; }
	bool isTraining() const { return _phase == BLEPhase::TRAINING; }
	bool isActive() const { return _phase == BLEPhase::ACTIVE; }
	bool canInfluenceAgents() const { return _phase == BLEPhase::ACTIVE; }
	long getTrainingStartMonth() const { return _trainingStartMonth; }
	long getTrainingEndMonth() const { return _trainingEndMonth; }
	int getMinTrainingMonths() const { return _minTrainingMonths; }
	std::string getPhaseString() const {
		switch (_phase) {
			case BLEPhase::OFF: return "OFF";
			case BLEPhase::TRAINING: return "TRAINING";
			case BLEPhase::ACTIVE: return "ACTIVE";
			default: return "UNKNOWN";
		}
	}

	// Modifiers (set from input parameters)
	void setLearningWindow(int window) { _learningWindow = window; }
	void setLearningGain(double gain) { _learningGain = gain; }
	void setMinTrainingMonths(int months) { _minTrainingMonths = months; }

	// Get expected output level (converts log growth to level factor)
	double getExpectedGrowthFactor() const { return exp(_expectedGrowth); }

	// Serialization
	friend ofstream& operator<<(ofstream& ofstrm, const CBLEExpectations& ble);
	friend ifstream& operator>>(ifstream& ifstrm, CBLEExpectations& ble);
};

ofstream& operator<<(ofstream& ofstrm, const CBLEExpectations& ble);
ifstream& operator>>(ifstream& ifstrm, CBLEExpectations& ble);

//======================  COVID-19 Pandemic  ===================================
// Pandemic simulation based on Poledna et al. (2023)
// DEPLOYERS uses Darwinian ABM paradigm: natural selection, not BLE learning

enum class PandemicPhase {
	PrePandemic = 0,        // Normal operation before pandemic start
	AcuteLockdown = 1,      // Maximum restrictions (March-May 2020 equivalent)
	GradualRecovery = 2,    // Phased reopening
	NewNormal = 3,          // Structural behavioral changes
	PostPandemic = 4        // Pandemic fully ended
};

// COVID-19 pandemic state management class
// Based on Poledna et al. (2023) but WITHOUT Behavioral Learning Equilibrium (BLE)
// DEPLOYERS relies on natural selection, not statistical learning

class CPandemicState
{
protected:
	// Activation status
	bool _isPandemicActive = false;
	long _pandemicStartMonth = 999999;
	long _pandemicEndMonth = 999999;

	// Phase management
	PandemicPhase _currentPhase = PandemicPhase::PrePandemic;
	long _acutePhaseMonths = 3;     // Default: 3 months acute lockdown
	long _recoveryPhaseMonths = 12; // Default: 12 months recovery

	// Lockdown intensity (0 = no lockdown, 1 = full lockdown)
	double _lockdownIntensity = 0.0;

	// Sector-specific shock multipliers (1.0 = no shock)
	map<GoodType, double> _sectorDemandShock;
	map<GoodType, double> _sectorSupplyShock;
	map<GoodType, double> _sectorLaborShock;

	// Physical confinement factors: 0.0 = fully closed, 1.0 = fully open
	// Different from demand shocks - this models PHYSICAL closure, not preferences
	map<GoodType, double> _sectorConfinement;

	// Accumulated forced savings from confinement (for statistics)
	double _totalForcedSavings = 0.0;
	double _monthlyForcedSavings = 0.0;

	// External sector shocks
	double _exportDemandFactor = 1.0;
	double _importSupplyFactor = 1.0;

	// Historical data mode (for Austria validation test)
	bool _useHistoricalData = false;
	string _historicalDataFile;
	vector<map<GoodType, double>> _historicalConfinement;  // Monthly sector confinement factors
	vector<double> _historicalKurzarbeit;                   // Monthly Kurzarbeit coverage rates
	vector<double> _historicalExports;                      // Monthly export demand factors
	vector<double> _historicalImports;                      // Monthly import supply factors
	vector<string> _historicalPhaseNames;                   // Phase names from CSV

	// Policy parameters (Kurzarbeit/short-time work)
	double _furloughSubsidyRate = 0.80;   // 80% wage maintenance
	double _furloughCoverageRate = 0.50;  // 50% of firms participate
	double _creditAvailabilityFactor = 1.0;

	// Statistics tracking
	double _totalKurzarbeitPaid = 0.0;
	double _monthlyKurzarbeitPaid = 0.0;  // Reset each month for logging
	int _firmsOnKurzarbeit = 0;
	int _workersOnKurzarbeit = 0;
	double _pandemicProductionReduction = 0.0;

public:
	CPandemicState() {};
	~CPandemicState() {};

	// Accessors
	bool isPandemicActive() const { return _isPandemicActive; }
	bool& pandemicActive() { return _isPandemicActive; }
	long getPandemicStartMonth() const { return _pandemicStartMonth; }
	long& pandemicStartMonth() { return _pandemicStartMonth; }
	long getPandemicEndMonth() const { return _pandemicEndMonth; }
	long& pandemicEndMonth() { return _pandemicEndMonth; }
	PandemicPhase getCurrentPhase() const { return _currentPhase; }
	PandemicPhase& currentPhase() { return _currentPhase; }
	double getLockdownIntensity() const { return _lockdownIntensity; }
	double& lockdownIntensity() { return _lockdownIntensity; }
	double getExportDemandFactor() const { return _exportDemandFactor; }
	double& exportDemandFactor() { return _exportDemandFactor; }
	double getImportSupplyFactor() const { return _importSupplyFactor; }
	double& importSupplyFactor() { return _importSupplyFactor; }
	double getFurloughSubsidyRate() const { return _furloughSubsidyRate; }
	double& furloughSubsidyRate() { return _furloughSubsidyRate; }
	double getFurloughCoverageRate() const { return _furloughCoverageRate; }
	double& furloughCoverageRate() { return _furloughCoverageRate; }
	double getCreditAvailabilityFactor() const { return _creditAvailabilityFactor; }
	double& creditAvailabilityFactor() { return _creditAvailabilityFactor; }

	// Statistics accessors
	int getFirmsOnKurzarbeit() const { return _firmsOnKurzarbeit; }
	int& firmsOnKurzarbeit() { return _firmsOnKurzarbeit; }
	int getWorkersOnKurzarbeit() const { return _workersOnKurzarbeit; }
	int& workersOnKurzarbeit() { return _workersOnKurzarbeit; }
	double getTotalKurzarbeitPaid() const { return _totalKurzarbeitPaid; }
	double& totalKurzarbeitPaid() { return _totalKurzarbeitPaid; }
	double getMonthlyKurzarbeitPaid() const { return _monthlyKurzarbeitPaid; }
	double& monthlyKurzarbeitPaid() { return _monthlyKurzarbeitPaid; }
	double getPandemicProductionReduction() const { return _pandemicProductionReduction; }
	double& pandemicProductionReduction() { return _pandemicProductionReduction; }

	// Phase duration accessors
	long& acutePhaseMonths() { return _acutePhaseMonths; }
	long& recoveryPhaseMonths() { return _recoveryPhaseMonths; }

	// Forced savings accessors
	double getTotalForcedSavings() const { return _totalForcedSavings; }
	double& totalForcedSavings() { return _totalForcedSavings; }
	double getMonthlyForcedSavings() const { return _monthlyForcedSavings; }
	double& monthlyForcedSavings() { return _monthlyForcedSavings; }

	// Methods
	void initialize();
	void reinitializeTiming();  // Recalculate pandemic timing after FinishCalibrationAt is finalized
	void updatePhase(long currentMonth);
	void updatePhaseHistorical(long currentMonth);  // Apply historical data for Austria validation
	void loadHistoricalData(const string& filename);  // Load CSV with monthly confinement data
	double getDemandShock(GoodType sector) const;
	double getSupplyShock(GoodType sector) const;
	double getLaborShock(GoodType sector) const;
	double getConfinementFactor(GoodType sector) const;
	bool isActive(long month) const;
	bool isInPandemicPeriod() const;  // Check if currently in pandemic period (convenience method)
	void loadSectorImpacts();
	void loadSectorConfinement();
	void loadPolednaLaborShocks();  // Load AMS March 2020 unemployment-based sector shocks (Poledna replication)
	void resetMonthlyStats();

	// Historical data mode accessors
	bool isUsingHistoricalData() const { return _useHistoricalData; }
	const string& getHistoricalDataFile() const { return _historicalDataFile; }

	// Forced savings accumulator (called from Individual.cpp during confinement)
	void addForcedSavings(double amount) { _monthlyForcedSavings += amount; _totalForcedSavings += amount; }

	// Kurzarbeit tracking (called from Producer.cpp and Government.cpp)
	void addKurzarbeitPayment(double amount) { _monthlyKurzarbeitPaid += amount; _totalKurzarbeitPaid += amount; }

	// Sector shock map accessors
	map<GoodType, double>& sectorDemandShock() { return _sectorDemandShock; }
	map<GoodType, double>& sectorSupplyShock() { return _sectorSupplyShock; }
	map<GoodType, double>& sectorLaborShock() { return _sectorLaborShock; }
	map<GoodType, double>& sectorConfinement() { return _sectorConfinement; }

	friend ofstream& operator<<(ofstream& ofstrm, const CPandemicState& state);
	friend ifstream& operator>>(ifstream& ifstrm, CPandemicState& state);
};

ofstream& operator<<(ofstream& ofstrm, const CPandemicState& state);
ifstream& operator>>(ifstream& ifstrm, CPandemicState& state);

const char* getPandemicPhaseName(PandemicPhase phase);

///////////////////////    CWorld    ////////////////////////////////

class CWorld
{
	CNeighborsVectorManager _NeighboringWorkersManager;
	CNeighborsVectorManager& NeighboringWorkersManager() { return _NeighboringWorkersManager; };

	CNeighborsVectorManager _NeighboringProducersManager;

	std::deque<std::pair<AgentID, GoodType>> _recentDemandEvents;

public:

	CNeighborsVectorManager& NeighboringProducersManager() { return _NeighboringProducersManager; };
	const CNeighborsVectorManager& getNeighboringWorkersManager() const { return _NeighboringWorkersManager; };
	const CNeighborsVectorManager& getNeighboringProducersManager() const { return _NeighboringProducersManager; };

	void addDemandEvent(AgentID buyerID, GoodType gType);
	GoodType getRandomGoodDemandedInNeighborhood(AgentID agentID);

	static string _SimulationName;
	static string& SimulationName() { return _SimulationName; };
	static const string& getSimulationName() { return _SimulationName; };

	static map<string, CSimulatedCountry*>* _pSimulatedCountries;
	static map<string, CSimulatedCountry*>& SimulatedCountries() { return *_pSimulatedCountries; };
	static const map<string, CSimulatedCountry*>& getSimulatedCountries() { return *_pSimulatedCountries; };

	CSAM* _pSAM;
	CFigaro* _pFIGARO;
	CFigaro*& pFigaro() { return _pFIGARO; };
	const CFigaro* getpFigaro() const { return _pFIGARO; };
	CGovernment* _pGovernment;
	map<GoodType, CExtSect*> _pExtSect;
	// CBank* _pCentralBank;
	CFinancialMarket* _pFinancialMarket;
	CFinancialMarket*& pFinancialMarket() { return _pFinancialMarket; };
	CFinancialMarket& FinancialMarket() { return *_pFinancialMarket; };
	const CFinancialMarket& getFinancialMarket() const { return *_pFinancialMarket; };
	long _InitMonth;

	long _nWorkers;
	long& nWorkers() { return _nWorkers; };
	long getnWorkers() const { return _nWorkers; };

	bool _bIndivsWealthTargetReached = false;
	long _monthIndivsWealthTargetReached = -1;
	bool& bIndivsWealthTargetReached() { return _bIndivsWealthTargetReached; };
	bool getbIndivsWealthTargetReached() const { return _bIndivsWealthTargetReached; };
	long getMonthIndivsWealthTargetReached() const { return _monthIndivsWealthTargetReached; };

	// GFCF stability tracking for dynamic AssistedProductionUpto
	bool _bGFCFStabilized = false;
	long _monthGFCFStabilized = -1;
	double _prevGFCF = 0;
	int _consecutiveStableGFCFMonths = 0;
	bool _bCalibrationEndpointSet = false;  // flag to set FinishCalibrationAt only once
	bool getbGFCFStabilized() const { return _bGFCFStabilized; };
	long getMonthGFCFStabilized() const { return _monthGFCFStabilized; };
	void updateAssistedProductionUptoFromGFCFStability();

	// COVID-19 Pandemic state (Darwinian approach - no BLE)
	CPandemicState _pandemicState;
	CPandemicState& PandemicState() { return _pandemicState; }
	const CPandemicState& getPandemicState() const { return _pandemicState; }
	void initializePandemic();
	void updatePandemicState();

	// Behavioral Learning Equilibrium (BLE) - Poledna parallel comparison mode
	CBLEExpectations _bleExpectations;
	CBLEExpectations& BLEExpectations() { return _bleExpectations; }
	const CBLEExpectations& getBLEExpectations() const { return _bleExpectations; }
	void initializeBLE();
	void updateBLEExpectations();

protected:

	double _InitSalaryCalibFactor;
	CTypeDoubleMap _ConsumPXFactor;
	CTypeDoubleMap& ConsumPXFactor();
	CTypeDoubleMap _RefTotalHouseholdPXConsum;
	CTypeDoubleMap& RefTotalHouseholdPXConsum();

	CTypeDoubleMap _GovConsumPXFactor;
	CTypeDoubleMap& GovConsumPXFactor();
	CTypeDoubleMap _RefGovConsum;
	CTypeDoubleMap& RefGovConsumPX();
	double _RefGDP_VA;
	double& RefGDP_VA() { return _RefGDP_VA; }

	CTypeDoubleMap _FixCapitalProductivity;
	CTypeDoubleMap& FixCapitalProductivity();

	long _currMonth;

	bool _bRunning, _bFinished, _bPaused, _bPlotBorders;

	vector<CWorker*>* _pWorkers;
	vector<CProducer*>* _pProducers;

	vector<CAgent*>* _pInteractingAgents;
	vector<AgentID>* _pRandomListOfIndivIDs;

	vector<vector<GoodQtty>>* _pExtSectorsToLabor = nullptr;
	vector<vector<GoodQtty>>*& pExtSectorsToLabor() { return _pExtSectorsToLabor; };
	vector<SortedAgent> _sorted_ProducersActiveDay; // typedef tuple<ActiveDay, AgentID, AgentType> SortedAgent;
	vector<SortedAgent>& sorted_ProducersActiveDay() { return _sorted_ProducersActiveDay; };

	static std::vector<int> _yearlyDeathsByAge;

public:

	double getRefGDP_VA() const { return _RefGDP_VA; }
	double& InitSalaryCalibFactor() { return _InitSalaryCalibFactor; };

	CFigaro& Figaro() { return *_pFIGARO; };
	const CFigaro& getFigaro() const { return *_pFIGARO; };
	CTypeDoubleMap getConsumPXFactor() const;
	CTypeDoubleMap getRefTotalHouseholdPXConsum() const;
	CTypeDoubleMap getGovConsumPXFactor() const;
	CTypeDoubleMap getRefGovConsumPX() const;
	CTypeDoubleMap getFixCapitalProductivity() const;
	const vector<SortedAgent>& getsorted_ProducersActiveDay() const { return _sorted_ProducersActiveDay; }; // typedef tuple<ActiveDay, AgentID, AgentType> SortedAgent;
	const vector<vector<GoodQtty>>& getExtSectorsToLabor() const { return *_pExtSectorsToLabor; };
	double getInitSalaryCalibFactor() const { return _InitSalaryCalibFactor; };
	void writeProducers(ofstream& ofstrm) const;
	void readProducers(ifstream& ifstrm);
	void writeWorkers(ofstream& ofstrm) const;
	void readWorkers(ifstream& ifstrm);
	void setupExtSectorsToLaborArray();
	void writeSnapshot(ofstream& ofstrm) const;
	void readSnapshot(string fileNameExt);
	bool hasSnapshotSection(const string& filename) const;
	void writerndstatus(ofstream& ofstrm) const;
	bool readrndstatus(string fileNameExt);
	void AdjustSAMParameters();
	void DismantleAndStartNewProducers();
	long DismantleProducer(CProducer*& pProducer);
	void writeFirmBirthDeathHistogram();
	void cleanIOFilesForReload();
	void recalculateSaveMonths();
	bool& bRunning();
	bool& bFinished();
	bool& bPaused();
	bool& bPlotBorders();
	void readDEPData(string& filename);
	void initializeWorld();

	CAgent* getFIDAgent(CAgentFID fid) const;
	CWorld();
	~CWorld();
	void writeTimeAndDate(ofstream& ofstrm) const;
	const double getRandom01();

	//------------------  const public Accessors  ----------------------
	static void ERRORmsg(string str, bool quit = true);

	bool getbRunning() const;
	bool getbFinished() const;
	bool getbPaused() const;
	const vector<CWorker*>* getpWorkers() const;
	long getNWorkers() const;
	const vector<CProducer*>* getpProducers() const;
	long getNProducers() const;

	//------------------  modifier accessors  ----------------------

	vector<CWorker*>*& pWorkers();
	vector<CWorker*>& Workers();
	CWorker* newWorker();
	void initializeWorkers();

	vector<CProducer*>*& pProducers();
	vector<CProducer*>& Producers();
	CProducer* newProducer(AgentType producerType, CAgent* pinitialOwner);

	/////////////////////  running the World  ////////////////////////////

	bool LoadWorld();
	void loadNeighborsRNGStates(const string& fileNameExt);
	void defineProductsAndProducersFromSAM() const;
	void GrossOutputAndDepreciationRate() const;
	void assignPointersToBanks(string fileNameExt);
	void SetPricesToOne();
	void loadNeighborsRNGStateFromStream(ifstream& ifstrm, CNeighborsVectorManager& manager);
	bool SaveWorld() const;
	void saveNeighborsRNGState(ofstream& outfile, const CNeighborsVectorManager& manager) const;
	void InitializeSimulation();
	bool RunSimulation();
	void InitializeAllNeighbors();
	void getNeighborsWorkersOf(CAgent* pAgent, vector<CAgent*>& neighbors);
	void getNeighborsProducersOf(CAgent* pAgent, vector<CAgent*>& myNeighbors);
	void initializeMonth();
	void runOneMonth();
	void logSimulationMilestones();  // Log stage transition milestones to log file
	void writeExternalSectorsIO();
	void waitMyExternalCountriesToReadMyIO() const;
	void waitMyExternalCountriesToWriteTheirIO() const;
	void readMyExternalSectorsIO() const;
	void readMyExternalSectorsIO_v2() const; // improved parser supporting disaggregated external sectors

	friend long ::currMonth();
	friend ifstream& operator>>(ifstream& ifstrm, CWorld& world);
	friend ofstream& operator<<(ofstream& ofstrm, const CWorld& world);
};

ifstream& operator>>(ifstream& ifstrm, CWorld& world);
ofstream& operator<<(ofstream& ofstrm, const CWorld& world);

////////////////////////  CDeployersGUI  ///////////////////////////////

class CPlotDefinition
{
public:
	string windowTitle;
	string plotTitle;

	long PlotType;
	long PlotX0, PlotY0;

	AgentType aType;
	string aName;

	bool UsecurrMonthNPoints;
	long NPoints;
	string XLabel, YLabel;
	double XLow, XHigh, YLow, YHigh;

	string XAxisLabelFormat, YAxisLabelFormat;
	string TooltipFormat, XAxisTooltipFormat, YAxisTooltipFormat;
	const vector<string>* pLabel_array; // "P001_Agric, P002_Energy..."

	CPlotDefinition();
};

// =====================  CPlotsInputParameters  =====================

class CPlotsInputParameters : public map<string, CPlotDefinition> // windowTitle, curves
{
public:
	CPlotsInputParameters() {};
	~CPlotsInputParameters() {};
};

#ifdef DEPLOYERS_GRAPHICS

class CDeployersGUI : public GlgObjectC
{
public:
	vector< CPlotsXYBase* > _Plots;
	vector< CPlotsXYBase* >& Plots() { return _Plots; };

	GlgLong TimerID;
	bool _GUIinitialized;
	bool& GUIinitialized() { return _GUIinitialized; };
	bool getGUIinitialized() const { return _GUIinitialized; };

public:
	CDeployersGUI(void);
	virtual ~CDeployersGUI(void);

	// ------------------- Interface SETUP -------------------

	void SetSize(GlgLong x, GlgLong y, GlgLong width, GlgLong height);
	void Initialize(void);
	void StartUpdates(void);
	void StopUpdates(void);

	//  ------------------- OUTPUT --------------------------

	void MessageStatus(string msg);
	void UpdateDrawing_OnTimer(void);
	void UpdateDrawing_WhileRunning(void);
	void UpdateAllPlotXScales(double newNYears);  // Update all plot X-axis scales
	void Update_AfterLoadWorld(void);

	//  ------------------- INPUT --------------------------

	virtual void Input(GlgObjectC& callback_viewport, GlgObjectC& message);

	//void Trace(GlgObjectC& callback_viewport, GlgTraceCBStruct* trace_data);

	// Input buttons
	bool GUISaveWorld();
	bool GUILoadWorld();
	void RunWorldButton(double value);
};

////////////////////////  PlotsXY  ///////////////////////////////

class DataPoint
{
public:
	double Xvalue;
	double Yvalue;
	bool value_valid;
	bool has_time_stamp;

	AgentType type;
	AgentID ID;

public:
	DataPoint(void) {
		Xvalue = Yvalue = -1;// time_stamp = -1;
		value_valid = has_time_stamp = false;
		type = -999;
		ID = -1;
	};
	~DataPoint(void) {};
};

class AgentInfo
{
public:
	AgentType type;
	AgentID ID;
};

class CPlotVars
{
public:
	string PlotName; // Annotation
	GlgLong PlotType; // 1:line, 32:markers, 33:linemarkers
	string TooltipFormat;
	long LineWidth;
	long MarkerVisibility; // 0: hide, 1: show
	long MarkerType; // 1:cross, 2:square, 4:filled square, 8: circle, 16: filled circle
	long MarkerSize;
	double fR, fG, fB; // FillColor
	CColor color;
};

class CPlotsXYBase : public GlgObjectC
{
protected:

	// -------------------------------------------------------------
	CPlotDefinition _PlotDefinition;
	CPlotDefinition& PlotDefinition() { return _PlotDefinition; };

	CPlotsVars _PlotsVars;
	CPlotsVars& PlotsVars() { return _PlotsVars; };
	CPlotVars& PlotsVars(long i) { return PlotsVars().at(i); };
	// -------------------------------------------------------------

	string UIname;

	long x, y, width, hight;

	GlgObjectC ChartVP;
	GlgObjectC Chart;

	GlgLong BufferSize;
	GlgLong BufferXSpan;
	GlgLong UpdateInterval;
	GlgLong TimerID;

	string title;

	string XLabel;
	double XLow, XHigh;
	string XAxisLabelFormat;
	string XAxisTooltipFormat;
	string YLabel;
	double YLow, YHigh;
	string YAxisLabelFormat;
	string YAxisTooltipFormat;

	string TooltipFormat;

	long PlotType; // 1:line, 32:markers, 33:linemarkers
	long MarkerVisibility; // 0: hide, 1: show
	long MarkerType; // 1:cross, 2:square, 4:filled square, 8: circle, 16: filled circle
	long MarkerSize;

	long nextMonth;

	vector<string> PlotNames;
	const vector<string>* pLabel_array;

	// Input and Trace callbacks.
	virtual void Input(GlgObjectC& callback_viewport, GlgObjectC& message);
	virtual void Trace(GlgObjectC& callback_viewport, GlgTraceCBStruct* trace_info);

	void Init(void);

public:
	CPlotsXYBase(void);
	~CPlotsXYBase(void);

	string windowTitle;
	// Store object IDs for each plot. Used for performance optimization.
	GlgObjectC* Plots;
	long NumPlots; // Number of plots in the chart, to be determined
	virtual void InitAfterH(void) = 0;
	virtual void updatePlots(void);
	virtual GlgBoolean GetPlotPoint(GlgLong plot_index, DataPoint& data_point) = 0;

	void InitBeforeH(void);
	void SetChartSpan(GlgLong span);
	void UpdateXScale(double newNYears);  // Update X-axis scale when NYears changes dynamically
	void RestoreInitialYRanges(void);
	GlgLong ZoomToMode(void);
	void AbortZoomTo(void);
	void PushPlotPoint(GlgObjectC& plot, DataPoint& data_point);
	GlgLong getcounter() const { return nextMonth; };
	double GetCurrTime(void);
	void StartUpdates(void);
	void StopUpdates(void);
	void SetSize(GlgLong x, GlgLong y, GlgLong width, GlgLong height);
	void error(CONST char* string, GlgBoolean quit);
	virtual void MarkDataSample(double x, double y);
	GlgDataSample* GetDataSample(double x, double y);
	long GetClickCount(double x, double y);

	friend void UpdateXYChart(CPlotsXYBase* chart, GlgLong* timer_id);
};

// --------------------------------------------------------------------------------

class CDEPplotWindow : public CPlotsXYBase
{
	vector<AgentInfo> _agentsInfo;
	vector<AgentInfo>& agentsInfo() { return _agentsInfo; };
public:
	CDEPplotWindow(const CPlotDefinition& plotDefinition);
	~CDEPplotWindow() {};
	void CreatePlots();

	virtual void InitAfterH(void);
	virtual void updatePlots();
	virtual GlgBoolean GetPlotPoint(GlgLong plot_index, DataPoint& data_point);
	virtual GlgBoolean GetPlotPoint(GlgLong plot_index, GlgLong point_index, DataPoint& data_point);
	virtual void MarkDataSample(double x, double y);
};

#endif

////////////////////////  CGDPtracker  ///////////////////////////////

class CGDPcomponent {
public:
	GoodType _gType;
	double _quantity;
	double _price;

	CGDPcomponent();
	CGDPcomponent(GoodType gTy, double q, double p);

	friend ofstream& operator<<(ofstream& ofstrm, const CGDPcomponent& comp);
	friend ifstream& operator>>(ifstream& ifstrm, CGDPcomponent& comp);
};

ofstream& operator<<(ofstream& ofstrm, const CGDPcomponent& comp);
ifstream& operator>>(ifstream& ifstrm, CGDPcomponent& comp);

class CGDPcomponents : public std::vector<CGDPcomponent> {
public:
	CGDPcomponents() {};

	friend ofstream& operator<<(ofstream& ofstrm, const CGDPcomponents& comps);
	friend ifstream& operator>>(ifstream& ifstrm, CGDPcomponents& comps);
};

ofstream& operator<<(ofstream& ofstrm, const CGDPcomponents& comps);
ifstream& operator>>(ifstream& ifstrm, CGDPcomponents& comps);

class CGDPtracker {
private:
	int _base_Month;
	CGDPcomponents _GDPcomponents;
	std::vector<CGDPcomponents> _Month_data;
	std::vector<double> _nominal_gdp;
	std::vector<double> _real_gdp;
	std::vector<double> _gdp_deflator;
	double _last_gdp_deflator;

public:
	CGDPtracker() : _base_Month(-1), _last_gdp_deflator(0) {}

	void addToGDPComponent(GoodType gType, GoodQtty qtty);
	void addMonthData();
	void calculateGDP();
	const double getGDPInflationRate() const;
	const double getMonthlyGDPInflationRate() const;
	void printResults() const;
	int& base_Month() { return _base_Month; };
	int getbase_Month() const { return _base_Month; };
	double getlast_gdp_deflator() const { return _last_gdp_deflator; };
	const double getnominal_gdp() const {
		if (_nominal_gdp.size() > 0)
			return _nominal_gdp.back();
		else
			return 0;
	};
	const double getreal_gdp() const {
		if (_real_gdp.size() > 0)
			return _real_gdp.back();
		else
			return 0;
	};

	friend class CData;
	friend ofstream& operator<<(ofstream& ofstrm, const CGDPtracker& tracker);
	friend ifstream& operator>>(ifstream& ifstrm, CGDPtracker& tracker);
};

ofstream& operator<<(ofstream& ofstrm, const CGDPtracker& tracker);
ifstream& operator>>(ifstream& ifstrm, CGDPtracker& tracker);

////////////////////////  CCPItracker  ///////////////////////////////

class CCPIcomponent {
public:
	GoodType _gType;
	double _quantity;
	double _price;

	CCPIcomponent();
	CCPIcomponent(GoodType gTy, double q, double p);

	friend ofstream& operator<<(ofstream& ofstrm, const CCPIcomponent& comp);
	friend ifstream& operator>>(ifstream& ifstrm, CCPIcomponent& comp);
};

ofstream& operator<<(ofstream& ofstrm, const CCPIcomponent& comp);
ifstream& operator>>(ifstream& ifstrm, CCPIcomponent& comp);

class CCPIcomponents : public std::vector<CCPIcomponent> {
public:
	CCPIcomponents() {};

	friend ofstream& operator<<(ofstream& ofstrm, const CCPIcomponents& comps);
	friend ifstream& operator>>(ifstream& ifstrm, CCPIcomponents& comps);
};

class CCPItracker {
private:
	int _base_Month;
	CCPIcomponents _CPIcomponents;
	std::vector<CCPIcomponents> _Month_data;
	std::vector<double> _BasketVal;
	std::vector<double> _CPI;
	std::vector<double>& CPI() { return _CPI; };
	std::vector<double> _period_indices;
	std::vector<double> _smooth_CPI;  // Add: smoothed CPI using 12-month moving average

public:
	CCPItracker() : _base_Month(-1) {};

	const std::vector<double>& getPeriodIndices() const { return _period_indices; }
	void addToCPIComponents(GoodType gType, GoodQtty qtty);
	void addMonthData();
	void calculateCPIperCent();
	void calculateSmoothCPIperCent();  // Add: new method for smoothed CPI
	const double getCPIInflationRate() const;
	const double getSmoothCPIInflationRate() const;  // Add: new method for smooth inflation
	void printResults() const;
	int& base_Month() { return _base_Month; };
	int getbase_Month() const { return _base_Month; };
	const std::vector<double>& getCPI() const { return _CPI; };
	const std::vector<double>& getSmoothCPI() const { return _smooth_CPI; };  // Add: getter for smooth CPI
	double getlastCPI() const
	{
		if (_CPI.size() > 0)
			return _CPI.back();
		else
			return 0;
	};
	const double getBasketVal() const
	{
		if (_BasketVal.size() > 0)
			return _BasketVal.back();
		else
			return 0;
	};

	friend class CData;
	friend ofstream& operator<<(ofstream& ofstrm, const CCPItracker& tracker);
	friend ifstream& operator>>(ifstream& ifstrm, CCPItracker& tracker);
};

ofstream& operator<<(ofstream& ofstrm, const CCPItracker& tracker);
ifstream& operator>>(ifstream& ifstrm, CCPItracker& tracker);

////////////////////////  CData  ///////////////////////////////

class CPlotData : public map<GoodType, CVectorDoubles> // gType, {values}
{
public:
	friend ofstream& operator<<(ofstream& ofstrm, const CPlotData& plotDat);
	friend ifstream& operator>>(ifstream& ifstrm, CPlotData& plotDat);

	CPlotData() = default;
};

ofstream& operator<<(ofstream& ofstrm, const CPlotData& plotDat);
ifstream& operator>>(ifstream& ifstrm, CPlotData& plotDat);

// --------------------------------------------------------------------------------

class CPlotsData : public map<string, CPlotData> // "HousehConsum", gType_curves
{
public:
	friend ofstream& operator<<(ofstream& ofstrm, const CPlotsData& plotsDat);
	friend ifstream& operator>>(ifstream& ifstrm, CPlotsData& plotsDat);

	CPlotsData() = default;
};

ofstream& operator<<(ofstream& ofstrm, const CPlotsData& plotsDat);
ifstream& operator>>(ifstream& ifstrm, CPlotsData& plotsDat);

// --------------------------------------------------------------------------------

class CData
{
protected:
	double _NYears;
	long _NMonths;
	long _FinishCalibrationAt;  // serialized calibration end month

	vector<int> _SaveMonths;  // absolute months to save (computed from SaveMonthsAfterCalibration or SaveMonthsFromMonth0)
	vector<int> _SaveMonthsAfterCalibration;  // user input: months relative to FinishCalibrationAt
	vector<int> _SaveMonthsFromMonth0;  // user input: absolute months from simulation start

	string _InputFileName;
	string _HistoricalDataFile;  // CSV file for historical pandemic validation (Austria)

	static std::map<string, double> _mInputParameters;
	CPlotsData _PlotsData;
	CPlotsInputParameters _PlotsInputParameters;

	CGDPtracker _GDPtracker;
	CCPItracker _CPItracker;

	CMovingAverage _MA_TotAssistedProd;

	vector<GoodQtty> _FirmBirths;
	vector<GoodQtty> _FirmDeaths;
	double _avgFirmBirths;
	double _avgFirmDeaths;

	vector<AgentType> _InitialProducerTypes;
	CGoods _nCurrentPProducers;
	GoodQtty _nTotalCurrentProducers;
	CGoods _Inventories;
	CGoods _ToSell;
	CGoods _Dismantled;
	CGoods _TotalToIndustry;

	CGoods _DataGrossOutput_mu;
	double _TotalDataGrossOutput_mu;

	double _DataHouseholdsGFCF_mu;

	double _TotalDataGov_mu;
	double _DataGovGFCF_mu;

	double _prevavgDeprecError2;
	CGoods _TotProduced;
	double _TotalProduction;
	CGoods _TotSupply;
	CGoods _WorkerID;
	CGoods _TotDemand;
	CGoods& WorkerID() { return _WorkerID; };
	CGoods _IndivsValues;
	CGoods& IndivsValues() { return _IndivsValues; };
	CGoods _ProdsValues;
	CGoods& ProdsValues() { return _ProdsValues; };
	CGoods _ProducerID;
	CGoods& ProducerID() { return _ProducerID; };
	CGoods _AssistedProd;
	CGoods& AssistedProd() { return _AssistedProd; };
	double _TotalAssistedProd;
	double& TotalAssistedProd() { return _TotalAssistedProd; };
	CGoods _Exports;
	CGoods& Exports() { return _Exports; };
	CGoods _Imports;
	CGoods& Imports() { return _Imports; };
	GoodQtty _TotalIndivsGoodsAndServ;
	CTypeDoubleMap _TotalHouseholdPXConsum;
	CTypeDoubleMap _GovConsum;
	CTypeDoubleMap _DepreciationRate;
	CTypeDoubleMap _DepreciationFraction;
	CTypeDoubleMap _prevDepreciationFraction;
	CTypeDoubleMap _DepreciationAdjust;
	CTypeDoubleMap _prevDepreciationAdjust;
	GoodQtty _GDPnominal;
	GoodQtty _TotalUnits;

	double _Unemployment;
	double _prevUnemployment;
	CHistory _UnemploymentHistory = CHistory(12);  // Initialize with default value
	double _TotalSalaries;
	double _TotalWorkedTime;
	CGoods  _nActiveWorkers;
	GoodQtty _nFullTimeWorkers;
	double _totalFullTimeWork;
	double& totalFullTimeWork() { return _totalFullTimeWork; };
	GoodQtty _nPartTimeWorkers;
	double _totalPartTimeWork;
	double& totalPartTimeWork() { return _totalPartTimeWork; };
	double _PartToFullTimeWorkersRatio;
	double& PartToFullTimeWorkersRatio() { return _PartToFullTimeWorkersRatio; };
	CTypeDoubleMap _MarketPrices;
	CStringDoubleMap _nEmplPerFirmsize;
	CTypeDoubleMap _SectorMarkups;
	CStringDoubleMap _nFirmsPerFirmsize;
	vector<double> _VABpE;
	vector<double> _VABpEv;
	vector<GoodQtty> _VABpEn;

	GoodQtty _TotalInitialCash;
	GoodQtty _TotalIndivsCash;
	GoodQtty _TotalIndivsBank;
	GoodQtty _TotalProducersCash;
	GoodQtty _IndivsWealth;
	GoodQtty _ProducersWealth;
	GoodQtty _dProducersWealth;
	vector<SortedAgent> _sorted_IndivsWealth; // typedef tuple<Wealth, AgentID, AgentType> SortedAgent;
	vector<SortedAgent> _sorted_ProducersWealth; // typedef tuple<Wealth, AgentID, AgentType> SortedAgent;

	GoodQtty _TotalBanksCash;
	vector<AgentID>* _pBanksIDs;

	GoodQtty _TotLoansQtty;
	double _TotLoansInterests;
	double _AvgLoansInterests;

public:

	CMovingAverage& MA_TotAssistedProd() { return _MA_TotAssistedProd; };
	const CMovingAverage& getMA_TotAssistedProd() const { return _MA_TotAssistedProd; };
	double getTotalAssistedProd() const { return _TotalAssistedProd; };
	string _MATRIXfileName;
	string& MATRIXfileName() { return _MATRIXfileName; };
	const string& getMATRIXfileName() const { return _MATRIXfileName; };

	CHistory& UnemploymentHistory() { return _UnemploymentHistory; }
	const CHistory& getUnemploymentHistory() const { return _UnemploymentHistory; }
	double getAvgUnemployment() const { return _UnemploymentHistory.getaverage(); }

	CGDPtracker& GDPtracker() { return _GDPtracker; };
	const CGDPtracker& getGDPtracker() const { return _GDPtracker; };
	CCPItracker& CPItracker() { return _CPItracker; };
	const CCPItracker& getCPItracker() const { return _CPItracker; };
	GoodQtty& TotLoansQtty() { return _TotLoansQtty; };
	double& TotLoansInterests() { return _TotLoansInterests; };
	double& AvgLoansInterests() { return _AvgLoansInterests; };

	GoodQtty getTotLoansQtty() const { return _TotLoansQtty; };
	double getTotLoansInterests() const { return _TotLoansInterests; };
	double getAvgLoansInterests() const { return _AvgLoansInterests; };

	double& TotalSalaries() { return _TotalSalaries; };
	double& TotalWorkedTime() { return _TotalWorkedTime; };
	double getTotalSalaries() const { return _TotalSalaries; };
	double getTotalWorkedTime() const { return _TotalWorkedTime; };
	double& avgFirmBirths() { return _avgFirmBirths; };
	double& avgFirmDeaths() { return _avgFirmDeaths; };
	vector<GoodQtty>& FirmBirths() { return _FirmBirths; };
	const vector<GoodQtty>& getFirmBirths() const { return _FirmBirths; };
	vector<GoodQtty>& FirmDeaths() { return _FirmDeaths; };
	const vector<GoodQtty>& getFirmDeaths() const { return _FirmDeaths; };
	CGoods& TotProduced() { return _TotProduced; }
	double& TotalProduction() { return _TotalProduction; }
	CGoods& TotSupply() { return _TotSupply; }
	CGoods& TotDemand() { return _TotDemand; }
	const CGoods& getTotProduced() const { return _TotProduced; }
	const double getTotalProduction() const { return _TotalProduction; }
	const CGoods& getTotSupply() const { return _TotSupply; }
	const CGoods& getTotDemand() const { return _TotDemand; }
	double& NYears();
	double getNYears() const;
	long getMonthsPerYear() const;
	long getCurrYear() const;
	long getEndMonth() const;  // Returns InitMonth + NMonths (the final simulation month)
	long& FinishCalibrationAt() { return _FinishCalibrationAt; }  // mutable accessor
	long getFinishCalibrationAt() const { return _FinishCalibrationAt; }  // Returns the serialized calibration end month
	string& InputFileName() { return _InputFileName; };
	const string& getInputFileName() const { return _InputFileName; };
	string& HistoricalDataFile() { return _HistoricalDataFile; };
	const string& getHistoricalDataFile() const { return _HistoricalDataFile; };
	vector<int>& SaveMonths() { return _SaveMonths; };
	const vector<int>& getSaveMonths() const { return _SaveMonths; };
	vector<int>& SaveMonthsAfterCalibration() { return _SaveMonthsAfterCalibration; };
	const vector<int>& getSaveMonthsAfterCalibration() const { return _SaveMonthsAfterCalibration; };
	vector<int>& SaveMonthsFromMonth0() { return _SaveMonthsFromMonth0; };
	const vector<int>& getSaveMonthsFromMonth0() const { return _SaveMonthsFromMonth0; };
	static const map<string, double>& InputParameters();
	static double InputParameter(string param);
	static map<string, double>& mInputParameters();
	static void writeInputParameters(ofstream& ofstrm);
	static void readInputParameters();
	static void initializeInputParameters();
	void writePlotsInputParameters(ofstream& ofstrm) const;
	void readPlotsInputParameters(string fileNameExt);
	void initializePlotsInputParameters() const;
	double gettotalFullTimeWork() const { return _totalFullTimeWork; };
	double gettotalPartTimeWork() const { return _totalPartTimeWork; };
	GoodQtty& nFullTimeWorkers() { return _nFullTimeWorkers; };
	GoodQtty& nPartTimeWorkers() { return _nPartTimeWorkers; };
	GoodQtty getnFullTimeWorkers() const { return _nFullTimeWorkers; };
	GoodQtty getnPartTimeWorkers() const { return _nPartTimeWorkers; };
	double getPartToFullTimeWorkersRatio() const { return _PartToFullTimeWorkersRatio; };
	vector<AgentType>& InitialProducerTypes() { return _InitialProducerTypes; };
	const vector<AgentType>& getInitialProducerTypes() const { return _InitialProducerTypes; };

	CPlotsInputParameters& PlotsInputParameters() { return _PlotsInputParameters; };
	const CPlotsInputParameters& getPlotsInputParameters() { return _PlotsInputParameters; };
	CPlotsData& PlotsData() { return _PlotsData; };
	const CPlotsData& getPlotsData() const { return _PlotsData; };
	void ResizePlotsDataForNewNYears(double newNYears);  // Resize all plot data vectors when NYears changes
	double PlotsDataSources(string plotWinName, GoodType gType, void* ptr = (void*)nullptr);

	CGoods& nCurrentPProducers() { return _nCurrentPProducers; };
	const CGoods& getnCurrentPProducers() const { return _nCurrentPProducers; };
	GoodQtty& nTotalCurrentProducers() { return _nTotalCurrentProducers; };
	GoodQtty getnTotalCurrentProducers() const { return _nTotalCurrentProducers; };
	GoodQtty& TotalInitialCash() { return _TotalInitialCash; };
	GoodQtty& TotalIndivsCash() { return _TotalIndivsCash; };
	GoodQtty& TotalProducersCash() { return _TotalProducersCash; };
	GoodQtty& TotalIndivsBank() { return _TotalIndivsBank; };
	GoodQtty& TotalBanksCash() { return _TotalBanksCash; };
	const GoodQtty CData::getTotalBanksCash() const { return _TotalBanksCash; }
	GoodQtty getTotalInitialCash() const { return _TotalInitialCash; };
	GoodQtty getTotalIndivsCash() const { return _TotalIndivsCash; };
	GoodQtty getTotalIndivsBank() const { return _TotalIndivsBank; };
	vector<double>& VABpE() { return _VABpE; };
	double& VABpE(GoodQtty firmSize);
	const vector<double>& getVABpE() const { return _VABpE; };
	double getVABpE(GoodQtty firmSize);
	vector<double>& VABpEv() { return _VABpEv; };
	double& VABpEv(GoodQtty firmSize);
	const vector<double>& getVABpEv() const { return _VABpEv; };
	double getVABpEv(GoodQtty firmSize);
	vector<GoodQtty>& VABpEn() { return _VABpEn; };
	GoodQtty& VABpEn(GoodQtty firmSize);
	const vector<GoodQtty>& getVABpEn() const { return _VABpEn; };
	GoodQtty getVABpEn(GoodQtty firmSize);

	CStringDoubleMap& nEmplPerFirmsize() { return _nEmplPerFirmsize; };
	CTypeDoubleMap& SectorMarkups() { return _SectorMarkups; };
	const CTypeDoubleMap& getSectorMarkups() const { return _SectorMarkups; };
	const CStringDoubleMap& getnEmplPerFirmsize() const { return _nEmplPerFirmsize; };
	CStringDoubleMap& nFirmsPerFirmsize() { return _nFirmsPerFirmsize; };
	const CStringDoubleMap& getnFirmsPerFirmsize() const { return _nFirmsPerFirmsize; };
	GoodQtty& GDPnominal() { return _GDPnominal; };
	GoodQtty getGDPnominal() const { return _GDPnominal; };
	GoodQtty& TotalUnits() { return _TotalUnits; };
	GoodQtty getTotalUnits() const { return _TotalUnits; };
	GoodQtty& TotalIndivsGoodsAndServ() { return _TotalIndivsGoodsAndServ; };
	CTypeDoubleMap& TotalHouseholdPXConsum() { return _TotalHouseholdPXConsum; };
	CTypeDoubleMap& GovConsumPX() { return _GovConsum; };
	CTypeDoubleMap& DepreciationRate() { return _DepreciationRate; };
	CTypeDoubleMap& DepreciationFraction() { return _DepreciationFraction; };
	CTypeDoubleMap& prevDepreciationFraction() { return _prevDepreciationFraction; };
	CTypeDoubleMap& DepreciationAdjust() { return _DepreciationAdjust; };
	CTypeDoubleMap& prevDepreciationAdjust() { return _prevDepreciationAdjust; };
	double& prevavgDeprecError2() { return _prevavgDeprecError2; };
	GoodQtty& IndivsWealth() { return _IndivsWealth; };
	GoodQtty getIndivsWealth() const { return _IndivsWealth; };
	GoodQtty getProducersWealth() const { return _ProducersWealth; };
	GoodQtty getdProducersWealth() const { return _dProducersWealth; };
	GoodQtty& ProducersWealth() { return _ProducersWealth; };
	GoodQtty& dProducersWealth() { return _dProducersWealth; };
	GoodQtty getTotalIndivsGoodsAndServ() const { return _TotalIndivsGoodsAndServ; };
	CTypeDoubleMap getTotalHouseholdPXConsum() const { return _TotalHouseholdPXConsum; };
	CTypeDoubleMap getGovConsumPX() const { return _GovConsum; };
	CTypeDoubleMap getDepreciationRate() const { return _DepreciationRate; };
	CTypeDoubleMap getDepreciationFraction() const { return _DepreciationFraction; };
	CTypeDoubleMap getprevDepreciationFraction() const { return _prevDepreciationFraction; };
	CTypeDoubleMap getDepreciationAdjust() const { return _DepreciationAdjust; };
	CTypeDoubleMap getprevDepreciationAdjust() const { return _prevDepreciationAdjust; };
	vector<SortedAgent>& sorted_IndivsWealth() { return _sorted_IndivsWealth; }; // typedef tuple<Wealth, AgentID, AgentType> SortedAgent;
	CGoods& nActiveWorkers() { return _nActiveWorkers; };
	CGoods& Dismantled();
	CGoods& Inventories();
	CGoods& ToSell();
	vector<SortedAgent>& sorted_ProducersWealth() { return _sorted_ProducersWealth; }; // typedef tuple<Wealth, AgentID, AgentType> SortedAgent;

	CData();
	~CData();
	void initialize_constructor();

	void initialize();
	void monthInitialize();
	long& NMonths() { return _NMonths; };
	long getNMonths() const { return _NMonths; };
	void AnalizeLastMonth();
	bool CheckAccountingBalance();

	double getUpscaleSimulationFactor() const;
	double toTotalPopulationYear() const;
	const CGoods& getDismantled() const;
	const CGoods& getInventories() const;
	const CGoods& getToSell() const;

	CTypeDoubleMap& MarketPrices() { return _MarketPrices; };
	const CTypeDoubleMap& getMarketPrices() const { return _MarketPrices; };

	double& Unemployment() { return _Unemployment; };
	const double getUnemployment() const { return _Unemployment; }
	double& prevUnemployment() { return _prevUnemployment; };
	double getprevUnemployment() const { return _prevUnemployment; };

	CGoods& TotalToIndustry() { return _TotalToIndustry; };
	GoodQtty& TotalToIndustryOf(GoodType gType) { return _TotalToIndustry.at(gType); };
	const CGoods& getTotalToIndustry() const { return _TotalToIndustry; };
	const GoodQtty getTotalToIndustry(GoodType gType) const { return _TotalToIndustry(gType); };

	CGoods& DataGrossOutput_mu() { return _DataGrossOutput_mu; };
	GoodQtty& DataGrossOutput_mu(GoodType gType) { return _DataGrossOutput_mu[gType]; };
	const CGoods& getDataGrossOutput_mu() const { return _DataGrossOutput_mu; };
	const GoodQtty getDataGrossOutput_mu(GoodType gType) const { return _DataGrossOutput_mu(gType); };
	double& TotalDataGrossOutput_mu() { return _TotalDataGrossOutput_mu; };
	const double& getTotalDataGrossOutput_mu() const { return _TotalDataGrossOutput_mu; };

	double& DataHouseholdsGFCF_mu() { return _DataHouseholdsGFCF_mu; };
	const double& getDataHouseholdsGFCF_mu() const { return _DataHouseholdsGFCF_mu; };

	double& DataGovGFCF_mu() { return _DataGovGFCF_mu; };
	const double& getDataGovGFCF_mu() const { return _DataGovGFCF_mu; };
	double& TotalDataGov_mu() { return _TotalDataGov_mu; };
	const double& getTotalDataGov_mu() const { return _TotalDataGov_mu; };

	const CGoods& getnActiveWorkers() const { return _nActiveWorkers; }
	const GoodQtty getnActiveWorkersOfType(GoodType type) const { return _nActiveWorkers.at(type); }

	const vector<SortedAgent>& getsorted_IndivsWealth() const // typedef tuple<Wealth, AgentID, AgentType> SortedAgent;
	{
		return _sorted_IndivsWealth;
	};

	const vector<SortedAgent>& getsorted_ProducersWealth() const // typedef tuple<Wealth, AgentID, AgentType> SortedAgent;
	{
		return _sorted_ProducersWealth;
	};

	friend ofstream& operator<<(ofstream& ofstrm, const CData& CData);
	friend ifstream& operator>>(ifstream& ifstrm, CData& CData);
};

ofstream& operator<<(ofstream& ofstrm, const CData& CData);
ifstream& operator>>(ifstream& ifstrm, CData& CData);

// --------------------------------------------------------------------------------

#endif //PCH_H
