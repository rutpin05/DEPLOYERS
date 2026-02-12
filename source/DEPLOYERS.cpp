
// DEPLOYERS v2

// DEPLOYERS.cpp : Defines the entry point for the application.

#include "./pch.h"

using namespace std;

// =======================================================================================

#ifndef no_init_all
#define no_init_all deprecated
#endif

// Globals & static

mt19937* _pmyRandomEngine;
mt19937*& pmyRandomEngine() { return _pmyRandomEngine; };
mt19937& myRandomEngine() { return *pmyRandomEngine(); };

map<thread::id, thread*> Threads_ID;

double const getInputParameter(string param)
{
	if (getDEPData().InputParameters().find(param) == getDEPData().InputParameters().end())
		return 0;
	else
		return getDEPData().InputParameters().at(param);
}
CWorld*& pWorld() { return _pWorld; };
const CWorld* getpWorld() { return pWorld(); }
CWorld& World() { return *pWorld(); };
const CWorld& getWorld() { return *pWorld(); }
long& InitMonth() { return pWorld()->_InitMonth; }
long getInitMonth() { return pWorld()->_InitMonth; }
long currMonth() { return pWorld()->_currMonth; };

// Simulation stage tracking functions
SimulationStage currSimulationStage() {
	long month = currMonth();
	if (month <= 0)
		return SimulationStage::Initialization;
	
	long startCalibAt = (long)getInputParameter("StartCalibrationAt");
	long assistedUpto = (long)getInputParameter("AssistedProductionUpto");
	long finishCalibAt = getDEPData().getFinishCalibrationAt();
	
	if (month < startCalibAt)
		return SimulationStage::PreCalibration;
	else if (month <= assistedUpto)
		return SimulationStage::AssistedCalibration;
	else if (month <= finishCalibAt)
		return SimulationStage::TransitionCalibration;
	else
		return SimulationStage::RealMarketSimulation;
}

const char* getSimulationStageName(SimulationStage stage) {
	switch (stage) {
		case SimulationStage::Initialization:        return "Initialization";
		case SimulationStage::PreCalibration:        return "PreCalibration";
		case SimulationStage::AssistedCalibration:   return "AssistedCalibration";
		case SimulationStage::TransitionCalibration: return "TransitionCalibration";
		case SimulationStage::RealMarketSimulation:  return "RealMarketSimulation";
		default:                                     return "Unknown";
	}
}

bool isPreCalibration() {
	return currSimulationStage() == SimulationStage::PreCalibration;
}

bool isAssistedCalibration() {
	return currSimulationStage() == SimulationStage::AssistedCalibration;
}

bool isTransitionCalibration() {
	return currSimulationStage() == SimulationStage::TransitionCalibration;
}

bool isRealMarketSimulation() {
	return currSimulationStage() == SimulationStage::RealMarketSimulation;
}

bool isCalibrationPhase() {
	SimulationStage stage = currSimulationStage();
	return stage == SimulationStage::AssistedCalibration || 
	       stage == SimulationStage::TransitionCalibration;
}

const double getRandom01() { return pWorld()->getRandom01(); };

GoodQtty doubleToGQtty(double d)
{
	GoodQtty gQ = (GoodQtty)floor(d);
	if (getRandom01() < d - gQ)
		++gQ;
	return gQ;
}

ofstream& operator<<(ofstream& ofstrm, const SortedAgent& sortedAgnt)
{
	ofstrm << " " << sortedAgnt._qtty << " " << sortedAgnt._ID << " " << sortedAgnt._aTy;
	return ofstrm;
}

ifstream& operator>>(ifstream& ifstrm, SortedAgent& sortedAgnt)
{
	ifstrm >> sortedAgnt._qtty >> sortedAgnt._ID >> sortedAgnt._aTy;
	return ifstrm;
}

