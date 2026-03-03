//  CData.cpp

//   DEPLOYERS v2

#include "./pch.h"

//=========================================================

map<string, double> CData::_mInputParameters;
map<string, double>& CData::mInputParameters() { return CData::_mInputParameters; };
const map<string, double>& CData::InputParameters() { return CData::_mInputParameters; };

//=======================   CGDPtracker   ==================================

CGDPcomponent::CGDPcomponent() : _gType(UndefGoodType), _quantity(0), _price(0) {}
CGDPcomponent::CGDPcomponent(GoodType gTy, double q, double p) : _gType(gTy), _quantity(q), _price(p) {}

void CGDPtracker::addToGDPComponent(GoodType gType, GoodQtty qtty)
{
	// START ACCUMULATING after StartCalibrationAt, not StartCalibrationAt!
	if (currMonth() <= getInputParameter("StartCalibrationAt")) return;

	_GDPcomponents[gType]._quantity += qtty; // _quantity: annualized total of gType
}

void CGDPtracker::addMonthData() {
	// START ACCUMULATING after StartCalibrationAt, not StartCalibrationAt!
	if (currMonth() <= getInputParameter("StartCalibrationAt")) return;

	CGDPcomponents components; // CGDPcomponent: gType, quantity, price

	// X: to include Imports in allocateBuyersQttyAndMoney
	for (long gType = 0; gType < getSAM().getnPXproducerTypes(); ++gType) // _quantity: annualized total of gType
		components.emplace_back(gType, _GDPcomponents.at(gType)._quantity, getDEPData().getMarketPrices().at(gType));

	_Month_data.push_back(components);

	calculateGDP();
}

void CGDPtracker::calculateGDP() {
	// Start accumulating data after StartCalibrationAt (not StartCalibrationAt!)
	if (currMonth() <= getInputParameter("StartCalibrationAt")) return;

	// Calculate base month indices
	int inflationFromMonth = (int)getInputParameter("StartCalibrationAt");
	int adjustFromMonth = (int)getInputParameter("StartCalibrationAt");
	int inflationFromIndex = inflationFromMonth - adjustFromMonth - 1;

	// Determine which base month to use
	int baseMonthIndex;

	if (inflationFromIndex < 0 || inflationFromIndex >= (int)_Month_data.size()) {
		// Before we have enough data: use earliest available month as base
		baseMonthIndex = 0;
	}
	else {
		// After StartCalibrationAt: use fixed base
		baseMonthIndex = inflationFromIndex;
	}

	// Calculate current GDP values
	double current_nominal_gdp = 0;
	double current_real_gdp = 0;

	for (long gType = 0; gType < getSAM().getnPXproducerTypes(); ++gType)
	{
		const auto& baseMonth = _Month_data.at(baseMonthIndex).at(gType);
		const auto& currMonth = _Month_data.back().at(gType);

		current_nominal_gdp += currMonth._quantity * currMonth._price;
		current_real_gdp += currMonth._quantity * baseMonth._price;
	}

	_nominal_gdp.push_back(current_nominal_gdp);
	_real_gdp.push_back(current_real_gdp);

	// Calculate deflator
	double deflator = 100.;
	if (current_real_gdp > 0.)
		deflator = 100. * current_nominal_gdp / current_real_gdp;

	_gdp_deflator.push_back(deflator);
	_last_gdp_deflator = deflator;
}

const double CGDPtracker::getGDPInflationRate() const {
	// Calculate inflation relative to the fixed base specified by InputParameter("StartCalibrationAt").
	if (_gdp_deflator.empty())
		return 0.0;

	// Read configured months
	int inflationFromMonth = (int)getInputParameter("StartCalibrationAt");
	int adjustFromMonth = (int)getInputParameter("StartCalibrationAt");

	// _gdp_deflator entries correspond to _Month_data which starts at month (StartCalibrationAt + 1)
	// Compute index within _gdp_deflator that corresponds to inflationFromMonth
	int inflationFromIndex = inflationFromMonth - adjustFromMonth - 1;

	// If we haven't reached the base month yet, return 0
	if (inflationFromIndex < 0 || inflationFromIndex >= (int)_gdp_deflator.size()) {
		return 0.0;
	}

	// Always use fixed base from StartCalibrationAt
	int baseIndex = inflationFromIndex;

	double current_deflator = _gdp_deflator.back();
	double base_deflator = _gdp_deflator.at(baseIndex);

	if (base_deflator <= 0.0)
		return 0.0;

	// Return inflation relative to the fixed base
	return (current_deflator - base_deflator) / base_deflator;
}

const double CGDPtracker::getMonthlyGDPInflationRate() const
{
	if (_gdp_deflator.size() < 2)
		return 0.0; // Not enough data for monthly rate

	// Get the last two deflator values
	double current_deflator = _gdp_deflator.back();
	double previous_deflator = _gdp_deflator[_gdp_deflator.size() - 2];

	if (previous_deflator <= 0)
		return 0.0;

	// Calculate month-over-month inflation rate
	double monthly_inflation_rate = (current_deflator - previous_deflator) / previous_deflator;

	return monthly_inflation_rate;
}

void CGDPtracker::printResults() const {
	// Output disabled - was used for debugging
}

//=======================   CCPItracker   ==================================

CCPIcomponent::CCPIcomponent() : _gType(UndefGoodType), _quantity(0), _price(0) {}
CCPIcomponent::CCPIcomponent(GoodType gTy, double q, double p) : _gType(gTy), _quantity(q), _price(p) {}

void CCPItracker::addToCPIComponents(GoodType gType, GoodQtty qtty)
{
	// START ACCUMULATING after StartCalibrationAt, not StartCalibrationAt!
	if (currMonth() <= getInputParameter("StartCalibrationAt")) return;

	_CPIcomponents[gType]._quantity += qtty; // _quantity: annualized total of gType
}

void CCPItracker::addMonthData() {
	// START ACCUMULATING after StartCalibrationAt, not StartCalibrationAt!
	if (currMonth() <= getInputParameter("StartCalibrationAt")) return;

	CCPIcomponents components; // CCPIcomponent: name, quantity, price

	// X: to include Imports in allocateBuyersQttyAndMoney
	for (long gType = 0; gType < getSAM().getnPXproducerTypes(); ++gType) // _quantity: annualized total of gType
		components.emplace_back(gType, _CPIcomponents.at(gType)._quantity, getDEPData().getMarketPrices().at(gType));

	_Month_data.push_back(components);

	calculateCPIperCent();
	calculateSmoothCPIperCent();
}

void CCPItracker::calculateCPIperCent() {
	// Start accumulating data after StartCalibrationAt (not StartCalibrationAt!)
	if (currMonth() <= getInputParameter("StartCalibrationAt") ||
		_Month_data.size() < 1) {
		_period_indices.push_back(100.0);
		CPI().push_back(100.0);
		return;
	}

	// Get BaseMonth parameter to determine calculation method
	int baseMonthParam = static_cast<int>(getInputParameter("BaseMonth"));

	// Calculate base period index
	int basePeriodIndex;

	if (baseMonthParam < 0) {
		// Rolling 12-month calculation (original behavior)
		if (_Month_data.size() < 13) {
			// Not enough data yet - use first month as base
			basePeriodIndex = 0;
		}
		else {
			// Use month 12 months ago as base
			basePeriodIndex = static_cast<int>(_Month_data.size()) - 13; // 12 months back + current = 13
		}
	}
	else {
		// Fixed base month calculation
		int adjustFromMonth = static_cast<int>(getInputParameter("StartCalibrationAt"));

		// Convert BaseMonth (absolute month number) to index in _Month_data
		// _Month_data starts at (StartCalibrationAt + 1)
		basePeriodIndex = baseMonthParam - adjustFromMonth - 1;

		// Check if we have data for the base month yet
		if (basePeriodIndex < 0 || basePeriodIndex >= static_cast<int>(_Month_data.size())) {
			// Not enough data to use the specified base month yet
			// Use earliest available month as temporary base
			basePeriodIndex = 0;
		}
	}

	// Get current and base period data
	const auto& currentPeriod = _Month_data.back();
	const auto& basePeriod = _Month_data[basePeriodIndex];

	// Calculate CPI using Laspeyres formula
	double numerator = 0.0;
	double denominator = 0.0;

	for (long gType = 0; gType < getSAM().getnPXproducerTypes(); ++gType) {
		numerator += basePeriod[gType]._quantity * currentPeriod[gType]._price;
		denominator += basePeriod[gType]._quantity * basePeriod[gType]._price;
	}

	double cpi = 100.0;
	if (denominator > 0) {
		cpi = 100.0 * numerator / denominator;
	}

	_period_indices.push_back(cpi);
	CPI().push_back(cpi);
}

void CCPItracker::calculateSmoothCPIperCent() {
	// Calculate smoothed CPI using a rolling 12-month moving average
	// This provides a less volatile inflation measure

	if (_CPI.size() < 1) {
		_smooth_CPI.push_back(100.0);
		return;
	}

	// For the first 12 months, calculate average of all available data
	if (_CPI.size() < 12) {
		double sum = 0.0;
		for (size_t i = 0; i < _CPI.size(); ++i) {
			sum += _CPI[i];
		}
		double smoothed = sum / _CPI.size();
		_smooth_CPI.push_back(smoothed);
		return;
	}

	// After 12 months, use rolling 12-month average
	double sum = 0.0;
	size_t startIndex = _CPI.size() - 12;
	for (size_t i = startIndex; i < _CPI.size(); ++i) {
		sum += _CPI[i];
	}
	double smoothed = sum / 12.0;
	_smooth_CPI.push_back(smoothed);
}

const double CCPItracker::getSmoothCPIInflationRate() const {
	// Return year-over-year inflation rate based on smoothed CPI
	// This is the rate to be used in salary adjustments

	if (_smooth_CPI.empty())
		return 0.0;

	// Need at least 13 months to calculate year-over-year from smoothed data
	if (_smooth_CPI.size() < 13) {
		// Not enough data for year-over-year comparison
		return 0.0;
	}

	// Compare current smoothed CPI to smoothed CPI from 12 months ago
	double current_smooth_cpi = _smooth_CPI.back();
	double year_ago_smooth_cpi = _smooth_CPI[_smooth_CPI.size() - 13];

	if (year_ago_smooth_cpi <= 0.0)
		return 0.0;

	// Calculate year-over-year inflation rate from smoothed values
	return (current_smooth_cpi - year_ago_smooth_cpi) / year_ago_smooth_cpi;
}

const double CCPItracker::getCPIInflationRate() const {
	if (getCPI().empty())
		return 0.0;

	int baseMonthParam = static_cast<int>(getInputParameter("BaseMonth"));

	if (baseMonthParam < 0) {
		// Rolling 12-month: CPI is already relative to 12 months ago (base = 100)
		// Inflation rate is simply: (current_index - 100) / 100
		return (getCPI().back() - 100.0) / 100.0;
	}
	else {
		// Fixed base month: CPI is relative to the fixed base (base = 100)
		// Inflation rate is: (current_index - 100) / 100
		return (getCPI().back() - 100.0) / 100.0;
	}
}

void CCPItracker::printResults() const {
	// Output disabled - was used for debugging
}

// ------------------------------    -------------------------------

ofstream& operator<<(ofstream& ofstrm, const CGDPtracker& tracker)
{
	ofstrm << "\n GDPtracker {";

	ofstrm << " base_Month " << tracker.getbase_Month();

	ofstrm << "\n GDPcomponents {";
	if (tracker._GDPcomponents.size() > 0)
		for (const auto& comp : tracker._GDPcomponents)
		{
			ofstrm << " {";
			ofstrm << comp; // one gType
			ofstrm << " }";
		}
	ofstrm << " }";

	ofstrm << " _Month_data {";
	if (tracker._Month_data.size() > 0)
		for (const auto& comps : tracker._Month_data)
		{
			ofstrm << " {";
			ofstrm << comps; // gTypes of one month
			ofstrm << " }";
		}
	ofstrm << " }";

	ofstrm << " _nominal_gdp {";
	if (tracker._nominal_gdp.size() > 0)
		for (double valdoub : tracker._nominal_gdp)
			ofstrm << " " << valdoub;
	ofstrm << " }";

	ofstrm << " _real_gdp {";
	if (tracker._real_gdp.size() > 0)
		for (double valdoub : tracker._real_gdp)
			ofstrm << " " << valdoub;
	ofstrm << " }";

	ofstrm << " _gdp_deflator {";
	if (tracker._gdp_deflator.size() > 0)
		for (double valdoub : tracker._gdp_deflator)
			ofstrm << " " << valdoub;
	ofstrm << " }";

	ofstrm << " last_gdp_deflator " << tracker.getlast_gdp_deflator();

	ofstrm << " }"; // GDPtracker {

	return ofstrm;
}
ifstream& operator>>(ifstream& ifstrm, CGDPtracker& tracker)
{
	string word;
	double valdoub = 0;

	ifstrm >> word >> word >> word >> tracker._base_Month;

	tracker._GDPcomponents.clear();
	ifstrm >> word >> word; // GDPcomponents {
	ifstrm >> tracker._GDPcomponents;

	tracker._Month_data.clear();
	ifstrm >> word >> word; // _Month_data {
	while (ifstrm >> word, word != "}") // } of _Month_data
	{
		CGDPcomponents comps;
		ifstrm >> comps;
		tracker._Month_data.push_back(comps);
	}

	tracker._nominal_gdp.clear();
	ifstrm >> word >> word; // _nominal_gdp {
	while (ifstrm >> word, word != "}")
	{
		valdoub = atof(word.c_str());
		tracker._nominal_gdp.push_back(valdoub);
	}

	tracker._real_gdp.clear();
	ifstrm >> word >> word; // _real_gdp {
	while (ifstrm >> word, word != "}")
	{
		valdoub = atof(word.c_str());
		tracker._real_gdp.push_back(valdoub);
	}

	tracker._gdp_deflator.clear();
	ifstrm >> word >> word; // _gdp_deflator {
	while (ifstrm >> word, word != "}")
	{
		valdoub = atof(word.c_str());
		tracker._gdp_deflator.push_back(valdoub);
	}

	ifstrm >> word; // "last_gdp_deflator"
	if (word == "last_gdp_deflator")
		ifstrm >> tracker._last_gdp_deflator;
	else // for backwards compatibility
	{
		ifstrm.putback(word[0]);
		for (size_t i = 1; i < word.length(); ++i)
			ifstrm.putback(word[i]);
	}

	ifstrm >> word; // } of GDPcomponents

	return ifstrm;
}

