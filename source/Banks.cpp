
// Banks.cpp

#include "./pch.h"

//======================  CBank  ===================================

/*
Banks intermediate in the financial markets by channelling savings from some
individuals or companies to others. This intermediation work consists of
two activities:

The first consists of attracting deposits, remunerated with
an interest rate to depositors. This attraction of
deposits is known as the liability transactions of the
banks. Depositors are remunerated with a
interest called liability interest but, in addition, they receive
a number of liquidity related services that
provide the accounts: payroll collection, direct debit
of receipts, possibility of payment by check or card, etc.

The second consists of lending (providing credit) to other
persons or companies with the funds obtained: these are
the asset operations. Those who receive the credits
must pay an asset rate of interest on them. The difference
between the asset and liability interest rate is
calls the brokerage margin and reflects the costs
of the operation of banks and their profits
*/

CBank::CBank(AgentType bankType) // CentralBankType or PrivateBankType
	: CAgent(
		0, // AgentID will be set properly
		bankType // BankType = CentralBankType or PrivateBankType
	)
{
	// REMOVED: Producer-specific initialization that was calling CProducer constructor

	initialize();
};
CBank::~CBank()
{
	// Cleanup - banks are no longer in the producers vector
}

ofstream& operator<<(ofstream& ofstrm, const CBank& bank)
{
	// Write base CAgent data
	ofstrm << (const CAgent&)bank; // Call base class operator

	ofstrm << " _rc " << bank._rc
		<< " _markup " << bank._markup
		<< " _markdown " << bank._markdown
		<< " _lambdaB " << bank._lambdaB;

	ofstrm
		<< " monthInflow " << bank.getmonthInflow()
		<< " monthOutflow " << bank.getmonthOutflow()
		<< " TotalBullion " << bank.getTotalBullion()
		<< " ReserveRatio " << bank.getReserveRatio()
		<< " RiskExposureAmount " << bank.getRiskExposureAmount()
		<< " prevCash_I_Own " << bank.getprevCash_I_Own()
		<< " Cash_I_Own " << bank.getCash_I_Own()
		<< " ClientsDeposits " << bank.getClientsDeposits()
		<< " ClientsLoans " << bank.getClientsLoans()
		<< " MaxNBanks " << bank.getMaxNBanks();

	if (bank.getAgentType() == getCentralBankType())
	{
		ofstrm << "\n BanksIDs {";
		for (const auto& pair : bank.getBanks())
			ofstrm << " " << pair.first;
		ofstrm << " }";
	}

	ofstrm << "\n ClientsAccounts {";
	for (const auto& pair : bank.getClientsAccounts())
	{
		auto entry = pair.second;
		ofstrm << entry;
	}
	ofstrm << " }" << endl;

	return ofstrm;
}
ifstream& operator>>(ifstream& ifstrm, CBank& bank)
{
	string word, word1;

	// Read base CAgent data
	ifstrm >> (CAgent&)bank; // Call base class operator

	ifstrm
		>> word >> bank._rc
		>> word >> bank._markup
		>> word >> bank._markdown
		>> word >> bank._lambdaB;

	ifstrm
		>> word >> bank.monthInflow()
		>> word >> bank.monthOutflow()
		>> word >> bank.TotalBullion()
		>> word >> bank.ReserveRatio()
		>> word >> bank.RiskExposureAmount()
		>> word >> bank.prevCash_I_Own()
		>> word >> bank.Cash_I_Own()
		>> word >> bank.ClientsDeposits()
		>> word >> bank.ClientsLoans()
		>> word1 >> bank.MaxNBanks();

	if (bank.getAgentType() == getCentralBankType())
	{
		ifstrm >> word >> word1; // " BanksIDs {"
		AgentID bID;
		while (ifstrm >> word, word != "}")
		{
			bID = atol(word.c_str());
			auto result = getCentralBank().getBanks().find(bID);
			assert(result != getCentralBank().getBanks().end());
		}
	}

	ifstrm >> word >> word1; // " ClientsAccounts {"
	CBankEntry entry;
	while (ifstrm >> word, word != "}")
	{
		ifstrm >> entry;
		bank.ClientsAccounts()[entry._pAgent->getID()] = entry;
	}
	ifstrm >> word; // "}"

	return ifstrm;
}