// typedef tuple<GoodQtty, AgentID, AgentType> SortedAgent: sort Producers by their MonthlyActivityMonth
bool operator>(const SortedAgent& ag1, const SortedAgent& ag2)
{
	if (ag1._qtty > ag2._qtty)
		return true;
	else if (ag1._qtty < ag2._qtty)
		return false;
	// equal qtty, sort by aType
	else if (ag1._aTy > ag2._aTy)
		return true;
	else if (ag1._aTy < ag2._aTy)
		return false;
	// equal qtty and aType, sort by ID
	else if (ag1._ID > ag2._ID)
		return true;
	else
		return false;
}
bool operator<(const SortedAgent& ag1, const SortedAgent& ag2)
{
	if (ag1._qtty < ag2._qtty)
		return true;
	else if (ag1._qtty > ag2._qtty)
		return false;
	// equal qtty, sort by aType
	else if (ag1._aTy < ag2._aTy)
		return true;
	else if (ag1._aTy > ag2._aTy)
		return false;
	// equal qtty and aType, sort by ID
	else if (ag1._ID < ag2._ID)
		return true;
	else
		return false;
}

// typedef tuple<double, AgentID, AgentType> ShareHolder
bool operator>(const ShareHolder& ag1, const ShareHolder& ag2)
{
	if (ag1._val > ag2._val)
		return true;
	else if (ag1._val < ag2._val)
		return false;
	// equal qtty, sort by aType
	else if (ag1._aTy > ag2._aTy)
		return true;
	else if (ag1._aTy < ag2._aTy)
		return false;
	// equal qtty and aType, sort by ID
	else if (ag1._ID > ag2._ID)
		return true;
	else
		return false;
}
bool operator<(const ShareHolder& ag1, const ShareHolder& ag2)
{
	if (ag1._val < ag2._val)
		return true;
	else if (ag1._val > ag2._val)
		return false;
	// equal qtty, sort by aType
	else if (ag1._aTy < ag2._aTy)
		return true;
	else if (ag1._aTy > ag2._aTy)
		return false;
	// equal qtty and aType, sort by ID
	else if (ag1._ID < ag2._ID)
		return true;
	else
		return false;
}

bool operator>(const AgentSortedByFirst& p1, const AgentSortedByFirst& p2)
{
	if (p1.first > p2.first)
		return true;
	else
		return false;
}
bool operator<(const AgentSortedByFirst& p1, const AgentSortedByFirst& p2)
{
	if (p1.first < p2.first)
		return true;
	else
		return false;
}

bool operator>(const pair<double, AgentID>& p1, const pair<double, AgentID>& p2) {
	// Compare the first elements of the pairs
	if (p1.first > p2.first)
		return true;
	else if (p1.first < p2.first)
		return false;
	// If first elements are equal, compare the second elements:
	else
		return p1.second > p2.second;
};

map< string, vector<CAccount*> >& AccountGroups() { return pSAM()->AccountGroups(); };
const map< string, vector<CAccount*> >& getAccountGroups() { return pSAM()->AccountGroups(); };
vector<CAccount*>& getAccountGroups(string str) { return pSAM()->AccountGroups()[str]; };
void addToAccountToGroup(CAccount* pESect, string str)
{
	long index = (long)SAM().AccountGroups()[str].size();
	pESect->accGroupIndex() = index;
	SAM().AccountGroups()[str].push_back(pESect);
};

CAccount* pAccount(GoodType gType)
{
	return &SAM().Account(gType);
};
const CAccount* getpAccount(GoodType gType)
{
	return &SAM().Account(gType);
};
CAccount& Account(GoodType gType) { return *pAccount(gType); };
const CAccount& getAccount(GoodType gType) { return *getpAccount(gType); };

CGovernment*& pGovernment() { return pWorld()->_pGovernment; };
CGovernment& Government() { return *pWorld()->_pGovernment; };
const CGovernment* getpGovernment() { return pWorld()->_pGovernment; };
const CGovernment& getGovernment() { return *pWorld()->_pGovernment; };

map<GoodType, CExtSect*>& ExtSectors() { return pWorld()->_pExtSect; };
const map<GoodType, CExtSect*>& getExtSectors() { return pWorld()->_pExtSect; };
void addExtSect(GoodType gType, CExtSect* pESect) { World()._pExtSect[gType] = pESect; };

CExtSect*& pExtSect(GoodType gType)
{
	return pWorld()->_pExtSect.at(gType);
};
const CExtSect* getpExtSect(GoodType gType)
{
	if (getSAM().IsExtSectType(gType))
		return pExtSect(gType);
	else
		return nullptr;
};
CExtSect& ExtSect(GoodType gType) { return *pExtSect(gType); };
const CExtSect& getExtSect(GoodType gType) { return *getpExtSect(gType); };

