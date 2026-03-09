
//  SAM.cpp

//   DEPLOYERS v2

#include "pch.h"

CSAM*& pSAM() { return pWorld()->_pSAM; };
CSAM& SAM() { return *pWorld()->_pSAM; };
const CSAM* getpSAM() { return pWorld()->_pSAM; };
const CSAM& getSAM() { return *pWorld()->_pSAM; };

CAccount::CAccount(int nAccounts)
{
	_accN = -1;
	_accGroupIndex = -1;
	_rowQtties.resize(nAccounts, 0);
};
CAccount::~CAccount() {};

//======================  CSAM  ==================================

CSAM::CSAM()
{
	_pSAMTXT = new vector<string>;
	_pSAMmonth = new vector<vector<double>>;
	pGFCFfractions() = new vector<double>;
	pRowSum() = new vector<double>;
	pColSum() = new vector<double>;

	_year = -1;
	_units = -1;
	_active = -1;
	_InitUnemploymentPercent = -1.0;
	_nAccounts = -1;
	_GFCFtype = UndefAgentType;

	_InitSalary = -1.0;
	_nPXproducerTypes = -1;
	_pAccounts = nullptr;
}
CSAM::~CSAM()
{
	delete pGFCFfractions();
	delete _pSAMTXT;
	delete _pAccounts;
}

long CSAM::nLaccounts() const { return (long)getAccountGroups().at("L").size(); };
long CSAM::nHaccounts() const { return (long)getAccountGroups().at("H").size(); };

bool CSAM::IsPublicSector(GoodType producerType)
{
	// Check if the producer type corresponds to sectors O, P, or Q in SAM
	string sectorName = getSAM().getAccNameOfN(producerType);
	// Using first character to identify sector
	if (sectorName.length() > 0)
	{
		char firstChar = sectorName[3];
		return (firstChar == 'O' || firstChar == 'P' || firstChar == 'Q');
	}
	return false;
}

bool CSAM::IsExtSectType(GoodType gType) const
{
	return gType > 0 && getAccount(gType).label().at(0) == 'X';
};
bool CSAM::IsGFCFType(GoodType gType) const
{
	return gType > 0 && getAccount(gType).label().at(0) == 'F';
};
GoodType CSAM::GFCFtype() const { return _GFCFtype; }

double const CSAM::GFCFfractionOf(GoodType gType) const
{
	return (*_pGFCFfractions)[gType];
};

GoodQtty& CSAM::setRowCol(GoodType row, GoodType col)
{
	return Account(row)._rowQtties.at(col);
}
GoodQtty& CSAM::setRowCol(GoodType row, string colLabel)
{
	auto inic = colLabel.find('_') < colLabel.length() ? colLabel.find('_') + 1 : 0;
	auto namec = colLabel.substr(inic);
	return Account(row)._rowQtties.at(getAccNofName(namec));
}
GoodQtty& CSAM::setRowCol(string rowLabel, string colLabel)
{
	auto inir = rowLabel.find('_') < rowLabel.length() ? rowLabel.find('_') + 1 : 0;
	auto namer = rowLabel.substr(inir);
	auto inic = colLabel.find('_') < colLabel.length() ? colLabel.find('_') + 1 : 0;
	auto namec = colLabel.substr(inic);
	return Account(getAccNofName(namer))._rowQtties.at(getAccNofName(namec));
}
GoodQtty& CSAM::setRowCol(string rowLabel, GoodType col)
{
	auto inir = rowLabel.find('_') < rowLabel.length() ? rowLabel.find('_') + 1 : 0;
	auto namer = rowLabel.substr(inir);
	return Account(getAccNofName(namer))._rowQtties.at(col);
}
GoodQtty CSAM::getRowCol(GoodType row, GoodType col) const
{
	return getAccount(row).rowQtty(col);
}
GoodQtty CSAM::getRowCol(GoodType row, string colLabel) const
{
	auto inic = colLabel.find('_') < colLabel.length() ? colLabel.find('_') + 1 : 0;
	auto namec = colLabel.substr(inic);
	auto col = getAccNofName(namec);

	return getAccount(row).rowQtty(col);
}
GoodQtty CSAM::getRowCol(string rowLabel, string colLabel) const
{
	auto inir = rowLabel.find('_') < rowLabel.length() ? rowLabel.find('_') + 1 : 0;
	auto namer = rowLabel.substr(inir);
	auto inic = colLabel.find('_') < colLabel.length() ? colLabel.find('_') + 1 : 0;
	auto namec = colLabel.substr(inic);
	auto row = getAccNofName(namer);

	auto col = getAccNofName(namec);

	return getAccount(row).rowQtty(col);
}
GoodQtty CSAM::getRowCol(string rowLabel, GoodType col) const
{
	auto inir = rowLabel.find('_') < rowLabel.length() ? rowLabel.find('_') + 1 : 0;
	auto namer = rowLabel.substr(inir);
	auto row = getAccNofName(namer);

	return getAccount(row).rowQtty(col);
}

void CSAM::fillCtoPfractions()
{
	// fill CtoPfraction()[Ccol][Prow] map

	CGoods colSum;
	for (const auto& Cacc : getAccountGroups().at("C"))
	{
		auto Ccol = Cacc->accN();
		for (const auto& Pacc : getAccountGroups().at("P"))
		{
			auto Prow = Pacc->accN();
			colSum[Ccol] += getRowCol(Prow, Ccol);
		}
	}
	for (const auto& Cacc : getAccountGroups().at("C"))
	{
		auto Ccol = Cacc->accN();
		for (const auto& Pacc : getAccountGroups().at("P"))
		{
			auto Prow = Pacc->accN();
			CtoPfraction()[Prow][Ccol] = (double)getRowCol(Prow, Ccol) / colSum[Ccol];
		}
	}
}
void CSAM::splitC_goodsIntoTheirP_goodsComponents()
{
	// CONSUMER Goods (combo sets)  =======================================

	if (getAccountGroups().find("C") == getAccountGroups().end())
		return;

	fillCtoPfractions();

	// ===========  split C-goods into their P-goods components  ================

	// 1. ICs of P cols (Producer definitions)

	// -----  Scan the CrPc matrix: [Crows x Pcols]   ------------------

	// scan the Crows
	for (const auto& Crow_Acc : getAccountGroups().at("C")) // Intermediate 'Consumer' goods
	{
		auto Crow_N = Crow_Acc->accN();

		// scan all the Cols of this Crow
		for (auto& Col_Acc : getAccounts())
		{
			if (Col_Acc->label() == "")
				continue;

			GoodType Col_N = Col_Acc->accN();

			auto Consum_value = getRowCol(Crow_N, Col_N);
			if (Consum_value == 0)
				continue;

			// ----- PrCc conversion matrix (Prows x Ccols)  -----------
			// 
			// split this Cr_value into its P-components using the fractions
			// at (PrCc_rowNs,PrCc_colN) and accumulate them in the (PrCc_rowNs,CPcolN) cells
			// 
			// Scan the Prows at the Cr_colN column
			for (auto& Prow_Acc : getAccountGroups().at("P")) // P rows
			{
				GoodType Prow_N = Prow_Acc->accN();
				setRowCol(Prow_N, Col_N) +=
					Consum_value * CtoPfraction().at(Prow_N).at(Crow_N);
			}
		}
	}
}

void CSAM::setupGFCFfractions()
{
	delete pGFCFfractions();
	pGFCFfractions() = new vector<double>(getnPXproducerTypes(), 0.0);

	double totICofGFCF = 0;
	for (int rowN = 0; rowN < pGFCFfractions()->size(); ++rowN)
	{
		double val = getRowCol(rowN, "GFCF");
		if (val < 0)
		{
			CWorld::ERRORmsg("WARNING: GFCFfraction = " + to_string(val) + " < 0, set to 0", false);
			setRowCol(rowN, "GFCF") = 0;
		}
		totICofGFCF += val;
	}

	for (int rowN = 0; rowN < pGFCFfractions()->size(); ++rowN)
	{
		double val = getRowCol(rowN, "GFCF");
		(*pGFCFfractions())[rowN] = val / totICofGFCF;
	}
}
void CSAM::initializeCentralDataStructures()
{
	CentralBankType() = GovernmentType - 1;
	PrivateBankType() = CentralBankType() - 1;
	pRowSum()->clear();
	pRowSum()->resize(nAccounts());
	pColSum()->resize(nAccounts());
}
void CSAM::finalizeAccountSetup()
{
	nPProducerTypes() = static_cast<long>(getAccountGroups().at("P").size());
	nPXproducerTypes() = getnPProducerTypes() + static_cast<long>(getAccountGroups().at("X").size());
	_GFCFtype = getAccNofLabel("F_GFCF");
	assert(_GFCFtype == nPXproducerTypes());

	// Let's compute the row and column sums for the SAM
	for (int rowSectorN = 0; rowSectorN < getnAccounts(); ++rowSectorN)
	{
		(*pRowSum())[rowSectorN] = 0;
		(*pColSum())[rowSectorN] = 0;
	}

	for (auto& pAcc : getAccounts())
	{
		int colN = pAcc->accN();
		for (auto& pRowAcc : getAccounts())
		{
			int rowN = pRowAcc->accN();
			(*pRowSum())[rowN] += getRowCol(rowN, colN);
			(*pColSum())[colN] += getRowCol(rowN, colN);
		}
	}

	// Finish redistribution of some cell values

	int GFCFidx = getAccNofLabel("F_GFCF");
	int Lidx = getAccNofLabel("L_CompEmployees");
	int Kidx = getAccNofLabel("K_GrossOpSurplus");
	int TNidx = getAccNofLabel("T_TaxProduction");
	int TUidx = getAccNofLabel("T_TaxProducts");
	int Gidx = getAccNofLabel("G_Government");
	int Hidx = getAccNofLabel("H_Households");

	// All compensation of employees is distributed to households
	setRowCol(Hidx, Lidx) = (*pRowSum())[Lidx];
	(*pColSum())[Lidx] += (*pRowSum())[Lidx];
	(*pRowSum())[Hidx] += (*pRowSum())[Lidx];

	// We assume that the Gross Operating Surplus is distributed between households and government
	// in proportion to their total expenditure

	double totalDomesticExpenditure_Gov = 0;
	double totalDomesticExpenditure_HH = 0;
	for (int rowSectorN = 0; rowSectorN < getnPXproducerTypes(); ++rowSectorN)
	{
		totalDomesticExpenditure_Gov += getRowCol(rowSectorN, Gidx);
		totalDomesticExpenditure_HH += getRowCol(rowSectorN, Hidx);
	}
	totalDomesticExpenditure_Gov += getRowCol("F_GFCF", Gidx);
	totalDomesticExpenditure_HH += getRowCol("F_GFCF", Hidx);
	double totalDomesticExpenditure = totalDomesticExpenditure_Gov + totalDomesticExpenditure_HH;

	setRowCol(Hidx, Kidx) = (*pRowSum())[Kidx] * totalDomesticExpenditure_HH / totalDomesticExpenditure;
	setRowCol(Gidx, Kidx) = (*pRowSum())[Kidx] * totalDomesticExpenditure_Gov / totalDomesticExpenditure;
	(*pRowSum())[Hidx] += getRowCol(Hidx, Kidx);
	(*pRowSum())[Gidx] += getRowCol(Gidx, Kidx);

	// All taxes are distributed to the government
	setRowCol(Gidx, TNidx) = (*pRowSum())[TNidx];
	(*pColSum())[Gidx] += (*pRowSum())[TNidx];
	(*pRowSum())[Gidx] += (*pRowSum())[TNidx];

	setRowCol(Gidx, TUidx) = (*pRowSum())[TUidx];
	(*pColSum())[Gidx] += (*pRowSum())[TUidx];
	(*pRowSum())[Gidx] += (*pRowSum())[TUidx];

	// Calculate gross output totals
	SAMGrossOutput_mu().resize(getnPProducerTypes(), 0);

	for (auto& pAcc : getAccountGroups().at("P"))
	{
		int colN = pAcc->accN();
		for (auto& pRowAcc : getAccounts())
		{
			int rowN = pRowAcc->accN();
			SAMGrossOutput_mu()[colN] += getRowCol(rowN, colN);
		}
	}
}
void CSAM::computeEconomicIndicators()
{
	// Calculate initial salary based on compensation of employees
	double CompEmployees = 0.0;

	for (const auto& pAcc : getAccountGroups().at("L"))
		for (int prodN = 0; prodN < getnPProducerTypes(); ++prodN)
			CompEmployees += pAcc->rowQtties()[prodN];

	InitSalary() = CompEmployees / (12 * _active * 0.01 * (100.0 - InitUnemploymentPercent()));

	// Setup GFCF fractions for capital formation
	setupGFCFfractions();
}