void CBank::initialize()
{
	ReserveRatio() = getInputParameter("ReserveRatio");
	RiskExposureAmount() = 0;
	ClientsDeposits() = 0;
	ClientsLoans() = 0;
	prevCash_I_Own() = 0;
	Cash_I_Own() = 0;
	monthInflow() = 0;
	monthOutflow() = 0;
	MaxNBanks() = getInputParameter("MaxNBanks");

	pUsedBank() = nullptr;
	myShares(getFID()) = 0;
	SharesToTrade() = CShare(getFID(), 0);
	Cash() = 0;
	InitCash() = 0;

	if (!IsCentralBank())
	{
		InitCash() = getWorld().getnWorkers()
			* getInputParameter("MonetaryBasePerActiveSalaries")
			* getSAM().getInitSalary();
		Cash() = InitCash();

		GoodQtty nShares = ceil(Cash()
			/ getWorld().getFinancialMarket().getInitShareValue());
		myShares(getFID()) = -nShares;
	}

	ConsumptionBudget() = 0.0;
	TotalBullion() = 0;

	_rc = getInputParameter("rc");
	_markup = getInputParameter("markup");
	_markdown = getInputParameter("markdown");
	_lambdaB = getInputParameter("lambdaB");
};

bool CBank::CheckBalance()
{
#ifdef GOODQTTY_IS_DOUBLE
	double totCash = InitCash() + ClientsDeposits() + Cash_I_Own();
	double totNeg = Cash() + myBankAccountStatus()._value + ClientsLoans();

	if (totCash == 0.0) {
		if (totNeg == 0.0 && Cash() >= 0)
			return true;
		else {
			assert(false);
			return false;
		}
	}

	GoodQtty mismatch = abs((totCash - totNeg) / totCash);
	if (Cash() >= 0 && abs(mismatch) < 1.0E-9)
#else
	GoodQtty mismatch = InitCash() + ClientsDeposits() + Cash_I_Own()
		- (Cash() + myBankAccountStatus()._value + ClientsLoans());

	if (Cash() >= 0 && mismatch == 0)
#endif
		return true;
	else
	{
		assert(false);
		return false;
	}
}

double CBank::getDepositsRate() const
{
	// _rc and _markdown are stored as annual rates
	double annual_rc = _rc;
	double annual_markdown = _markdown;

	// Calculate base annual deposit rate
	double baseRate_annual = annual_rc - annual_markdown;

	// Convert to monthly rate for calculations
	double baseRate_monthly = baseRate_annual / 12.0;

	// Apply inflation adjustment (CalculateInflationAdjustedRate expects monthly rate)
	double inflationAdjustedRate = CalculateInflationAdjustedRate(baseRate_monthly);

	return inflationAdjustedRate;  // Returns monthly rate
}

double CBank::getCurrentInterestRateOf(CAgent* pClient) const
{
	if (pClient->IsPrivateBank() || pClient->IsGovernment())
		return _rc / 12.0;  // Convert annual _rc to monthly
	else {
		const auto& account = getClientsAccounts().at(pClient->getID());
		return account._rate;  // account._rate is already stored as monthly rate
	}
}

const GoodQtty CBank::getClientCurrentBalance(const CAgent* pClient) const
{
	if (getClientsAccounts().size() == 0)
		return 0;

	AgentID clientID = pClient->getID();
	if (getClientsAccounts().find(clientID) == getClientsAccounts().end())
		return 0;

	const auto& account = getClientsAccounts().at(clientID);

	GoodQtty currentBalance = account._value
		+ (GoodQtty)(account._value * account._rate
			* (currMonth() - account._initialMonth) / (double)getDEPData().getMonthsPerYear());

	return currentBalance;
}
CBankEntry CBank::ClientDefaultClose(CAgent* pClient)
{
	AgentID clientID = pClient->getID();
	if (ClientsAccounts().find(clientID) != ClientsAccounts().end())
	{
		// Close this account
		auto balance = this->getClientCurrentBalance(pClient);
		ClientsAccounts().erase(clientID);
		assert(CheckBalance());
	}

	return CBankEntry();
}
void CBank::AddValToAccount(CBankEntry& account, GoodQtty value)
{
	if (account._value >= 0) // it's a Deposit account
	{
		assert(_ClientsDeposits >= account._value);
		if (account._value + value >= 0) // 5 + (-3) = 2 > 0
		{
			account._value += value;
			_ClientsDeposits += value;
		}
		else // 5 + (-9) = -4 change to Loan account
		{
			_ClientsDeposits -= account._value;// remove from Deposits
			account._value += value; // now negative (Loan) acc.
			_ClientsLoans += -account._value;
		}
	}
	else // (account._value < 0)  it's a Loan account
	{
		assert(_ClientsLoans >= -account._value);
		if (account._value + value <= 0) // -5 + (+3) = -2 < 0
		{
			account._value += value;
			_ClientsLoans += -value;
		}
		else // -5 + (+9) = +4 change to Deposit account
		{
			_ClientsLoans -= -account._value;// up to 0
			account._value += value; // now positive (Deposit) acc.
			_ClientsDeposits += account._value;
		}
	}
}
bool CBank::GetEnoughReserveFromCB(GoodQtty withdraw)
{
	assert(CheckBalance());
	if (withdraw <= 0)
		return true;

	GoodQtty bullResIncrement = ceil(ClientsDeposits() * ReserveRatio()) + ceil(withdraw * 1.1)
		- Cash();

	if (bullResIncrement <= 0)
		return true;

	if (IsCentralBank())// No CentralBank BullionReserve change
		CWorld::ERRORmsg("Error: Not enough CentralBank BullionReserve", true);

	// this Bank asks the CentralBank for more bullion reserve

	CBankEntry loanStatus = pCentralBank()->ClientCashOperation(this, -bullResIncrement,
		10 * getDEPData().getMonthsPerYear());
	if (loanStatus._operationOK)
	{
		myBankAccountStatus() = loanStatus;
		//assert(CheckBalance());
		//assert(Cash() >= 0);
		return true;
	}
	else
		return false;
}