ofstream& operator<<(ofstream& ofstrm, const CGDPcomponents& comps)
{
	for (const auto& comp : comps)
	{
		ofstrm << " {";
		ofstrm << comp;
		ofstrm << " }";
	}

	return ofstrm;
}
ifstream& operator>>(ifstream& ifstrm, CGDPcomponents& comps)
{
	string word;

	while (ifstrm >> word, word != "}") // } of comps
	{
		CGDPcomponent comp;
		ifstrm >> comp;
		comps.push_back(comp);
		ifstrm >> word; // }
	}

	return ifstrm;
}

ofstream& operator<<(ofstream& ofstrm, const CGDPcomponent& comp)
{
	ofstrm << " " << comp._gType << " " << comp._quantity << " " << comp._price;

	return ofstrm;
}
ifstream& operator>>(ifstream& ifstrm, CGDPcomponent& comp)
{
	ifstrm >> comp._gType >> comp._quantity >> comp._price;

	return ifstrm;
}

// ------------------------------  CCPItracker  -------------------------------

ofstream& operator<<(ofstream& ofstrm, const CCPItracker& tracker)
{
	ofstrm << "\n CPItracker {";

	ofstrm << " base_Month " << tracker.getbase_Month();

	ofstrm << "\n CPIcomponents {";
	if (tracker._CPIcomponents.size() > 0)
		for (const auto& comp : tracker._CPIcomponents)
		{
			ofstrm << " {";
			ofstrm << comp; // one gType
			ofstrm << " }";
		}
	ofstrm << " }";

	ofstrm << " _Month_data {";
	if (tracker._Month_data.size() > 0)
		for (const auto& comps : tracker._Month_data)
		{
			ofstrm << " {";
			ofstrm << comps; // gTypes of one month
			ofstrm << " }";
		}
	ofstrm << " }";

	ofstrm << " _BasketVal {";
	if (tracker._BasketVal.size() > 0)
		for (double valdoub : tracker._BasketVal)
			ofstrm << " " << valdoub;
	ofstrm << " }";

	ofstrm << " _period_indices {";
	if (tracker._period_indices.size() > 0)
		for (double valdoub : tracker._period_indices)
			ofstrm << " " << valdoub;
	ofstrm << " }";

	ofstrm << " CPI {";
	if (tracker.getCPI().size() > 0)
		for (double valdoub : tracker.getCPI())
			ofstrm << " " << valdoub;
	ofstrm << " }";

	ofstrm << " _smooth_CPI {";  // Add: save smoothed CPI
	if (tracker._smooth_CPI.size() > 0)
		for (double valdoub : tracker._smooth_CPI)
			ofstrm << " " << valdoub;
	ofstrm << " }";

	ofstrm << " }"; // CPItracker {

	return ofstrm;
}

ifstream& operator>>(ifstream& ifstrm, CCPItracker& tracker)
{
	string word;
	double valdoub = 0;

	ifstrm >> word >> word >> word >> tracker._base_Month;

	tracker._CPIcomponents.clear();
	ifstrm >> word >> word; // CPIcomponents {
	ifstrm >> tracker._CPIcomponents;

	tracker._Month_data.clear();
	ifstrm >> word >> word; // _Month_data {
	while (ifstrm >> word, word != "}") // } of _Month_data
	{
		CCPIcomponents comps;
		ifstrm >> comps;
		tracker._Month_data.push_back(comps);
	}

	tracker._BasketVal.clear();
	ifstrm >> word >> word; // _BasketVal {
	while (ifstrm >> word, word != "}")
	{
		valdoub = atof(word.c_str());
		tracker._BasketVal.push_back(valdoub);
	}

	tracker._period_indices.clear();
	ifstrm >> word >> word; // _period_indices {
	while (ifstrm >> word, word != "}")
	{
		valdoub = atof(word.c_str());
		tracker._period_indices.push_back(valdoub);
	}

	tracker.CPI().clear();
	ifstrm >> word >> word; // CPI {
	while (ifstrm >> word, word != "}")
	{
		valdoub = atof(word.c_str());
		tracker.CPI().push_back(valdoub);
	}

	// Add: load smoothed CPI (with backward compatibility)
	tracker._smooth_CPI.clear();
	ifstrm >> word; // Could be "_smooth_CPI" or "}"
	if (word == "_smooth_CPI") {
		ifstrm >> word; // {
		while (ifstrm >> word, word != "}")
		{
			valdoub = atof(word.c_str());
			tracker._smooth_CPI.push_back(valdoub);
		}
		ifstrm >> word; // } of CPItracker
	}
	// else word is already "}" of CPItracker

	return ifstrm;
}

ofstream& operator<<(ofstream& ofstrm, const CCPIcomponents& comps)
{
	for (const auto& comp : comps)
	{
		ofstrm << " {";
		ofstrm << comp;
		ofstrm << " }";
	}

	return ofstrm;
}
ifstream& operator>>(ifstream& ifstrm, CCPIcomponents& comps)
{
	string word;

	while (ifstrm >> word, word != "}") // } of comps
	{
		CCPIcomponent comp;
		ifstrm >> comp;
		comps.push_back(comp);
		ifstrm >> word; // }
	}

	return ifstrm;
}

ofstream& operator<<(ofstream& ofstrm, const CCPIcomponent& comp)
{
	ofstrm << " " << comp._gType << " " << comp._quantity << " " << comp._price;

	return ofstrm;
}
ifstream& operator>>(ifstream& ifstrm, CCPIcomponent& comp)
{
	ifstrm >> comp._gType >> comp._quantity >> comp._price;

	return ifstrm;
}

// ----------------------------  curve of plot window  -------------------------
ofstream& operator<<(ofstream& ofstrm, const CVectorDoubles& curve)
{
	ofstrm << " {";

	if (curve.size() > 0)
	{
		ofstrm << " " << curve.size(); // nYvalues
		for (const auto& Yvalue : curve)
			ofstrm << " " << Yvalue; // Yvalue
	}

	ofstrm << " }";

	return ofstrm;
}
ifstream& operator>>(ifstream& ifstrm, CVectorDoubles& Yvalues)
{
	string word, bracket;

	// { 4 (nYvalues) 0 1 2 3 }

	ifstrm >> bracket; // "{"

	if (ifstrm >> word, word != "}")
	{
		long nYvalues = atol(word.c_str());

		if (nYvalues > (long)Yvalues.size())
			Yvalues.resize(nYvalues, 0);

		for (long n = 0; n < nYvalues; ++n)
			ifstrm >> Yvalues[n];

		ifstrm >> bracket; // "}"
	}

	return ifstrm;
}

// ----------------------------  plot window  ---------------------------------
ofstream& operator<<(ofstream& ofstrm, const CPlotData& plotDat)
{
	ofstrm << " {";

	if (plotDat.size() > 0)
	{
		ofstrm << " " << plotDat.size(); // nCurves
		for (const auto& pair : plotDat) // Curves
		{
			ofstrm << "\n " << pair.first; // curveIndex
			ofstrm << pair.second; // { Yvalues }
		}
	}

	ofstrm << " }";

	return ofstrm;
}
ifstream& operator>>(ifstream& ifstrm, CPlotData& plotDat)
{
	string word, bracket;

	//            { 2 (nCurves)
	//  0 { 4 (nYvalues) 0 1 2 3 }
	//  1 { 3 (nYvalues) 0 1 2 }
	//  }
	// }

	ifstrm >> bracket; // "{"

	if (ifstrm >> word, word != "}")
	{
		long nCurves = atol(word.c_str()); // 2
		GoodType gType;
		for (long n = 0; n < nCurves; ++n)
		{
			ifstrm >> gType;
			ifstrm >> plotDat[gType];
		}

		// }
		ifstrm >> bracket;
	}

	return ifstrm;
}

// ------------------------------  PlotsData  -------------------------------
ofstream& operator<<(ofstream& ofstrm, const CPlotsData& plotsDat)
{
	ofstrm << " {";

	if (plotsDat.size() > 0)
	{
		ofstrm << " " << plotsDat.size();
		for (const auto& pair : plotsDat)
		{
			ofstrm << "\n " << pair.first; // windowTitle
			if (pair.first == "IndivsWealth")
			{
				long n = 0;
				ofstrm << " { 1\n";// << n << " { " << DEPData().getsorted_IndivsWealth().size();
				ofstrm << " 0 { " << DEPData().getsorted_IndivsWealth().size();

				for (const auto& sortedAgent : DEPData().getsorted_IndivsWealth())
					//	ofstrm << sortedAgent;
					ofstrm << " " << sortedAgent._qtty;

				ofstrm << " }";
				ofstrm << " }";
			}
			else if (pair.first == "ProducersWealth")
			{
				long n = 0;
				ofstrm << " { 1\n";// << n << " { " << DEPData().getsorted_ProducersWealth().size();
				ofstrm << " 0 { " << DEPData().getsorted_ProducersWealth().size();

				for (const auto& sortedAgent : DEPData().getsorted_ProducersWealth())
					//	ofstrm << sortedAgent;
					ofstrm << " " << sortedAgent._qtty;

				ofstrm << " }";
				ofstrm << " }";
			}
			else
				ofstrm << pair.second; // rows of CPlotData (vectors)
		}
	}

	ofstrm << " }";

	return ofstrm;
}
ifstream& operator>>(ifstream& ifstrm, CPlotsData& plotsDat)
{
	string word, bracket, windowTitle;
	long npoints = 0;

	// PlotsData: { 1 (nWindows)
	//  windowTitle0 { 2 (nCurves)
	//  0 { 4 (nYvalues) 0 1 2 3 }
	//  1 { 3 (nYvalues) 0 1 2 }
	//  }
	// }

	//           { 1 (nWindows)

	ifstrm >> bracket; // "{"
	if (ifstrm >> word, word != "}")
	{
		long nWindows = atol(word.c_str()); // 1
		for (long n = 0; n < nWindows; ++n)
		{
			ifstrm >> windowTitle; // "windowTitle0 "
			if (windowTitle == "IndivsWealth")
			{
				ifstrm >> bracket >> word >> word >> bracket >> npoints;
				DEPData().sorted_IndivsWealth().clear();
				DEPData().sorted_IndivsWealth().resize(npoints);

				for (long ix = 0; ix < npoints; ++ix)
					//	ifstrm >> DEPData().sorted_IndivsWealth()[ix];
					ifstrm >> DEPData().sorted_IndivsWealth()[ix]._qtty;

				ifstrm >> bracket >> bracket;
			}
			else if (windowTitle == "ProducersWealth")
			{
				ifstrm >> bracket >> word >> word >> bracket >> npoints;
				DEPData().sorted_ProducersWealth().clear();
				DEPData().sorted_ProducersWealth().resize(npoints);

				for (long ix = 0; ix < npoints; ++ix)
					//	ifstrm >> DEPData().sorted_ProducersWealth()[ix];
					ifstrm >> DEPData().sorted_ProducersWealth()[ix]._qtty;

				ifstrm >> bracket >> bracket;
			}
			else
				ifstrm >> plotsDat[windowTitle];
		}

		// }
		ifstrm >> bracket;
	}

	return ifstrm;
}

// ------------------------------  CData  -------------------------------

