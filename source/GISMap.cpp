
//  GISMap.cpp

//   DEPLOYERS v2

#include "./pch.h"


//======================  CIntVector  ===================================

CIntVector::CIntVector()
{
	_x = -1;
	_y = -1;
}
CIntVector::CIntVector(IntCoord cx, IntCoord cy)
{
	_x = cx;
	_y = cy;
}
CIntVector::CIntVector(const CIntVector& vObj)
{
	_x = vObj._x;
	_y = vObj._y;
}
CIntVector::~CIntVector()
{
}

ofstream& operator<<(ofstream& ofstrm, const CIntVector& v)
{
	ofstrm << " ix= " << v._x << " iy=" << v._y;

	return ofstrm;
}

ifstream& operator>>(ifstream& ifstrm, CIntVector& v)
{
	string word;
	IntCoord ix, iy;

	ifstrm >> word >> ix >> word >> iy;
	v = CIntVector(ix, iy);

	return ifstrm;
}

CIntVector CIntVector::operator+(const CIntVector& vObj) const
{
	CIntVector vTemp;
	vTemp.Setx(_x + vObj._x);
	vTemp.Sety(_y + vObj._y);
	return vTemp;
}
CIntVector CIntVector::operator-(const CIntVector& vObj) const
{
	CIntVector vTemp;
	vTemp.Setx(_x - vObj._x);
	vTemp.Sety(_y - vObj._y);
	return vTemp;
}
void CIntVector::operator+=(const CIntVector& vObj)
{
	_x += vObj._x;
	_y += vObj._y;
}
void CIntVector::operator-=(const CIntVector& vObj)
{
	_x -= vObj._x;
	_y -= vObj._y;
}
CIntVector CIntVector::operator/(const  IntCoord cNum) const
{
	assert(cNum != 0);
	CIntVector vResult(*this);
	vResult._x /= cNum;
	vResult._y /= cNum;
	return vResult;
}
CIntVector CIntVector::operator*(const  IntCoord cNum) const
{
	CIntVector vResult(*this);
	vResult._x *= cNum;
	vResult._y *= cNum;
	return vResult;
}
void CIntVector::operator/=(const  IntCoord cNum)
{
	assert(cNum != 0);
	_x /= cNum;
	_y /= cNum;
}
void CIntVector::operator*=(const  IntCoord cNum)
{
	_x /= cNum;
	_y /= cNum;
}
bool CIntVector::operator==(const CIntVector& vObj) const
{
	return(vObj._x == _x && vObj._y == _y);
}

IntCoord CIntVector::Module() const
{
	return((IntCoord)sqrt(_x * _x + _y * _y));
}
inline IntCoord CIntVector::Module2() const
{
	return(_x * _x + _y * _y);
}
inline CIntVector CIntVector::Unitary() const
{
	return *this / Module();
}

//======================  CExtVector  ===================================

CExtVector::CExtVector()
{
	_x = -1;
	_y = -1;
}
CExtVector::CExtVector(ExtCoord cx, ExtCoord cy)
{
	_x = cx;
	_y = cy;
}
CExtVector::CExtVector(const CExtVector& vObj)
{
	_x = vObj._x;
	_y = vObj._y;
}
CExtVector::~CExtVector()
{
}

ofstream& operator<<(ofstream& ofstrm, const CExtVector& v)
{
	ofstrm << " KmX= " << v._x << " KmY=" << v._y;

	return ofstrm;
}

ifstream& operator>>(ifstream& ifstrm, CExtVector& v)
{
	string word;
	ExtCoord ix, iy;

	ifstrm >> word >> ix >> word >> iy;
	v = CExtVector(ix, iy);

	return ifstrm;
}