void CSAM::readSAMTXT(string fileNameExt)
{
	string word;
	const int maxchars = 2000;
	char field[maxchars]{};

	ifstream ifstrm(fileNameExt);
	if (!ifstrm.is_open())
	{
		getWorld().ERRORmsg("Cannot open file: " + fileNameExt, true);
		return;
	}

	// Find COUNTRIES block
	bool foundCountries = false;
	while (ifstrm >> word)
	{
		if (word == "COUNTRIES")
		{
			foundCountries = true;
			break;
		}
	}

	if (!foundCountries || ifstrm.eof())
	{
		getWorld().ERRORmsg("No COUNTRIES block found in " + fileNameExt, true);
		ifstrm.close();
		return;
	}

	// Clear existing content
	SAMTXT().clear();

	// Read the opening brace
	ifstrm >> word;
	if (word != "{")
	{
		// If not immediately after COUNTRIES, read rest of line
		ifstrm.getline(field, maxchars);
		SAMTXT().push_back(" " + word + field);
	}
	else
	{
		SAMTXT().push_back(" {");
	}

	// Read content with brace tracking
	int braceCount = 1;
	string line;

	while (braceCount > 0 && getline(ifstrm, line))
	{
		// Store the line first
		SAMTXT().push_back(line);

		// Count braces
		for (char c : line)
		{
			if (c == '{')
				braceCount++;
			else if (c == '}')
			{
				braceCount--;
				if (braceCount == 0)
				{
					// Remove the last line if it only contains the closing brace
					string lastLine = SAMTXT().back();
					lastLine.erase(0, lastLine.find_first_not_of(" \t"));
					if (lastLine == "}")
						SAMTXT().pop_back();
					break;
				}
			}
		}
	}

	ifstrm.close();
}

// writeSAMTXT remains the same but ensure it's correct
void CSAM::writeSAMTXT(ofstream& ofstrm) const
{
	ofstrm << "\nCOUNTRIES";
	for (const auto& line : getSAMTXT())
		ofstrm << line << endl;
	ofstrm << "}" << endl;  // Closing brace for COUNTRIES block
}
void CSAM::writeInputSAM() const
{
	// Column width for formatting
	const int labelWidth = 18;
	const int valueWidth = 14;
	ofstream& Outf = LogFile();

	// Calculate row and column sums
	vector<GoodQtty> RowSum(getnAccounts(), 0);
	vector<GoodQtty> ColSum(getnAccounts(), 0);

	for (int nRow = 0; nRow < getnAccounts(); ++nRow)
	{
		for (int nCol = 0; nCol < getnAccounts(); ++nCol)
		{
			double cellValue = getRowCol(nRow, nCol);
			RowSum[nRow] += cellValue;
			ColSum[nCol] += cellValue;
		}
	}

	// Write header
	Outf << endl;
	getWorld().writeTimeAndDate(Outf);
	Outf << "\n------------------  Input SAM  ------------------\n";

	// Write column labels
	Outf << "\n" << std::right << std::setw(labelWidth) << "";
	for (int nc = 0; nc < getnAccounts(); ++nc)
		Outf << std::right << std::setw(valueWidth) << getAccount(nc).label().substr(0, valueWidth - 1);

	Outf << std::right << std::setw(valueWidth) << "RowSum";

	// Write each row with values
	for (int nRow = 0; nRow < getnAccounts(); ++nRow)
	{
		Outf << "\n";
		const auto& account = getAccount(nRow);

		// Write row label
		Outf << std::left << std::setw(labelWidth) << account.label().substr(0, labelWidth - 1);

		// Write cell values
		for (int nCol = 0; nCol < getnAccounts(); ++nCol)
		{
			GoodQtty cellValue = getRowCol(nRow, nCol);

			/*
			// Special formatting for GFCF row
			if (nRow == GFCFtype())
			{
				if (nCol >= getnPProducerTypes())
					Outf << std::right << std::setw(valueWidth) << cellValue;
				else
					Outf << std::left << std::setw(valueWidth) << "";
			}
			else
			*/
			if (cellValue == 0) // Skip empty cells
				Outf << std::left << std::setw(valueWidth) << "";
			else
				Outf << std::right << std::setw(valueWidth) << cellValue;
		}

		// Add row sum and repeat row label at end
		Outf << std::right << std::setw(valueWidth) << RowSum[nRow];
		Outf << "  " << std::left << std::setw(labelWidth) << account.label().substr(0, labelWidth - 1);
	}

	// Write column sums row
	Outf << "\n" << std::left << std::setw(labelWidth) << "ColSum";
	for (int nCol = 0; nCol < getnAccounts(); ++nCol)
		Outf << std::right << std::setw(valueWidth) << ColSum[nCol];

	// Write column labels again at the bottom
	Outf << "\n" << std::right << std::setw(labelWidth) << "";
	for (int nc = 0; nc < getnAccounts(); ++nc)
	{
		Outf << std::right << std::setw(valueWidth) << getAccount(nc).label().substr(0, valueWidth - 1);

		// Add "RowSum" label after the last column
		if (nc == getAccounts().size() - 1)
			Outf << "      RowSum";
	}

	Outf.flush();
}
void CSAM::writeSAMmonth() const
{
	if (!(bool)getInputParameter("WriteSAMmonth"))
		return;

	bool ifSAMperCent = currMonth() % 2;

	ofstream& Outf = LogFile();

	Outf << endl;
	getWorld().writeTimeAndDate(Outf);

	if (ifSAMperCent)
		Outf << "SAM Error(%) after absoluteMonth " << currMonth();
	else
		Outf << "SAM calculated after absoluteMonth " << currMonth() << " ";
	Outf << "  --------------------------------------------------------";

	Outf.flush();

	int wl = 18;
	int w = 14;
	GoodQtty gVal = 0;
	vector<GoodQtty> RowSum(getnAccounts(), 0);
	vector<GoodQtty> ColSum(getnAccounts(), 0);

	// RowSum, ColSum

	int nr = 0;
	for (const auto& rowVect : (*getpSAMmonth()))
	{
		int nc = 0;
		for (const auto& colVal : rowVect)
		{
			gVal = colVal; // round(colVal);
			RowSum[nr] += gVal;
			ColSum[nc] += gVal;
			nc++;
		}
		nr++;
	}

	// Header labels row

	Outf << "\n" << std::right << std::setw(wl) << "";
	for (int nc = 0; nc < getAccounts().size(); ++nc)
	{
		Outf << std::right << std::setw(w) << getAccount(nc).label().substr(0, w - 1);

		if (nc == (getpSAMmonth()->size() - 1))
		{
			if (ifSAMperCent)
				Outf << "      RowSum(%)";
			else
				Outf << "      RowSum";
		}
	}

	// Write the simulated SAM cell values

	nr = 0;
	for (const auto& rowVect : (*getpSAMmonth()))
	{
		int nc = 0;
		Outf << "\n";
		for (double colVal : rowVect)
		{
			if (nc == 0)
				Outf << std::left << std::setw(wl) << getAccount(nr).label().substr(0, wl - 1);

			if (nr == GFCFtype())
			{
				if (getInputParameter("PrintFullSAM") == 1 || nc >= getnPProducerTypes())
				{
					if (ifSAMperCent)
					{
						if (getRowCol(nr, nc) == 0)
							Outf << std::left << std::setw(w) << "";
						else
						{
							GoodQtty perCent = round(100. * colVal / getRowCol(nr, nc));
							Outf << std::right << std::setw(w) << (perCent - 100);
						}
					}
					else
					{
						gVal = colVal;
						Outf << std::right << std::setw(w) << GoodQtty(gVal);
					}
				}
				else
					Outf << std::left << std::setw(w) << "";
			}
			else if (getRowCol(nr, nc) == 0) // skip empty SAM cells
				Outf << std::left << std::setw(w) << "";
			else
			{
				if (getInputParameter("PrintFullSAM") == 1 || nc >= getnPProducerTypes())
				{
					if (ifSAMperCent)
					{
						GoodQtty perCent = round(100. * colVal / getRowCol(nr, nc));
						Outf << std::right << std::setw(w) << (perCent - 100);
					}
					else
					{
						gVal = colVal;
						Outf << std::right << std::setw(w) << GoodQtty(gVal);
					}
				}
				else
					Outf << std::left << std::setw(w) << "";
			}

			nc++;

			// Add the RowSum col and a labels col again

			if (nc == rowVect.size())
			{
				if (ifSAMperCent)
				{
					GoodQtty perCent = round(100. * RowSum[nr] / (*getpRowSum()).at(nr));
					Outf << std::right << std::setw(w) << GoodQtty(perCent - 100);
				}
				else
					Outf << std::right << std::setw(w) << GoodQtty(RowSum[nr]);

				Outf << "  " << std::left << std::setw(wl) << getAccount(nr).label().substr(0, wl - 1);
			}
		}
		nr++;

		// Add the ColSum row and a labels row again

		nc = 0;
		if (nr == getpSAMmonth()->size())
		{
			if (ifSAMperCent)
				Outf << "\n" << std::right << std::setw(wl) << "ColSum(%)";
			else
				Outf << "\n" << std::right << std::setw(wl) << "ColSum";

			for (const auto& colVal : rowVect)
			{
				if (nc < getnPProducerTypes() && getInputParameter("PrintFullSAM") == 0)
					Outf << std::right << std::setw(w) << "";
				else
				{
					if (!ifSAMperCent)
						Outf << std::right << std::setw(w) << GoodQtty(ColSum[nc]);
					else
					{
						//	GoodQtty perCent = round(100. * ColSum[nc] / (*getpColSum()).at(nc));
						GoodQtty perCent = 100. * ColSum[nc] / (*getpColSum()).at(nc);
						Outf << std::right << std::setw(w) << GoodQtty(perCent - 100);
					}
				}

				nc++;
			}

			// A last row with account labels again

			Outf << "\n" << std::right << std::setw(wl) << "";
			for (int nc = 0; nc < getAccounts().size(); ++nc)
			{
				Outf << std::right << std::setw(w) << getAccount(nc).label().substr(0, w - 1);

				if ((nc == getpSAMmonth()->size() - 1) && !ifSAMperCent)
					Outf << "      RowSum";
			}
		}
	}

	Outf << "\n";
	Outf.flush();
};
void CSAM::saveInputSAM(const std::string& filename, const std::string& separator)
{
	// Open output file
	std::ofstream csvFile(filename);
	if (!csvFile.is_open())
	{
		LogFile() << "Error: Unable to open file " << filename << " for writing." << std::endl;
		return;
	}

	// Calculate row and column sums
	vector<GoodQtty> RowSum(getnAccounts(), 0);
	vector<GoodQtty> ColSum(getnAccounts(), 0);

	for (int nRow = 0; nRow < getnAccounts(); ++nRow)
	{
		for (int nCol = 0; nCol < getnAccounts(); ++nCol)
		{
			double cellValue = getRowCol(nRow, nCol);
			RowSum[nRow] += cellValue;
			ColSum[nCol] += cellValue;
		}
	}

	// Write header row with account labels
	csvFile << "Account";
	for (int nc = 0; nc < getnAccounts(); ++nc)
	{
		csvFile << separator << getAccount(nc).label();
	}
	csvFile << separator << "RowSum" << std::endl;

	// Write each data row
	for (int nRow = 0; nRow < getnAccounts(); ++nRow)
	{
		const auto& account = getAccount(nRow);

		// Write row label
		csvFile << account.label();

		// Write cell values
		for (int nCol = 0; nCol < getnAccounts(); ++nCol)
		{
			GoodQtty cellValue = getRowCol(nRow, nCol);
			csvFile << separator << cellValue;
		}

		// Add row sum
		csvFile << separator << RowSum[nRow] << std::endl;
	}

	// Write column sums row
	csvFile << "ColSum";
	for (int nCol = 0; nCol < getnAccounts(); ++nCol)
	{
		csvFile << separator << ColSum[nCol];
	}
	csvFile << std::endl;

	csvFile.close();

	LogFile() << "SAM data saved to CSV file: " << filename << std::endl;
}