ofstream& operator<<(ofstream& ofstrm, const CData& cData)
{
	ofstrm << "\nDEPData {";

	ofstrm << "\n TotalInitialCash " << cData.getTotalInitialCash();
	ofstrm << "\n Unemployment_currMonth " << cData.getUnemployment();
	ofstrm << "\n UnemploymentHistory "; ofstrm << cData.getUnemploymentHistory();
	ofstrm << "\n totalPartTimeWork " << cData.gettotalPartTimeWork();
	ofstrm << "\n totalFullTimeWork " << cData.gettotalFullTimeWork();
	ofstrm << "\n IndivsWealth " << cData.getIndivsWealth();
	ofstrm << "\n ProducersWealth " << cData.getProducersWealth();
	ofstrm << "\n dProducersWealth " << cData.getdProducersWealth();
	ofstrm << "\n GDPnominal " << cData.getGDPnominal();
	ofstrm << "\n TotalUnits " << cData.getTotalUnits();

	ofstrm << cData.getGDPtracker();

	ofstrm << cData.getCPItracker();

	ofstrm << "\n TotLoansQtty " << cData.getTotLoansQtty();
	ofstrm << "\n TotLoansInterests " << cData.getTotLoansInterests();
	ofstrm << "\n AvgLoansInterests " << cData.getAvgLoansInterests();

	ofstrm << "\n DepreciationRate ";
	ofstrm << cData.getDepreciationRate();

	ofstrm << "\n nCurrentPProducers "; ofstrm << cData.getnCurrentPProducers();

	ofstrm << "\n FirmBirths { ";
	for (int month = 0; month < 12; ++month)
	{
		long index = (currMonth() - 11 + month);
		if (index < 0)
			ofstrm << "0 ";
		else
			ofstrm << "" << cData.getFirmBirths().at(index % cData.getFirmBirths().size()) << " ";
	}
	ofstrm << "} ";

	ofstrm << "\n FirmDeaths { ";
	for (int month = 0; month < 12; ++month)
	{
		long index = (currMonth() - 11 + month);
		if (index < 0)
			ofstrm << "0 ";
		else
			ofstrm << "" << cData.getFirmDeaths().at(index % cData.getFirmDeaths().size()) << " ";
	}
	ofstrm << "} ";

	ofstrm << "\n MarketPrices_currMonth {";
	for (const auto& pair : cData.getMarketPrices())
	{
		double price = pair.second;
		ofstrm << " " << price;
	}
	ofstrm << " }";

	ofstrm << "\n SectorMarkups ";
	ofstrm << cData.getSectorMarkups();

	ofstrm << "\n DataGrossOutput_mu ";
	ofstrm << cData.getDataGrossOutput_mu();

	ofstrm << "\n TotalDataGrossOutput_mu " << cData.getTotalDataGrossOutput_mu();

	ofstrm << "\n DataHouseholdsGFCF_mu " << cData.getDataHouseholdsGFCF_mu();

	ofstrm << "\n DataGovGFCF_mu " << cData.getDataGovGFCF_mu();
	ofstrm << "\n TotalDataGov_mu " << cData.getTotalDataGov_mu();

	ofstrm << "\n TotProduced ";
	ofstrm << cData.getTotProduced();

	ofstrm << "\n TotalProduction " << cData.getTotalProduction();

	ofstrm << "\n TotSupply ";
	ofstrm << cData.getTotSupply();

	ofstrm << "\n TotDemand ";
	ofstrm << cData.getTotDemand();

	ofstrm << "\n PlotsData ";
	ofstrm << cData.getPlotsData();

	ofstrm << "\n TotalAssistedProd " << cData.getTotalAssistedProd();

	// Serialized cached values (added at END for backward compatibility)
	ofstrm << "\n FinishCalibrationAt " << cData.getFinishCalibrationAt();

	ofstrm << "\n}\n";

	return ofstrm;
}
ifstream& operator>>(ifstream& ifstrm, CData& cData)
{
	string word, word1;
	double price;
	long gType = 0;
	long prevMonth = max((long)0, currMonth() - 1);

	ifstrm >> word; // "{ "

	ifstrm >> word >> cData.TotalInitialCash();
	ifstrm >> word >> cData.Unemployment();
	ifstrm >> word; ifstrm >> cData.UnemploymentHistory();

	ifstrm >> word >> cData.totalPartTimeWork();
	ifstrm >> word >> cData.totalFullTimeWork();
	ifstrm >> word >> cData.IndivsWealth();
	ifstrm >> word >> cData.ProducersWealth();
	ifstrm >> word >> cData.dProducersWealth();
	ifstrm >> word >> cData.GDPnominal();
	ifstrm >> word >> cData.TotalUnits();

	ifstrm >> cData.GDPtracker();

	ifstrm >> cData.CPItracker();

	ifstrm >> word >> cData.TotLoansQtty();
	ifstrm >> word >> cData.TotLoansInterests();
	ifstrm >> word >> cData.AvgLoansInterests();

	ifstrm >> word; ifstrm >> cData.DepreciationRate();

	cData.InitialProducerTypes().clear();

	ifstrm >> word; ifstrm >> cData.nCurrentPProducers();

	ifstrm >> word >> word; // FirmBirths {
	cData.FirmBirths().clear();
	cData.FirmBirths().resize(10 * 12, 0);
	for (int month = 0; month < 12; ++month)
	{
		long index = (currMonth() - 11 + month);
		if (index >= 0)
			ifstrm >> cData.FirmBirths()[index % cData.FirmBirths().size()];
		else
		{
			double dummy;
			ifstrm >> dummy;
		}
	}
	ifstrm >> word; // }

	ifstrm >> word >> word; // FirmDeaths {
	cData.FirmDeaths().clear();
	cData.FirmDeaths().resize(10 * 12, 0);
	for (int month = 0; month < 12; ++month)
	{
		long index = (currMonth() - 11 + month);
		if (index >= 0)
			ifstrm >> cData.FirmDeaths()[index % cData.FirmDeaths().size()];
		else
		{
			double dummy;
			ifstrm >> dummy;
		}
	}
	ifstrm >> word; // }

	ifstrm >> word1 >> word; // "MarketPrices_currMonth {"
	gType = 0;
	while (ifstrm >> word, word != "}")
	{
		price = atof(word.c_str());
		cData.MarketPrices()[gType++] = price;
	}

	ifstrm >> word1; // "SectorMarkups"
	ifstrm >> cData.SectorMarkups();

	ifstrm >> word; // "DataGrossOutput_mu"
	ifstrm >> cData.DataGrossOutput_mu();

	ifstrm >> word >> cData.TotalDataGrossOutput_mu();

	ifstrm >> word >> cData.DataHouseholdsGFCF_mu();

	ifstrm >> word >> cData.DataGovGFCF_mu();
	ifstrm >> word >> cData.TotalDataGov_mu();

	ifstrm >> word; // "TotProduced"
	ifstrm >> cData.TotProduced();

	ifstrm >> word >> cData.TotalProduction();

	ifstrm >> word; // "TotSupply"
	ifstrm >> cData.TotSupply();

	ifstrm >> word; // "TotDemand"
	ifstrm >> cData.TotDemand();

	ifstrm >> word; // "PlotsData"
	ifstrm >> cData.PlotsData();

	ifstrm >> word >> cData.TotalAssistedProd();

	// Read serialized cached values (added at END for backward compatibility)
	// Check if FinishCalibrationAt exists (for backward compatibility with old snapshots)
	ifstrm >> word;
	if (word == "FinishCalibrationAt")
	{
		ifstrm >> cData.FinishCalibrationAt();
		ifstrm >> word; // "}"
	}
	// else word is already "}" from old format

	return ifstrm;
}

//=====================  CData  ====================================

CData::CData()
{
	InputFileName() = "__InitialInput";
};
CData::~CData() {};

double CData::InputParameter(string param) { return InputParameters().at(param); }
void CData::initializeInputParameters()
{
	mInputParameters().clear();
	mInputParameters()["SimulationName"] = 0;
	mInputParameters()["PlotsX0"] = 0;
	mInputParameters()["nSimulatedWorkersPerSector"] = 17;	// Austria with old param=4 had ~1075 workers / 64 sectors ≈ 17
	mInputParameters()["PlotProducerID"] = 43;
	mInputParameters()["MonthsBetweenUpdates"] = 12;
	mInputParameters()["LogPlots"] = 0;
	// SaveMonthsAfterCalibration: relative to FinishCalibrationAt, converted to absolute SaveMonths dynamically

	mInputParameters()["WorkDaysPerMonth"] = 20;
	mInputParameters()["Nproduction_days"] = 1;
	mInputParameters()["nInitSalaries"] = 50;

	mInputParameters()["UsePrevProvidersFrom"] = 0;
	mInputParameters()["StartCalibrationAt"] = 24;

	mInputParameters()["GFCFStabilityCheckStartMonth"] = 36;  // start checking GFCF stability after this month
	mInputParameters()["GFCFStabilityTolerance"] = 0.02;  // 2% relative change tolerance
	mInputParameters()["GFCFRequiredStableMonths"] = 12;  // consecutive months of stability required
	mInputParameters()["AssistedMonthsAfterGFCFStability"] = 60;  // wait this many months after GFCF stabilizes

	mInputParameters()["ResetPricesToOneAt"] = 999360;
	mInputParameters()["BaseMonth"] = 60;  // -1 means use rolling 12-month, >=0 means use fixed base month
	mInputParameters()["AssistedProductionUpto"] = 240;  // initial default, updated dynamically by GFCF stability
	mInputParameters()["AssistedMonthsAfterWealthTarget"] = 60;  // months to continue assisted calibration after reaching IndivsWealthTarget
	mInputParameters()["StabilizationMonthsAfterAssisted"] = 24;  // months to smoothly transition after assisted phase
	// GFCF stability parameters for dynamic AssistedProductionUpto
	mInputParameters()["StartUnemploymentCalibrationAt"] = 132;
	mInputParameters()["FinishCalibrationAt"] = 240;  // safe initial upper bound, will be set dynamically when conditions are met

	mInputParameters()["SimulateNYearsAfterCalibration"] = 5;  // years to simulate after calibration ends
	mInputParameters()["NYears"] = 25.0;  // default, will be extended dynamically after calibration

	mInputParameters()["PercentDemand"] = 0.90;
	mInputParameters()["PercentDemandFrom"] = 999576;
	mInputParameters()["PercentDemandUpto"] = 579;

	mInputParameters()["FiscalPolicyShockFrom"] = 9999384;
	mInputParameters()["FiscalPolicyShockUpto"] = 528;
	mInputParameters()["FiscalPolicyShockFactor"] = 1.10;

	mInputParameters()["TaxChangeFrom"] = 9999504;
	mInputParameters()["TaxChangeUpto"] = 528;
	mInputParameters()["ProducerAccN"] = 35;
	mInputParameters()["TaxAccN"] = 69;
	mInputParameters()["TaxChangeFactor"] = 1.50;

	// =============== Develop & debug parameters  ===========================

	mInputParameters()["PlotIndivID"] = 9;
	mInputParameters()["PlotsY0"] = 0;
	mInputParameters()["PlotLineWidth"] = 1;
	mInputParameters()["PlotMarkerSize"] = 5;
	mInputParameters()["WriteSAMmonth"] = 0;
	mInputParameters()["PlotExtSectIndex"] = 0;
	mInputParameters()["PrintFullSAM"] = 1;
	mInputParameters()["medianTarget"] = 150000;
	mInputParameters()["LoadMonthN"] = 0;
	mInputParameters()["Autorun"] = 0;

	mInputParameters()["FixCap"] = 1;
	mInputParameters()["RunFIGARODebug"] = 0; // set to 1 for Debug executable

	mInputParameters()["NsleepForIO"] = 900; // 900*sleep_seconds = 90 sec
	mInputParameters()["sleep_seconds"] = 0.1; // 0.1 sec

	mInputParameters()["ExitOnQuit"] = 1;
	mInputParameters()["IndivConsum"] = 1;
	mInputParameters()["GovConsum"] = 1;
	mInputParameters()["ExtSectConsum"] = 1;
	mInputParameters()["ReadIOfilesFromMonth"] = 0;

	mInputParameters()["MaxNBanks"] = 2;
	mInputParameters()["MonetaryBasePerActiveSalaries"] = 800;
	mInputParameters()["LeverageRatio"] = 10;
	mInputParameters()["ReserveRatio"] = 0.10;
	mInputParameters()["markup"] = 0.10;
	mInputParameters()["markdown"] = 0.10;
	mInputParameters()["lambdaB"] = 2;
	mInputParameters()["EpsilonScale"] = 0.01;
	mInputParameters()["NuProbabDefault"] = 0.05;
	mInputParameters()["CPItoInterestRateFactor"] = 1.0; // see also BaseMonth
	mInputParameters()["TargetRealRate"] = 3.5;
	mInputParameters()["MinInterestRate"] = 0.1;

	mInputParameters()["FinancialMarket"] = 1;
	mInputParameters()["UseSecondaryMarket"] = 0;
	mInputParameters()["MinWorthForSMarket"] = 1.2e6;
	mInputParameters()["BondToSharesRatio"] = 0.0;

	mInputParameters()["MaxOwnedProducers"] = 99992;
	mInputParameters()["MinSalariesToStartProducer"] = 0;
	mInputParameters()["ProducerStartupProbab"] = 0.009;
	mInputParameters()["MinStartupOwnerWealth"] = 0;
	mInputParameters()["StartupCapitalFraction"] = 0.3; // Fraction of owner's wealth transferred to new producer as startup capital
	mInputParameters()["MonthsWithNegativeWealthToDismantle"] = 1;
	mInputParameters()["nBufferSalaries"] = 16;
	mInputParameters()["IwishSpreadFraction"] = 0.50;
	mInputParameters()["gammaC"] = 4;
	mInputParameters()["MaxConsumPXFactor"] = 50.;
	mInputParameters()["InitialConsumPXFactor"] = 0.5;  // Start lower to avoid initial overshoot

	mInputParameters()["InflationProbability"] = 1.0;
	mInputParameters()["CPItoSalaryFactor"] = 1.0; // see also BaseMonth
	mInputParameters()["LowUnsoldFraction"] = 0.35;
	mInputParameters()["InflationFactor"] = 1.002;
	mInputParameters()["MinCurrentPProducers"] = 2;
	mInputParameters()["KtoFixCapitalFactor"] = 20;
	mInputParameters()["KchangeFraction"] = 0.05; // Max monthly capital growth rate (5% = ~80% annual)
	mInputParameters()["DepreciationRate"] = 0.01;
	mInputParameters()["PriceAdaptFactor"] = 1.005;
	mInputParameters()["MaxPriceFactor"] = 14;
	mInputParameters()["MinUtilizationFactor"] = 0.33;
	mInputParameters()["DesiredStockLevelFactor"] = 2.5;
	mInputParameters()["ChangeFractionAssisted"] = 0.10;
	mInputParameters()["ChangeFraction"] = 0.02;
	mInputParameters()["HISTORY_LENGTH"] = 12;
	mInputParameters()["ExportFraction"] = 0.20;
	mInputParameters()["TradeDisaggMode"] = 0; // 0 = Country (default), 1 = Sector

	mInputParameters()["MaxNeighboringWorkers"] = 250;
	mInputParameters()["MaxNeighboringProducersPerSector"] = 3;

	// Labor market friction parameters (Phillips curve)
	mInputParameters()["NAIRU"] = 0.05; // Natural rate of unemployment (5%)
	mInputParameters()["LaborFrictionSensitivity"] = 2.0; // How strongly friction increases as unemployment falls below NAIRU

	// COVID-19 Pandemic parameters (Darwinian ABM approach - no BLE)
	// Set PandemicActive=0 for NO pandemic effects (baseline)
	mInputParameters()["PandemicActive"] = 0;           // 0=disabled, 1=enabled
	mInputParameters()["PandemicStartMonthAfterCalibrated"] = 24; // Months AFTER FinishCalibrationAt when pandemic begins
	mInputParameters()["PandemicDurationMonths"] = 24;  // Total pandemic duration (acute + recovery + new normal)
	mInputParameters()["PandemicAcutePhaseMonths"] = 3; // Duration of acute lockdown phase
	mInputParameters()["PandemicRecoveryMonths"] = 12;  // Duration of gradual recovery phase
	mInputParameters()["AmplifyPandemicTo20PercentGDP"] = 0; // 0=realistic ~6% GDP drop, 1=amplified ~20-30% for visual testing
	mInputParameters()["FurloughSubsidyRate"] = 0.80;   // Government pays 80% of wages for furloughed workers
	mInputParameters()["FurloughCoverageRate"] = 0.50;  // 50% of affected workers eligible for furlough
	mInputParameters()["CreditTighteningFactor"] = 0.90; // Banks reduce lending capacity by 10%
	mInputParameters()["ExportDemandShockAcute"] = 0.95; // Exports drop 5% during acute phase (mild)
	mInputParameters()["ExportDemandShockRecovery"] = 0.98; // Exports at 98% during recovery
	mInputParameters()["ImportSupplyShockAcute"] = 0.95; // Import supply at 95% during acute phase (mild)
	mInputParameters()["ImportSupplyShockRecovery"] = 0.98; // Import supply at 98% during recovery
	// Historical validation mode (Austria COVID-19 test)
	mInputParameters()["UseHistoricalData"] = 0;        // 0=use naive binary mode, 1=load historical CSV data
	// HistoricalDataFile is a string parameter, handled separately in readInputParameters()
	// Poledna AMS labor shock mode
	mInputParameters()["UseAMSLaborShocks"] = 0;        // 0=use default shocks, 1=load AMS March 2020 unemployment data

	// Behavioral Learning Equilibrium (BLE) - Poledna parallel comparison mode
	// When UseBLE=1, agents form expectations using AR(1) rules and learn parameters
	// This enables a fair comparison with Poledna et al. (2023) methodology
	mInputParameters()["UseBLE"] = 0;                   // 0=disabled (backward-looking), 1=enabled (BLE learning)
	mInputParameters()["BLEWeight"] = 1.0;              // Weight for BLE expectations (0=pure backward, 1=pure BLE)
	mInputParameters()["BLELearningGain"] = 0.02;       // Constant gain learning rate (higher = faster adaptation)
	mInputParameters()["BLELearningWindow"] = 20;       // Months of history for parameter estimation
	mInputParameters()["BLEMinTrainingMonths"] = 24;    // Months of training before BLE influences agents
	mInputParameters()["LogBLEExpectations"] = 0;       // 0=quiet, 1=log BLE state every 12 months

	mInputParameters()["DebugFromMonth"] = 9999;
	mInputParameters()["DebugLevel"] = 0;
	mInputParameters()["ControlXSize"] = 340;
	mInputParameters()["ControlYSize"] = 360;
	mInputParameters()["PlotsXSize"] = 420;
	mInputParameters()["PlotsYSize"] = 250;
	mInputParameters()["SortIncreasingWealth"] = 0;
};

