
//  CFIGARO.cpp

//   DEPLOYERS v2

#include "pch.h"

//======================  CFigaro  ==================================

CFigProducer::CFigProducer(int Ncountries, int Nsectors)
{
	_L = 0;
	_K = 0;
	_Tproduction = 0;
	_Tproducts = 0;

	_pIC = new vector< vector<double> >;
	for (long sn = 0; sn <= Ncountries; ++sn)
		_pIC->push_back(*new vector<double>(Nsectors + 1, 0));
};
CFigProducer::~CFigProducer() {};

//======================  CFigaro  ==================================

CFigaro::CFigaro()
{
	_Ncountries = 0;
	_Nsectors = 0;
	_pCountryTypeToCode = nullptr;
	_pCountryCodeToType = nullptr;
	_pSectorTypeToCode = nullptr;
	_pSectorCodeToType = nullptr;
	_pDisaggExtSectCountries = nullptr;
	_pAggExtSectCountries = nullptr;
	_pAllExtSectCountries = nullptr;
	_pFigProducers = nullptr;
	_pFC_Government = nullptr;
	_pFC_Households = nullptr;
	_pFC_GFCF = nullptr;
	_pFC_NPISH = nullptr;
	_pFC_ChgInvent = nullptr;

	_pTransfers_Government = nullptr;
	_pTransfers_Households = nullptr;
	_pTransfers_GFCF = nullptr;
	_pTransfers_NPISH = nullptr;
	_pTransfers_ChgInvent = nullptr;
}
CFigaro::~CFigaro() {}

void CFigaro::initialize()
{
	delete _pFigProducers;
	_pFigProducers = new vector< vector<CFigProducer> >;
	for (long cn = 0; cn <= getNcountries(); ++cn)
	{
		auto& vect = *new vector<CFigProducer>;
		for (long sn = 0; sn < getNsectors(); ++sn)
			vect.push_back(*new CFigProducer(getNcountries(), getNsectors()));

		_pFigProducers->push_back(vect);
	}

	delete _pFC_Government;
	_pFC_Government = new vector < vector< vector<double> > >;
	for (long col_cn = 0; col_cn <= getNcountries(); ++col_cn)
	{
		auto& vect2 = *new vector< vector<double> >;
		for (long row_cn = 0; row_cn <= getNcountries(); ++row_cn)
		{
			auto& vect1 = *new vector<double>(getNsectors(), 0);
			vect2.push_back(vect1);
		}
		_pFC_Government->push_back(vect2);
	}

	delete _pFC_Households;
	_pFC_Households = new vector < vector< vector<double> > >;
	for (long col_cn = 0; col_cn <= getNcountries(); ++col_cn)
	{
		auto& vect2 = *new vector< vector<double> >;
		for (long row_cn = 0; row_cn <= getNcountries(); ++row_cn)
		{
			auto& vect1 = *new vector<double>(getNsectors(), 0);
			vect2.push_back(vect1);
		}
		_pFC_Households->push_back(vect2);
	}

	delete _pFC_GFCF;
	_pFC_GFCF = new vector < vector< vector<double> > >;
	for (long col_cn = 0; col_cn <= getNcountries(); ++col_cn)
	{
		auto& vect2 = *new vector< vector<double> >;
		for (long row_cn = 0; row_cn <= getNcountries(); ++row_cn)
		{
			auto& vect1 = *new vector<double>(getNsectors(), 0);
			vect2.push_back(vect1);
		}
		_pFC_GFCF->push_back(vect2);
	}

	delete _pFC_NPISH;
	_pFC_NPISH = new vector < vector< vector<double> > >;
	for (long col_cn = 0; col_cn <= getNcountries(); ++col_cn)
	{
		auto& vect2 = *new vector< vector<double> >;
		for (long row_cn = 0; row_cn <= getNcountries(); ++row_cn)
		{
			auto& vect1 = *new vector<double>(getNsectors(), 0);
			vect2.push_back(vect1);
		}
		_pFC_NPISH->push_back(vect2);
	}

	delete _pFC_ChgInvent;
	_pFC_ChgInvent = new vector < vector< vector<double> > >;
	for (long col_cn = 0; col_cn <= getNcountries(); ++col_cn)
	{
		auto& vect2 = *new vector< vector<double> >;
		for (long row_cn = 0; row_cn <= getNcountries(); ++row_cn)
		{
			auto& vect1 = *new vector<double>(getNsectors(), 0);
			vect2.push_back(vect1);
		}
		_pFC_ChgInvent->push_back(vect2);
	}

	// 	vector<CInstiTransfers>* _pTransfers_Government; // indices: countryCol

	delete _pTransfers_Government;
	_pTransfers_Government = new vector<CInstiTransfers>(getNcountries() + 1);
	delete _pTransfers_Households;
	_pTransfers_Households = new vector<CInstiTransfers>(getNcountries() + 1);
	delete _pTransfers_GFCF;
	_pTransfers_GFCF = new vector<CInstiTransfers>(getNcountries() + 1);
	delete _pTransfers_NPISH;
	_pTransfers_NPISH = new vector<CInstiTransfers>(getNcountries() + 1);
	delete _pTransfers_ChgInvent;
	_pTransfers_ChgInvent = new vector<CInstiTransfers>(getNcountries() + 1);
}