void LaunchSimulationsAndSynchronize()
{
	std::string simulationNameA = CWorld::getSimulationName();
	std::wstring simulationNameW(simulationNameA.begin(), simulationNameA.end());

	wchar_t exePath[MAX_PATH];
	GetModuleFileNameW(NULL, exePath, MAX_PATH);

	std::vector<HANDLE> processHandles;

	for (const auto& pair : CWorld::SimulatedCountries())
	{
		const std::string& countryCode = pair.first;
		std::wstring countryCodeW(countryCode.begin(), countryCode.end());

		// 1. Prepare the input file name for the child process
		std::wstring inputFileName = simulationNameW + L"_" + countryCodeW;
		if (getInputParameter("LoadMonthN") > 0)
			inputFileName += L"_" + std::to_wstring((int)getInputParameter("LoadMonthN"));
		inputFileName += L".dep";

		// 2. Prepare the command line
		std::wstring commandLine = L"\"" + std::wstring(exePath) + L"\" \"" + inputFileName + L"\"";

		STARTUPINFOW si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		// 3. Create the child process
		if (!CreateProcessW(
			NULL,
			&commandLine[0],
			NULL,
			NULL,
			FALSE,
			0,
			NULL,
			NULL,
			&si,
			&pi))
		{
			CWorld::ERRORmsg("CreateProcess failed for country " + countryCode, true);
			continue;
		}

		std::cout << "Launched simulation for country: " << countryCode << std::endl;
		processHandles.push_back(pi.hProcess);
		CloseHandle(pi.hThread); // Close the thread handle as we don't need it
	}

	// 4. Wait for all child processes to complete
	if (!processHandles.empty())
	{
		std::cout << "Waiting for all country simulations to finish..." << std::endl;
		WaitForMultipleObjects((DWORD)processHandles.size(), processHandles.data(), TRUE, INFINITE);
	}

	// 5. Clean up process handles
	for (HANDLE hProcess : processHandles)
	{
		CloseHandle(hProcess);
	}

	std::cout << "All country simulations have finished." << std::endl;
	exit(0);
}