CBankEntry CBank::ClientCashOperation(CAgent* pClient,
	GoodQtty value, long nMonths)
{
	if (DebugLevel() > 2 && currMonth() >= getInputParameter("DebugFromMonth"))
	{
		LogFile() << "\n month " << currMonth()
			<< " ClientCashOperation ID " << pClient->getID() << " value " << value;
		LogFile().flush();
	}

	if (value == 0)
	{
		pClient->myBankAccountStatus()._operationOK = true;
		return pClient->myBankAccountStatus();
	}

	assert(CheckBalance());
	//this may not be a complete operation: cannot assert(_pGeneration->CheckGenerationBalance());
	// Returns the final account status of Client

	GoodQtty withdraw = -value;

	if (withdraw > 0 && !GetEnoughReserveFromCB(withdraw))
	{
		pClient->myBankAccountStatus()._operationOK = false;
		return pClient->myBankAccountStatus();
	}

	// Carry out the operation ------------------------------------------

	AgentID clientID = pClient->getID();
	if (ClientsAccounts().find(clientID) == ClientsAccounts().end()) // new client
	{
		assert(CheckBalance());
		if (pClient->IsWorker() && value <= 0)
		{ // don't open a new account to individuals as a loan
			pClient->myBankAccountStatus()._operationOK = false;
			return pClient->myBankAccountStatus();
		}

		CBankEntry& account = ClientsAccounts()[clientID];
		account._pAgent = pClient;
		account._value = value;

		if (value > 0) {
			if (isPreCalibration())
				account._rate = 0;
			else
				account._rate = getDepositsRate();
		}
		else {
			// Calculate proper annual loan rate using the same logic as GetCashFromBank
			double annual_rc = _rc;  // _rc is already annual
			double epsilonScale = getInputParameter("EpsilonScale");
			double epsilon = epsilonScale * World().getRandom01();
			double baseInterestRate_annual = annual_rc * (1. + _lambdaB * /* default risk factor */ 0.001 + epsilon);
			baseInterestRate_annual = max(baseInterestRate_annual, annual_rc * (1. + _markup));

			// Convert to monthly for storage and apply inflation adjustment
			double baseInterestRate_monthly = baseInterestRate_annual / 12.0;
			account._rate = CalculateInflationAdjustedRate(baseInterestRate_monthly);
		}

		account._initialMonth = currMonth();
		account._endMonth = currMonth() + nMonths;
		account._operationOK = true;

		Cash() += value;
		pClient->Cash() -= value;
		assert(Cash() >= 0 && pClient->Cash() >= 0);

		if (value > 0)
			ClientsDeposits() += value;
		else
			ClientsLoans() += abs(value);

		pClient->myBankAccountStatus() = account;
		assert(CheckBalance());

		return pClient->myBankAccountStatus();
	}
	else // old friend, but this is like a new loan/deposit after updating pending interests
	{
		AgentID clientID = pClient->getID();
		CBankEntry& account = ClientsAccounts().at(clientID);

		double rate = getCurrentInterestRateOf(pClient);
		GoodQtty interests = account._value
			* rate * (currMonth() - account._initialMonth) / (double)getDEPData().getMonthsPerYear();

		if (pClient->IsWorker()
			&& (account._value + interests + value) < 0)
		{
			// don't allow individuals go to negative balance
			pClient->myBankAccountStatus()._operationOK = false;
			return pClient->myBankAccountStatus();
		}

		account._initialMonth = currMonth(); // reset interests counter
		account._endMonth = currMonth() + nMonths;

		AddValToAccount(account, interests);
		Cash_I_Own() -= interests;
		if (pClient->IsPrivateBank())
			((CBank*)pClient)->Cash_I_Own() += interests;

		AddValToAccount(account, value);

		Cash() += value;
		pClient->Cash() -= value;
		if (Cash() < 0 || (pClient->Cash() < 0 && !pClient->IsGovernment()))
			CWorld::ERRORmsg("Cash() < 0 || pClient->Cash() < 0 in ClientCashOperation", true);

		// Check if this account should be closed

		auto balance = account._value;
		if (balance == 0 && !pClient->IsGovernment() && !pClient->IsProducer())
		{
			// Close this account

			ClientsAccounts().erase(pClient->getID());
			assert(CheckBalance());
			pClient->myBankAccountStatus() = CBankEntry();
			return CBankEntry();
		}

		assert(CheckBalance());

		if (balance > 0) {
			if (isPreCalibration())
				account._rate = 0;
			else
				account._rate = getDepositsRate();
		}

		account._operationOK = true;
		pClient->myBankAccountStatus() = account;
		//may not be a complete operation to assert(CheckAccountingBalance());

		if (DebugLevel() > 3 && currMonth() >= getInputParameter("DebugFromMonth"))
			LogFile() << "\n month " << currMonth()
			<< " ClientDeposits " << _ClientsDeposits << " Loans " << _ClientsLoans;
		if (pClient->IsPrivateBank())
			assert(((CBank*)pClient)->CheckBalance());

		return pClient->myBankAccountStatus();
	}
}

