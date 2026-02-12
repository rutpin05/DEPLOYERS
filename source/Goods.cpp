// Goods.cpp

//   DEPLOYERS v2  CGood

#include "./pch.h"

ofstream& operator<<(ofstream& ofstrm, const CTypeQttyVector& vect)
{
	for (GoodType gType = 0; gType < vect.size(); ++gType)
		ofstrm << " " << vect[gType];

	return ofstrm;
};
ifstream& operator>>(ifstream& ifstrm, CTypeQttyVector& vect)
{
	for (GoodType gType = 0; gType < vect.size(); ++gType)
		ifstrm >> vect[gType];

	return ifstrm;
};

//==================   CGoods   ===========================

ofstream& operator<<(ofstream& ofstrm, const CGoods& cgoods)
{
	ofstrm << "{";
	for (const auto& pair : cgoods)
	{
		ofstrm << " " << getSAM().getAccNameOfN(pair.first) << " " << pair.second;
	}
	ofstrm << " }";

	return ofstrm;
}
ifstream& operator>>(ifstream& ifstrm, CGoods& goods)
{
	string name, bracket;
	GoodQtty qtty = -1;
	ifstrm >> bracket; // "{"

	CGood good;
	while (ifstrm >> name && name != "}")
	{
		ifstrm >> qtty;
		goods[getSAM().getAccNofName(name)] = qtty;
	}

	return ifstrm;
}

// ____________________ static ____________________________

vector<string>			CGoods::_GoodTypeToName;
map< string, GoodType >	CGoods::_GoodNameToType;

void CGoods::clearGoodsDefinitions() // static
{
	_GoodTypeToName.clear();
	_GoodNameToType.clear();
}
void CGoods::defineNewGoodType(const string& gName) // static
{
	_GoodNameToType[gName] = static_cast<GoodType>(_GoodTypeToName.size());
	_GoodTypeToName.push_back(gName);
}
vector<string>& CGoods::vGoodTypeToName()
{
	return _GoodTypeToName;
}
const vector<string>& CGoods::GoodTypeToName() // static
{
	return _GoodTypeToName;
}
const string& CGoods::GoodTypeToName(GoodType gTy) // static
{
	return _GoodTypeToName[gTy];
}
map<string, GoodType>& CGoods::mGoodNameToType()
{
	return _GoodNameToType;
}
const map<string, GoodType>& CGoods::GoodNameToType() // static
{
	return _GoodNameToType;
}
GoodType CGoods::GoodNameToType(const string& gname) // static
{
	return _GoodNameToType.at(gname);
}

// ____________________ end of static ____________________________

CGoods::CGoods() = default;
CGoods::~CGoods() = default;

GoodQtty& CGoods::operator[](GoodType gType)
{
	auto result = find(gType);
	if (result == end())
		this->emplace(gType, 0);

	return at(gType);
}
GoodQtty CGoods::operator()(GoodType gType) const
{
	auto result = find(gType);
	if (result != end())
		return result->second;
	else
		return 0;
}

CGoods& CGoods::operator=(const CGoods& cgds)
{
	if (this != &cgds) {
		clear();
		for (const auto& cgd : cgds)
			(*this)[cgd.first] = cgd.second;
	}
	return *this;
}
CGoods CGoods::operator+(const CGoods& cgds) const
{
	CGoods ret;
	for (const auto& cgd : cgds)
		ret[cgd.first] = (*this).at(cgd.first) + cgd.second;

	return ret;
}
CGoods& CGoods::operator+=(const CGoods& cgds)
{
	for (const auto& cgd : cgds)
		(*this)[cgd.first] += cgd.second;

	return *this;
}
CGoods CGoods::operator-(const CGoods& cgds) const
{
	CGoods ret;
	for (const auto& cgd : cgds)
		ret[cgd.first] = (*this).at(cgd.first) - cgd.second;

	return ret;
}
CGoods& CGoods::operator-=(const CGoods& cgds)
{
	for (const auto& cgd : cgds)
		(*this)[cgd.first] -= cgd.second;

	return *this;
}

//====================  CGood  ================================================

CGood::CGood() : _type(static_cast<GoodType>(-1)), _quantity(static_cast<GoodQtty>(-1)) {}
CGood::CGood(GoodType gTy, GoodQtty q) : _type(gTy), _quantity(q) {}
CGood::CGood(const CGood& cg) : _type(cg.getType()), _quantity(cg.getQuantity()) {}

GoodType& CGood::type()
{
	return _type;
}
GoodType CGood::getType() const
{
	return _type;
}