void CSAM::readSAM(string fileNameExt)
{
	ifstream ifstrm;
	ifstrm.open(fileNameExt);

	if (!ifstrm.is_open())
	{
		getWorld().ERRORmsg("Cannot open input file: " + fileNameExt, true);
		return;
	}

	string word;

	// Find COUNTRIES block
	while (ifstrm >> word && word != "COUNTRIES");

	if (ifstrm.eof())
	{
		getWorld().ERRORmsg("No COUNTRIES block found in input file", true);
		ifstrm.close();
		return;
	}

	// Read opening brace
	ifstrm >> word;
	if (word != "{")
	{
		getWorld().ERRORmsg("Expected '{' after COUNTRIES", true);
		ifstrm.close();
		return;
	}

	// Process countries
	readSimulatedCountries(ifstrm);
	ifstrm.close();

	if (CWorld::SimulatedCountries().size() > 1)
	{
		writeFigaroInputFiles();
		LaunchSimulationsAndSynchronize();
	}

	BuildSAM();
}
void CSAM::readSimulatedCountries(ifstream& ifstrm)
{
	const int maxchars = 2000;
	char field[maxchars]{};
	string gName, word;
	double value = 0;

	// Initialize Figaro
	delete World().pFigaro();
	World().pFigaro() = new CFigaro();

	// Read FIGARO filename (first item after COUNTRIES {)
	ifstrm >> gName;
	DEPData().MATRIXfileName() = gName;

	// Extract year from filename
	// The year is the 4 digits before the last dot in the filename
	size_t lastDotPos = gName.find_last_of('.');
	if (lastDotPos != string::npos && lastDotPos >= 4)
	{
		// Extract exactly 4 characters before the last dot
		string yearStr = gName.substr(lastDotPos - 4, 4);

		// Verify all 4 characters are digits
		bool isValidYear = true;
		for (char c : yearStr)
		{
			if (!isdigit(c))
			{
				isValidYear = false;
				break;
			}
		}

		if (isValidYear)
		{
			_year = stol(yearStr);
			World().pFigaro()->_year = _year;
		}
		else
		{
			getWorld().ERRORmsg("Cannot extract valid 4-digit year from filename: " + gName, true);
			return;
		}
	}
	else
	{
		getWorld().ERRORmsg("Cannot find year in filename: " + gName, true);
		return;
	}

	// Read Units
	ifstrm >> word;
	if (word != "Units")
	{
		getWorld().ERRORmsg("Expected 'Units' after filename, got: " + word, true);
		return;
	}

	// Read units value and currency
	ifstrm >> value >> word;
	_units = (long)value;
	World().pFigaro()->_units = _units;
	// word contains the currency (e.g., "euro")

	// Process countries
	while (ifstrm >> word)
	{
		if (word == "}")
		{
			// End of COUNTRIES block
			break;
		}

		if (word == "Country")
		{
			// Create new country
			CSimulatedCountry* pCountry = new CSimulatedCountry();

			// Read country code
			ifstrm >> pCountry->_code;
			pCountry->_name = pCountry->_code; // Initially set name same as code
			pCountry->_AllExtSectCountries.insert(pCountry->_code);

			// Read country-specific configuration
			string paramName;
			while (ifstrm >> paramName)
			{
				if (paramName == "Country" || paramName == "}")
				{
					// Put back for next iteration
					ifstrm.putback(' ');
					for (int i = (int)paramName.length() - 1; i >= 0; --i)
						ifstrm.putback(paramName[i]);
					break;
				}

				if (paramName == "DisaggExtSectCountries")
				{
					ifstrm >> word; // {
					if (word != "{")
					{
						getWorld().ERRORmsg("Expected '{' after DisaggExtSectCountries", true);
						continue;
					}

					while (ifstrm >> gName && gName != "}")
					{
						pCountry->_DisaggExtSectCountries.insert(gName);
						pCountry->_AllExtSectCountries.insert(gName);
					}
				}
				else if (paramName == "AggExtSectCountries")
				{
					ifstrm >> word; // {
					if (word != "{")
					{
						getWorld().ERRORmsg("Expected '{' after AggExtSectCountries", true);
						continue;
					}

					string RW = "RW";
					if (pCountry->_DisaggExtSectCountries.find(RW) == pCountry->_DisaggExtSectCountries.end())
						pCountry->_AggExtSectCountries.insert(RW);

					while (ifstrm >> gName && gName != "}")
					{
						pCountry->_AggExtSectCountries.insert(gName);
						pCountry->_AllExtSectCountries.insert(gName);
					}
				}
				else if (paramName == "X0")
				{
					// Read X0 and Y0 as double values
					double x0, y0;
					ifstrm >> x0;

					ifstrm >> word; // Should be "Y0"
					if (word != "Y0")
					{
						getWorld().ERRORmsg("Expected 'Y0' after X0 value, got: " + word, true);
						continue;
					}

					ifstrm >> y0;

					// Store coordinates as doubles
					pCountry->_X0 = x0;
					pCountry->_Y0 = y0;

					// Also store in input parameters for plotting
					DEPData().mInputParameters()["PlotsX0"] = x0;
					DEPData().mInputParameters()["PlotsY0"] = y0;
				}
				else
				{
					// Unexpected parameter in COUNTRIES block
					getWorld().ERRORmsg("Unexpected parameter in COUNTRIES block: " + paramName, false);
					// Skip until next keyword or number
					ifstrm >> word;
				}
			}

			// Load economic parameters from external file if available
			readCountryParameters(pCountry->_code, *pCountry);

			// Store the country
			CWorld::SimulatedCountries()[pCountry->_code] = pCountry;
		}
		else
		{
			getWorld().ERRORmsg("Expected 'Country' or '}' in COUNTRIES block, got: " + word, true);
		}
	}

	// Validate that at least one country was read
	if (CWorld::SimulatedCountries().empty())
	{
		getWorld().ERRORmsg("No countries found in COUNTRIES block", true);
	}
}
void CSAM::readCountryParameters(const string& countryCode, CSimulatedCountry& country)
{
	string paramsFileName = "FIGARO_country_params_2010.dep";
	ifstream paramsFile(paramsFileName);

	if (!paramsFile.is_open())
	{
		CWorld::ERRORmsg("Cannot open country parameters file: " + paramsFileName, true);
		return;
	}

	string word;
	bool foundCountry = false;

	// Simple token-based parsing
	while (paramsFile >> word)
	{
		if (word == "Country")
		{
			string code;
			paramsFile >> code;

			if (code == countryCode)
			{
				foundCountry = true;

				// Read parameters until we hit another Country or end
				string paramName;
				while (paramsFile >> paramName)
				{
					// Check for next country
					if (paramName == "Country")
					{
						// Put it back and break
						paramsFile.putback(' ');
						for (int i = (int)paramName.length() - 1; i >= 0; --i)
							paramsFile.putback(paramName[i]);
						break;
					}

					// Check for end markers
					if (paramName == "}" || paramsFile.eof())
						break;

					// Handle special cases
					if (paramName == "X0")
					{
						double x0, y0;
						paramsFile >> x0 >> word >> y0; // X0 value Y0 value
						continue;
					}

					if (paramName == "DisaggExtSectCountries" || paramName == "AggExtSectCountries")
					{
						paramsFile >> word; // {
						while (paramsFile >> word && word != "}");
						continue;
					}

					// Read numeric value
					double value;
					if (!(paramsFile >> value))
					{
						// Not a parameter line, skip
						continue;
					}

					// Store parameters
					if (paramName == "ActivePop")
						country._ActivePop = value;
					else if (paramName == "InitUnemp")
						country._InitUnemp = value;
					else
						DEPData().mInputParameters()[paramName] = value;
				}

				break; // Done with this country
			}
		}
	}

	paramsFile.close();

	if (!foundCountry)
	{
		CWorld::ERRORmsg("Country " + countryCode + " not found in " + paramsFileName, true);
	}
}

bool CSAM::writeFigaroInputFiles()
{
	if (getInputParameter("LoadMonthN") > 0)
		return false;

	string inputFileName = getDEPData().getInputFileName() + ".dep";

	// First pass: Read and parse the entire input file into structured data
	ifstream ifstrm(inputFileName);
	if (!ifstrm.is_open())
	{
		CWorld::ERRORmsg("Cannot open input file: " + inputFileName, true);
		return false;
	}

	// Store all content before COUNTRIES
	stringstream beforeSAMContent;
	string word;
	const int maxchars = 20000;
	char field[maxchars]{};

	while (ifstrm >> word)
	{
		if (word == "COUNTRIES")
		{
			beforeSAMContent << word;
			break;
		}
		ifstrm.getline(field, maxchars);
		beforeSAMContent << word << field << endl;
	}

	if (ifstrm.eof())
	{
		CWorld::ERRORmsg("No COUNTRIES found in input file", true);
		ifstrm.close();
		return false;
	}

	// Read the opening brace and store it
	ifstrm.getline(field, maxchars);
	string samHeaderBrace = field;

	// Parse COUNTRIES header data (Year, Units, etc.)
	vector<string> samHeaderLines;
	string line;

	while (ifstrm >> word)
	{
		if (word == "Country")
		{
			// Put the word back for country parsing
			for (int i = static_cast<int>(word.length()) - 1; i >= 0; --i)
				ifstrm.putback(word[i]);
			ifstrm.putback(' ');
			break;
		}

		getline(ifstrm, line);
		samHeaderLines.push_back(word + line);
	}

	// Parse all country data into a structured map
	map<string, vector<string>> countryDataMap;

	while (ifstrm >> word)
	{
		if (word == "}")
		{
			// End of COUNTRIES block
			break;
		}

		if (word == "Country")
		{
			string countryCode;
			ifstrm >> countryCode;

			vector<string> countryParams;
			string paramWord;

			// Read all parameters for this country
			while (ifstrm >> paramWord)
			{
				// Check if we've reached the next country or end of block
				if (paramWord == "Country" || paramWord == "}")
				{
					// Put the word back for the next iteration
					for (int i = static_cast<int>(paramWord.length()) - 1; i >= 0; --i)
						ifstrm.putback(paramWord[i]);
					ifstrm.putback(' ');
					break;
				}

				// Special handling for parameters with nested structures
				if (paramWord == "DisaggExtSectCountries" || paramWord == "AggExtSectCountries")
				{
					string braceLine;
					getline(ifstrm, braceLine);
					countryParams.push_back(paramWord + braceLine);

					// Read until closing brace
					string innerWord;
					while (ifstrm >> innerWord)
					{
						if (innerWord == "}")
						{
							countryParams.push_back(innerWord);
							break;
						}
						countryParams.push_back(innerWord);
					}
				}
				else
				{
					// Regular parameter with value on same line
					string paramLine;
					getline(ifstrm, paramLine);
					countryParams.push_back(paramWord + paramLine);
				}
			}

			countryDataMap[countryCode] = countryParams;
		}
	}

	// Search for PLOTS section as a standalone keyword
	stringstream plotsContent;
	bool plotsFound = false;

	ifstrm.clear();
	ifstrm.seekg(0);

	while (ifstrm >> word)
	{
		if (word == "PLOTS" && ifstrm.peek() != '_')  // Ensure it's not part of a compound word
		{
			// Verify next character is whitespace or opening brace
			char nextChar = ifstrm.peek();
			if (nextChar == ' ' || nextChar == '\t' || nextChar == '{' || nextChar == '\n' || nextChar == '\r')
			{
				plotsFound = true;
				plotsContent << endl << word;

				// Read the rest of the PLOTS block
				getline(ifstrm, line);
				plotsContent << line << endl;

				// Read until we find the closing brace
				int braceDepth = 1;

				while (braceDepth > 0 && getline(ifstrm, line))
				{
					plotsContent << line << endl;

					// Count braces to handle proper nesting
					for (char c : line)
					{
						if (c == '{') braceDepth++;
						else if (c == '}') braceDepth--;
					}
				}

				break;
			}
		}
	}

	// Search for MAP section as a standalone keyword
	stringstream mapContent;
	bool mapFound = false;

	ifstrm.clear();
	ifstrm.seekg(0);

	while (ifstrm >> word)
	{
		if (word == "MAP" && ifstrm.peek() != '_')  // Ensure it's not part of a compound word
		{
			// Verify next character is whitespace or opening brace
			char nextChar = ifstrm.peek();
			if (nextChar == ' ' || nextChar == '\t' || nextChar == '{' || nextChar == '\n' || nextChar == '\r')
			{
				mapFound = true;
				mapContent << endl << word;

				// Read the rest of the MAP block
				getline(ifstrm, line);
				mapContent << line << endl;

				// Read until we find the closing brace
				int braceDepth = 1;

				while (braceDepth > 0 && getline(ifstrm, line))
				{
					mapContent << line << endl;

					// Count braces to handle proper nesting
					for (char c : line)
					{
						if (c == '{') braceDepth++;
						else if (c == '}') braceDepth--;
					}
				}

				break;
			}
		}
	}

	ifstrm.close();

	// Second pass: Write individual country files
	for (const auto& simCountryPair : CWorld::SimulatedCountries())
	{
		const string& countryCode = simCountryPair.first;

		// Verify country data exists
		if (countryDataMap.find(countryCode) == countryDataMap.end())
		{
			CWorld::ERRORmsg("Country " + countryCode + " not found in COUNTRIES", true);
			continue;
		}

		string outputFileName = CWorld::getSimulationName() + "_" + countryCode + ".dep";
		ofstream ofstrm(outputFileName);

		if (!ofstrm.is_open())
		{
			CWorld::ERRORmsg("Cannot create output file: " + outputFileName, true);
			continue;
		}

		// Write everything before COUNTRIES
		ofstrm << beforeSAMContent.str();

		// Write COUNTRIES header
		ofstrm << samHeaderBrace << endl;

		// Write SAM header lines
		for (const auto& headerLine : samHeaderLines)
		{
			ofstrm << headerLine << endl;
		}

		// Write country-specific data
		ofstrm << "Country " << countryCode << endl;
		for (const auto& paramLine : countryDataMap[countryCode])
		{
			ofstrm << paramLine << endl;
		}

		// Close COUNTRIES block
		ofstrm << "}" << endl;

		// Append PLOTS section if found
		if (plotsFound)
		{
			ofstrm << plotsContent.str();
		}

		// Append MAP section if found
		if (mapFound)
		{
			ofstrm << mapContent.str();
		}

		ofstrm.close();
	}

	return true;
}