bool CBank::GetCashFromBank(CAgent& agent, GoodQtty& loanQtty)
{
	if (loanQtty == 0)
		return true;

	// also updates agent.myBankAccountStatus()

	if (loanQtty < 0)
		CWorld::ERRORmsg("loanQtty < 0 in  CBank::GetCashFromBank", true);

	CBankEntry agentAccStatus = agent.myBankAccountStatus();
	double balance = agent.getmyBankBalance();

	if (balance >= loanQtty) // enough balance, no need for a loan, grant cash
	{
		agentAccStatus = ClientCashOperation(&agent, -loanQtty, 999999);
		return agentAccStatus._operationOK;
	}
	else
	{
		if (agent.IsGovernment()) // loanQtty granted, special rate treatment
		{
			agentAccStatus = ClientCashOperation(&agent, -loanQtty, 3 * getDEPData().getMonthsPerYear());
			if (!agentAccStatus._operationOK)
				getWorld().ERRORmsg("Government failed getCash", true);

			// FIXED: Convert annual rate to monthly for government loans
			agentAccStatus._rate = _rc / 12.0;  // CHANGED: Divide by 12 for monthly rate
			ClientsAccounts().at(agent.getID()) = agentAccStatus;
			agent.myBankAccountStatus() = agentAccStatus;

			return true;
		}

		// Reserve requirement: final loanQtty (max. loan qtty)

		// RESERVE REQUIREMENT:
		if (loanQtty > ExcessLiquidityBudget())
			loanQtty = ExcessLiquidityBudget();

		double prevDebt = -min((GoodQtty)0, agent.getmyBankBalance());
		double firmEquity = agent.getWealth();
		double ProbOfDefault = 1.0;
		if (firmEquity > 0)
			ProbOfDefault = max(3.e-4, 1. - exp(-getInputParameter("NuProbabDefault")
				* (prevDebt + loanQtty) / firmEquity));
		double ExposAtDefault = ProbOfDefault * loanQtty;

		// Capital adequacy to RISK EXPOSURE: Dawid leverage = 10 * capital
		if (getRiskExposureBudget() < 0)
		{
			LogFile() << " ***WARNING: Bank RiskExposureBudget() < 0***, ";
			CWorld::ERRORmsg("Bank RiskExposureBudget() < 0", true);
			// loan is denied, "fully rationing" rule
			loanQtty = 0;

			agentAccStatus._operationOK = false;
			agent.myBankAccountStatus() = agentAccStatus;

			return false;
		}
		else // loanQtty can be granted, set interestRate
		{
			agentAccStatus = ClientCashOperation(&agent, -loanQtty,
				1. * getDEPData().getMonthsPerYear());
			if (!agentAccStatus._operationOK)
				return false;

			RiskExposureAmount() += ExposAtDefault;

			// ========== CALCULATE INFLATION-ADJUSTED INTEREST RATE ==========

			// FIXED: Convert annual central bank rate to monthly rate
			double rc_monthly = _rc / 12.0;  // CHANGED: Convert annual to monthly

			// 1. Calculate base rate from central bank policy and risk premium
			double epsilonScale = getInputParameter("EpsilonScale"); // Add new parameter, e.g., 0.01
			double epsilon = epsilonScale * World().getRandom01();
			double baseInterestRate = rc_monthly * (1. + _lambdaB * ProbOfDefault + epsilon);  // CHANGED: Use rc_monthly
			baseInterestRate = max(baseInterestRate, rc_monthly * (1. + _markup));  // CHANGED: Use rc_monthly


			// 2. Apply inflation adjustment (similar to salary adjustment mechanism)
			double inflationAdjustedRate = CalculateInflationAdjustedRate(baseInterestRate);

			// 3. Update loan tracking and statistics
			DEPData().TotLoansInterests() += loanQtty * inflationAdjustedRate;
			DEPData().TotLoansQtty() += loanQtty;

			// 4. Set the inflation-adjusted rate in the account
			agentAccStatus._rate = inflationAdjustedRate;
			ClientsAccounts().at(agent.getID()) = agentAccStatus;
			agent.myBankAccountStatus() = agentAccStatus;

			// Optional: Log interest rate adjustments for debugging
			if (false) //////
				LogFile() << "\n abmonth= " << currMonth() << ": rate= "
				<< inflationAdjustedRate << ", baseRate= " << baseInterestRate
				<< ", aType= " << agent.getAgentType() << ", ID= " << agent.getID();

			return true;
		}
	}
}