GoodQtty& CGood::quantity()
{
	return _quantity;
}
GoodQtty CGood::getQuantity() const
{
	return _quantity;
}

bool CGood::operator==(const CGood& g) const
{
	return (getType() == g.getType() && getQuantity() == g.getQuantity());
}
bool CGood::operator!=(const CGood& g) const
{
	return !(*this == g);
}
bool CGood::operator>(const CGood& g) const
{
	return (getType() == g.getType() && getQuantity() > g.getQuantity());
}
bool CGood::operator<(const CGood& g) const
{
	return (getType() == g.getType() && getQuantity() < g.getQuantity());
}
bool CGood::operator>=(const CGood& g) const
{
	return (getType() == g.getType() && getQuantity() >= g.getQuantity());
}
bool CGood::operator<=(const CGood& g) const
{
	return (getType() == g.getType() && getQuantity() <= g.getQuantity());
}
CGood& CGood::operator=(const CGood& g)
{
	if (this != &g) {
		if (getType() == g.getType())
			quantity() = g.getQuantity();
		else
			assert(false);
	}
	return *this;
}
CGood CGood::operator+(const CGood& g) const
{
	if (getType() != g.getType())
		assert(false);

	return CGood(getType(), getQuantity() + g.getQuantity());
}
CGood CGood::operator-(const CGood& g) const
{
	if (getType() != g.getType())
		assert(false);

	return CGood(getType(), getQuantity() - g.getQuantity());
}
CGood& CGood::operator+=(const CGood& g)
{
	if (getType() == g.getType())
		quantity() += g.getQuantity();
	else
		assert(false);

	return *this;
}
CGood& CGood::operator-=(const CGood& g)
{
	if (getType() == g.getType())
		quantity() -= g.getQuantity();
	else
		assert(false);

	return *this;
}
CGood& CGood::operator=(GoodQtty qtty)
{
	quantity() = qtty;

	return *this;
}
CGood CGood::operator*(double d) const
{
	return CGood(getType(), getQuantity() * d);
}
CGood CGood::operator/(double d) const
{
	assert(d != 0);
	return CGood(getType(), getQuantity() / d);
}
CGood CGood::operator+(GoodQtty q) const
{
	return CGood(getType(), getQuantity() + q);
}
CGood CGood::operator-(GoodQtty q) const
{
	return CGood(getType(), getQuantity() - q);
}
bool CGood::operator==(GoodQtty q) const
{
	return (getQuantity() == q);
}
bool CGood::operator!=(GoodQtty q) const
{
	return (getQuantity() != q);
}
bool CGood::operator>(GoodQtty q) const
{
	return (getQuantity() > q);
}
bool CGood::operator>=(GoodQtty q) const
{
	return (getQuantity() >= q);
}
bool CGood::operator<(GoodQtty q) const
{
	return (getQuantity() < q);
}
bool CGood::operator<=(GoodQtty q) const
{
	return (getQuantity() <= q);
}
CGood& CGood::operator*=(double d)
{
	quantity() *= d;
	return *this;
}
CGood& CGood::operator/=(double d)
{
	assert(d != 0);
	quantity() /= d;
	return *this;
}
CGood& CGood::operator+=(GoodQtty q)
{
	quantity() += q;
	return *this;
}
CGood& CGood::operator-=(GoodQtty q)
{
	quantity() -= q;
	return *this;
}
CGoods& operator+=(CGoods& thisGoods, const CGood& cgd)
{
	GoodType gType = cgd.getType();
	thisGoods[gType] = thisGoods[gType] + cgd.getQuantity();

	return thisGoods;
}
CGoods& operator-=(CGoods& thisGoods, const CGood& cgd)
{
	GoodType gType = cgd.getType();
	thisGoods[gType] = thisGoods[gType] - cgd.getQuantity();

	return thisGoods;
}
CGood operator*(double d, const CGood& g)
{
	return CGood(g.getType(), g.getQuantity() * d);
}
/*
*/
ofstream& operator<<(ofstream& ofstrm, const CGood& cgood)
{
	ofstrm << CGoods::GoodTypeToName(cgood.getType()) << " " << cgood.getQuantity();

	return ofstrm;
}
ifstream& operator>>(ifstream& ifstrm, CGood& good)
{
	string typeName;
	ifstrm >> typeName >> good.quantity();
	good.type() = CGoods::GoodNameToType().at(typeName);

	return ifstrm;
}

///////////////////////////////////////////////////////////////////////////////////////