// Add this helper function to CFIGARO.cpp
double CFigaro::readCSVField(ifstream& ifstrm, char separator, bool isLastColumn) const
{
	const int maxchars = 30000;
	char field[maxchars]{};

	if (isLastColumn)
	{
		// Read until newline for last column
		ifstrm.getline(field, maxchars, '\n');
		// Extract numeric value, ignoring any trailing content
		string fieldStr(field);
		size_t pos = fieldStr.find_first_not_of("0123456789.-+eE ");
		if (pos != string::npos)
			fieldStr = fieldStr.substr(0, pos);
		return _units * atof(fieldStr.c_str());
	}
	else
	{
		// Read until separator for other columns
		ifstrm.getline(field, maxchars, separator);
		return _units * atof(field);
	}
}

bool CFigaro::readFIGAROmatrix() // file name with extension
{
	const int maxchars = 30000;
	char field[maxchars]{};
	char firstChar = 0;
	// char separator = ';';
	char separator = ',';

	string producerName, gName, word;
	double value = 0;

	// check input file name
	string FigaroFileName = getDEPData().getMATRIXfileName();
	ifstream& ifstrmFig = *new ifstream(FigaroFileName);
	if (ifstrmFig.fail())
	{
		ifstrmFig.close();
		string msg = "Couldn't read " + FigaroFileName;
		CWorld::ERRORmsg(msg.c_str(), true);
	}
	else
		ifstrmFig.close();

	ifstream ifstrm(FigaroFileName);
	ifstrm.getline(field, maxchars, separator);
	word = string(field);

	// ==============  Read types of Countries and Sectors from rowLabels  ====================================

	_Ncountries = 0;
	_Nsectors = 0;
	long CountryType = 0, SectorType = 0;
	string prevCountryCode = "";
	delete _pCountryTypeToCode;
	delete _pCountryCodeToType;
	delete _pSectorTypeToCode;
	delete _pSectorCodeToType;

	_pCountryTypeToCode = new vector<string>;
	_pCountryCodeToType = new map<string, AgentType>;
	string countryCode = "RW"; // Rest of World
	_pCountryTypeToCode->push_back(countryCode);
	(*_pCountryCodeToType)[countryCode] = (AgentType)(*_pCountryCodeToType).size();

	_pSectorTypeToCode = new vector<string>;
	_pSectorCodeToType = new map<string, AgentType>;
	string sectorCode = "RW";// Rest of World, not simulated as a country
	_pSectorTypeToCode->push_back(sectorCode);
	(*_pSectorCodeToType)[sectorCode] = (AgentType)(*_pSectorCodeToType).size(); // 0
	// Simulated country types are 1, 2,...

	while (ifstrm.getline(field, maxchars, separator), word = string(field), true)
	{
		auto separ = word.find('_');

		auto countryCode = word.substr(0, separ);

		// check if this is the beginning of Final Consummers
		if (word.substr(2, 4) == "_P3_" // all countries and sectors have been read
			&& _pCountryCodeToType->find(countryCode) != _pCountryCodeToType->end())
		{
			ifstrm.getline(field, maxchars); // read and discard the rest of current line
			break;
		}

		// start of new country?
		size_t countryTy = (*_pCountryCodeToType).size();
		if (countryCode != prevCountryCode)
		{
			++_Ncountries;
			_pCountryTypeToCode->push_back(countryCode);
			(*_pCountryCodeToType)[countryCode] = (AgentType)countryTy;
			prevCountryCode = countryCode;
		}

		// count sectors while on the first country

		auto sectorCode = word.substr(separ + 1);
		size_t sectorTy = (*_pSectorCodeToType).size();
		if (_Ncountries == 1)
		{
			++_Nsectors;
			_pSectorTypeToCode->push_back(sectorCode);
			(*_pSectorCodeToType)[sectorCode] = (AgentType)sectorTy;
		}
	}

	_thisCountryType = _pCountryCodeToType->at(_thisCountryCode);

	initialize();

	// ======  Read Activity rows, each column is a Producer IC, FinalConsumers at the end of row =======

	for (int rowCountry = 1; rowCountry <= getNcountries(); ++rowCountry)
	{
		for (int rowSector = 0; rowSector < getNsectors(); ++rowSector)
		{
			ifstrm.getline(field, maxchars, separator);

			string label = field;

			// read this country_sector row: first IC columns
			for (int colCountry = 1; colCountry <= getNcountries(); ++colCountry)
			{
				for (int colSector = 0; colSector < getNsectors(); ++colSector)
				{
					if (colCountry == 1 && colSector == 1)
					{
						(FigProducers()[rowCountry][rowSector]).label() = label;
						(FigProducers()[rowCountry][rowSector])._countryType = rowCountry;
						(FigProducers()[rowCountry][rowSector])._sectorType = rowSector;
					}

					ifstrm.getline(field, maxchars, separator);
					value = _units * atof(field);
					(FigProducers()[colCountry][colSector]).IC()[rowCountry][rowSector] = value;
				}
			}

			// and last columns of row are the FinalConsumers submatrix of each country

			for (int colCountry = 1; colCountry <= getNcountries(); ++colCountry)
			{
				ifstrm.getline(field, maxchars, separator);
				value = _units * atof(field);
				FC_Government()[colCountry][rowCountry][rowSector] = value;

				ifstrm.getline(field, maxchars, separator);
				value = _units * atof(field);
				FC_Households()[colCountry][rowCountry][rowSector] = value;

				ifstrm.getline(field, maxchars, separator);
				value = _units * atof(field);
				FC_NPISH()[colCountry][rowCountry][rowSector] = value;

				ifstrm.getline(field, maxchars, separator);
				value = _units * atof(field);
				FC_GFCF()[colCountry][rowCountry][rowSector] = value;

				//ifstrm.getline(field, maxchars, separator);
				//value = _units * atof(field);
				value = readCSVField(ifstrm, separator, colCountry == getNcountries());
				FC_ChgInvent()[colCountry][rowCountry][rowSector] = value;
			}
		}
	}

	// ======  Read Value Added (first country columns) and Transfers (last country columns) rows  ================================================

	// T_TaxProducts W2_D21X31 row
	ifstrm.getline(field, maxchars, separator);
	string Figarolabel = field;

	assert(Figarolabel == "W2_D21X31");
	// Value Added submatrix (first country columns)
	for (int colCountry = 1; colCountry <= getNcountries(); ++colCountry)
	{
		for (int colSector = 0; colSector < getNsectors(); ++colSector)
		{
			ifstrm.getline(field, maxchars, separator);
			value = _units * atof(field);
			(FigProducers()[colCountry][colSector])._Tproducts = value;
		}
	}

	// Transfers submatrix (last country columns)
	for (int colCountry = 1; colCountry <= getNcountries(); ++colCountry)
	{
		ifstrm.getline(field, maxchars, separator);
		value = _units * atof(field);
		_pTransfers_Government->at(colCountry).ToTproducts = value;
		ifstrm.getline(field, maxchars, separator);
		value = _units * atof(field);
		_pTransfers_Households->at(colCountry).ToTproducts = value;
		ifstrm.getline(field, maxchars, separator);
		value = _units * atof(field);
		_pTransfers_NPISH->at(colCountry).ToTproducts = value;
		ifstrm.getline(field, maxchars, separator);
		value = _units * atof(field);
		_pTransfers_GFCF->at(colCountry).ToTproducts = value;

		//ifstrm.getline(field, maxchars, separator);
		//value = _units * atof(field);
		value = readCSVField(ifstrm, separator, colCountry == getNcountries());
		_pTransfers_ChgInvent->at(colCountry).ToTproducts = value;
	}

	ifstrm.getline(field, maxchars); //W2_OP_RES read and discard the rest of current line

	ifstrm.getline(field, maxchars); //W2_OP_NRES read and discard the rest of current line

	// L_CompEmployees row
	ifstrm.getline(field, maxchars, separator);
	Figarolabel = field;
	assert(Figarolabel == "W2_D1");
	// SAMlabel = "L_CompEmployees";
	for (int colCountry = 1; colCountry <= getNcountries(); ++colCountry)
	{
		for (int colSector = 0; colSector < getNsectors(); ++colSector)
		{
			ifstrm.getline(field, maxchars, separator);
			value = _units * atof(field);
			(FigProducers()[colCountry][colSector])._L = value;
		}
	}
	ifstrm.getline(field, maxchars); // read and discard the rest of current line, all 0's

	// T_TaxProduction row
	ifstrm.getline(field, maxchars, separator);
	Figarolabel = field;
	assert(Figarolabel == "W2_D29X39");
	// SAMlabel = "T_TaxProduction";
	for (int colCountry = 1; colCountry <= getNcountries(); ++colCountry)
	{
		for (int colSector = 0; colSector < getNsectors(); ++colSector)
		{
			ifstrm.getline(field, maxchars, separator);
			value = _units * atof(field);
			(FigProducers()[colCountry][colSector])._Tproduction = value;
		}
	}
	ifstrm.getline(field, maxchars); // read and discard the rest of current line, all 0's

	// K_GrossOpSurplus row
	ifstrm.getline(field, maxchars, separator);
	Figarolabel = field;
	assert(Figarolabel == "W2_B2A3G");
	// SAMlabel = "K_GrossOpSurplus";
	for (int colCountry = 1; colCountry <= getNcountries(); ++colCountry)
	{
		for (int colSector = 0; colSector < getNsectors(); ++colSector)
		{
			ifstrm.getline(field, maxchars, separator);
			value = _units * atof(field);
			(FigProducers()[colCountry][colSector])._K = value;
		}
	}
	ifstrm.getline(field, maxchars); // read and discard the rest of current line, all 0's

	ifstrm.close();

	return true;
}