// ===========================  build SAM  ===========================

bool CSAM::BuildSAM()
{
	for (const auto& pair : CWorld::SimulatedCountries()) // Using structured binding for better readability
	{
		auto& countryCode = pair.first;
		auto& country = *pair.second;
		assert(!CWorld::SimulatedCountries().empty());

		// Initialize Figaro with country data
		initializeFigaroWithCountryData(country);

		// Read FIGARO matrix
		bool ok = World().pFigaro()->readFIGAROmatrix();
		if (!ok) return false;

		const auto& FIGARO = getWorld().getFigaro();

		// Set SAM properties based on FIGARO data
		initializeSAMProperties(FIGARO);

		// Build country accounts
		buildCountrySAMAccounts(FIGARO);

		// Compute economic indicators
		computeEconomicIndicators();

		// Finalize SAM setup
		pSAMmonth()->clear();
		pSAMmonth()->resize(getnAccounts(), vector<double>(getnAccounts(), 0.0));
	}

	return true;
}

void CSAM::initializeFigaroWithCountryData(CSimulatedCountry& country)
{
	World().pFigaro()->_thisCountryCode = country._code;
	World().pFigaro()->_thisCountryName = country._name;
	World().pFigaro()->_active = country._ActivePop;
	World().pFigaro()->_InitUnemploymentPercent = country._InitUnemp;
	World().pFigaro()->_pDisaggExtSectCountries = &country._DisaggExtSectCountries;
	World().pFigaro()->_pAggExtSectCountries = &country._AggExtSectCountries;
	World().pFigaro()->_pAllExtSectCountries = &country._AllExtSectCountries;
}
void CSAM::initializeSAMProperties(const CFigaro& FIGARO)
{
	_SAMname = FIGARO._thisCountryCode;
	_year = FIGARO._year;
	_active = FIGARO._active;
	_InitUnemploymentPercent = FIGARO._InitUnemploymentPercent;

	// Calculate the number of P_ producer types and total external sector accounts X_
	_nPProducerTypes = FIGARO.getNsectors();
	if (getInputParameter("TradeDisaggMode") == 1)
		_nPXproducerTypes = _nPProducerTypes + FIGARO.getNsectors(); // Sector mode: one X_ per sector
	else
		_nPXproducerTypes = _nPProducerTypes +
			static_cast<int>(FIGARO._pDisaggExtSectCountries->size()) * FIGARO.getNsectors() +
			static_cast<int>(FIGARO._pAggExtSectCountries->size());

	_nAccounts = _nPXproducerTypes + 7; // 7: GFCF, L, K, Tproducts, Tproduction, Gov, HH; not explicitely included: NPISH, ChInv
	_units = FIGARO._units; // units of the SAM, typically 1e6 euros

	// Initialize data structures
	SAMGrossOutput_mu().resize(_nPXproducerTypes, 0);

	delete _pAccounts;
	pAccounts() = new vector<CAccount*>(_nAccounts, nullptr);
	long NrowQtties = _nAccounts;
	for (auto& pAcc : *pAccounts())
		pAcc = new CAccount(NrowQtties);

	accNofName().clear();
	accNameOfN().clear();
}
set<GoodType> CSAM::createCountryTypeSet(const set<string>& countryCodes, const CFigaro& FIGARO)
{
	set<GoodType> countryTypes;
	for (const auto& code : countryCodes) {
		countryTypes.insert(FIGARO._pCountryCodeToType->at(code));
	}
	return countryTypes;
}
void CSAM::registerAccountInMaps(const CAccount& account, int accountNumber)
{
	accNofLabel()[account.label()] = accountNumber;
	accNofName()[account.accName()] = accountNumber;
	accNameOfN()[accountNumber] = account.accName();
}

// ===========================  build accounts  ==========================================

void CSAM::buildCountrySAMAccounts(const CFigaro& FIGARO)
{
	// Initialize central data structures
	initializeCentralDataStructures();

	// Create sets of country types for various categories
	auto typeDisaggExtSectCountries = createCountryTypeSet(*FIGARO._pDisaggExtSectCountries, FIGARO);
	auto typeAggExtSectCountries = createCountryTypeSet(*FIGARO._pAggExtSectCountries, FIGARO);
	auto allMyCountriesTypes = createCountryTypeSet(*FIGARO._pAllExtSectCountries, FIGARO);

	// Build producer accounts (P group)
	int SAMaccRowN = buildProducerAccounts(FIGARO);

	// Build external sector accounts (X group)
	SAMaccRowN = buildExternalSectorAccounts(FIGARO, SAMaccRowN, allMyCountriesTypes);

	// Build GFCF account (F group)
	SAMaccRowN = buildGFCFAccount(FIGARO, SAMaccRowN);

	// Build value-added accounts (L, K, T groups)
	SAMaccRowN = buildValueAddedAccounts(FIGARO, SAMaccRowN);

	// Build government and household accounts (G, H groups)
	SAMaccRowN = buildInstitutionalAccounts(SAMaccRowN);

	// Complete account setup
	finalizeAccountSetup();
}

// ===========================  accounts by ROWS  ==========================================

// ===========================  build PProducer accounts  ==========================================

int CSAM::buildProducerAccounts(const CFigaro& FIGARO)
{
	// _nAccounts = nPX + 7 (GFCF+ChInv, L, K, Tproducts, Tproduction, Gov, HH+NPISH)
	for (int SAMaccRowN = 0; SAMaccRowN < _nAccounts; ++SAMaccRowN)
	{
		auto& account = *Accounts()[SAMaccRowN];
		account._accN = SAMaccRowN;

		auto FIGsectorN = SAMaccRowN;
		if (SAMaccRowN < _nPProducerTypes)
		{
			auto& FigProducer = (*FIGARO._pFigProducers).at(FIGARO._thisCountryType).at(FIGsectorN);

			// Initialize producer account
			account._label = "P_" + FigProducer.getlabel();
			account._accName = account._label.substr(2);

			registerAccountInMaps(account, SAMaccRowN);
			addToAccountToGroup(&Account(SAMaccRowN), "P");
		}
		else
		{
			// Uninitialized sector account
			account._label = "";
			account._accName = "";
		}
	}

	// Process producer accounts (P group) Rows
	for (int SAMaccRowN = 0; SAMaccRowN < FIGARO.getNsectors(); ++SAMaccRowN)
	{
		auto FIGsectorN = SAMaccRowN;
		auto& FigProducer = (*FIGARO._pFigProducers).at(FIGARO._thisCountryType).at(FIGsectorN);

		// Initialize producer account
		auto& account = *Accounts()[SAMaccRowN];

		// Process intermediate consumption columns within the country
		processIntermediateConsumption(account, FIGARO, SAMaccRowN, FIGsectorN);

		// Process exports columns to external sectors
		if (getInputParameter("TradeDisaggMode") == 1)
			processExportsToSectorExtSectors(account, FIGARO, SAMaccRowN);
		else
		{
			processExportsToDisaggCountries(account, FIGARO, SAMaccRowN);
			processExportsToAggCountries(account, FIGARO, SAMaccRowN);
		}

		// Process final consumption columns GFCF, Gov, HH
		processFinalConsumption(account, FIGARO, FIGsectorN);
	}

	return FIGARO.getNsectors();
}

// ===========================  build external (XProducer) sector accounts  ==========================================