void CData::readInputParameters()
{
	initializeInputParameters();
	string word;
	double value = 0;

	string fileNameExt = DEPData().InputFileName() + ".dep";

	ifstream& ifstrm = *new ifstream();
	ifstrm.open(fileNameExt);
	if (ifstrm.fail())
		CWorld::ERRORmsg("Couldn't read " + fileNameExt, true);

	while (ifstrm >> word, word != "INPUT_PARAMETERS");

	value = 0;
	ifstrm >> word; // "{"

	ifstrm >> word;
	while (word != "}")
	{
		// Support for // comments: skip to end of line if word starts with //
		if (word.length() >= 2 && word.substr(0, 2) == "//")
		{
			// Skip the rest of the line
			string restOfLine;
			getline(ifstrm, restOfLine);
			ifstrm >> word;
			continue;
		}

		if (word == "SimulationName")
		{
			ifstrm >> CWorld::SimulationName();
		}
		else if (word == "HistoricalDataFile")
		{
			ifstrm >> DEPData().HistoricalDataFile();
		}
		else if (word == "SaveMonthsAfterCalibration")
		{
			DEPData().SaveMonthsAfterCalibration().clear();
			while (ifstrm >> word, !isalpha(word.c_str()[0]))
				DEPData().SaveMonthsAfterCalibration().push_back(atoi(word.c_str()));
			continue;
		}
		else if (word == "SaveMonthsFromMonth0")
		{
			DEPData().SaveMonthsFromMonth0().clear();
			while (ifstrm >> word, !isalpha(word.c_str()[0]))
				DEPData().SaveMonthsFromMonth0().push_back(atoi(word.c_str()));
			continue;
		}
		else
		{
			// All other parameters are followed by numeric values
			ifstrm >> value;
			mInputParameters()[word] = value;
		}

		ifstrm >> word;
	}

	// Finished reading INPUT_PARAMETERS

	if (getInputParameter("nSimulatedWorkersPerSector") < 2.0)
	{
		CWorld::ERRORmsg("WARNING: nSimulatedWorkersPerSector should be >= 2.0", false);
	}

	ifstrm.close();
}

void CData::writeInputParameters(ofstream& ofstrm)
{
	ofstrm << "\nINPUT_PARAMETERS {" << endl;

	// First write SimulationName if it exists
	auto simNameIt = InputParameters().find("SimulationName");
	if (simNameIt != InputParameters().end())
	{
		ofstrm << " SimulationName " << CWorld::SimulationName() << endl;
	}

	// Write SaveMonthsAfterCalibration if they exist
	if (getDEPData().getSaveMonthsAfterCalibration().size() > 0)
	{
		ofstrm << " SaveMonthsAfterCalibration";
		for (const auto& monthN : getDEPData().getSaveMonthsAfterCalibration())
			ofstrm << " " << monthN;
		ofstrm << endl;  // CRITICAL: Add newline after SaveMonthsAfterCalibration
	}

	// Write SaveMonthsFromMonth0 if they exist
	if (getDEPData().getSaveMonthsFromMonth0().size() > 0)
	{
		ofstrm << " SaveMonthsFromMonth0";
		for (const auto& monthN : getDEPData().getSaveMonthsFromMonth0())
			ofstrm << " " << monthN;
		ofstrm << endl;
	}

	// Write HistoricalDataFile if it exists
	if (!getDEPData().getHistoricalDataFile().empty())
	{
		ofstrm << " HistoricalDataFile " << getDEPData().getHistoricalDataFile() << endl;
	}

	// Write all other parameters
	for (const auto& pair : InputParameters())
	{
		// Skip SimulationName and SaveMonths* as they were already handled
		if (pair.first == "SimulationName" || pair.first == "SaveMonthsAfterCalibration" || pair.first == "SaveMonthsFromMonth0")
			continue;

		ofstrm << " " << pair.first << " " << pair.second << endl;
	}

	ofstrm << "}" << endl;  // Close INPUT_PARAMETERS block
};

void CData::writePlotsInputParameters(ofstream& ofstrm) const
{
	ofstrm << "\nPLOTS {\n";

	auto const& Plots = DEPData().getPlotsInputParameters();
	if (Plots.size() > 0)
	{
		long ColumnX = 0, RowY = 0, nplotWin = 0;
		while (nplotWin < Plots.size())
		{
			bool found = false;
			for (const auto& pair : Plots)
			{
				if (pair.second.PlotX0 == ColumnX && pair.second.PlotY0 == RowY)
				{
					if (RowY == 0)
						ofstrm << "\n =Column= " << ColumnX << endl;

					ofstrm << " " << pair.second.windowTitle << " " << pair.second.PlotType << endl;
					++nplotWin;

					found = true;
					break;
				}
			}

			++RowY;

			if (RowY == Plots.size())
			{
				RowY = 0;
				++ColumnX;
			}
		}
	}

	ofstrm << "}\n";
};
void CData::readPlotsInputParameters(string fileNameExt)
{
	string word, title, lbl;
	GoodType plotType = -1;
	long ID = UndefAgentID;

	ifstream ifstrm(fileNameExt);
	if (ifstrm.fail())
		CWorld::ERRORmsg("Failed to open " + fileNameExt, true);

	while (ifstrm >> word, word != "PLOTS")
		if (ifstrm.eof())
			return;

	DEPData().PlotsInputParameters().clear();
	ifstrm >> word; // "{"
	int ColN = -1, RowN = 0;
	while (ifstrm >> word, word != "}")
	{
		if (word == "=Column=") // new column
		{
			ifstrm >> ColN;
			RowN = 0;
		}
		else
		{
			CPlotDefinition* plotDefinition = new CPlotDefinition();

			plotDefinition->pLabel_array = nullptr;

			plotDefinition->windowTitle = word;
			plotDefinition->plotTitle = word;

			// ------------------------------------------------------------------

			ifstrm >> plotType;
			plotDefinition->PlotType = plotType;
			plotDefinition->PlotX0 = ColN;
			plotDefinition->PlotY0 = RowN;

			plotDefinition->TooltipFormat = "<plot_string:%s>: <sample_y:%.3lf>, month <sample_x:%.0lf>";

			// ---------------------------------------------------------------------

			if (plotDefinition->PlotType >= 0)
				DEPData().PlotsInputParameters()[plotDefinition->windowTitle] = *plotDefinition;

			RowN++;
		}
	}
}

long CData::getMonthsPerYear() const { return 12; }
long CData::getCurrYear() const { return currMonth() / getMonthsPerYear(); };
double& CData::NYears() { return _NYears; };
double CData::getNYears() const { return _NYears; }
long CData::getEndMonth() const { return ::getInitMonth() + getNMonths(); }
// getFinishCalibrationAt() is now inline in pch.h, using serialized _FinishCalibrationAt
double CData::getUpscaleSimulationFactor() const {
	return (double)getSAM().getActive() / getWorld().getNWorkers();
}
double CData::toTotalPopulationYear() const {
	return 12. * getUpscaleSimulationFactor();
}
double& CData::VABpE(GoodQtty firmSize)
{
	if (firmSize >= 250)
		return VABpE()[4];
	else if (firmSize >= 50)
		return VABpE()[3];
	else if (firmSize >= 20)
		return VABpE()[2];
	else if (firmSize >= 10)
		return VABpE()[1];
	else
		return VABpE()[0];
}
double CData::getVABpE(GoodQtty firmSize)
{
	return VABpE(firmSize);
}
double& CData::VABpEv(GoodQtty firmSize)
{
	if (firmSize >= 250)
		return VABpEv()[4];
	else if (firmSize >= 50)
		return VABpEv()[3];
	else if (firmSize >= 20)
		return VABpEv()[2];
	else if (firmSize >= 10)
		return VABpEv()[1];
	else
		return VABpEv()[0];
}
double CData::getVABpEv(GoodQtty firmSize)
{
	return VABpEv(firmSize);
}
GoodQtty& CData::VABpEn(GoodQtty firmSize)
{
	if (firmSize >= 250)
		return VABpEn()[4];
	else if (firmSize >= 50)
		return VABpEn()[3];
	else if (firmSize >= 20)
		return VABpEn()[2];
	else if (firmSize >= 10)
		return VABpEn()[1];
	else
		return VABpEn()[0];
}
GoodQtty CData::getVABpEn(GoodQtty firmSize)
{
	return VABpEn(firmSize);
}
CGoods& CData::Dismantled() { return _Dismantled; }
CGoods& CData::Inventories() { return _Inventories; }
CGoods& CData::ToSell() { return _ToSell; }
const CGoods& CData::getDismantled() const { return _Dismantled; }
const CGoods& CData::getInventories() const { return _Inventories; }
const CGoods& CData::getToSell() const { return _ToSell; }

