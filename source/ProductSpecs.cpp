
//  Product.cpp

//   DEPLOYERS v2

#include "./pch.h"

//======================  CProductSpecs  ==================================

CProductSpecs::CProductSpecs(GoodType productTy)
{
	_productType = productTy;
	_pNeededPerFigUnit = nullptr;

	// Product fabrication parameters. They are constant throughout the simulation

	initialize();
}
CProductSpecs::~CProductSpecs() {}

void CProductSpecs::initialize()
{
	_TotalIC = 0.0;
	NeededPerUnit().clear();

	if(&getWorld().getFigaro() != nullptr)
	{
		delete pNeededPerFigUnit();
		pNeededPerFigUnit() = new vector<vector<double>>;
		for (long cn = 0; cn < getWorld().getFigaro().getNcountries(); ++cn)
		{
			auto& vect = *new vector<double>(getWorld().getFigaro().getNsectors(), 0);
			pNeededPerFigUnit()->push_back(vect);
		}
	}

	initMonth() = -1;

	GrossOutput_mu() = 0;
	CompEmployees() = 0;
	GrossOpSurplus() = 0;
	Households() = 0;
	Government() = 0;

	TaxAlcohTab() = 0;
	TransfNetas() = 0;
	PrestDesemp() = 0;
	PrestPension() = 0;
	IRPF() = 0;
	Men65AgrBaj() = 0;
	Men65AgrAlt() = 0;
	Men65NoAgr1q() = 0;
	Men65NoAgr2q() = 0;
	Men65NoAgr3q() = 0;
	Men65NoAgr4q() = 0;
	Men65NoAgr5q() = 0;
	Mas65RurBaj() = 0;
	Mas65RurAlt() = 0;
	Mas65UrbBaj() = 0;
	Mas65UrbAlt() = 0;

	CuentaDeCapital() = 0;
	TaxProducts() = 0;
	TaxProduction() = 0;
	TaxImportCE() = 0;
	TaxImportRW() = 0;
	GrossOpSurplus() = 0;
	TransfersToHouseholds() = 0;
	TransfersToGovernment() = 0;
	TransfersToCapitalAcc() = 0;

	ImportPrice() = 0;
	bIsService() = false;
	bImported() = false;
}