CData*& pDEPData() { return _pCData; };
const CData* getpDEPData() { return _pCData; };
CData& DEPData() { return *_pCData; };
const CData& getDEPData() { return *pDEPData(); };

ofstream*& pLogFile() { return _pLogf; }
ofstream& LogFile() { return *_pLogf; }
ofstream& ExternalSectorsIO() { return *_pExternalSectorsIO; };
long& DebugLevel() { return _DebugLevel; };

// =========================================================================

#ifdef DEPLOYERS_GRAPHICS

#include "GlgMain.h"// Defines a platform-specific program entry point.

GlgAppContext* _pAppContext;
GlgAppContext& AppContext() { return *_pAppContext; }

CColorDefinitions _ColorDefinitions;
CColorDefinitions& ColorDefinitions() { return _ColorDefinitions; };
void defineColors()
{
	//  ----------------------- ColorDefinitions  -----------------------

	ColorDefinitions().clear();
	//   avoid nearly black and nearly white colors
	double r, g, b, s, min = 0.1, max = 1.0 - 2. * min, f = 0.7;
	long Ncolors = 300;
	for (long type = 0; type < Ncolors; ++type)
	{
		r = min + max * GlgRand(0., 1.);
		g = min + max * GlgRand(0., 1.);
		b = min + max * GlgRand(0., 1.);

		s = f * (r + g + b);

		r /= s;
		g /= s;
		b /= s;

		ColorDefinitions()[type] = CColor(r, g, b);
	}
}

const GlgLong update_interval = 10;// 100; // ms
extern "C" void OnTimerEvent(CDeployersGUI* deployersGUI, GlgLong* timer_id) {
	// Invoked periodically to update drawing with new data values.
	deployersGUI->UpdateDrawing_OnTimer();

	deployersGUI->TimerID = // Restart the timer
		GlgAddTimeOut(AppContext, update_interval,
			(GlgTimerProc)OnTimerEvent, deployersGUI);
};

CDeployersGUI* _pglg_deployersGUI;
static CDeployersGUI*& pglg_deployersGUI() { return _pglg_deployersGUI; };
static CDeployersGUI& glg_deployersGUI() { return *pglg_deployersGUI(); };

int GlgMain(int argc, char* argv[], GlgAppContext InitAppContext)
{
	GlgSessionC glg_session(False, InitAppContext, argc, argv);

	GlgAppContext AppContext = glg_session.GetAppContext();
	_pAppContext = &AppContext;

	//while (true);

	delete pDEPData();
	pDEPData() = new CData();

	auto dot_pos = string::npos;
	string defaultinputFName = "__InitialInput"; // Default input file name if no argument is provided.

	// If a command-line argument is provided, use it as the input file name.
	string inputFName = argv[0];
	if (inputFName == "GLG") // DeployersDebug compilation with graphics
	{
		if (argc > 1)
			inputFName = argv[1];
		else
			inputFName = defaultinputFName; // Default input file name if no argument is provided.

		//CWorld::ERRORmsg("InputFileNameD() = " + inputFName, false);
	}
	else // DeployersRelease compilation with graphics
	{
		dot_pos = inputFName.rfind(".exe");
		if (dot_pos != string::npos)
		{
			if (argc > 1)
			{
				inputFName = argv[1];
			}
			else
			{
				inputFName = defaultinputFName; // Default input file name if no argument is provided.
			}
		}

		//CWorld::ERRORmsg("argc = " + std::to_string(argc) + " " + argv[0] + " InputFileNameR() = " + inputFName, false);
	}

	dot_pos = inputFName.rfind(".dep");
	if (dot_pos != string::npos)
		inputFName.erase(dot_pos, 4);

	//CWorld::ERRORmsg("InputFileName() = " + inputFName, false);

	DEPData().InputFileName() = inputFName;

	DEPData().readInputParameters();

	// ---------  GUI  ------------------------------------------

	//CDeployersGUI glg_deployersGUI;
	pglg_deployersGUI() = new CDeployersGUI();

	// Load a GLG  drawing from a file.
	glg_deployersGUI().LoadWidget("DeployersGUI.glg");
	if (glg_deployersGUI().IsNull())
	{
		GlgError(GLG_USER_ERROR, (char*)"Can't load drawing.");
		exit(GLG_EXIT_ERROR);
	}

	// Set widget dimensions
	const auto& InputParameters = getDEPData().InputParameters();
	if (DEPData().getPlotsInputParameters().size() > 0)
	{
		const auto& ControlWindow = DEPData().getPlotsInputParameters().at("Control");
		glg_deployersGUI().SetSize((InputParameters.at("PlotsX0") + ControlWindow.PlotX0) * InputParameters.at("PlotsXSize"),
			(InputParameters.at("PlotsY0") + ControlWindow.PlotY0) * InputParameters.at("PlotsYSize"),
			InputParameters.at("ControlXSize"), InputParameters.at("ControlYSize"));
	}
	else
	{
		glg_deployersGUI().SetSize((InputParameters.at("PlotsX0")) * InputParameters.at("PlotsXSize"),
			(InputParameters.at("PlotsY0")) * InputParameters.at("PlotsYSize"),
			InputParameters.at("ControlXSize"), InputParameters.at("ControlYSize"));
	}

	// Set initial drawing parameters.
	glg_deployersGUI().Initialize();

	// Enable input callback.
	glg_deployersGUI().EnableCallback(GLG_INPUT_CB);
	glg_deployersGUI().EnableCallback(GLG_TRACE_CB);

	// Display GLG window.
	glg_deployersGUI().InitialDraw();

	// Start periodic dynamic updates.
	glg_deployersGUI().StartUpdates();

	return (long)GlgMainLoop(AppContext);
}