GoodQtty CBank::ClientsLoansMax()
{
	return TotalBullion() + ClientsDeposits() - ceil(ClientsDeposits() * ReserveRatio());
}
GoodQtty CBank::ExcessLiquidityBudget() const
{
	return max(0., getCash() + getmyBankBalance() - ceil(getClientsDeposits() * getReserveRatio()));
}
GoodQtty CBank::Equity() const
{
	// to be more accurate, Loans and Deposits should have updated their interests
	return getCash() + getmyBankBalance() + getClientsLoans() - getClientsDeposits();
}
GoodQtty CBank::getWealth() const
{
	GoodQtty Wealth = getCash() + getmyBankBalance()
		+ getClientsLoans() - getClientsDeposits();

	return Wealth;
}
void CBank::updateRiskExposureAmount()
{
	RiskExposureAmount() = 0;
	for (const auto& pair : getClientsAccounts())
	{
		const auto& acc = pair.second;
		if (acc._value < 0)
		{
			const auto& firm = *getWorld().getFIDAgent(acc._pAgent->getFID());
			if (firm.IsGovernment())
				continue;

			const double loanQtty = -acc._value;
			double firmEquity = firm.getWealth();
			double ProbOfDefault = 1.0;
			if (firmEquity > 0)
				ProbOfDefault = max(3.e-4, 1. - exp(-getInputParameter("NuProbabDefault")
					* loanQtty / firmEquity));
			double ExposAtDefault = ProbOfDefault * loanQtty;
			RiskExposureAmount() += ExposAtDefault;
		}
	}
}
const GoodQtty CBank::getRiskExposureBudget() const
{
	double LeverageRatio = getInputParameter("LeverageRatio");
	auto REbudget = LeverageRatio * Equity() - getRiskExposureAmount();
	return REbudget;
};
double CBank::getCurrInterests(CAgent& agent)
{
	CBankEntry account = agent.getmyBankAccountStatus();

	double rate = getCurrentInterestRateOf(&agent);  // rate is monthly
	double interests = (currMonth() - account._initialMonth)
		* rate * account._value;  // No division by 12 needed since rate is monthly

	return interests;
}
void CBank::CollectInterests()
{
	assert(CheckBalance());
	if (DebugLevel() > 1 && currMonth() >= getInputParameter("DebugFromMonth"))
		assert(DEPData().CheckAccountingBalance());

	for (auto& pair : ClientsAccounts())
	{
		auto& account = pair.second;
		auto pClient = getWorld().getFIDAgent(account._pAgent->getFID());
		if (!pClient) continue; // Agent no longer exists

		double rate = getCurrentInterestRateOf(pClient); // rate is monthly
		GoodQtty interests = (currMonth() - account._initialMonth)
			* rate * account._value;  // No division needed since rate is monthly

		if (pClient->IsProducer())
			((CProducer*)pClient)->currDepositInterests_mu() += interests;

		account._initialMonth = currMonth();
		account._endMonth = account._initialMonth;

		AddValToAccount(account, interests);
		Cash_I_Own() -= interests;
		if (pClient->IsPrivateBank() || pClient->IsCentralBank())
			((CBank*)pClient)->Cash_I_Own() += interests;

		if (account._value == 0) // Close this account and update the client's
		{
			assert(false);

			account._pAgent->myBankAccountStatus() = CBankEntry();
			ClientsAccounts().erase(account._pAgent->getID());
			assert(CheckBalance());
			return;
		}

		account._operationOK = true;

		account._pAgent->myBankAccountStatus() = account;

		assert(getClientsAccounts().at(account._pAgent->getID())._value
			== account._pAgent->myBankAccountStatus()._value);
	}

	assert(CheckBalance());
	if (DebugLevel() > 1 && currMonth() >= getInputParameter("DebugFromMonth"))
		assert(DEPData().CheckAccountingBalance());
}
void CBank::ReturnExcessCashToCB()
{
	assert(!IsCentralBank());

	if (Cash() > (GoodQtty)(ClientsDeposits() * ReserveRatio()))
	{
		// Deposit in CentralBank (and decrease my TotalBullion reserve)

		GoodQtty deposit = Cash()
			- (InitCash() + (GoodQtty)(ClientsDeposits() * ReserveRatio()));

		if (deposit > 0)
		{
			CBankEntry loanStatus = pUsedBank()->ClientCashOperation(this, deposit);
			if (!loanStatus._operationOK) // if balance = 0, account closed
				getWorld().ERRORmsg("Error in CBank::ReturnExcessCashToCB");

			//myBankAccountStatus() = loanStatus;// smaller or null loan
		}
	}
}