int CSAM::buildExternalSectorAccounts(const CFigaro& FIGARO, int SAMaccRowN, const set<GoodType>& allMyCountriesTypes)
{
	if (getInputParameter("TradeDisaggMode") == 1)
	{
		// Sector mode: one X_ account per sector, aggregating all foreign countries
		SAMaccRowN = buildSectorExtSectorAccounts(FIGARO, SAMaccRowN);
	}
	else
	{
		// Country mode (default): disaggregated + aggregated countries
		SAMaccRowN = buildDisaggregatedExtSectorAccounts(FIGARO, SAMaccRowN);
		SAMaccRowN = buildAggregatedExtSectorAccounts(FIGARO, SAMaccRowN, allMyCountriesTypes);
	}

	return SAMaccRowN;
}
int CSAM::buildDisaggregatedExtSectorAccounts(const CFigaro& FIGARO, int SAMaccRowN)
{
	for (const auto& extCountryCode : *FIGARO._pDisaggExtSectCountries)
	{
		auto extCountryN = FIGARO._pCountryCodeToType->at(extCountryCode);

		for (int rowExtSectN = 0; rowExtSectN < FIGARO.getNsectors(); ++rowExtSectN)
		{
			auto& extSectFigProducer = (*FIGARO._pFigProducers).at(extCountryN).at(rowExtSectN);

			// Initialize external sector account
			auto& account = *Accounts()[SAMaccRowN];
			account._accN = SAMaccRowN;

			// Process intermediate consumption imports
			for (int colSectN = 0; colSectN < FIGARO.getNsectors(); ++colSectN)
			{
				auto value = (*FIGARO._pFigProducers).at(FIGARO._thisCountryType).at(colSectN)
					.getIC().at(extCountryN).at(rowExtSectN);
				account._rowQtties[colSectN] += value;
				(*pRowSum())[SAMaccRowN] += value;
				(*pColSum())[colSectN] += value;
			}

			// Set account metadata
			account._label = "X_" + extSectFigProducer._label;
			account._accName = extSectFigProducer.getlabel();

			registerAccountInMaps(account, SAMaccRowN);
			addToAccountToGroup(&account, "X");

			++SAMaccRowN;
		}
	}

	return SAMaccRowN;
}
int CSAM::buildAggregatedExtSectorAccounts(const CFigaro& FIGARO, int SAMaccRowN, const set<GoodType>& allMyCountriesTypes)
{
	for (const auto& aggExtCountryCode : *FIGARO._pAggExtSectCountries)
	{
		auto extCountryN = FIGARO._pCountryCodeToType->at(aggExtCountryCode);

		if (aggExtCountryCode != "RW")
		{
			SAMaccRowN = buildRegularAggExtSectorAccount(FIGARO, SAMaccRowN, aggExtCountryCode, extCountryN);
		}
		else
		{
			SAMaccRowN = buildRestOfWorldAccount(FIGARO, SAMaccRowN, aggExtCountryCode, allMyCountriesTypes);
		}
	}

	return SAMaccRowN;
}
int CSAM::buildRegularAggExtSectorAccount(const CFigaro& FIGARO, int SAMaccRowN, const string& aggExtCountryCode, GoodType extCountryN)
{
	auto& account = *Accounts()[SAMaccRowN];
	account._accN = SAMaccRowN;
	account._label = "X_" + aggExtCountryCode;
	account._accName = aggExtCountryCode;

	registerAccountInMaps(account, SAMaccRowN);
	addToAccountToGroup(&account, "X");

	for (int colThisCountrySectN = 0; colThisCountrySectN < FIGARO.getNsectors(); ++colThisCountrySectN)
	{
		for (int rowExtSectN = 0; rowExtSectN < FIGARO.getNsectors(); ++rowExtSectN)
		{
			auto value = (*FIGARO._pFigProducers).at(FIGARO._thisCountryType).at(colThisCountrySectN)
				.getIC().at(extCountryN).at(rowExtSectN);
			account._rowQtties[colThisCountrySectN] += value;
			(*pRowSum())[SAMaccRowN] += value;
			(*pColSum())[colThisCountrySectN] += value;
		}
	}

	return SAMaccRowN + 1;
}
int CSAM::buildRestOfWorldAccount(const CFigaro& FIGARO, int SAMaccRowN, const string& aggExtCountryCode, const set<GoodType>& allMyCountriesTypes)
{
	auto& accountX = *Accounts()[SAMaccRowN];
	accountX._accN = SAMaccRowN;
	accountX._label = "X_" + aggExtCountryCode;
	accountX._accName = aggExtCountryCode;
	registerAccountInMaps(accountX, SAMaccRowN);
	addToAccountToGroup(&Account(SAMaccRowN), "X");

	int SAMrowN = SAMaccRowN;

	++SAMrowN;
	auto& account1 = *Accounts()[SAMrowN];
	account1._accN = SAMrowN;
	account1._label = "F_GFCF";
	account1._accName = "GFCF";
	registerAccountInMaps(account1, SAMrowN);
	addToAccountToGroup(&Account(SAMrowN), "F");

	++SAMrowN;
	auto& account2 = *Accounts()[SAMrowN];
	account2._accN = SAMrowN;
	account2._label = "L_CompEmployees";
	account2._accName = "CompEmployees";
	registerAccountInMaps(account2, SAMrowN);
	addToAccountToGroup(&Account(SAMrowN), "L");

	++SAMrowN;
	auto& account3 = *Accounts()[SAMrowN];
	account3._accN = SAMrowN;
	account3._label = "K_GrossOpSurplus";
	account3._accName = "GrossOpSurplus";
	registerAccountInMaps(account3, SAMrowN);
	addToAccountToGroup(&Account(SAMrowN), "K");

	++SAMrowN;
	auto& account4 = *Accounts()[SAMrowN];
	account4._accN = SAMrowN;
	account4._label = "T_TaxProduction";
	account4._accName = "TaxProduction";
	registerAccountInMaps(account4, SAMrowN);
	addToAccountToGroup(&Account(SAMrowN), "T");

	++SAMrowN;
	auto& account5 = *Accounts()[SAMrowN];
	account5._accN = SAMrowN;
	account5._label = "T_TaxProducts";
	account5._accName = "TaxProducts";
	registerAccountInMaps(account5, SAMrowN);
	addToAccountToGroup(&Account(SAMrowN), "T");

	++SAMrowN;
	auto& account6 = *Accounts()[SAMrowN];
	account6._accN = SAMrowN;
	account6._label = "G_Government";
	account6._accName = "Government";
	registerAccountInMaps(account6, SAMrowN);
	addToAccountToGroup(&Account(SAMrowN), "G");

	++SAMrowN;
	auto& account7 = *Accounts()[SAMrowN];
	account7._accN = SAMrowN;
	account7._label = "H_Households";
	account7._accName = "Households";
	registerAccountInMaps(account7, SAMrowN);
	addToAccountToGroup(&Account(SAMrowN), "H");

	for (auto allCountriesRowN = 0; allCountriesRowN < getWorld().getFigaro().getNcountries(); ++allCountriesRowN)
	{
		if (allMyCountriesTypes.find(allCountriesRowN) != allMyCountriesTypes.end())
			continue;

		for (int colThisCountrySectN = 0; colThisCountrySectN < FIGARO.getNsectors(); ++colThisCountrySectN)
		{
			// Activity columns, this country sectors
			for (int rowExtSectN = 0; rowExtSectN < FIGARO.getNsectors(); ++rowExtSectN)
			{
				auto value = (*FIGARO._pFigProducers).at(FIGARO._thisCountryType).at(colThisCountrySectN)
					.getIC().at(allCountriesRowN).at(rowExtSectN);
				accountX._rowQtties[colThisCountrySectN] += value;
				(*pRowSum())[SAMaccRowN] += value;
				(*pColSum())[colThisCountrySectN] += value;
			}

			// Final consumption columns, this country consumers
			auto value = FIGARO.getFC_Government().at(FIGARO._thisCountryType).at(allCountriesRowN).at(colThisCountrySectN);
			setRowCol("X_RW", "G_Government") += value;
			(*pRowSum())[SAMaccRowN] += value;
			(*pColSum())[colThisCountrySectN] += value;

			value = FIGARO.getFC_Households().at(FIGARO._thisCountryType).at(allCountriesRowN).at(colThisCountrySectN)
				+ FIGARO.getFC_NPISH().at(FIGARO._thisCountryType).at(allCountriesRowN).at(colThisCountrySectN);
			setRowCol("X_RW", "H_Households") += value;
			(*pRowSum())[SAMaccRowN] += value;
			(*pColSum())[colThisCountrySectN] += value;

			value = FIGARO.getFC_GFCF().at(FIGARO._thisCountryType).at(allCountriesRowN).at(colThisCountrySectN)
				+ FIGARO.getFC_ChgInvent().at(FIGARO._thisCountryType).at(allCountriesRowN).at(colThisCountrySectN);
			if (value < 0)
			{
			//MJ	getWorld().ERRORmsg("WARNING, buildRestOfWorldAccount: Negative exports to Rest of World. Setting it to 0", false);
				value = 0;
			}
			setRowCol("X_RW", "F_GFCF") += value;
			(*pRowSum())[SAMaccRowN] += value;
			(*pColSum())[colThisCountrySectN] += value;
		}
	}

	return SAMaccRowN + 1;
}

// ===========================  build sector-based external sector accounts  ==========================================

int CSAM::buildSectorExtSectorAccounts(const CFigaro& FIGARO, int SAMaccRowN)
{
	int firstSectorAccRowN = SAMaccRowN;

	// Create one X_ account per sector, aggregating ALL foreign countries
	for (int sectorN = 0; sectorN < FIGARO.getNsectors(); ++sectorN)
	{
		auto& account = *Accounts()[SAMaccRowN];
		account._accN = SAMaccRowN;

		string sectorCode = FIGARO._pSectorTypeToCode->at(sectorN + 1); // +1 to skip "RW" pseudo-sector at index 0
		account._label = "X_" + sectorCode;
		account._accName = sectorCode;

		registerAccountInMaps(account, SAMaccRowN);
		addToAccountToGroup(&account, "X");

		// Accumulate IC imports from ALL foreign countries for this sector
		for (int foreignCountryN = 0; foreignCountryN < getWorld().getFigaro().getNcountries(); ++foreignCountryN)
		{
			if (foreignCountryN == FIGARO._thisCountryType)
				continue;

			for (int colThisCountrySectN = 0; colThisCountrySectN < FIGARO.getNsectors(); ++colThisCountrySectN)
			{
				auto value = (*FIGARO._pFigProducers).at(FIGARO._thisCountryType).at(colThisCountrySectN)
					.getIC().at(foreignCountryN).at(sectorN);
				account._rowQtties[colThisCountrySectN] += value;
				(*pRowSum())[SAMaccRowN] += value;
				(*pColSum())[colThisCountrySectN] += value;
			}
		}

		++SAMaccRowN;
	}

	// Pre-create institutional accounts at positions after X_Sector accounts
	// (same pattern as buildRestOfWorldAccount - these will be reused by
	// buildGFCFAccount, buildValueAddedAccounts, buildInstitutionalAccounts)
	int SAMrowN = SAMaccRowN;

	auto& account1 = *Accounts()[SAMrowN];
	account1._accN = SAMrowN;
	account1._label = "F_GFCF";
	account1._accName = "GFCF";
	registerAccountInMaps(account1, SAMrowN);
	addToAccountToGroup(&Account(SAMrowN), "F");

	++SAMrowN;
	auto& account2 = *Accounts()[SAMrowN];
	account2._accN = SAMrowN;
	account2._label = "L_CompEmployees";
	account2._accName = "CompEmployees";
	registerAccountInMaps(account2, SAMrowN);
	addToAccountToGroup(&Account(SAMrowN), "L");

	++SAMrowN;
	auto& account3 = *Accounts()[SAMrowN];
	account3._accN = SAMrowN;
	account3._label = "K_GrossOpSurplus";
	account3._accName = "GrossOpSurplus";
	registerAccountInMaps(account3, SAMrowN);
	addToAccountToGroup(&Account(SAMrowN), "K");

	++SAMrowN;
	auto& account4 = *Accounts()[SAMrowN];
	account4._accN = SAMrowN;
	account4._label = "T_TaxProduction";
	account4._accName = "TaxProduction";
	registerAccountInMaps(account4, SAMrowN);
	addToAccountToGroup(&Account(SAMrowN), "T");

	++SAMrowN;
	auto& account5 = *Accounts()[SAMrowN];
	account5._accN = SAMrowN;
	account5._label = "T_TaxProducts";
	account5._accName = "TaxProducts";
	registerAccountInMaps(account5, SAMrowN);
	addToAccountToGroup(&Account(SAMrowN), "T");

	++SAMrowN;
	auto& account6 = *Accounts()[SAMrowN];
	account6._accN = SAMrowN;
	account6._label = "G_Government";
	account6._accName = "Government";
	registerAccountInMaps(account6, SAMrowN);
	addToAccountToGroup(&Account(SAMrowN), "G");

	++SAMrowN;
	auto& account7 = *Accounts()[SAMrowN];
	account7._accN = SAMrowN;
	account7._label = "H_Households";
	account7._accName = "Households";
	registerAccountInMaps(account7, SAMrowN);
	addToAccountToGroup(&Account(SAMrowN), "H");

	// Now populate final demand imports by sector
	for (int sectorN = 0; sectorN < FIGARO.getNsectors(); ++sectorN)
	{
		string sectorCode = FIGARO._pSectorTypeToCode->at(sectorN + 1); // +1 to skip "RW" pseudo-sector at index 0
		string xLabel = "X_" + sectorCode;

		for (int foreignCountryN = 0; foreignCountryN < getWorld().getFigaro().getNcountries(); ++foreignCountryN)
		{
			if (foreignCountryN == FIGARO._thisCountryType)
				continue;

			// Government imports of this sector
			auto value = FIGARO.getFC_Government().at(FIGARO._thisCountryType).at(foreignCountryN).at(sectorN);
			setRowCol(xLabel, "G_Government") += value;
			(*pRowSum())[firstSectorAccRowN + sectorN] += value;

			// Households + NPISH imports of this sector
			value = FIGARO.getFC_Households().at(FIGARO._thisCountryType).at(foreignCountryN).at(sectorN)
				+ FIGARO.getFC_NPISH().at(FIGARO._thisCountryType).at(foreignCountryN).at(sectorN);
			setRowCol(xLabel, "H_Households") += value;
			(*pRowSum())[firstSectorAccRowN + sectorN] += value;

			// GFCF + ChgInvent imports of this sector
			value = FIGARO.getFC_GFCF().at(FIGARO._thisCountryType).at(foreignCountryN).at(sectorN)
				+ FIGARO.getFC_ChgInvent().at(FIGARO._thisCountryType).at(foreignCountryN).at(sectorN);
			if (value < 0)
				value = 0;
			setRowCol(xLabel, "F_GFCF") += value;
			(*pRowSum())[firstSectorAccRowN + sectorN] += value;
		}
	}

	return SAMaccRowN; // Position after last X_Sector account
}