bool CData::CheckAccountingBalance()
{
	auto CurrentMonth = currMonth();

	GoodQtty TotalCash = 0;

	TotalCash += pGovernment()->getCash();

	// CENTRALBANK & BANKS are Producers, else

	if (getInputParameter("MaxNBanks") > 0)
	{
	}
	TotalCash += pCentralBank()->getCash();
	for (const auto& pair : getCentralBank().getBanks())
		TotalCash += pair.second->getCash();

	// Workers and Producers

	TotalIndivsCash() = 0;
	for (const auto& pIndiv : *getWorld().getpWorkers())
	{
		TotalIndivsCash() += pIndiv->Cash();
		TotalCash += pIndiv->getCash();
	}

	for (const auto& pProducer : *getWorld().getpProducers())
	{
		if (pProducer == nullptr)
			continue;
		TotalCash += pProducer->getCash();
	}

	for (auto& gPair : ExtSectors())
		TotalCash += gPair.second->getCash();

	// Check Bank Client Pointers
	auto validate_bank_clients = [&](CBank* bank) {
		if (!bank) return true;
		for (const auto& account_pair : bank->getClientsAccounts()) {
			const auto& account = account_pair.second;
			CAgent* client = account._pAgent;
			if (client == nullptr) {
				// This might be a valid case if accounts can exist without a client temporarily.
				// If not, this should be an error.
				continue;
			}

			bool found = false;
			if (client->IsWorker()) {
				for (const auto& worker : *getWorld().getpWorkers()) {
					if (worker == client) {
						found = true;
						break;
					}
				}
			}
			else if (client->IsProducer()) {
				for (const auto& producer : *getWorld().getpProducers()) {
					if (producer == client) {
						found = true;
						break;
					}
				}
			}
			else if (client->IsGovernment()) {
				if (pGovernment() == client) {
					found = true;
				}
			}

			if (!found) {
				LogFile() << "\n month " << currMonth() << " Bank " << bank->getAgentName()
					<< " has an invalid client pointer for account ID " << account_pair.first << ".";
				LogFile().flush();
				getWorld().ERRORmsg("Invalid client pointer in bank account", true);
				return false;
			}
		}
		return true;
		};

	if (!validate_bank_clients(pCentralBank())) return false;
	for (const auto& pair : getCentralBank().getBanks()) {
		if (!validate_bank_clients(pair.second)) return false;
	}

	// Check Total Cash  ======================================================

#ifdef GOODQTTY_IS_DOUBLE
	// if GoodQtty is defined as double, the following code is correct

	double mismatch = (getTotalInitialCash() - TotalCash) / TotalCash;
	double absmismatch = abs(mismatch);
	double maxmismatch = 1.0E-9;

	if (absmismatch > maxmismatch)
	{
		LogFile() << "\n month " << currMonth() << " mismatch " << mismatch;
		LogFile().flush();
		getWorld().ERRORmsg("mismatch", true);

		return false;
	}
	else
		return true;
#else
	auto mismatch = getTotalInitialCash() - TotalCash;

	if (mismatch == 0)
	{
		return true;
	}
	else
	{
		LogFile() << "\n month " << currMonth() << " mismatch " << mismatch;
		LogFile().flush();
		getWorld().ERRORmsg("mismatch", true);

		return false;
	}
#endif
}

void CData::initialize_constructor()
{
	NYears() = getInputParameter("NYears");
	NMonths() = getMonthsPerYear() * NYears();
	FinishCalibrationAt() = (long)getInputParameter("FinishCalibrationAt");

	_pBanksIDs = new vector<AgentID>;

	TotalIndivsGoodsAndServ() = 0;
	TotalIndivsCash() = 0;
	TotalIndivsBank() = 0;
	TotalProducersCash() = 0;
	TotalBanksCash() = 0;
	nTotalCurrentProducers() = 0;
	nFullTimeWorkers() = 0;
	nPartTimeWorkers() = 0;
	totalFullTimeWork() = 0;
	totalPartTimeWork() = 0;
	PartToFullTimeWorkersRatio() = 0;

	IndivsWealth() = 0;
	ProducersWealth() = 0;
	dProducersWealth() = 0;
	prevUnemployment() = 0;
	Unemployment() = 0;

	// Initialize unemployment history with HISTORY_LENGTH window
	// Must be done AFTER getInputParameter is available
	int historyLength = static_cast<int>(getInputParameter("HISTORY_LENGTH"));
	_UnemploymentHistory = CHistory(historyLength);

	GDPnominal() = 0;
	TotalUnits() = 0;
	DataGrossOutput_mu().clear();
	TotalDataGrossOutput_mu() = 0;
	TotalProduction() = 0;
	DataHouseholdsGFCF_mu() = 0;
	DataGovGFCF_mu() = 0;
	TotalDataGov_mu() = 0;
	prevavgDeprecError2() = 0;

	TotalInitialCash() = 0;

	delete _pBanksIDs;
	_pBanksIDs = new vector<AgentID>;

	long historyYears = 10;
	long historyMonths = historyYears * getMonthsPerYear();
	FirmBirths().resize(historyMonths, 0);
	FirmDeaths().resize(historyMonths, 0);
	avgFirmBirths() = 0.;
	avgFirmDeaths() = 0.;
}
void CData::initialize()
{
	initialize_constructor();

	TotalAssistedProd() = 0;
	MA_TotAssistedProd().clear();

	nEmplPerFirmsize().clear();
	nEmplPerFirmsize().clear();
	nEmplPerFirmsize()["0-9"] = 0.;
	nEmplPerFirmsize()["10-49"] = 0.;
	nEmplPerFirmsize()["50-249"] = 0.;
	nEmplPerFirmsize()[">=250"] = 0.;

	nFirmsPerFirmsize().clear();
	nFirmsPerFirmsize()["0-9"] = 0.;
	nFirmsPerFirmsize()["10-49"] = 0.;
	nFirmsPerFirmsize()["50-249"] = 0.;
	nFirmsPerFirmsize()[">=250"] = 0.;

	TotLoansQtty() = 0;
	TotLoansInterests() = 0.;
	AvgLoansInterests() = 0.;

	TotalInitialCash() = 0;
	TotalInitialCash() += pGovernment()->getInitCash();

	TotalInitialCash() += pCentralBank()->getCash();
	for (const auto& pair : getCentralBank().getBanks())
		TotalInitialCash() += pair.second->getCash();

	for (auto& gPair : ExtSectors())
		TotalInitialCash() += gPair.second->getInitCash();

	for (const auto& pIndiv : *getWorld().getpWorkers())
	{
		TotalInitialCash() += pIndiv->getInitCash();
		TotalIndivsCash() += pIndiv->getInitCash();
	}

	for (const auto& gProducer : *getWorld().getpProducers())
	{
		if (gProducer == nullptr)
			continue;

		TotalInitialCash() += gProducer->getInitCash();
	}

	SAM().pSAMmonth()->clear();
	SAM().pSAMmonth()->resize(getSAM().getnAccounts(),
		vector<double>(getSAM().getnAccounts(), 0.0));

	// MarketPrices  -------------------------

	MarketPrices().clear();
	SectorMarkups().clear();
	TotalSalaries() = 0; // last market price will be avgSalary
	TotalWorkedTime() = 0;
	for (GoodType gType = 0; gType <= getSAM().getnPXproducerTypes(); ++gType)
		MarketPrices()[gType] = 1.0;

	MarketPrices()[getSAM().getnPXproducerTypes()] = 1.0; // used for PriceOfSalary

	nActiveWorkers().clear();
	Inventories().clear();
	for (GoodType gType = 0; gType < getSAM().getnPProducerTypes(); ++gType)
	{
		nActiveWorkers()[gType] = 0;
		Inventories()[gType] = 0;
		DepreciationRate()[gType] = getInputParameter("DepreciationRate");
	}
	prevavgDeprecError2() = 1.0;

	GDPtracker()._GDPcomponents.resize(getSAM().getnPXproducerTypes());
	GDPtracker().base_Month() = 0; getInputParameter("StartCalibrationAt");// / 12. - 1; // always 1 year before current one
	CPItracker()._CPIcomponents.resize(getSAM().getnPXproducerTypes());
	CPItracker().base_Month() = 0; getInputParameter("StartCalibrationAt");// / 12. - 1; // always 1 year before current one
}

void CData::monthInitialize()
{
	TotLoansQtty() = 0;
	TotLoansInterests() = 0.;

	SAM().pSAMmonth()->clear();
	if (getWorld().getpFigaro() == nullptr)
		SAM().pSAMmonth()->resize(getSAM().getnAccounts(),
			vector<double>(getSAM().getnAccounts(), 0.0));
	else
		SAM().pSAMmonth()->resize(getSAM().getnAccounts(),
			vector<double>(getSAM().getnAccounts(), 0.0));

	DataGrossOutput_mu().clear();
	TotalDataGrossOutput_mu() = 0;
	TotalProduction() = 0;
	DataHouseholdsGFCF_mu() = 0;
	DataGovGFCF_mu() = 0;
	TotalDataGov_mu() = 0;
	GDPnominal() = 0;
	TotalUnits() = 0;
	Dismantled().clear();

	VABpE().clear(); // updated in AnalizeLastMonth
	VABpEv().clear();
	VABpEv().resize(5, 0.0);
	VABpEn().clear();
	VABpEn().resize(5, 0.0);

	TotalSalaries() = 0;
	TotalWorkedTime() = 0;

	//	if (currMonth() >= GDPtracker().base_Month() - 1)
	if (currMonth() >= getInputParameter("StartCalibrationAt"))
	{
		GDPtracker()._GDPcomponents.clear();
		for (long gType = 0; gType < getSAM().getnPXproducerTypes(); ++gType)
			GDPtracker()._GDPcomponents.push_back(CGDPcomponent(gType, 0, 0));
	}

	//	if (currMonth() >= GDPtracker().base_Month() - 1)
	if (currMonth() >= getInputParameter("StartCalibrationAt"))
	{
		CPItracker()._CPIcomponents.clear();
		for (long gType = 0; gType < getSAM().getnPXproducerTypes(); ++gType)
			CPItracker()._CPIcomponents.push_back(CCPIcomponent(gType, 0, 0));
	}
}