// =======================    Initialization    ============================

CDeployersGUI::CDeployersGUI(void)
{
	TimerID = 0;
	GUIinitialized() = false;
}
CDeployersGUI::~CDeployersGUI(void)
{
}
void CDeployersGUI::SetSize(GlgLong x = 5, GlgLong y = 300,
	GlgLong width = 1000, GlgLong height = 1000)
{
	SetResource("Point1", 0., 0., 0.);
	SetResource("Point2", 0., 0., 0.);

	SetResource("Screen/XHint", (double)x);
	SetResource("Screen/YHint", (double)y);
	SetResource("Screen/WidthHint", (double)width);
	SetResource("Screen/HeightHint", (double)height);
}
void CDeployersGUI::Initialize()
{
	SetResource("ScreenName", "DEPLOYERS-v2.0");
	SetResource("RunButton/OnState", 0.);
	SetResource("NIndivText/Value", 0.);
	SetResource("MaxYearsText/Value", 0.);
	SetResource("CurrYearText/Value", 0.);
	SetResource("CurrStep/Value", 0.);
	SetResource("FileNameText/TextString", DEPData().InputFileName().c_str());


	if (getDEPData().InputParameters().size() == 0)
		MessageStatus("Couldn't read __InitialInput.dep");
	else
		MessageStatus("No initial *.dep file loaded");

	Update();

	GUIinitialized() = true;
}
void CDeployersGUI::StartUpdates()
{
	TimerID = GlgAddTimeOut(AppContext, update_interval,
		(GlgTimerProc)OnTimerEvent,
		(GlgAnyType)this);
}
void CDeployersGUI::StopUpdates()
{
	if (TimerID)
	{
		GlgRemoveTimeOut(TimerID);
		TimerID = 0;
	}
}
void CDeployersGUI::MessageStatus(string msg)
{
	SetResource("MessageText/TextString", msg.c_str());
}

// =======================    Input    ====================================