void CSAM::processExportsToSectorExtSectors(CAccount& account, const CFigaro& FIGARO, int SAMaccRowN)
{
	int colN = FIGARO.getNsectors(); // Start after domestic sector columns

	// First pass: calculate IC exports per foreign sector
	vector<double> icPerSector(FIGARO.getNsectors(), 0.);
	double totalFinalDemand = 0.;
	double totalIC = 0.;

	for (int sectorN = 0; sectorN < FIGARO.getNsectors(); ++sectorN)
	{
		for (int foreignCountryN = 1; foreignCountryN <= getWorld().getFigaro().getNcountries(); ++foreignCountryN)
		{
			if (foreignCountryN == FIGARO._thisCountryType)
				continue;

			icPerSector[sectorN] += FIGARO.getFigProducers().at(foreignCountryN).at(sectorN)
				.getIC().at(FIGARO._thisCountryType).at(SAMaccRowN);
		}
		totalIC += icPerSector[sectorN];
	}

	// Calculate total final demand exports (not sector-attributable)
	for (int foreignCountryN = 1; foreignCountryN <= getWorld().getFigaro().getNcountries(); ++foreignCountryN)
	{
		if (foreignCountryN == FIGARO._thisCountryType)
			continue;

		totalFinalDemand += FIGARO.getFC_Government().at(foreignCountryN).at(FIGARO._thisCountryType).at(SAMaccRowN);
		totalFinalDemand += FIGARO.getFC_Households().at(foreignCountryN).at(FIGARO._thisCountryType).at(SAMaccRowN);
		totalFinalDemand += FIGARO.getFC_NPISH().at(foreignCountryN).at(FIGARO._thisCountryType).at(SAMaccRowN);
		totalFinalDemand += FIGARO.getFC_GFCF().at(foreignCountryN).at(FIGARO._thisCountryType).at(SAMaccRowN);

		double chgInvent = FIGARO.getFC_ChgInvent().at(foreignCountryN).at(FIGARO._thisCountryType).at(SAMaccRowN);
		if (chgInvent >= 0)
			totalFinalDemand += chgInvent;
	}

	// Second pass: fill X_Sector columns (IC + proportional final demand)
	for (int sectorN = 0; sectorN < FIGARO.getNsectors(); ++sectorN)
	{
		account._rowQtties[colN] = icPerSector[sectorN];

		// Distribute final demand exports proportionally to IC share per sector
		if (totalIC > 0)
			account._rowQtties[colN] += totalFinalDemand * icPerSector[sectorN] / totalIC;
		else if (FIGARO.getNsectors() > 0)
			account._rowQtties[colN] += totalFinalDemand / FIGARO.getNsectors();

		++colN;
	}
}

// ===========================  build GFCF account  ==========================================

int CSAM::buildGFCFAccount(const CFigaro& FIGARO, int SAMaccRowN) // Fill account row
{
	_GFCFtype = SAMaccRowN;
	auto& account = *Accounts()[SAMaccRowN];

	for (int colSectorN = 0; colSectorN < FIGARO.getNsectors(); ++colSectorN)
	{
		double value = 0; // Fill all GFCF row with 0's
		account._rowQtties[colSectorN] = value;
		(*pRowSum())[SAMaccRowN] += value;
		(*pColSum())[colSectorN] += value;
	}

	// The SAM account rows are already available. Now we need to fill the GFCF account row with the correct values.
	// Important: GFCF exported is already included in the activity rows of the RX column. We should not include here again.
	// Therefore, we need to calculate the total domestic GFCF value and domestic expenditure of Gov and HH.
	double totalDomesticGFCF = 0;
	double totalDomesticExpenditure_Gov = 0;
	double totalDomesticExpenditure_HH = 0;
	for (int rowSectorN = 0; rowSectorN < getnPXproducerTypes(); ++rowSectorN)
	{
		totalDomesticGFCF += getRowCol(rowSectorN, _GFCFtype);
		totalDomesticExpenditure_Gov += getRowCol(rowSectorN, accNofLabel().at("G_Government"));
		totalDomesticExpenditure_HH += getRowCol(rowSectorN, accNofLabel().at("H_Households"));
	}
	double totalDomesticExpenditure = totalDomesticExpenditure_Gov + totalDomesticExpenditure_HH;
	// Now we want to distribute the total domestic GFCF value to the GFCF row of RW, Gov, HH columns in proportion to their domestic expenditure
	setRowCol("F_GFCF", "G_Government") = totalDomesticGFCF * totalDomesticExpenditure_Gov / totalDomesticExpenditure;
	setRowCol("F_GFCF", "H_Households") = totalDomesticGFCF * totalDomesticExpenditure_HH / totalDomesticExpenditure;

	return SAMaccRowN + 1;
}

// ===========================  build value-added accounts  ==========================================

int CSAM::buildValueAddedAccounts(const CFigaro& FIGARO, int SAMaccRowN)
{
	vector<string> valueAddedLabelsRow = {
		"L_CompEmployees",
		"K_GrossOpSurplus",
		"T_TaxProduction",
		"T_TaxProducts",
	};
	for (const string& RowLabel : valueAddedLabelsRow)
	{
		auto& account = *Accounts()[SAMaccRowN];
		// Value-added accounts
		for (int colSectN = 0; colSectN < FIGARO.getNsectors(); ++colSectN)
		{
			double value = 0;
			if (RowLabel == "L_CompEmployees")
				value = FIGARO.getFigProducers().at(FIGARO._thisCountryType).at(colSectN)._L;
			else if (RowLabel == "K_GrossOpSurplus")
				value = FIGARO.getFigProducers().at(FIGARO._thisCountryType).at(colSectN)._K;
			else if (RowLabel == "T_TaxProduction")
				value = FIGARO.getFigProducers().at(FIGARO._thisCountryType).at(colSectN)._Tproduction;
			else if (RowLabel == "T_TaxProducts")
				value = FIGARO.getFigProducers().at(FIGARO._thisCountryType).at(colSectN)._Tproducts;

			account._rowQtties[colSectN] = value;
			(*pRowSum())[SAMaccRowN] += value;
			(*pColSum())[colSectN] += value;
		}

		// Transfers accounts
		double value = 0;
		if (RowLabel == "L_CompEmployees")
		{
			setRowCol("L_CompEmployees", "G_Government") = FIGARO._pTransfers_Government->at(FIGARO._thisCountryType).ToL;
			setRowCol("L_CompEmployees", "H_Households") = FIGARO._pTransfers_Households->at(FIGARO._thisCountryType).ToL
				+ FIGARO._pTransfers_NPISH->at(FIGARO._thisCountryType).ToL;
			setRowCol("L_CompEmployees", "F_GFCF") = FIGARO._pTransfers_GFCF->at(FIGARO._thisCountryType).ToL
				+ FIGARO._pTransfers_ChgInvent->at(FIGARO._thisCountryType).ToL;
		}
		else if (RowLabel == "K_GrossOpSurplus")
		{
			setRowCol("K_GrossOpSurplus", "G_Government") = FIGARO._pTransfers_Government->at(FIGARO._thisCountryType).ToK;
			setRowCol("K_GrossOpSurplus", "H_Households") = FIGARO._pTransfers_Households->at(FIGARO._thisCountryType).ToK
				+ FIGARO._pTransfers_NPISH->at(FIGARO._thisCountryType).ToK;
			setRowCol("K_GrossOpSurplus", "F_GFCF") = FIGARO._pTransfers_GFCF->at(FIGARO._thisCountryType).ToK
				+ FIGARO._pTransfers_ChgInvent->at(FIGARO._thisCountryType).ToK;
		}
		else if (RowLabel == "T_TaxProduction")
		{
			setRowCol("T_TaxProduction", "G_Government") = FIGARO._pTransfers_Government->at(FIGARO._thisCountryType).ToTproduction;
			setRowCol("T_TaxProduction", "H_Households") = FIGARO._pTransfers_Households->at(FIGARO._thisCountryType).ToTproduction
				+ FIGARO._pTransfers_NPISH->at(FIGARO._thisCountryType).ToTproduction;
			setRowCol("T_TaxProduction", "F_GFCF") = FIGARO._pTransfers_GFCF->at(FIGARO._thisCountryType).ToTproduction
				+ FIGARO._pTransfers_ChgInvent->at(FIGARO._thisCountryType).ToTproduction;
		}
		else if (RowLabel == "T_TaxProducts")
		{
			setRowCol("T_TaxProducts", "G_Government") = FIGARO._pTransfers_Government->at(FIGARO._thisCountryType).ToTproducts;
			setRowCol("T_TaxProducts", "H_Households") = FIGARO._pTransfers_Households->at(FIGARO._thisCountryType).ToTproducts
				+ FIGARO._pTransfers_NPISH->at(FIGARO._thisCountryType).ToTproducts;
			setRowCol("T_TaxProducts", "F_GFCF") = FIGARO._pTransfers_GFCF->at(FIGARO._thisCountryType).ToTproducts
				+ FIGARO._pTransfers_ChgInvent->at(FIGARO._thisCountryType).ToTproducts;
		}

		++SAMaccRowN;
	}

	return SAMaccRowN;
}