CExtVector CExtVector::operator+(const CExtVector& vObj) const
{
	CExtVector vTemp;
	vTemp.Setx(_x + vObj._x);
	vTemp.Sety(_y + vObj._y);
	return vTemp;
}
CExtVector CExtVector::operator-(const CExtVector& vObj) const
{
	CExtVector vTemp;
	vTemp.Setx(_x - vObj._x);
	vTemp.Sety(_y - vObj._y);
	return vTemp;
}
void CExtVector::operator+=(const CExtVector& vObj)
{
	_x += vObj._x;
	_y += vObj._y;
}
void CExtVector::operator-=(const CExtVector& vObj)
{
	_x -= vObj._x;
	_y -= vObj._y;
}
CExtVector CExtVector::operator/(const  ExtCoord cNum) const
{
	assert(cNum != 0);
	CExtVector vResult(*this);
	vResult._x /= cNum;
	vResult._y /= cNum;
	return vResult;
}
CExtVector CExtVector::operator*(const  ExtCoord cNum) const
{
	CExtVector vResult(*this);
	vResult._x *= cNum;
	vResult._y *= cNum;
	return vResult;
}
void CExtVector::operator/=(const  ExtCoord cNum)
{
	assert(cNum != 0);
	_x /= cNum;
	_y /= cNum;
}
void CExtVector::operator*=(const  ExtCoord cNum)
{
	_x /= cNum;
	_y /= cNum;
}
bool CExtVector::operator==(const CExtVector& vObj) const
{
	return(vObj._x == _x && vObj._y == _y);
}

ExtCoord CExtVector::Module() const
{
	return(sqrt(_x * _x + _y * _y));
}
inline ExtCoord CExtVector::Module2() const
{
	return(_x * _x + _y * _y);
}
inline CExtVector CExtVector::Unitary() const
{
	return *this / Module();
}

//======================  CLocation  ===================================

CLocation::CLocation() { _pBackward = nullptr; _pForward = nullptr; _pCell = nullptr; }
CLocation::CLocation(ExtCoord cx, ExtCoord cy) : CExtVector(cx, cy)
{
	_pBackward = nullptr; _pForward = nullptr; _pCell = nullptr;
}
CLocation::CLocation(const CExtVector& v) : CExtVector(v)
{
	_pBackward = nullptr; _pForward = nullptr; _pCell = nullptr;
}
CLocation::~CLocation() {}

ofstream& operator<<(ofstream& ofstrm, const CLocation& loc)
{
	ofstrm << " Loc {"
		<< " KmX= " << loc.Getx() << " KmY= " << loc.Gety();

	ofstrm << " AgType_AgID";

	if (loc.getpForward())
		ofstrm << " Fwd " << loc.getpForward()->getAgentType() << " " << loc.getpForward()->getID();
	else
		ofstrm << " Fwd " << UndefAgentType << " " << UndefAgentID;

	if (loc.getpBackward())
		ofstrm << " Bwd " << loc.getpBackward()->getAgentType() << " " << loc.getpBackward()->getID();
	else
		ofstrm << " Bwd " << UndefAgentType << " " << UndefAgentID;

	ofstrm << " }";

	return ofstrm;
}

ifstream& operator>>(ifstream& ifstrm, CLocation& loc)
{
	string word, word1;
	ExtCoord KmX, KmY;
	AgentID FwdAgType = UndefAgentType;
	AgentID FwdID = UndefAgentID;
	AgentID BwdAgType = UndefAgentType;
	AgentID BwdID = UndefAgentID;

	ifstrm >> word >> word1; // " Loc {"
	ifstrm >> word >> KmX >> word1 >> KmY;
	loc.Setx(KmX);
	loc.Sety(KmY);

	ifstrm >> word; // " AgType_AgID"
	ifstrm >> word >> FwdAgType >> FwdID
		>> word >> BwdAgType >> BwdID;

	// Pointers are set in World::LoadWorld:	Map().initializeLinksFrom(fileNameExt);

	ifstrm >> word; // " }"

	return ifstrm;
}

//======================  CMapCell  ===================================