ifstream& operator>>(ifstream& ifstrm, CProductSpecs& productSpecs)
{
	string word;
	double value = 0, GrossOutput = 0;
	productSpecs.bImported() = false;
	productSpecs.bIsService() = false;
	ifstrm >> word;// bracket"{"
	while (ifstrm >> word, word != "}")
	{
		if (word == "bIsService")
		{
			ifstrm >> value;
			productSpecs.bIsService() = (bool)value;
			continue;
		}
		if (word == "bImported")
		{
			ifstrm >> value;
			productSpecs.bImported() = (bool)value;
			continue;
		}

		// -----  SAM inputs  ------------
		if (word == "GrossOutput")
		{
			ifstrm >> GrossOutput;
			productSpecs.GrossOutput_mu() = GrossOutput;
			continue;
		}
		if (word == "TaxProducts")
		{
			ifstrm >> value;
			productSpecs.TaxProducts() = value / GrossOutput;
			continue;
		}
		if (word == "TaxProduction")
		{
			ifstrm >> value;
			productSpecs.TaxProduction() = value / GrossOutput;
			continue;
		}
		if (word == "Households")
		{
			ifstrm >> value;
			productSpecs.Households() = value / GrossOutput;
			continue;
		}
		if (word == "Government")
		{
			ifstrm >> value;
			productSpecs.Government() = value / GrossOutput;
			continue;
		}
		if (word == "GrossOpSurplus")
		{
			ifstrm >> value;
			productSpecs.GrossOpSurplus() = value / GrossOutput;
			continue;
		}
		if (word == "TotalIC")
		{
			ifstrm >> value;
			productSpecs._TotalIC = value;
			continue;
		}

		//  ------------------------------------------

		if (word == "ImportPrice")
		{
			ifstrm >> value;
			productSpecs.ImportPrice() = value;
			continue;
		}
		if (word == "initMonth")
		{
			ifstrm >> value;
			productSpecs.initMonth() = value;
			continue;
		}
		if (word == "Needed_Init")
		{
			ifstrm >> word;// bracket"{"
			string inputName;

			while (ifstrm >> inputName, inputName != "}")
			{
				if (CGoods::GoodNameToType().find(inputName) == CGoods::GoodNameToType().end())
				{
					assert(false);
					CWorld::ERRORmsg("Unexpected input word " + inputName, true);
				}
				ifstrm >> value;
				GoodType inputID = CGoods::GoodNameToType(inputName);
				productSpecs.Needed_Init()[inputID] = value;
			}
			continue;
		}
		if (word == "NeededPerUnit")
		{
			ifstrm >> word;// bracket"{"
			string inputName;

			while (ifstrm >> inputName, inputName != "}")
			{
				if (CGoods::GoodNameToType().find(inputName) == CGoods::GoodNameToType().end())
				{
					assert(false);
					CWorld::ERRORmsg("Unexpected input word " + inputName, true);
				}
				ifstrm >> value;
				GoodType inputID = CGoods::GoodNameToType(inputName);
				productSpecs.NeededPerUnit()[inputID] = value;
			}
			continue;
		}

		CWorld::ERRORmsg("Unexpected word reading PRODUCT_DEFINITIONS "
			+ word, true);
	}

	if (productSpecs.GrossOutput_mu() > 0 && productSpecs.GrossOpSurplus() < 0)
		getWorld().ERRORmsg("If GrossOutput>0 then GrossOpSurplus should also be > 0");

	return ifstrm;
}
ofstream& operator<<(ofstream& ofstrm, const CProductSpecs& productSpecs)
{
	string productName, word, bracket;
	string tab = "", tab1 = "\t", tab2 = "\t\t", tab3 = "\t\t\t",
		tab4 = "\t\t\t\t", tab5 = "\t\t\t\t\t";
	double value = 0, GrossOutput = productSpecs.getGrossOutput_mu();

	ofstrm << tab1 << CGoods::GoodTypeToName(productSpecs.getproductType()) << " {" << endl;
	tab = tab2;

	// Product fabrication parameters
	if (productSpecs.getbIsService())
		ofstrm << tab << "bIsService 1" << endl;
	if (productSpecs.getbImported())
		ofstrm << tab << "bImported 1" << endl;

	if (productSpecs.getGrossOutput_mu() != 0)
		ofstrm << tab << "GrossOutput " << GrossOutput << endl;
	if (productSpecs.getTaxProducts() != 0)
		ofstrm << tab << "TaxProducts " << productSpecs.getTaxProducts() * GrossOutput << endl;
	if (productSpecs.getTaxProduction() != 0)
		ofstrm << tab << "TaxProduction " << productSpecs.getTaxProduction() * GrossOutput << endl;
	if (productSpecs.HouseholdsGoods() != 0)
		ofstrm << tab << "Households " << productSpecs.HouseholdsGoods() * GrossOutput << endl;
	if (productSpecs.getGovernment() != 0)
		ofstrm << tab << "Government " << productSpecs.getGovernment() * GrossOutput << endl;
	if (productSpecs.getGrossOpSurplus() != 0)
		ofstrm << tab << "GrossOpSurplus " <<
		productSpecs.getGrossOpSurplus() * GrossOutput << endl;
	if (productSpecs._TotalIC != 0)
		ofstrm << tab << "TotalIC " <<
		productSpecs._TotalIC << endl;

	if (productSpecs.getbIsService() == true)
		ofstrm << tab << "bIsService 1" << endl;
	if (productSpecs.getImportPrice() != 0)
		ofstrm << tab << "ImportPrice " << productSpecs.getImportPrice() << endl;
	if (productSpecs.getinitMonth() >= 0)
		ofstrm << tab << "initMonth " << productSpecs.getinitMonth() << endl;

	if (productSpecs.getNeeded_Init().size() > 0)
	{
		tab = tab2;
		ofstrm << tab << "Needed_Init {" << endl;
		tab = tab3;

		for (const auto& pair : productSpecs.getNeeded_Init())
			ofstrm << tab << CGoods::GoodTypeToName(pair.first) << " " << pair.second << endl;
		ofstrm << tab2 << "}" << endl;
	}

	if (productSpecs.getNeededPerUnit().size() > 0)
	{
		tab = tab2;
		ofstrm << tab << "NeededPerUnit {" << endl;
		tab = tab3;

		for (const auto& pair : productSpecs.getNeededPerUnit())
			ofstrm << tab << CGoods::GoodTypeToName(pair.first) << " " << pair.second << endl;
		ofstrm << tab2 << "}" << endl;
	}

	tab = tab2;

	ofstrm << tab1 << "}" << endl;

	return ofstrm;
}