void CData::AnalizeLastMonth() {
	long CurrentMonth = currMonth();
	long nMonths = getNYears() * getMonthsPerYear();
	long nWorkers = getWorld().getNWorkers();
	auto GFCFtype = getSAM().GFCFtype();
	double upscaleFactor = getUpscaleSimulationFactor();
	double totalPopulationYear = 12. * upscaleFactor;

	// 1. ========================  UPDATE VALUES OF PlotsData SOURCES  ========================

	// ---------------  Market Prices  -----------------------
	for (long gType = 0; gType < getSAM().getnPXproducerTypes(); ++gType)
	{
		if (gType < getSAM().getnPProducerTypes() && getSAM().getSAMGrossOutput_mu(gType) == 0)
			continue; // skip sectors with no production

		MarketPrices()[gType] = 1.0;
		double totValue = 0, totQtty = 0;
		for (const auto& pIndiv : *getWorld().getpWorkers()) {
			double myPrice = pIndiv->getmyPriceOf(gType);
			if (myPrice == 0) continue;
			double qtty = pIndiv->getGoodsIhave(gType);
			totQtty += qtty;
			totValue += qtty * myPrice;
		}

		for (const auto& pProducer : *getWorld().getpProducers()) {
			if (!pProducer) continue;
			double myPrice = pProducer->getmyPriceOf(gType);
			if (myPrice == 0) continue;

			double qtty = pProducer->getGoodsIhave(gType); // purchased now

			totQtty += qtty;
			totValue += qtty * myPrice;
		}

		double myPrice = getpGovernment()->getmyPriceOf(gType);
		if (myPrice > 0) {
			double qtty = getpGovernment()->getGoodsIhave(gType); // not purchased +getpGovernment()->getToSell(gType);
			totQtty += qtty;
			totValue += qtty * myPrice;
		}

		if (totQtty > 0)
			MarketPrices()[gType] = totValue / totQtty;
	}

	// salary

	if (TotalWorkedTime() > 0)
		MarketPrices()[getSAM().getnPXproducerTypes()] = TotalSalaries() / TotalWorkedTime();

	// ---------------  Workers  -----------------------
	sorted_IndivsWealth().clear();

	prevUnemployment() = getUnemployment();
	Unemployment() = 0;
	nPartTimeWorkers() = 0;
	totalPartTimeWork() = 0;
	nFullTimeWorkers() = 0;
	totalFullTimeWork() = 0;
	PartToFullTimeWorkersRatio() = 0;

	TotalIndivsCash() = 0;
	TotalIndivsBank() = 0;

	GoodQtty prevIndivsWealth = getIndivsWealth();
	IndivsWealth() = 0;

	TotalIndivsGoodsAndServ() = 0;
	TotalHouseholdPXConsum().clear();
	DataHouseholdsGFCF_mu() = 0;
	for (const auto& pIndiv : *getWorld().getpWorkers()) {
		auto agentID = pIndiv->getID();
		const CWorker& indiv = *pIndiv;
		auto Wealth = pIndiv->getWealth();
		pIndiv->WealthPrevMonth() = Wealth;
		IndivsWealth() += Wealth;
		TotalIndivsCash() += pIndiv->Cash();
		TotalIndivsBank() += pIndiv->getmyBankBalance();

		if (!pIndiv->getpOwnedBank()) {
			sorted_IndivsWealth().emplace_back(Wealth, agentID, WorkerType);
		}

		Unemployment() += pIndiv->getmyAvailableTime();
		if (pIndiv->getbPartTimeWorker()) {
			++nPartTimeWorkers();
			totalPartTimeWork() += 1.0 - pIndiv->getmyAvailableTime();
		}
		else if (pIndiv->getmyAvailableTime() == 0) {
			++nFullTimeWorkers();
			totalFullTimeWork() += 1.0;
		}

		for (const auto& pair : pIndiv->getGoodsIhave()) // does not include purchased GFCF components
		{
			TotalHouseholdPXConsum()[pair.first] += pair.second * totalPopulationYear;
			TotalIndivsGoodsAndServ() += pair.second * totalPopulationYear;
		}

		DataHouseholdsGFCF_mu() += pIndiv->getmyGFCF();
	}
	DataHouseholdsGFCF_mu() *= totalPopulationYear;

	IndivsWealth() *= upscaleFactor;
	Unemployment() /= nWorkers;
	_UnemploymentHistory.updateHistory(Unemployment());

	double TotalIndivsGoodsAndServSAM = 0; // should not include purchased GFCF components
	for (const auto pAcc : getAccountGroups().at("H"))
	{
		auto Hcol = pAcc->accN();
		for (long gType = 0; gType < getSAM().getnPXproducerTypes(); ++gType)
		{
			//	if (getSAM().getSAMGrossOutput_mu(gType) == 0) // skip sectors with no production
			//		continue;

			TotalIndivsGoodsAndServSAM += getSAM().getRowCol(gType, Hcol);
		}
	}

	TotalDataGov_mu() = 0;
	GovConsumPX().clear();
	for (long gType = 0; gType < getSAM().getnPXproducerTypes(); ++gType) {
		GovConsumPX()[gType] += getGovernment().getGoodsIhave(gType) * totalPopulationYear;
		TotalDataGov_mu() += getGovernment().getGoodsIhave(gType) * totalPopulationYear;
	}
	DataGovGFCF_mu() = getGovernment().getmyGFCF() * totalPopulationYear;

	// ---------------  Loan Interest  -----------------------
	AvgLoansInterests() = (TotLoansQtty() > 0) ? (TotLoansInterests() / TotLoansQtty()) : 0.0;

	// ---- GDP & CPI  ----------------------------

	GDPtracker().addMonthData();
	GDPtracker().printResults();

	CPItracker().addMonthData();
	CPItracker().printResults();

	// ---------------  Producers  -----------------------

	sorted_ProducersWealth().clear();
	CGoods prevInventories = Inventories();
	Inventories().clear();
	ToSell().clear();
	sorted_ProducersWealth().clear();
	_TotalProducersCash = 0;
	GoodQtty prevProducersWealth = _ProducersWealth;
	_ProducersWealth = 0;
	nTotalCurrentProducers() = 0;
	AssistedProd().clear();
	for (long gType = 0; gType < getSAM().getnPProducerTypes(); ++gType) {
		if (getSAM().getSAMGrossOutput_mu(gType) == 0) // skip sectors with no production
			continue;

		AssistedProd()[gType] = 0;
		nActiveWorkers()[gType] = 0;
		if (gType != GFCFtype) {
			nTotalCurrentProducers() += getnCurrentPProducers().at(gType);
		}
	}

	// Initialize maps
	nEmplPerFirmsize().clear();
	nEmplPerFirmsize().insert({ "0-9", 0. });
	nEmplPerFirmsize().insert({ "10-49", 0. });
	nEmplPerFirmsize().insert({ "50-249", 0. });
	nEmplPerFirmsize().insert({ ">=250", 0. });

	nFirmsPerFirmsize().clear();
	nFirmsPerFirmsize().insert({ "0-9", 0. });
	nFirmsPerFirmsize().insert({ "10-49", 0. });
	nFirmsPerFirmsize().insert({ "50-249", 0. });
	nFirmsPerFirmsize().insert({ ">=250", 0. });

	SectorMarkups().clear();

	CGoods totalToSell;
	TotalAssistedProd() = 0;
	for (const auto& pProducer : *getWorld().getpProducers()) {
		if (!pProducer) continue;

		auto agentID = pProducer->getID();
		auto& producer = *pProducer;
		auto aType = producer.getAgentType();

		for (long gType = 0; gType < getSAM().getnPProducerTypes(); ++gType)
		{
			if (getSAM().getSAMGrossOutput_mu(gType) == 0) // skip sectors with no production
				continue;

			double assistedPerYear = pProducer->assistedQttiesOf(gType) * 12. * getUpscaleSimulationFactor(); // per year
			AssistedProd()[gType] += assistedPerYear;
			TotalAssistedProd() += assistedPerYear;
		}

		auto Wealth = pProducer->getWealth();
		pProducer->WealthPrevMonth() = Wealth;
		_ProducersWealth += Wealth;
		_TotalProducersCash += pProducer->Cash();

		if (aType != getSAM().getAccNofName("CentralBank") && aType != getSAM().getAccNofName("PrivateBanks")) {
			sorted_ProducersWealth().emplace_back(Wealth, agentID, aType);
		}

		double firmSize = pProducer->gettimeWorked();
		nActiveWorkers()[aType] += firmSize;

		if (firmSize >= 249.5) {
			nEmplPerFirmsize()[">=250"] += firmSize;
			nFirmsPerFirmsize()[">=250"] += 1.;
		}
		else if (firmSize >= 49.5) {
			nEmplPerFirmsize()["50-249"] += firmSize;
			nFirmsPerFirmsize()["50-249"] += 1.;
		}
		else if (firmSize >= 9.5) {
			nEmplPerFirmsize()["10-49"] += firmSize;
			nFirmsPerFirmsize()["10-49"] += 1.;
		}
		else {
			nEmplPerFirmsize()["0-9"] += firmSize;
			nFirmsPerFirmsize()["0-9"] += 1.;
		}

		Inventories() += pProducer->getInventory();
		ToSell() += pProducer->getToSell();

		double myToSell = pProducer->getToSell().at(aType);
		if (myToSell > 0 && currMonth() > getInputParameter("StartCalibrationAt")
			) {
			double myMarkup = pProducer->getmyPriceOf(aType) - pProducer->productionPrice();
			SectorMarkups()[aType] += myMarkup * myToSell;
			totalToSell[aType] += myToSell;
		}
	}

	MA_TotAssistedProd().update(TotalAssistedProd());

	for (long aType = 0; aType < getSAM().getnPProducerTypes(); ++aType)
	{
		if (getSAM().getSAMGrossOutput_mu(aType) == 0) // skip sectors with no production
			continue;

		if (totalToSell(aType))
			SectorMarkups()[aType] /= (double)totalToSell(aType);
		else
			SectorMarkups()[aType] = 0;
	}

	avgFirmBirths() = 0;
	avgFirmDeaths() = 0;
	long firstMonth = max(0L, currMonth() - 11);
	for (long month = firstMonth; month <= currMonth(); ++month) {
		avgFirmBirths() += FirmBirths()[month % FirmBirths().size()];
		avgFirmDeaths() += FirmDeaths()[month % FirmDeaths().size()];
	}
	avgFirmBirths() *= upscaleFactor / (currMonth() - firstMonth + 1); // annualized avgFirmBirths
	avgFirmDeaths() *= upscaleFactor / (currMonth() - firstMonth + 1); // annualized avgFirmDeaths

	double totalSalaries = (1.0 - Unemployment()) * nWorkers;
	if (getnFullTimeWorkers() > 0) {
		double nTotWorkers = getnPartTimeWorkers() + getnFullTimeWorkers();
		PartToFullTimeWorkersRatio() = getnPartTimeWorkers() / nTotWorkers;
	}
	else {
		PartToFullTimeWorkersRatio() = 1;
	}

	_dProducersWealth = _ProducersWealth - prevProducersWealth;

	// 2. ========================  PlotsData() curves  ========================
	for (const auto& pIndiv : *getWorld().getpWorkers()) {
		if (pIndiv->getID() == getInputParameter("PlotIndivID")) {
			if (PlotsData().find("WorkerID") != PlotsData().end()) {
				for (int curveN = 0; curveN < PlotsData()["WorkerID"].size(); ++curveN) {
					PlotsData()["WorkerID"][curveN][currMonth()] = PlotsDataSources("WorkerID", curveN, pIndiv);
				}
			}
		}

		if (PlotsData().find("IndivsValues") != PlotsData().end()) {
			for (int curve = 0; curve < PlotsData()["IndivsValues"].size(); ++curve) {
				PlotsData()["IndivsValues"][curve][currMonth()] += PlotsDataSources("IndivsValues", curve, pIndiv);
			}
		}
	}

	for (const auto& pProducer : *getWorld().getpProducers()) {
		if (!pProducer) continue;

		auto agentID = pProducer->getID();

		if (PlotsData().find("LeftToSell") != PlotsData().end()) {
			for (GoodType gType = 0; gType < PlotsData()["LeftToSell"].size(); ++gType) {
				PlotsData()["LeftToSell"][gType][currMonth()] += PlotsDataSources("LeftToSell", gType, pProducer);
			}
		}

		if (PlotsData().find("ProdsValues") != PlotsData().end()) {
			for (int curve = 0; curve < PlotsData()["ProdsValues"].size(); ++curve) {
				PlotsData()["ProdsValues"][curve][currMonth()] += PlotsDataSources("ProdsValues", curve, pProducer);
			}
		}

		if (PlotsData().find("ProducerID") != PlotsData().end()) {
			if (agentID == getInputParameter("PlotProducerID")) {
				for (int curveN = 0; curveN < PlotsData()["ProducerID"].size(); ++curveN) {
					PlotsData()["ProducerID"][curveN][currMonth()] = PlotsDataSources("ProducerID", curveN, pProducer);
				}
			}
		}
	}

	Exports().clear();
	if (PlotsData().find("Exports") != PlotsData().end()) {
		auto& pExtSect = getExtSectors().at(getSAM().getAccountGroups().at("X").at(getInputParameter("PlotExtSectIndex"))->accN());
		for (GoodType gType = 0; gType < getSAM().getnPProducerTypes(); ++gType) {
			if (getSAM().getSAMGrossOutput_mu(gType) == 0) // skip sectors with no production
				continue;

			Exports()[gType] = pExtSect->getExports().at(gType);
		}
	}

	Imports().clear();
	if (PlotsData().find("Imports") != PlotsData().end()) {
		auto& pExtSect = getExtSectors().at(getSAM().getAccountGroups().at("X").at(getInputParameter("PlotExtSectIndex"))->accN());
		for (GoodType gType = 0; gType < getSAM().getnPProducerTypes(); ++gType) {
			if (getSAM().getSAMGrossOutput_mu(gType) == 0) // skip sectors with no production
				continue;

			Imports()[gType] = pExtSect->getImports().at(gType);
		}
	}

	auto WinTitles = {
		"ConsumHouseholds", "ConsumGov", "Exports", "Imports", "Supply-Demand",
		"Production", "GrossOutput", "Supply", "Demand",
		"Employment", "EmployeesPerSect", "ProducersPerSect", "ProducersAvgSize",
		"SectorMarkups", "EmplPerFirmsize", "FirmsPerFirmsize", "FirmsBirth&Death",
		"CentralBank", "Bank_0", "MarketPrices", "GovValues", "Indices_and_%",
		"AssistedProd", "ConsumPXFactor"
	};

	for (auto plotw : WinTitles) {
		if (PlotsData().find(plotw) != PlotsData().end()) {
			for (GoodType gType = 0; gType < PlotsData()[plotw].size(); ++gType)
			{
				if (gType < getSAM().getnPProducerTypes() && getSAM().getSAMGrossOutput_mu(gType) == 0)
					continue; // skip sectors with no production

				if (!getSAM().IsExtSectType(gType) || getInputParameter("PlotExtSectIndex")
					== getSAM().getAccount(gType).getaccGroupIndex())
				{
					PlotsData()[plotw][gType][currMonth()] = PlotsDataSources(plotw, gType);
				}
			}
		}
	}

	// ====================  Sorted Values  ======================
	if (getInputParameter("SortIncreasingWealth")) {
		std::sort(sorted_IndivsWealth().begin(), sorted_IndivsWealth().end(), less<SortedAgent>());
	}
	else {
		std::sort(sorted_IndivsWealth().begin(), sorted_IndivsWealth().end(), greater<SortedAgent>());
	}

	long maxNToPlot = 200;
	auto toPlot = vector<SortedAgent>();
	auto nDat = sorted_IndivsWealth().size();
	for (long n = 0; n < nDat; ++n) {
		if (nDat <= maxNToPlot || n % (long)(nDat / maxNToPlot) == 0) {
			toPlot.push_back(sorted_IndivsWealth()[n]);
		}
	}

	sorted_IndivsWealth().resize(toPlot.size());
	for (long n = 0; n < sorted_IndivsWealth().size(); ++n) {
		sorted_IndivsWealth()[n] = toPlot[n];
	}

	if (getInputParameter("SortIncreasingWealth")) {
		std::sort(sorted_ProducersWealth().begin(), sorted_ProducersWealth().end(), less<SortedAgent>());
	}
	else {
		std::sort(sorted_ProducersWealth().begin(), sorted_ProducersWealth().end(), greater<SortedAgent>());
	}

	toPlot.clear();
	nDat = sorted_ProducersWealth().size();
	for (long n = 0; n < nDat; ++n)
		if (nDat <= maxNToPlot || n % (long)(nDat / maxNToPlot) == 0)
			toPlot.push_back(sorted_ProducersWealth()[n]);

	sorted_ProducersWealth().resize(toPlot.size());
	for (long n = 0; n < sorted_ProducersWealth().size(); ++n)
		sorted_ProducersWealth()[n] = toPlot[n];

	// ---------------------------------------------------------------------

	// Fill out SAMmonth:

	if (getWorld().getpFigaro() == nullptr)
	{
		double TaxProductsSAM = getSAM().getRowCol("TaxProducts", "Households");
		double TaxProductsSimul = TotalIndivsGoodsAndServ() * TaxProductsSAM / TotalIndivsGoodsAndServSAM;
		SAM().SAMmonthAt("TaxProducts", "Households") = TaxProductsSimul;
	}
}