CMapCell::CMapCell()
{
	_type = 0;
	_pFirstAgent = nullptr;
	MaxlastGoodsDemanded() = 0;
	if (getpSAM() != nullptr)
		MaxlastGoodsDemanded() = getSAM().getnPProducerTypes();
	lastGoodsDemandedIndex() = 0;
};
CMapCell::~CMapCell() {};

ofstream& operator<<(ofstream& ofstrm, const CMapCell& mapcell)
{
	ofstrm << " " << mapcell._type;

	return ofstrm;
}

ifstream& operator>>(ifstream& ifstrm, CMapCell& mapcell)
{
	CellType type;
	ifstrm >> type;
	mapcell._type = type;

	return ifstrm;
}

CellType& CMapCell::type() { return _type; };
CAgent*& CMapCell::pFirstAgent() { return _pFirstAgent; };

const CellType& CMapCell::gettype() const { return _type; };
const CAgent* CMapCell::getpFirstAgent() { return _pFirstAgent; }
void CMapCell::setlastGoodsDemanded(GoodType gType)
{
	if (gType < 0 || gType >= getSAM().getnPProducerTypes())
		return;

	if (lastGoodsDemanded().size() < MaxlastGoodsDemanded())
		lastGoodsDemanded().push_back(gType);
	else
	{
		auto index = lastGoodsDemandedIndex() % MaxlastGoodsDemanded();
		lastGoodsDemanded()[index] = gType;
	}

	lastGoodsDemandedIndex()++;
};

//======================  CGISMap  ===================================

map< CellType, CellName > CGISMap::_CellTypeToName;
map< CellName, CellType > CGISMap::_CellNameToType;

CGISMap::CGISMap()
{
	pMapCells() = nullptr;
	pNeighbors() = nullptr;
	pNeiIndivs() = nullptr;
	pNeiProducers() = nullptr;
}
CGISMap::~CGISMap()
{
}

ofstream& operator<<(ofstream& ofstrm, const CGISMap& map)
{
	ofstrm << "\n YmaxKms " << map.getYmaxKms();
	ofstrm << "\n XmaxKms " << map.getXmaxKms();
	ofstrm << "\n CellSideKm " << map.getCellSideKms() << endl;

	// 1. Write CellTypes names

	ofstrm << "\n CellTypes {";

	for (const auto& cell : map.CellTypeToName())
		ofstrm << "\n " << cell.first << " " << cell.second;

	ofstrm << "\n }\n";

	// 2. Write CellTypes map

	ofstrm << "\n Rows " << map.getmaxYN()
		<< "\n Columns " << map.getmaxXN() << endl;

	const auto& MapCells = map.getMapCells();
	for (long yn = map.getmaxYN() - 1; yn >= 0; yn--)
	{
		ofstrm << "\n";
		for (long xn = 0; xn < map.getmaxXN(); xn++)
		{
			const auto& mapcell = map.getMapCells()[yn][xn];
			ofstrm << mapcell;
		}
	}

	// 3. Write lastGoodsDemanded in each cell
	ofstrm << endl << " lastGoodsDemanded";

	for (long yn = map.getmaxYN() - 1; yn >= 0; yn--)
	{
		ofstrm << "\n";
		for (long xn = 0; xn < map.getmaxXN(); xn++)
		{
			const auto& mapcell = map.getMapCells()[yn][xn];

			ofstrm << " {";
			for (const auto& gType : mapcell._lastGoodsDemanded)
				ofstrm << " " << gType;
			ofstrm << " Idx " << mapcell._lastGoodsDemandedIndex;
			ofstrm << " }";
		}
	}

	return ofstrm;
}