void CBank::monthInitialize()
{
	monthInflow() = 0;
	monthOutflow() = 0;
	updateRiskExposureAmount();
};
void CBank::monthActivity()
{
	if (getInputParameter("MaxNBanks") == 0)
		return;

	assert(CheckBalance());

	updateRiskExposureAmount();

	CollectInterests();

	if (!IsCentralBank())
	{
		ReturnExcessCashToCB(); // decrease Loan if any, try to retain InitCash

		// pay profit as dividends in RunProducer
	}
}

//======================  CCentralBank  ===================================

CCentralBank::CCentralBank() : CBank(getCentralBankType())
{
	initialize();
};
CCentralBank::~CCentralBank() {}

ofstream& operator<<(ofstream& ofstrm, const CCentralBank& bank)
{
	ofstrm << "\n";
	if (bank.getAgentType() == getCentralBankType())
		ofstrm << *(CAgent*)getpCentralBank() << endl;
	else
		ofstrm << *(CAgent*)(getCentralBank().getBanks().at(bank.getID())) << endl;

	ofstrm << " _rc " << bank._rc
		<< " _markup " << bank._markup
		<< " _markdown " << bank._markdown
		<< " _lambdaB " << bank._lambdaB;

	ofstrm
		<< " monthInflow " << bank.getmonthInflow()
		<< " monthOutflow " << bank.getmonthOutflow()
		<< " TotalBullion " << bank.getTotalBullion()
		<< " ReserveRatio " << bank.getReserveRatio()
		<< " RiskExposureAmount " << bank.getRiskExposureAmount()
		<< " prevCash_I_Own " << bank.getprevCash_I_Own()
		<< " Cash_I_Own " << bank.getCash_I_Own()
		<< " ClientsDeposits " << bank.getClientsDeposits()
		<< " ClientsLoans " << bank.getClientsLoans()
		<< " MaxNBanks " << bank.getMaxNBanks();

	if (bank.getAgentType() == getCentralBankType())
	{
		ofstrm << "\n BanksIDs {";
		for (const auto& pair : bank.getBanks())
			ofstrm << " " << pair.first;
		ofstrm << " }";
	}

	ofstrm << "\n ClientsAccounts {";
	for (const auto& pair : bank.getClientsAccounts())
	{
		auto entry = pair.second;

		ofstrm << entry;
	}
	ofstrm << " }" << endl;

	return ofstrm;
}
ifstream& operator>>(ifstream& ifstrm, CCentralBank& bank)
{
	string word, word1;
	AgentID ID = bank.getID();

	if (bank.getAgentType() == getCentralBankType())
		ifstrm >> *(CAgent*)pCentralBank();
	else
		ifstrm >> *(CAgent*)(getCentralBank().getBanks().at(bank.getID()));

	ifstrm
		>> word >> bank._rc
		>> word >> bank._markup
		>> word >> bank._markdown
		>> word >> bank._lambdaB;

	ifstrm
		>> word >> bank.monthInflow()
		>> word >> bank.monthOutflow()
		>> word >> bank.TotalBullion()
		>> word >> bank.ReserveRatio()
		>> word >> bank.RiskExposureAmount()
		>> word >> bank.prevCash_I_Own()
		>> word >> bank.Cash_I_Own()
		>> word >> bank.ClientsDeposits()
		>> word >> bank.ClientsLoans()
		>> word1 >> bank.MaxNBanks();

	if (bank.getAgentType() == getCentralBankType())
	{
		ifstrm >> word >> word1; // " BanksIDs {"
		AgentID bID;
		while (ifstrm >> word, word != "}")
		{
			bID = atol(word.c_str());
			auto result = getCentralBank().getBanks().find(bID);
			assert(result != getCentralBank().getBanks().end());
		}
	}

	ifstrm >> word >> word1; // " ClientsAccounts {"
	CBankEntry entry;
	while (ifstrm >> word, word != "}")
	{
		ifstrm >> entry;
		bank.ClientsAccounts()[entry._pAgent->getID()] = entry;
	}
	ifstrm >> word; // "}"

	return ifstrm;
}