void CDeployersGUI::Input(GlgObjectC& viewport, GlgObjectC& message)
{
	CONST char
		* format,
		* action,
		* origin;

	// Get the message's format, action and origin.
	message.GetResource("Format", &format);
	message.GetResource("Action", &action);
	message.GetResource("Origin", &origin);

	// Handle window closing. May use viewport's name.
	if (strcmp(format, "Window") == 0 &&
		strcmp(action, "DeleteWindow") == 0)
		exit(0);

	// Input event occurred in a button.
	if (strcmp(format, "Button") == 0)
	{
		//Push button events.
		if (strcmp(action, "Activate") == 0)
		{
			// User selected a Quit button: exit the program.
			if (strcmp(origin, "QuitButton") == 0)
			{
				if (pWorld() != nullptr && World().getbRunning()
					&& !World().getbPaused())
					return;

				exit(0);
			}
			else if (strcmp(origin, "LoadWorld_Button") == 0)
			{
				if (pWorld() != nullptr && World().getbRunning()
					&& !World().getbPaused())
					return;

				const char* input_FileName[1000];
				GetResource("FileNameText/TextString", input_FileName);
				string inputFName = *input_FileName;
				//if (inputFName == "")
				//	return;

				DEPData().InputFileName() = string(inputFName);
				GUILoadWorld();
			}
			else if (strcmp(origin, "SaveWorld_Button") == 0)
			{
				if (pWorld() == nullptr || World().getbRunning()
					&& !World().getbPaused())
					return;

				GUISaveWorld();
			}
		}

		//Toggle button events.
		else if (strcmp(action, "ValueChanged") == 0)
		{
			if (strcmp(origin, "RunButton") == 0)
			{
				double value;
				message.GetResource("OnState", &value);
				RunWorldButton(value);
			}
		}

		Update();
	}

	// Input event occurred in an input mapCell (long, double, text).
	else if (strcmp(format, "Text") == 0)
	{
		if (strcmp(origin, "MaxYearsText") == 0
			&& strcmp(action, "ValueChanged") == 0)
		{
			World().ERRORmsg("not set up to read MaxYearsText", true);
			double value;
			GetResource("MaxYearsText/Value", &value);
			DEPData().NYears() = (long)value;
		}
	}
}

// =======================    Display    ====================================

void CDeployersGUI::UpdateDrawing_OnTimer()
{
	if (pWorld() != nullptr)
	{
		long month = currMonth();
		bool run = true;
		if (World().getbRunning() && !World().getbPaused())
		{
			for (auto pPlot : Plots()) // sync Plots
				if (pPlot != nullptr && pPlot->getcounter() <= month)
				{
					run = false;
					break;
				}

			if (run)
			{
				SetResource("RunButton/OnState", 1.);
				World().RunSimulation(); // one month
			}
		}

		if (World().getbFinished())
		{
			UpdateDrawing_WhileRunning();

			SetResource("RunButton/OnState", 0.);
			string str = " ...simulation FINISHED";
			MessageStatus(str);
		}
		else if (World().getbRunning()
			&& !World().getbPaused())
			UpdateDrawing_WhileRunning();
	}
	else if ((bool)getInputParameter("Autorun"))
	{
		double run = 1.;
		SetResource("RunButton/OnState", run);
		RunWorldButton(run);
	}

	Update();
	Sync();
}
void CDeployersGUI::Update_AfterLoadWorld(void)
{
	for (auto pPlot : Plots())
		if (pPlot != nullptr)
			pPlot->InitAfterH();

	UpdateDrawing_WhileRunning();

	MessageStatus("File read OK");

	Update();
}
void CDeployersGUI::UpdateDrawing_WhileRunning(void)
{
	SetResource("NIndivText/Value", getWorld().getNWorkers());
	SetResource("MaxYearsText/Value", getDEPData().getNYears());
	SetResource("CurrYearText/Value", getDEPData().getCurrYear());
	SetResource("CurrStep/Value", currMonth());
}

void CDeployersGUI::UpdateAllPlotXScales(double newNYears)
{
	// Update X-axis scale for all plot windows when NYears changes dynamically
	for (auto pPlot : Plots())
	{
		if (pPlot != nullptr)
			pPlot->UpdateXScale(newNYears);
	}
}

// Global function to update plot scales from World.cpp
void UpdateAllPlotScales(double newNYears)
{
	if (_pglg_deployersGUI != nullptr)
		_pglg_deployersGUI->UpdateAllPlotXScales(newNYears);
}

// =======================    Control    ====================================