ifstream& operator>>(ifstream& ifstrm, CGISMap& map)
{
	string word, name, bracket;
	ifstrm >> word; // "{"

	ifstrm >> word >> map.YmaxKms() >> word >> map.XmaxKms() >> word >> map.CellSideKms();

	map.initialize(); // set up Neighbors and MapCells arrays

	// 1. Read CellTypes names

	map.CellTypeToName().clear();
	map.CellNameToType().clear();
	char ch;
	ifstrm >> word >> bracket; // "CellTypes {"
	while (ifstrm >> word, word != "}")
	{
		ch = *word.c_str(); // single character: 3 * # ...
		ifstrm >> name;
		map.CellTypeToName()[ch] = name;
		map.CellNameToType()[name] = ch;
	}

	// 2. Read CellTypes map

	long myn, mxn;
	ifstrm >> word >> myn >> word >> mxn;
	assert(map.maxYN() == myn && map.maxXN() == mxn);

	for (long yn = map.getmaxYN() - 1; yn >= 0; yn--)
	{
		for (long xn = 0; xn < map.getmaxXN(); xn++)
		{
			auto& mapcell = map.MapCells()[yn][xn];
			ifstrm >> mapcell;
		}
	}

	// 3. Read lastGoodsDemanded in each cell

	GoodType gType;
	ifstrm >> word;
	if (word == "lastGoodsDemanded")
	{
		for (long yn = map.getmaxYN() - 1; yn >= 0; yn--)
		{
			for (long xn = 0; xn < map.getmaxXN(); xn++)
			{
				auto& mapcell = map.MapCells()[yn][xn];

				ifstrm >> name; // " {"
				while (ifstrm >> name, name != "}")
				{
					if (name == "Idx")
						ifstrm >> mapcell.lastGoodsDemandedIndex();
					else
					{
						gType = atoi(name.c_str());
						mapcell.lastGoodsDemanded().push_back(gType);
					}
				}
			}
		}
	}

	ifstrm >> word; // "}"

	return ifstrm;
}