double CData::PlotsDataSources(string plotWinName, GoodType gType, void* ptr) // define data sources here
{
	double value = 0;
	auto GFCFtype = getSAM().GFCFtype();

	if (plotWinName == "LeftToSell")
	{
		value = ((CProducer*)ptr)->getToSell(gType) * toTotalPopulationYear();

		if (getInputParameter("LogPlots") == 0)
			value *= 1.e-9; // kMmu
		else
		{
			if (value > 0)
				value = log10(value);
			else
				value = 0.;
		}
	}
	else if (plotWinName == "ConsumHouseholds")
	{
		if (gType <= getSAM().getnPXproducerTypes()) // = is GFCF flow
			value = getTotalHouseholdPXConsum().at(gType);

		if (getInputParameter("LogPlots") == 0)
			value *= 1.e-9; // kMmu
		else
		{
			if (value > 0)
				value = log10(value);
			else
				value = 0.;
		}
	}
	else if (plotWinName == "ConsumGov")
	{
		if (gType <= getSAM().getnPXproducerTypes()) // = is GFCF flow
			value = getGovernment().getGoodsIhave(gType) * toTotalPopulationYear();
		/*		else if (gType == getSAM().getnPXproducerTypes())
					value = getTotalDataGov_mu();
				else
				{
					assert(gType == getSAM().GFCFtype());
					value = getGovernment().getGoodsIhave(gType) * toTotalPopulationYear();
				}
		*/
		if (getInputParameter("LogPlots") == 0)
			value *= 1.e-9; // kMmu
		else
		{
			if (value > 0)
				value = log10(value);
			else
				value = 0.;
		}
	}
	else if (plotWinName == "Exports")
	{
		value = Exports().at(gType);

		if (getInputParameter("LogPlots") == 0)
			value *= 1.e-9; // kMmu
		else
		{
			if (value > 0)
				value = log10(value);
			else
				value = 0.;
		}
	}
	else if (plotWinName == "IndivsValues")
	{
		CWorker* pIndiv = (CWorker*)ptr;
		switch (gType) {
		case 0:	// labels will be listed in alphabetic order:
			value = pIndiv->getmyBankBalance();
			break;
		case 1:
			value = pIndiv->getmyTotBondsValue();
			break;
		case 2:
			value = pIndiv->getCash();
			break;
		case 3:
			value = pIndiv->getmyTotSharesValue();
			break;
		case 4:
			value = pIndiv->getWealth();
			break;
		case 5:
			value = pIndiv->getmyGFCF();
			break;
		default:
			value = 1.e18;
			break;
		}

		value *= getUpscaleSimulationFactor() * 1.e-9;
	}
	else if (plotWinName == "ProdsValues")
	{
		CProducer* pProducer = (CProducer*)ptr;
		switch (gType) {
		case 0:	// labels will be listed in alphabetic order:
			value = pProducer->getmyBankBalance();
			break;
		case 1:
			value = pProducer->getCash();
			break;
		case 2:
			value = pProducer->getWealth();
			break;
		case 3:
			value = pProducer->valGoodsIhave();
			break;
		case 4:
			value = pProducer->valToSell();
			break;
		case 5:
			value = pProducer->getmyFixCapital();
			break;
		default:
			value = 1.e18;
			break;
		}

		value *= getUpscaleSimulationFactor() * 1.e-9;
	}
	else if (plotWinName == "GovValues")
	{
		switch (gType) {
		case 0:
			value = getGovernment().getmyBankBalance() * getUpscaleSimulationFactor();
			break;
		case 1:
			value = getGovernment().getmyTotBondsValue() * getUpscaleSimulationFactor();
			break;
		case 2:
			value = getGovernment().getCash() * getUpscaleSimulationFactor();
			break;
		case 3:
			value = getGovernment().getWealth() * getUpscaleSimulationFactor();
			break;
		case 4:
			value = getGDPnominal();
			break;
		case 5:
			value = getTotalUnits();
			break;
		case 6:
			value = getGDPtracker().getnominal_gdp();
			break;
		case 7:
			value = getGDPtracker().getreal_gdp();
			break;
		default:
			value = 1.0e18;
			break;
		}

		value *= 1.e-9;
	}

	else if (plotWinName == "CentralBank")
	{
		switch (gType) {
		case 0:
			value = getCentralBank().getCash();
			break;
		case 1:
			value = getGovernment().getmyBankBalance();
			break;
		case 2:
			value = getCentralBank().ExcessLiquidityBudget();
			break;
			//Central Bank Data()["RiskBudget"] =
			//	getCentralBank().getRiskExposureBudget() * getUpscaleSimulationFactor() * 1.e-6;
		case 3:
			value = getCentralBank().getClientsDeposits();
			break;
		case 4:
			value = getCentralBank().getClientsLoans();
			break;
		case 5:
			value = getCentralBank().getCash_I_Own();
			break;
		default:
			value = 1.0e18;
			break;
		}

		value *= getUpscaleSimulationFactor() * 1.e-9;
	}
	else if (plotWinName == "Imports")
		value = Imports().at(gType) * 1.e-9;// kMmu per year

	else if (plotWinName == "Bank_0")
	{
		if (getCentralBank().getBanks().size() == 0)
			value = 0;
		else
		{
			const auto& Bank = *getCentralBank().getBanks().begin()->second;
			switch (gType) {
			case 0:
				value = Bank.getmyBankBalance();
				break;
			case 1:
				value = Bank.getCash();
				break;
			case 2:
				value = Bank.ExcessLiquidityBudget();
				break;
			case 3:
				value = Bank.getRiskExposureBudget();
				break;
			case 4:
				value = Bank.getWealth();
				break;
			case 5:
				value = Bank.getClientsDeposits();
				break;
			case 6:
				value = Bank.getClientsLoans();
				break;
			case 7:
				value = Bank.getCash_I_Own();
				break;
			default:
				value = 1.0e18;
				break;
			}
		}

		value *= getUpscaleSimulationFactor() * 1.e-9;
	}
	else if (plotWinName == "WorkerID")
	{
		CWorker* pIndiv = (CWorker*)ptr;
		switch (gType) {
		case 0:
			value = pIndiv->getmyBankBalance();
			break;
		case 1:
			value = pIndiv->getmyTotBondsValue();
			break;
		case 2:
			value = pIndiv->getCash();
			break;
		case 3:
			value = pIndiv->getmyTotSharesValue();
			break;
		case 4:
			value = pIndiv->getWealth();
			break;
		case 5:
			value = pIndiv->getmyGFCF();
			break;
		default:
			value = 1.0e18;
			break;
		}

		value *= 1.e-3;
	}
	else if (plotWinName == "ProducerID")
	{
		CProducer* pProducer = (CProducer*)ptr;
		GoodType productType = pProducer->getAgentType();

		switch (gType) {
		case 0:// "AssistProd"
			if (pProducer->getassistedQtties().size() > 0)
			{
				value = 0;
				for (const auto& pair : pProducer->getassistedQtties())
					value += pair.second;
			}
			break;
		case 1:// "AvgLeftToSell"
			value = pProducer->getAvgLeftToSell();
			break;
		case 2:// "AvgMyDemand"
			value = pProducer->getavgmyDemand();
			break;
		case 3:// "AvgMySupply"
			value = pProducer->getAvgmySupply();
			break;
		case 4:// "AvgProduction"
			value = pProducer->getAvgProduction();
			break;
		case 5:// "BankBalance"
			value = pProducer->getmyBankBalance();
			break;
		case 6:// "Cash"
			value = pProducer->getCash();
			break;
		case 7:// "Employees.e5"
			value = 1.e5 * pProducer->getEmployees().size();
			break;
		case 8:// "FixCapital"
			value = pProducer->getmyFixCapital();
			break;
		case 9:// "IC"
			value = pProducer->valGoodsIhave();
			break;
		case 10:// "LeftToSell"
			value = pProducer->LeftToSell();
			break;
		case 11:// "MarkupFact.e5"
			value = 1.e5 * pProducer->getmyMarkupFactor();
			break;
		case 12:// "MyDemand"
			value = pProducer->getprevmyDemand();
			break;
		case 13:// "mySupply"
			value = pProducer->getmySupply();
			break;
		case 14:// "myGOSfactor.e5"
			value = 1.e5 * pProducer->getmyGOSfactor();
			break;
		case 15:// "ProdType.e5"
			value = 1.e5 * pProducer->getAgentType();
			break;
		case 16:// "Produced"
			value = pProducer->getproducedUnitsOf(productType);
			break;
		case 17:// "StockRef"
			value = pProducer->getStockReference();
			break;
		case 18:// "ToBeProd"
			value = pProducer->getprevToBeProduced();
			break;
		case 19:// "Wealth"
			value = pProducer->getWealth();
			break;
		default:
			value = 1.0e18;
			break;
		}

		value *= 1.e-3;
	}

	else if (plotWinName == "Employment")
	{
		switch (gType) {
		case 0:
			//value = 100. * (double)getnFullTimeWorkers() / getWorld().getNWorkers();
			value = 100. * gettotalFullTimeWork() / getWorld().getNWorkers();
			break;
		case 1:
			value = 100. * getUnemployment();
			//	value = 100. * getAvgUnemployment();
			break;
		case 2:
			//value = 100. * (double)getnPartTimeWorkers() / getWorld().getNWorkers();
			value = 100. * gettotalPartTimeWork() / getWorld().getNWorkers();
			break;
		default:
			value = 1.0e18;
			break;
		}
	}
	else if (plotWinName == "EmployeesPerSect")
		value = getnActiveWorkersOfType(gType) * getUpscaleSimulationFactor();
	else if (plotWinName == "ProducersPerSect")
	{
		if (gType >= getnCurrentPProducers().size())
		{
			value = 0;
			for (int ty = 0; ty < getnCurrentPProducers().size(); ++ty)
				value += getnCurrentPProducers().at(ty);

			value *= getUpscaleSimulationFactor();
		}
		else if (getnTotalCurrentProducers() == 0
			|| gType == GFCFtype || getSAM().IsExtSectType(gType))
			value = 0;
		else
			//perCent value = 100. * getnCurrentPProducers().at(gType) / getnTotalCurrentProducers();
			value = getnCurrentPProducers().at(gType) * getUpscaleSimulationFactor();
	}
	else if (plotWinName == "ProducersAvgSize")
	{
		if (gType == GFCFtype || getnCurrentPProducers().at(gType) == 0)
			value = 0;
		else
			value = (double)getnActiveWorkersOfType(gType) / getnCurrentPProducers().at(gType);
	}
	else if (plotWinName == "SectorMarkups")
	{
		value = 100. * (double)getSectorMarkups().at(gType);
	}
	else if (plotWinName == "EmplPerFirmsize")
	{
		int n = -1;
		for (const auto& p : DEPData().nEmplPerFirmsize())
		{
			if (++n == gType)
			{
				value = p.second * getUpscaleSimulationFactor();

				break;
			}
		}
	}
	else if (plotWinName == "FirmsBirth&Death")
	{
		if (currMonth() < 12)
			return 0;

		value = 1.0e18;
		switch (gType) {
		case 0:
			value = DEPData().avgFirmBirths();
			break;
		case 1:
			value = DEPData().avgFirmDeaths();
			break;
		default:
			break;
		}
	}
	else if (plotWinName == "FirmsPerFirmsize")
	{
		int n = -1;
		for (const auto& p : DEPData().nFirmsPerFirmsize())
		{
			if (++n == gType)
			{
				if (getnTotalCurrentProducers() == 0)
					value = 0;
				else
					value = 100. * p.second / getnTotalCurrentProducers();

				break;
			}
		}
	}

	else if (plotWinName == "Indices_and_%")
	{
		value = 1.0e18;
		switch (gType) {
		case 0: // %GDP_YoY_Inflation - Year-over-year GDP deflator inflation
			value = 0.0;
			if (getGDPtracker()._gdp_deflator.size() >= 13) {
				// Calculate year-over-year inflation rate from GDP deflator
				// This works similar to CPI year-over-year calculation
				double current_deflator = getGDPtracker()._gdp_deflator.back();
				double year_ago_deflator = getGDPtracker()._gdp_deflator[getGDPtracker()._gdp_deflator.size() - 13];

				if (year_ago_deflator > 0.0) {
					value = 100.0 * (current_deflator - year_ago_deflator) / year_ago_deflator;
				}
			}
			break;
		case 1: // %CPIinflation - Year-over-year inflation rate
			value = 0.0;
			if (getCPItracker().getCPI().size() >= 13) {
				// Calculate year-over-year inflation rate
				// This works for BOTH rolling and fixed base modes
				double current_cpi = getCPItracker().getCPI().back();
				double year_ago_cpi = getCPItracker().getCPI()[getCPItracker().getCPI().size() - 13];

				if (year_ago_cpi > 0.0) {
					value = 100.0 * (current_cpi - year_ago_cpi) / year_ago_cpi;
				}
			}
			break;
		case 2: // %AvgInterestRate - Annualized average loan interest rate
			value = 12 * 100.0 * AvgLoansInterests();
			break;
			case 3: // %AvgRealInterestRate - Real interest rate (nominal - inflation)  
				{
					double nominal_rate = AvgLoansInterests();  // Monthly rate
					double inflation_rate = getCPItracker().getSmoothCPIInflationRate(); // Annual rate
					double monthly_inflation_rate = inflation_rate / 12.0; // Convert to monthly
					double real_rate = nominal_rate - monthly_inflation_rate;
					// Convert to annualized percentage  
					value = 12 * 100.0 * real_rate;
				}
				break;
		case 4: // InitSalaryCalibFactor
			value = getWorld().getInitSalaryCalibFactor();
			break;
		default:
			break;
		}
	}

	else if (plotWinName == "MarketPrices")
	{
		if (gType < getSAM().getnPXproducerTypes()) {
			// Sector prices
			value = 100.0 * MarketPrices().at(gType);
		}
		else if (gType == getSAM().getnPXproducerTypes()) {
			// Salary
			value = 100.0 * MarketPrices().at(gType);
		}
		else if (gType == getSAM().getnPXproducerTypes() + 1) {
			// %CPIfromBase - Cumulative inflation from base month (when BaseMonth >= 0)
			value = 100.0; // Default value when no CPI data
			if (getCPItracker().getCPI().size() > 0) {
				int baseMonthParam = static_cast<int>(getInputParameter("BaseMonth"));
				if (baseMonthParam >= 0) {
					// Fixed base month: show cumulative inflation from base month
					// CPI is already calculated relative to base (base = 100)
					// So we just show the CPI value directly
					value = getCPItracker().getCPI().back();
				}
				else {
					// Rolling base: show current CPI value
					value = getCPItracker().getCPI().back();
				}
			}
		}
	}
	else if (plotWinName == "GrossOutput")
	{
		if (gType < getSAM().getnPProducerTypes())
			value = getDataGrossOutput_mu()(gType) * 1.e-9;
		else
			value = getTotalDataGrossOutput_mu() * 1.e-9;
	}
	else if (plotWinName == "Supply-Demand")
		value = (TotSupply()(gType) - TotDemand()(gType)) * 12. // per year
		* getUpscaleSimulationFactor() * 1.e-9;
	else if (plotWinName == "Supply")
		value = TotSupply()(gType) * 12. // per year
		* getUpscaleSimulationFactor() * 1.e-9;
	else if (plotWinName == "Demand")
		value = TotDemand()(gType) * 12. // per year
		* getUpscaleSimulationFactor() * 1.e-9;
	else if (plotWinName == "Production")
	{
		if (gType < getSAM().getnPProducerTypes())
			value = (TotProduced()(gType));
		else if (gType == getSAM().getnPProducerTypes())
			value = getTotalProduction();

		if (getInputParameter("LogPlots") == 0)
			value *= 1.e-9; // kMmu
		else
		{
			if (value > 0)
				value = log10(value);
			else
				value = 0.;
		}
	}
	else if (plotWinName == "AssistedProd")
	{
		if (gType < getSAM().getnPProducerTypes())
			value = AssistedProd().at(gType);
		else if (gType == getSAM().getnPProducerTypes())
			//value = getTotalAssistedProd();
			value = getDEPData().getMA_TotAssistedProd().getCurrentAverage();

		if (getInputParameter("LogPlots") == 0)
			value *= 1.e-9; // kMmu
		else
		{
			if (value > 0)
				value = log10(value);
			else
				value = 0.;
		}
	}
	else if (plotWinName == "ConsumPXFactor")
		value = getWorld().getConsumPXFactor().at(gType);

	return value;
}