void CCentralBank::initialize()
{
	CBank::initialize();

	pUsedBank() = nullptr;
	InitCash() = getWorld().getnWorkers()
		* getInputParameter("MonetaryBasePerActiveSalaries")
		* getSAM().getInitSalary();
	Cash() = InitCash();
};
GoodQtty CCentralBank::Equity() const
{
	// to be more accurate, Loans and Deposits should have updated their interests
	return getCash() + getmyBankBalance() + getClientsLoans() - getClientsDeposits();
}
const GoodQtty CCentralBank::getRiskExposureBudget() const
{
	double LeverageRatio = getInputParameter("LeverageRatio");
	auto REbudget = LeverageRatio * Equity() - getRiskExposureAmount();
	return REbudget;
};

void CCentralBank::monthInitialize()
{
	monthInflow() = 0;
	monthOutflow() = 0;
	updateRiskExposureAmount();
};
void CCentralBank::monthActivity()
{
	if (getInputParameter("MaxNBanks") == 0)
		return;

	assert(CheckBalance());

	updateRiskExposureAmount();

	CollectInterests();

	if (!IsCentralBank())
	{
		ReturnExcessCashToCB(); // decrease Loan if any, try to retain InitCash

		// pay profit as dividends in RunProducer
	}
}

// -------------  CentralBank only  -------------------------

CBank* CCentralBank::BankFoundationRequest(CWorker* pIndivOwner, GoodQtty initialCash)
{
	assert(IsCentralBank());
	if (Banks().size() >= MaxNBanks())
		return nullptr;

	//M J	AgentID BankID = pIndivOwner->getID();
	AgentID BankID = 0;
	// One Worker can only own 1 Bank
	assert(Banks().find(BankID) == Banks().end());
	CBank* pBank = new CBank(PrivateBankType());
	Banks()[BankID] = pBank;

	return pBank;
}
CBank* CCentralBank::getRandomBank()
{
	assert(IsCentralBank());

	if (_Banks.size() == 0)
		return this;

	long bankN = _Banks.size() * getRandom01();
	long nn = 0;
	for (auto pair : _Banks)
	{
		if (bankN == nn++)
			return _Banks[pair.first];
	}
	assert(false);
	return nullptr;
}

//======================  CBankEntry  ===================================