void CGISMap::initialize()
{
	if (!(CellSideKms() > 0 && CellSideKms() < XmaxKms() && CellSideKms() < YmaxKms()))
		CWorld::ERRORmsg(
			"CellSideKms() > 0 && CellSideKms() < XmaxKms()"
			" && CellSideKms() < YmaxKms() is FALSE", true);

	maxXN() = ceil(XmaxKms() / CellSideKms());
	maxYN() = ceil(YmaxKms() / CellSideKms());

	delete[] pNeighbors();
	pNeighbors() = new vector< vector<CNeighbors*> >(getmaxYN(), vector<CNeighbors*>(getmaxXN(), nullptr));
	delete[] pNeiIndivs();
	pNeiIndivs() = new vector< vector<CNeiIndivs*> >(getmaxYN(), vector<CNeiIndivs*>(getmaxXN(), nullptr));
	delete[] pNeiProducers();
	pNeiProducers() = new vector< vector<CNeiProducers*> >(getmaxYN(), vector<CNeiProducers*>(getmaxXN(), nullptr));
	for (long yn = 0; yn < getmaxYN(); yn++)
		for (long xn = 0; xn < getmaxXN(); xn++)
		{
			(*pNeighbors())[yn][xn] = new CNeighbors();
			(*pNeiIndivs())[yn][xn] = new CNeiIndivs();
			(*pNeiProducers())[yn][xn] = new CNeiProducers();
		}

	delete[] pMapCells();
	pMapCells() = new vector< vector<CMapCell> >(getmaxYN(), vector<CMapCell>(getmaxXN(), CMapCell()));
}
void CGISMap::initializeLinksFrom(string fileNameExt)
{
	ifstream& ifstrm = *new ifstream();
	ifstrm.open(fileNameExt);

	string word, prodName, bracket;
	AgentID AgID = UndefAgentID;
	AgentID AgType = UndefAgentType;
	AgentID FwdAgType = UndefAgentType;
	AgentID FwdID = UndefAgentID;
	AgentID BwdAgType = UndefAgentType;
	AgentID BwdID = UndefAgentID;

	long nProducers;
	long nWorkers;

	// 1. Read and insert Producers

	while (ifstrm >> word, !ifstrm.eof() && word != "nProducers");
	if (ifstrm.eof())
		return;

	ifstrm >> nProducers >> word >> nWorkers;

	if (nProducers > 0)
		ifstrm >> word >> nProducers >> bracket; // "Producers: nn {"


	for (long nProd = 0; nProd < nProducers; ++nProd)
	{
		CProducer* pProducer = World().Producers().at(nProd);
		if (pProducer == nullptr)
			continue;

		CProducer& producer = *pProducer;
		prodName = CProducer::getProducerLabelOfType(producer.getAgentType());
		while (ifstrm >> word, word != prodName);

		ifstrm >> bracket >> word >> AgID;
		AgType = CProducer::getProducerTypeOfLabel(prodName);
		assert(AgType == producer.getAgentType());
		while (ifstrm >> word, word != "Fwd");

		ifstrm >> FwdAgType >> FwdID >> word >> BwdAgType >> BwdID;

		CLocation& loc = producer.Location();

		// Link this Producer to mapcell co-residents

		if (FwdAgType == WorkerType)
			loc.pForward() = World().pWorkers()->at(FwdID);
		else if (FwdAgType > WorkerType)
		{
			loc.pForward() = World().pProducers()->at(FwdID);
			assert(loc.pForward()->getAgentType() == FwdAgType);
		}

		if (BwdAgType == WorkerType)
			loc.pBackward() = World().pWorkers()->at(BwdID);
		else if (BwdAgType > WorkerType)
		{
			loc.pBackward() = World().pProducers()->at(BwdID);
			assert(loc.pBackward()->getAgentType() == BwdAgType);
		}

		loc.pCell() = &World().pMap()->MapCell(loc);
		if (BwdAgType == UndefAgentType) // Cell top agent
			World().pMap()->MapCell(loc).pFirstAgent() = &producer;
	}

	// 2. Read and insert Workers

	if (nWorkers > 0)
		while (ifstrm >> word, word != "Workers");
	ifstrm >> word >> word; // " nn {"

	for (long nIndiv = 0; nIndiv < nWorkers; ++nIndiv)
	{
		// Get a ref to next Worker

		while (ifstrm >> word, word != "Indiv");
		ifstrm >> word >> word >> AgID; // " { ID"
		CWorker& indiv = *World().Workers().at(AgID);

		while (ifstrm >> word, word != "Fwd");

		ifstrm >> FwdAgType >> FwdID >> word >> BwdAgType >> BwdID;

		// Link this Worker to mapcell co-residents

		CLocation& loc = indiv.Location();

		if (FwdAgType == WorkerType)
			loc.pForward() = World().pWorkers()->at(FwdID);
		else if (FwdAgType > WorkerType)
			loc.pForward() = World().pProducers()->at(FwdID);
		assert(loc.pForward() == nullptr
			|| loc.pForward()->getAgentType() == FwdAgType);

		if (BwdAgType == WorkerType)
			loc.pBackward() = World().pWorkers()->at(BwdID);
		else if (BwdAgType > WorkerType)
			loc.pBackward() = World().pProducers()->at(BwdID);
		assert(loc.pBackward() == nullptr
			|| loc.pBackward()->getAgentType() == BwdAgType);

		loc.pCell() = &World().pMap()->MapCell(loc);
		if (BwdAgType == UndefAgentType) // Cell top agent
			World().pMap()->MapCell(loc).pFirstAgent() = &indiv;
	}

	ifstrm.close();
}
CIntVector CGISMap::toIntVector(const CExtVector extVect)
{
	return CIntVector(
		floor(getmaxXN() * (extVect.Getx() / getmaxXN())),
		floor(getmaxYN() * (extVect.Gety() / getmaxYN())));
}

CMapCell& CGISMap::MapCell(const CExtVector extVect)
{
	IntCoord ix = floor(getmaxXN() * (extVect.Getx() / getmaxXN()));
	IntCoord iy = floor(getmaxYN() * (extVect.Gety() / getmaxYN()));
	return (*pMapCells())[iy][ix];
}
const CMapCell& CGISMap::getMapCell(const CExtVector extVect)
{
	const auto& cell = MapCell(extVect);
	return cell;
}