GoodType& CProductSpecs::productType() { return _productType; };
double CProductSpecs::getTotalIC() const { return _TotalIC; }
CTypeDoubleMap& CProductSpecs::NeededPerUnit() { return _NeededPerUnit; }
CTypeDoubleMap& CProductSpecs::Needed_Init() { return _Needed_Init; }
const CTypeDoubleMap& CProductSpecs::getNeeded_Init() const { return _Needed_Init; }
double& CProductSpecs::NeededPerUnit(GoodType gType) { return _NeededPerUnit[gType]; }
double& CProductSpecs::Needed_Init(GoodType gType) { return _Needed_Init[gType]; }
double& CProductSpecs::NeededPerFigUnit(GoodType country, GoodType sector)
{
	return (*_pNeededPerFigUnit)[country][sector];
}
const double CProductSpecs::getNeededPerFigUnitOf(GoodType country, GoodType sector) const
{
	return (*_pNeededPerFigUnit)[country][sector];
}
const CTypeDoubleMap& CProductSpecs::getNeededPerUnit() const { return _NeededPerUnit; }
const double CProductSpecs::getNeededPerUnitOf(GoodType gType) const
{
	if (getNeededPerUnit().find(gType) == getNeededPerUnit().end())
		return 0.0;
	else
		return getNeededPerUnit().at(gType);
};
long& CProductSpecs::initMonth() { return _initMonth; };
double& CProductSpecs::ImportPrice() { return _ImportPrice; }
bool& CProductSpecs::bImported() { return _bImported; };
double& CProductSpecs::CompEmployees() { return _CompEmployees; };
double& CProductSpecs::GrossOpSurplus() { return _GrossOpSurplus; };
double& CProductSpecs::GrossOutput_mu() { return _GrossOutput_mu; };
double& CProductSpecs::CuentaDeCapital() { return _CuentaDeCapital; };
double& CProductSpecs::Households() { return _Households; };
double& CProductSpecs::Government() { return _Government; };
double& CProductSpecs::TaxProduction() { return _TaxProduction; };
double& CProductSpecs::TaxProducts() { return _TaxProducts; }
double& CProductSpecs::TaxImportCE() { return _TaxImportCE; }
double& CProductSpecs::TaxImportRW() { return _TaxImportRW; }
double& CProductSpecs::TransfersToHouseholds() { return _TransfersToHouseholds; }
double& CProductSpecs::TransfersToGovernment() { return _TransfersToGovernment; }
double& CProductSpecs::TransfersToCapitalAcc() { return _TransfersToCapitalAcc; }
bool& CProductSpecs::bIsService() { return _bIsService; }
const GoodType& CProductSpecs::getproductType() const { return _productType; };
const long CProductSpecs::getinitMonth() const { return _initMonth; };
const double CProductSpecs::getImportPrice() const { return _ImportPrice; };
const bool CProductSpecs::getbImported() const { return _bImported; };
const double CProductSpecs::getTaxProduction() const { return _TaxProduction; }

const double CProductSpecs::getCompEmployees() const
{
	if (getWorld().getpFigaro() != nullptr)
		return _CompEmployees;

	static bool once = false;
	const auto& accL = getSAM().getAccountGroups().at("L");
	if (accL.size() > 1 && !once)
	{
		getWorld().ERRORmsg("Warning: Max 1 L account in SAM, adding all CompEmployees", false);
		once = true;
	}

	double compEmp = 0;
	for (const auto& pLacc : getSAM().getAccountGroups().at("L"))
		compEmp += getSAM().getRowCol(pLacc->accN(), getproductType());

	compEmp /= getGrossOutput_mu();

	return compEmp;
};

double CProductSpecs::getTaxImportCE() const { return _TaxImportCE; }
double CProductSpecs::getTaxImportRW() const { return _TaxImportRW; }
double CProductSpecs::getTransfersToHouseholds() const { return _TransfersToHouseholds; }
double CProductSpecs::getTransfersToGovernment() const { return _TransfersToGovernment; }
double CProductSpecs::getTransfersToCapitalAcc() const { return _TransfersToCapitalAcc; }
const bool& CProductSpecs::getbIsService() const { return _bIsService; };

///////////////////////////////////////////////////////////////////////////////////////