CBankEntry::CBankEntry(CAgent* pAgent, GoodQtty value, double rate,
	long initialMonth, long endMonth, bool ok)
{
	_pAgent = pAgent;
	_value = value;
	_rate = rate;
	_initialMonth = initialMonth;
	_endMonth = endMonth;
	_operationOK = ok;
}
CBankEntry::CBankEntry() { clear(); }
CBankEntry::~CBankEntry() {}
ofstream& operator<<(ofstream& ofstrm, const CBankEntry& entry)
{
	ofstrm << " {";
	//<< " AType " << entry._pAgent->getAgentType() << " A_ID " << entry._pAgent->getID()
	if (entry._pAgent == nullptr)
		ofstrm << " " << UndefAgentType << " " << UndefAgentID;
	else
		ofstrm << " " << entry._pAgent->getAgentType() << " " << entry._pAgent->getID();
	ofstrm << " " << entry._value
		<< " " << entry._rate
		<< " " << entry._initialMonth
		<< " " << entry._endMonth
		<< " " << entry._operationOK
		<< " }";

	return ofstrm;
}
ifstream& operator>>(ifstream& ifstrm, CBankEntry& entry)
{
	string word;
	AgentType aTy;
	AgentID aID;
	//ifstrm >> word;  "{"
	ifstrm >> aTy >> aID
		>> entry._value >> entry._rate
		>> entry._initialMonth >> entry._endMonth >> entry._operationOK;
	ifstrm >> word; // "}"

	if (aTy >= 0) // Producer client
	{
		entry._pAgent = getWorld().getpProducers()->at(aID);
		assert(entry._pAgent->getID() == aID);
	}
	else if (aTy == WorkerType)
		entry._pAgent = getWorld().getpWorkers()->at(aID);
	else if (aID == getpGovernment()->getAgentType())
		entry._pAgent = (CAgent*)getpGovernment();
	else if (aTy == getCentralBankType())
		entry._pAgent = pCentralBank();
	else if (aTy == PrivateBankType())
		entry._pAgent = pCentralBank()->Banks().at(aID);
	else
	{
		assert(aTy == UndefAgentType);
		entry._pAgent = nullptr;
	}

	return ifstrm;
}
void CBankEntry::clear()
{
	_pAgent = nullptr;
	_value = 0;
	_rate = 0;
	_initialMonth = 0;
	_endMonth = 0;
	_operationOK = false;
}

//==========================  Interest rates  ============================

/*
INTEREST RATE STORAGE CONVENTION:
- All base parameters (_rc, _markup, _markdown, _lambdaB) are stored as ANNUAL rates
- Account rates (CBankEntry._rate) are stored as MONTHLY rates for calculation efficiency
- CalculateInflationAdjustedRate() expects and returns MONTHLY rates
- When displaying rates, multiply monthly rates by 12 for annual equivalent
*/

double CBank::CalculateInflationAdjustedRate(double baseRate) const
{
	// Only apply inflation adjustment after calibration period starts
	if (isPreCalibration()) {
		return baseRate;
	}

	// Get the CPI values to calculate actual year-over-year inflation
	const auto& cpiVector = getDEPData().getCPItracker().getCPI();

	// Need at least 13 months of data for year-over-year comparison
	if (cpiVector.size() < 13) {
		return baseRate;
	}

	double currentCPI = cpiVector.back();
	double yearAgoCPI = cpiVector[cpiVector.size() - 13]; // 12 months ago

	if (yearAgoCPI <= 0) {
		return baseRate;
	}

	// Calculate actual year-over-year inflation rate
	double yearOverYearInflationRate = (currentCPI - yearAgoCPI) / yearAgoCPI;

	// FIXED: Use geometric conversion from annual to monthly inflation rate
	// Formula: (1 + annual_rate)^(1/12) - 1
	double monthlyInflationRate;
	if (yearOverYearInflationRate > -0.99) {  // Avoid taking root of negative number
		monthlyInflationRate = pow(1.0 + yearOverYearInflationRate, 1.0 / 12.0) - 1.0;
	}
	else {
		// For extreme deflation, use linear approximation to avoid mathematical issues
		monthlyInflationRate = yearOverYearInflationRate / 12.0;
	}

	// Apply target real rate above inflation
	// This ensures the bank earns a net real return above inflation
	double monthlytargetRealRate = getInputParameter("TargetRealRate") / (12 * 100.0); // e.g., 3.5% annual
	double adjustedMonthlyRate = monthlyInflationRate + monthlytargetRealRate;

	// Add inflation adjustment to base rate
	double inflationAdjustedRate = baseRate + adjustedMonthlyRate;

	// Ensure rate stays within reasonable bounds
	double monthlyminRate = getInputParameter("MinInterestRate") / (12 * 100.0);  // e.g., 0.1% annual
	// Adjust rate bounds based on inflation environment
	double monthlymaxRate = max(0.05, monthlyInflationRate + 0.02); // Always allow 2% real rate

	double finalRate = max(monthlyminRate, min(monthlymaxRate, inflationAdjustedRate));

	return finalRate;
}