CExtVector CGISMap::getCentralPosition()
{
	CExtVector position = CExtVector(XmaxKms() * 0.5, YmaxKms() * 0.5);
	return position;
}

CExtVector CGISMap::getRandomPosition()
{
	CExtVector position =
		CExtVector(XmaxKms() * getRandom01(),
			YmaxKms() * getRandom01());
	return position;
}
CExtVector CGISMap::getRandomPositionOnType(CellType type)
{
	CExtVector position;
	while (true)
	{
		position = getRandomPosition();
		CMapCell cell = getMapCell(position);
		if (cell.gettype() == type)
			break;
	}
	return position;
}
CExtVector CGISMap::getRandomPositionWithin(ExtCoord Xmax, ExtCoord Ymax)
{
	CExtVector position =
		CExtVector(min(XmaxKms(), Xmax) * getRandom01(),
			min(YmaxKms(), Ymax) * getRandom01());
	return position;
}
CExtVector CGISMap::getRandomPositionNear(CExtVector refPosition, CellType type)
{
	ExtCoord hintDistance = getInputParameter("ProbabDistanceToOwner");
	if (hintDistance <= 0)
		CWorld::ERRORmsg("ProbabDistanceToOwner should be > 0", true);

	ExtCoord distance;
	CExtVector nearPosition;

	long iter = 0;
	long maxIter = getInputParameter("MaxFindLocationIters");
	while (iter++ < maxIter)
	{
		nearPosition = getRandomPositionOnType(type);

		distance = (nearPosition - refPosition).Module();
		double rnd01 = getRandom01();
		auto ex = exp(-distance / hintDistance);

		if (rnd01 < ex)
			break;
	}
	if (iter >= maxIter)
		return CExtVector(-1.0, -1.0);
	else
		return nearPosition;
}

void CGISMap::InsertInCell(CAgent* pAgent)
{
	CExtVector xy = pAgent->getLocation();
	CIntVector ixiy = toIntVector(xy);
	CMapCell& mapCell = (*pMapCells())[ixiy.Gety()][ixiy.Getx()];

	if (mapCell.pFirstAgent() == nullptr)
	{
		mapCell.pFirstAgent() = pAgent;
		pAgent->Location().pCell() = &mapCell;

		pAgent->Location().pBackward() = nullptr;
		pAgent->Location().pForward() = nullptr;
	}
	else
	{
		// insert pAgent just below FirstAgent
		CAgent*& pNeighbor = mapCell.pFirstAgent();

		pAgent->Location().pForward() = pNeighbor->Location().pForward();
		if (pNeighbor->Location().pForward() != nullptr)
			(pNeighbor->Location().pForward())->Location().pBackward() = pAgent;

		pNeighbor->Location().pForward() = pAgent;
		pAgent->Location().pBackward() = pNeighbor;
		assert(pAgent != pNeighbor);
		pAgent->Location().pCell() = &mapCell;
	}
}
void CGISMap::RemoveFromCell(CAgent* pAgent)
{
	// Unlink from Cell

	auto pBack = pAgent->Location().pBackward();
	if (pBack != nullptr) // if pAgent is not first in mapCell
		pBack->Location().pForward() = pAgent->Location().pForward();
	else
		pAgent->Location().pCell()->pFirstAgent() = pAgent->Location().pForward();

	auto pForw = pAgent->Location().pForward();
	if (pForw != nullptr) // if pAgent is not last in mapCell
		pForw->Location().pBackward() = pAgent->Location().pBackward();

	pAgent->Location().pForward() = nullptr;
	pAgent->Location().pBackward() = nullptr;
	pAgent->Location().pCell() = nullptr;
}

void CGISMap::clearlastGoodsDemanded()
{
	for (long y0 = 0; y0 < getmaxYN(); y0++)
	{
		for (long x0 = 0; x0 < getmaxXN(); x0++)
		{
			MapCells()[y0][x0].lastGoodsDemanded().clear();
			MapCells()[y0][x0].lastGoodsDemandedIndex() = 0;
		}
	}
}