void CDeployersGUI::RunWorldButton(double value)
{
	if (pWorld() == nullptr)
	{
		bool loadOK = GUILoadWorld();
		if (!loadOK)
		{
			CWorld::ERRORmsg("Could'n LoadWorld", true);
			//SetResource("RunButton/OnState", 0.);
			//return;
		}
	}

	switch ((long)value)
	{
	case 0: // pause
		if (World().getbRunning() && !World().getbPaused())
		{
			World().bPaused() = true;
			MessageStatus(" PAUSED ");
		}
		else
		{
			SetResource("RunButton/OnState", 1.);
		}
		break;
	case 1: // start / continue
		if (!World().getbRunning()) // start
		{
			if (!World().getbFinished()
				&& getDEPData().getNYears() > 0) //  start run
			{
				World().bRunning() = true;
				MessageStatus(" RUNNING... ");
			}
			else
			{
				SetResource("RunButton/OnState", 0.);
			}
		}
		else // continue
		{
			World().bPaused() = false;
			MessageStatus("RUNNING... ");
		}
		break;
	default: break;
	}
}

bool CDeployersGUI::GUISaveWorld(void)
{
	bool ok = World().SaveWorld();
	if (ok)
	{
		MessageStatus("File write OK");
		return true;
	}
	else
	{
		MessageStatus("File write FAILED");
		return false;
	}
}

bool CDeployersGUI::GUILoadWorld()
{
	string inputFName = getDEPData().getInputFileName();
	if (inputFName.empty())
	{
		// As a fallback, if the input file name is not set, get it from the GUI.
		const char* input_FileName_cstr[1000];
		GetResource("FileNameText/TextString", input_FileName_cstr);
		inputFName = *input_FileName_cstr;
		if (inputFName.empty())
			return false; // No file name available.
		DEPData().InputFileName() = inputFName;
	}

	string fileNameExt = inputFName + ".dep";

	// check input file name
	ifstream ifstrm(fileNameExt);
	if (ifstrm.fail())
	{
		ifstrm.close();
		string msg = "Couldn't read " + fileNameExt + ", reading __InitialInput.dep instead";
		CWorld::ERRORmsg(msg.c_str(), false);
		DEPData().InputFileName() = "__InitialInput";
		fileNameExt = getDEPData().getInputFileName() + ".dep";
		ifstrm.open(fileNameExt);
		if (ifstrm.fail())
		{
			CWorld::ERRORmsg("Couldn't read " + fileNameExt + ", EXIT", false);
			return false;
		}
	}
	ifstrm.close();

	DEPData().readInputParameters();

	delete pWorld();
	pWorld() = new CWorld();
	bool ok = World().LoadWorld();
	if (!ok)
	{
		CWorld::ERRORmsg("Couldn't Load World from " + fileNameExt + ", EXIT", true);
		return false;
	}

	// After loading, the simulation name might be updated from the file's contents.
	// We should reflect this in the GUI, but not change the input file name itself.
	string loadedSimName = CWorld::getSimulationName();
	if (World().pFigaro() != nullptr)
		loadedSimName += "_" + World().Figaro()._thisCountryCode;

	SetResource("FileNameText/TextString", loadedSimName.c_str());

	// ==================  Plots windows  ==================================

	for (const auto& pair : DEPData().PlotsInputParameters())
	{
		const auto& title = pair.first;
		if (title == "Control")
			continue;
		else
		{
			bool found = false;
			for (int n = 0; n < Plots().size(); ++n)
				if (pair.first == Plots().at(n)->windowTitle)
				{
					found = true;
					break;
				}

			if (found)
				continue;

			CPlotsXYBase* pPlot = new CDEPplotWindow(pair.second);
			Plots().push_back(pPlot);
		}
	}

	Update_AfterLoadWorld();

	return ok;
}

#else //ifndef DEPLOYERS_GRAPHICS

#ifdef LINUX_VERSION // NoGRAPH, Linux
//M J int main(int argc, char* argv[])
int main(void)
#else // def _Win32, NoGRAPH, Windows
int WinMain(int argc, char* argv[])
#endif // NoGRAPH, _WINDOWS or Linux
{
	delete pDEPData();
	pDEPData() = new CData();
	DEPData().readInputParameters();

	_pLogf = new ofstream();
	_pExternalSectorsIO = new ofstream();

	pWorld() = new CWorld();

	bool done = World().LoadWorld();
	if (!done)
		CWorld::ERRORmsg("Error: failed to load World input file", true);

	done = false;
	while (!done)
		done = World().RunSimulation();

	return 0;
};

#endif // ifndef DEPLOYERS_GRAPHICS
// =========================================================================