void CData::initializePlotsInputParameters() const
{
	// =============  Specific parameters (cuve names, axis titles, formats...)  ===================

	// List of cuve names, axis titles, formats, ...

	for (auto& pair : DEPData().PlotsInputParameters())
	{
		CPlotDefinition& plotDefinition = pair.second;

		// Default number of points (Xaxis) = 1 + currMonth

		plotDefinition.UsecurrMonthNPoints = true;

		// Default list of curves, labels will be listed in alphabetic order  --------------------

		vector<string>* pSectorLabels = new vector<string>;
		for (const auto& pAcc : getSAM().getAccountGroups().at("P"))
			pSectorLabels->push_back(pAcc->label());

		plotDefinition.pLabel_array = pSectorLabels;

		// Specific cases  -------------------------------------------------------

		vector<string>* pLblArray = new vector<string>;
		if (plotDefinition.windowTitle == "IndivsValues")
		{
			delete pLblArray;
			// labels will be listed in alphabetic order:
			pLblArray = new vector<string>({ "BankBalance", "Bonds",
				"Cash", "Shares", "Wealth", "myGFCF" });

			plotDefinition.pLabel_array = pLblArray;
		}
		else if (plotDefinition.windowTitle == "ProdsValues")
		{
			delete pLblArray;
			// labels will be listed in alphabetic order:
			pLblArray = new vector<string>({ "BankBalance", "Cash",
				"Wealth", "gIC", "gToSell", "myFixCapit" });

			plotDefinition.pLabel_array = pLblArray;
		}
		else if (plotDefinition.windowTitle == "GovValues")
		{
			delete pLblArray;
			// labels will be listed in alphabetic order:
			pLblArray = new vector<string>({ "BankBalance", "BondsSold",
				"Cash", "Wealth", "GDPnominal", "TotalUnits", "nominalGDP", "realGDP" });

			plotDefinition.pLabel_array = pLblArray;
		}

		else if (plotDefinition.windowTitle == "CentralBank")
		{
			delete pLblArray;
			// labels will be listed in alphabetic order:
			pLblArray = new vector<string>({ "Cash", "GovBalance", "MaxLoanQtty",
				"clientDepos", "clientLoans", "interests" });

			plotDefinition.pLabel_array = pLblArray;
		}

		else if (plotDefinition.windowTitle == "Bank_0")
		{
			delete pLblArray;
			// labels will be listed in alphabetic order:
			pLblArray = new vector<string>({ "BankBalance", "Cash", "MaxLoanQtty",
				"RiskBudget", "Wealth", "clientDepos", "clientLoans", "interests" });

			plotDefinition.pLabel_array = pLblArray;
		}
		else if (plotDefinition.windowTitle == "WorkerID")
		{
			plotDefinition.YLabel = "k mu";
			AgentID ID = getInputParameter("PlotIndivID");
			plotDefinition.plotTitle = "Worker_" + to_string(ID);

			delete pLblArray;
			// labels will be listed in alphabetic order:
			pLblArray = new vector<string>({ "BankBalance", "Bonds", "Cash",
				"SharesVal", "Wealth", "myGFCF" });

			plotDefinition.pLabel_array = pLblArray;
		}
		else if (plotDefinition.windowTitle == "ProducerID")
		{
			plotDefinition.YLabel = "k mu";
			AgentID ID = getInputParameter("PlotProducerID");
			plotDefinition.plotTitle = "Producer_" + to_string(ID);

			delete pLblArray;
			// labels will be listed in alphabetic order:
			pLblArray = new vector<string>({ "AssistProd", "AvgLeftToSell",
				"AvgMyDemand", "AvgMySupply", "AvgProduction", "BankBalance", "Cash",
				"Employees.e5", "FixCapital", "IC", "LeftToSell", "MarkupFact.e5",
				"MyDemand", "MySupply", "myGOSfactor", "ProdType.e5", "Produced",
				"StockRef", "ToBeProd", "Wealth" });

			plotDefinition.pLabel_array = pLblArray;
		}

		else if (plotDefinition.windowTitle == "Employment")
		{
			plotDefinition.YLabel = "%";
			plotDefinition.YHigh = 100.;

			delete pLblArray;
			// labels will be listed in alphabetic order:
			//pLblArray = new vector<string>({ "nFullTime", "nPart/nFull", "nPartTime", "Unempl" });
			pLblArray = new vector<string>({ "FullTime", "AvgUnempl", "PartTime" });

			plotDefinition.pLabel_array = pLblArray;
		}
		else if (plotDefinition.windowTitle == "EmployeesPerSect")
		{
			plotDefinition.YLabel = "n";
		}
		else if (plotDefinition.windowTitle == "ProducersPerSect")
		{
			pLblArray->clear();
			for (GoodType gType = 0; gType < getSAM().getnPProducerTypes(); ++gType)
				pLblArray->push_back(getSAM().getAccount(gType).label());

			pLblArray->push_back("TotalActiveProducers");
			plotDefinition.pLabel_array = pLblArray;

			plotDefinition.YLabel = "n";
		}
		else if (plotDefinition.windowTitle == "GrossOutput")
		{
			pLblArray->clear();
			for (GoodType gType = 0; gType < getSAM().getnPProducerTypes(); ++gType)
				pLblArray->push_back(getSAM().getAccount(gType).label());

			pLblArray->push_back("TotalGrossOutput");
			plotDefinition.pLabel_array = pLblArray;
		}
		else if (plotDefinition.windowTitle == "ConsumHouseholds")
		{
			pLblArray->clear();

			for (GoodType gType = 0; gType < getSAM().getnPProducerTypes(); ++gType)
				pLblArray->push_back(getSAM().getAccount(gType).label());

			pLblArray->push_back("Imports_Households");
			pLblArray->push_back("dGFCF_Households");

			plotDefinition.pLabel_array = pLblArray;
		}
		else if (plotDefinition.windowTitle == "ConsumGov")
		{
			pLblArray->clear();

			for (GoodType gType = 0; gType < getSAM().getnPProducerTypes(); ++gType)
				pLblArray->push_back(getSAM().getAccount(gType).label());

			pLblArray->push_back("ImportsGov");
			pLblArray->push_back("dGFCFGov");

			plotDefinition.pLabel_array = pLblArray;
		}
		else if (plotDefinition.windowTitle == "ProducersAvgSize")
		{
			plotDefinition.YLabel = "nEmployees";
		}
		else if (plotDefinition.windowTitle == "SectorMarkups")
		{
			plotDefinition.YLabel = "%";
			plotDefinition.YAxisLabelFormat = "%.2lf";
		}
		else if (plotDefinition.windowTitle == "EmplPerFirmsize")
		{
			delete pLblArray;
			// labels will be listed in alphabetic order:
			pLblArray = new vector<string>({ "0-9", "10-49", "50-249", ">=250" });

			plotDefinition.pLabel_array = pLblArray;

			plotDefinition.YLabel = "n";
			plotDefinition.YHigh = 1.;
		}
		else if (plotDefinition.windowTitle == "FirmsBirth&Death")
		{
			delete pLblArray;
			// labels will be listed in alphabetic order:
			pLblArray = new vector<string>({ "Births/Year", "Deaths/Year" });

			plotDefinition.pLabel_array = pLblArray;

			plotDefinition.YLabel = "n";
			plotDefinition.YHigh = 1.;
		}
		else if (plotDefinition.windowTitle == "FirmsPerFirmsize")
		{
			delete pLblArray;
			// labels will be listed in alphabetic order:
			pLblArray = new vector<string>({ "0-9", "10-49", "50-249", ">=250" });

			plotDefinition.pLabel_array = pLblArray;

			plotDefinition.YLabel = "%";
			plotDefinition.YHigh = 1.;
		}
		else if (plotDefinition.windowTitle == "Indices_and_%")
		{
			plotDefinition.YAxisLabelFormat = "%.3lf";
			plotDefinition.YAxisTooltipFormat = "%.3lf";
			plotDefinition.YLabel = "%";
			plotDefinition.YHigh = 2.;

			delete pLblArray;
			pLblArray = new vector<string>({
				"%GDPInflRate", "%CPIinflation", "%AvgInterest", "%AvgRealInterest",
				"InitSalaryCalibFactor" });

			plotDefinition.pLabel_array = pLblArray;
		}
		else if (plotDefinition.windowTitle == "IndivsWealth")
		{
			plotDefinition.UsecurrMonthNPoints = false;
			plotDefinition.YAxisLabelFormat = "%.2lf";
			plotDefinition.YAxisTooltipFormat = "%.2lf";
			plotDefinition.YLabel = "Mmu";
			plotDefinition.YHigh = 1.;
			plotDefinition.XLabel = "index";

			pLblArray->clear();
			for (const auto& acc : getSAM().getAccountGroups().at("H"))
				pLblArray->push_back(acc->label());

			plotDefinition.pLabel_array = pLblArray;
		}
		else if (plotDefinition.windowTitle == "ProducersWealth")
		{
			plotDefinition.UsecurrMonthNPoints = false;
			plotDefinition.YAxisLabelFormat = "%.2lf";
			plotDefinition.YAxisTooltipFormat = "%.2lf";
			plotDefinition.YLabel = "Mmu";
			plotDefinition.YHigh = 1.;
			plotDefinition.XLabel = "index";
		}
		else if (plotDefinition.windowTitle == "MarketPrices")
		{
			pLblArray->clear();
			for (GoodType gType = 0; gType < getSAM().getnPXproducerTypes(); ++gType)
				pLblArray->push_back(getSAM().getAccount(gType).label());

			pLblArray->push_back("salary");
			pLblArray->push_back("%CPIfromBase");

			plotDefinition.pLabel_array = pLblArray;

			plotDefinition.YLabel = "%";
			plotDefinition.YLow = 0.9;
			plotDefinition.YHigh = 1.1;
			plotDefinition.YAxisLabelFormat = "%.1lf";

			plotDefinition.YAxisTooltipFormat = "<axis_value:%.3lf>";
		}
		else if (plotDefinition.windowTitle == "AssistedProd")
		{
			pLblArray->clear();

			for (GoodType gType = 0; gType < getSAM().getnPProducerTypes(); ++gType)
				pLblArray->push_back(getSAM().getAccount(gType).label());

			pLblArray->push_back("MA_TotAssistedProd");

			plotDefinition.pLabel_array = pLblArray;
		}
		else if (plotDefinition.windowTitle == "Production")
		{
			pLblArray->clear();

			for (GoodType gType = 0; gType < getSAM().getnPProducerTypes(); ++gType)
				pLblArray->push_back(getSAM().getAccount(gType).label());

			pLblArray->push_back("TotalProduction");

			plotDefinition.pLabel_array = pLblArray;
		}
		else if (plotDefinition.windowTitle == "ConsumPXFactor")
		{
			plotDefinition.YLabel = "Factor";
			plotDefinition.YAxisLabelFormat = "%.2lf";
		}
	}

	DEPData().PlotsData().clear();
	set<string> specialWindows = { "Control" };
	for (const auto& pair : DEPData().PlotsInputParameters())
	{
		const auto& windowTitle = pair.first;
		if (specialWindows.find(windowTitle) != specialWindows.end())
			continue;

		const auto& plotDefinition = pair.second;
		for (GoodType curveN = 0; curveN < plotDefinition.pLabel_array->size(); ++curveN)
		{
			DEPData().PlotsData()[windowTitle][curveN].clear();
			DEPData().PlotsData()[windowTitle][curveN].resize(InitMonth() + getMonthsPerYear() * (1 + getNYears()), 0);
		}
	}
}

void CData::ResizePlotsDataForNewNYears(double newNYears)
{
	// Resize all plot data vectors when NYears is dynamically extended
	long newSize = InitMonth() + getMonthsPerYear() * (1 + (long)newNYears);
	
	for (auto& windowPair : PlotsData())
	{
		for (auto& curvePair : windowPair.second)
		{
			if (curvePair.second.size() < newSize)
			{
				curvePair.second.resize(newSize, 0);
			}
		}
	}
}