void CGISMap::updateAllNeighbors()
{
	for (long yn = 0; yn < getmaxYN(); yn++)
		for (long xn = 0; xn < getmaxXN(); xn++)
		{
			(*pNeighbors())[yn][xn]->clear();
			(*pNeiIndivs())[yn][xn]->clear();
			(*pNeiProducers())[yn][xn]->clear();
		}

	CMapCell cell0;
	CMapCell cell1;
	IntCoord d = ceil(getInputParameter("MaxNeighborsDistanceKm")
		/ getWorld().getGISMap().getCellSideKms());
	long MaxInteractingAgents =
		getSAM().getnPProducerTypes() * getInputParameter("MaxInteractsFactor");

	// Assign, as neighbors of cell0 Agents, all Agents in cell0 and
	// in its surrounding cells within a square radius d

	for (long y0 = 0; y0 < getmaxYN(); y0++)
	{
		for (long x0 = 0; x0 < getmaxXN(); x0++)
		{
			cell0 = MapCells()[y0][x0];

			// Same neighbors for all residents of cell0

			CAgent* pAgent0 = cell0.pFirstAgent(); // start only from the first agent in mapCell
			if (pAgent0 != nullptr) // and follow the agents' linked list
			{
				typedef pair<long, long> XY;
				vector<XY> rndXY;

				IntCoord xyn = 0;
				IntCoord x1, y1;
				for (y1 = max(y0 - d, (IntCoord)0); y1 < min(y0 + d, getmaxYN()); y1++)
					for (x1 = max(x0 - d, (IntCoord)0); x1 < min(x0 + d, getmaxXN()); x1++)
						rndXY.push_back(XY(x1, y1));

				std::shuffle(rndXY.begin(), rndXY.end(), myRandomEngine());

				for (const auto& xy1 : rndXY)
				{
					x1 = xy1.first;
					y1 = xy1.second;
					double d1 = sqrt(x1 * x1 + y1 * y1);
					if (getRandom01() * d1 > double(d)) // 0412
						continue;

					cell1 = MapCells()[y1][x1];
					if (cell1.pFirstAgent() == nullptr) // empty mapCell
						continue;

					CAgent* pAgent1 = cell1.pFirstAgent(); // first agent in mapCell
					while (pAgent1 != nullptr) // for all residents of cell1
					{
						if (pAgent1 != pAgent0) // cell1 also goes over cell0
						{
							if ((*Neighbors()[y0][x0]).size() < MaxInteractingAgents) // 0412
								(Neighbors()[y0][x0])->push_back(pAgent1);

							if (pAgent1->IsWorker()
								&& (*NeiIndivs()[y0][x0]).size() < MaxInteractingAgents)
								(NeiIndivs()[y0][x0])->push_back((CWorker*)pAgent1);

							if (pAgent1->IsProducer()
								&& (*NeiProducers()[y0][x0]).size() < MaxInteractingAgents)
								(NeiProducers()[y0][x0])->push_back((CProducer*)pAgent1);
						}

						pAgent1 = pAgent1->Location().pForward();
					}
				}
				std::shuffle((*Neighbors()[y0][x0]).begin(), (*Neighbors()[y0][x0]).end(), myRandomEngine());
				std::shuffle((*NeiIndivs()[y0][x0]).begin(), (*NeiIndivs()[y0][x0]).end(), myRandomEngine());
				std::shuffle((*NeiProducers()[y0][x0]).begin(), (*NeiProducers()[y0][x0]).end(), myRandomEngine());
			}
		}
	}
}
const CNeiIndivs& CGISMap::getNeiIndivs(CExtVector xy)
{
	return *NeiIndivs()[toIntVector(xy).Gety()][toIntVector(xy).Getx()];
}
const CNeiProducers& CGISMap::getNeiProducers(CExtVector xy)
{
	return *NeiProducers()[toIntVector(xy).Gety()][toIntVector(xy).Getx()];
}