// ===========================  build institutional accounts  ==========================================

int CSAM::buildInstitutionalAccounts(int SAMaccRowN)
{
	// Build government account
	SAMaccRowN = buildGovernmentAccount(SAMaccRowN);

	// Build household account
	SAMaccRowN = buildHouseholdAccount(SAMaccRowN);

	return SAMaccRowN;
}
int CSAM::buildGovernmentAccount(int SAMaccRowN)
{
	auto& accGov = *Accounts()[SAMaccRowN];

	for (int colSectN = 0; colSectN < getWorld().getFigaro().getNsectors(); ++colSectN)
	{
		double value = 0;
		accGov._rowQtties[colSectN] = value;
		(*pRowSum())[SAMaccRowN] += value;
		(*pColSum())[colSectN] += value;
	}

	return SAMaccRowN + 1;
}
int CSAM::buildHouseholdAccount(int SAMaccRowN)
{
	auto& accHouse = *Accounts()[SAMaccRowN];

	for (int colSectN = 0; colSectN < getWorld().getFigaro().getNsectors(); ++colSectN)
	{
		double value = 0;
		accHouse._rowQtties[colSectN] = value;
		(*pRowSum())[SAMaccRowN] += value;
		(*pColSum())[colSectN] += value;
	}

	return SAMaccRowN + 1;
}

// ===========================  accounts by COLUMNS  ==========================================

// ===========================  process accounts (columns)  ===========================

void CSAM::processIntermediateConsumption(CAccount& account, const CFigaro& FIGARO, int SAMaccRowN, int FIGsectorN)
{
	for (int colSectorN = 0; colSectorN < FIGARO.getNsectors(); ++colSectorN)
	{
		auto value = FIGARO.getFigProducers().at(FIGARO._thisCountryType).at(colSectorN)
			.getIC().at(FIGARO._thisCountryType).at(FIGsectorN);
		account._rowQtties[colSectorN] = value;
		(*pRowSum())[SAMaccRowN] += value;
		(*pColSum())[colSectorN] += value;
	}
}


void CSAM::processExportsToDisaggCountries(CAccount& account, const CFigaro& FIGARO, int SAMaccRowN)
{
	int colN = FIGARO.getNsectors();

	for (const auto& code : *FIGARO._pDisaggExtSectCountries)
	{
		auto colExtCountry = FIGARO._pCountryCodeToType->at(code);

		for (int sectorN = 0; sectorN < FIGARO.getNsectors(); ++sectorN)
		{
			account._rowQtties[colN] += FIGARO.getFigProducers().at(colExtCountry).at(sectorN)
				.getIC().at(FIGARO._thisCountryType).at(SAMaccRowN);

			colN++;
		}
	}
}
void CSAM::processExportsToAggCountries(CAccount& account, const CFigaro& FIGARO, int SAMaccRowN)
{
	int colN = FIGARO.getNsectors() + (int)FIGARO._pDisaggExtSectCountries->size() * FIGARO.getNsectors();

	for (const auto& extCountryCode : *FIGARO._pAggExtSectCountries)
	{
		auto extCountryN = FIGARO._pCountryCodeToType->at(extCountryCode);

		if (extCountryCode != "RW")
		{
			processRegularAggCountryExports(account, FIGARO, SAMaccRowN, extCountryN, colN);
		}
		else
		{
			processRestOfWorldExports(account, FIGARO, SAMaccRowN, colN);
		}

		colN++;
	}
}
void CSAM::processRegularAggCountryExports(CAccount& account, const CFigaro& FIGARO, int SAMaccRowN, GoodType extCountryN, int colN)
{
	for (int sectorN = 0; sectorN < FIGARO.getNsectors(); ++sectorN)
	{
		account._rowQtties[colN] += FIGARO.getFigProducers().at(extCountryN).at(sectorN)
			.getIC().at(FIGARO._thisCountryType).at(SAMaccRowN);
	}
}
void CSAM::processRestOfWorldExports(CAccount& account, const CFigaro& FIGARO, int SAMaccRowN, int colN)
{
	for (auto RWcountriesColN = 1; RWcountriesColN <= getWorld().getFigaro().getNcountries(); ++RWcountriesColN)
	{
		set<GoodType> allMyCountriesTypes;
		for (const auto& code : *FIGARO._pAllExtSectCountries) {
			allMyCountriesTypes.insert(FIGARO._pCountryCodeToType->at(code));
		}

		if (allMyCountriesTypes.find(RWcountriesColN) != allMyCountriesTypes.end())
			continue;

		// Track the starting value to detect which component might cause issues
		double startValue = account._rowQtties[colN];

		// Process each sector of this RW country
		for (int sectorColIndex = 0; sectorColIndex < FIGARO.getNsectors(); ++sectorColIndex) {
			account._rowQtties[colN] += FIGARO.getFigProducers().at(RWcountriesColN).at(sectorColIndex)
				.getIC().at(FIGARO._thisCountryType).at(SAMaccRowN);
		}

		// Process final consumption components from this RW country
		account._rowQtties[colN] += FIGARO.getFC_Government().at(RWcountriesColN).at(FIGARO._thisCountryType).at(SAMaccRowN);
		account._rowQtties[colN] += FIGARO.getFC_Households().at(RWcountriesColN).at(FIGARO._thisCountryType).at(SAMaccRowN);
		account._rowQtties[colN] += FIGARO.getFC_NPISH().at(RWcountriesColN).at(FIGARO._thisCountryType).at(SAMaccRowN);
		account._rowQtties[colN] += FIGARO.getFC_GFCF().at(RWcountriesColN).at(FIGARO._thisCountryType).at(SAMaccRowN);

		// Handle change in inventories separately as it's often the problematic component
		double chgInvent = FIGARO.getFC_ChgInvent().at(RWcountriesColN).at(FIGARO._thisCountryType).at(SAMaccRowN);

		// Only add change in inventories if it won't make the total negative
		if (account._rowQtties[colN] + chgInvent >= 0) {
			account._rowQtties[colN] += chgInvent;
		}
		else {
			// If adding full ChgInvent would make it negative, add as much as possible
		//MJ	getWorld().ERRORmsg("WARNING: ChgInvent component in processRestOfWorldExports would cause negative value. Adjusting.", true);
			// Keep the value at 0 instead of going negative
			account._rowQtties[colN] = 0;
		}

		// Final check for any negative values (should not happen with the above change)
		if (account._rowQtties[colN] < 0)
		{
			//MJ getWorld().ERRORmsg("WARNING, processRestOfWorldExports: Negative exports to Rest of World. Setting it to 0", true);
			account._rowQtties[colN] = 0;
		}
	}
}

void CSAM::processFinalConsumption(CAccount& account, const CFigaro& FIGARO, int FIGsectorN)
{
	int SAMaccRowN = FIGsectorN; // just for info

	// GFCF+ChgInvent columns
	int colN = _nPXproducerTypes;
	double val = FIGARO.getFC_GFCF().at(FIGARO._thisCountryType).at(FIGARO._thisCountryType).at(FIGsectorN)
		+ FIGARO.getFC_ChgInvent().at(FIGARO._thisCountryType).at(FIGARO._thisCountryType).at(FIGsectorN);
	if (val < 0)
	{
		auto sectorName = getAccNameOfN(FIGsectorN);
		long valM = val * 1.e-6;
		//MJ CWorld::ERRORmsg("WARNING: negative GFCF " + sectorName + " component= " + to_string(valM) + " millions. Setting it to 0", false);
		val = 0;
	}
	account._rowQtties[colN++] = val;

	// Transfer accounts: Value added columns (L, K, T)
	account._rowQtties[colN++] = 0; // L_CompEmployees
	account._rowQtties[colN++] = 0; // K_GrossOpSurplus
	account._rowQtties[colN++] = 0; // T_Production
	account._rowQtties[colN++] = 0; // T_Products

	// Final consumers columns: Gov, HH+NPISH
	account._rowQtties[colN++] = FIGARO.getFC_Government().at(FIGARO._thisCountryType).at(FIGARO._thisCountryType).at(FIGsectorN);
	account._rowQtties[colN++] = FIGARO.getFC_Households().at(FIGARO._thisCountryType).at(FIGARO._thisCountryType).at(FIGsectorN)
		+ FIGARO.getFC_NPISH().at(FIGARO._thisCountryType).at(FIGARO._thisCountryType).at(FIGsectorN);
}

// ==========================================================================
