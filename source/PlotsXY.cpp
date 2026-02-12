
//  PlotsXY.cpp

//   DEPLOYERS v2

#include "./pch.h"

CPlotDefinition::CPlotDefinition()
{
	windowTitle = "UndefWindowTitle";
	plotTitle = "UndefPlotTitle";

	// ---------------------------------------------------------------------

	PlotType = 0; // 0:line, 1:markers, 2:linemarkers
	PlotX0 = 0;
	PlotY0 = 0;

	aType = UndefAgentType;
	aName = "UndefAgentName";

	XLabel = "month";
	YLabel = "kMmu";

	UsecurrMonthNPoints = true; // true means nPoints = 1 + currMonth() in CreatePlots
	NPoints = 0;
	XLow = 0;
	XHigh = 100;
	YLow = 0;
	YHigh = 1;

	TooltipFormat = "<plot_string:%s>: <sample_y:%.0lf>, month <sample_x:%.0lf>";
	XAxisTooltipFormat = "<axis_value:%.0lf>";
	YAxisTooltipFormat = "<axis_value:%.0lf>";

	XAxisLabelFormat = "%.0lf";
	YAxisLabelFormat = "%.0lf";

	pLabel_array = nullptr;
}

#ifdef DEPLOYERS_GRAPHICS

// Global functions.
DataPoint XYdata_point;// Used by GetPlotPoint to return data values.
void UpdateXYChart(CPlotsXYBase* chart, GlgLong* timer_id);
GlgLong GetAdjustedTimeoutXY(GlgULong sec1, GlgULong microsec1, GlgLong interval);
GlgBoolean isSingleClick = False; // Flag to identify a double-click.
double dx = 5., dy = 5.; // Selection delta in pixels.

// ====================================================================================

static ofstream& operator<<(ofstream& ofstrm, const C2DVectorDoubles& curves)
{
	ofstrm << " {";

	if (curves.size() > 0)
	{
		ofstrm << " " << curves.size();
		for (const auto& curve : curves)
			ofstrm << curve;
	}

	ofstrm << " }";

	return ofstrm;
}
static ifstream& operator>>(ifstream& ifstrm, C2DVectorDoubles& curves)
{
	string name, bracket;
	ifstrm >> bracket; // "{"

	if (ifstrm >> name, name != "}")
	{
		long size = atol(name.c_str());
		curves.clear();
		curves.resize(size);
		for (long n = 0; n < size; ++n)
			ifstrm >> curves[n];
	}

	return ifstrm;
}

// =====================  CDEPplotWindow  =====================

CDEPplotWindow::CDEPplotWindow(const CPlotDefinition& plotDefinition)
{
	PlotDefinition() = plotDefinition;
	windowTitle = PlotDefinition().windowTitle;

	// Size and Position

	width = DEPData().InputParameter("PlotsXSize");
	hight = DEPData().InputParameter("PlotsYSize");

	x = (DEPData().InputParameter("PlotsX0") + PlotDefinition().PlotX0) * width;
	y = (DEPData().InputParameter("PlotsY0") + PlotDefinition().PlotY0) * hight;

	Init();
}
GlgBoolean CDEPplotWindow::GetPlotPoint(GlgLong plot_index,
	// ***NOT USED***
	DataPoint& XYdata_point)
{
	long month = currMonth();
	if (month < 1)
	{
		XYdata_point.value_valid = false;
		return GlgFalse;
	}

	assert(month == nextMonth);

	// Obtain one datasample for a given plot index:

	XYdata_point.Xvalue = plot_index;
	XYdata_point.Yvalue = DEPData().PlotsData().at(windowTitle).at(plot_index).at(currMonth());

	XYdata_point.value_valid = true;

	return GlgTrue;
}

GlgBoolean CDEPplotWindow::GetPlotPoint(GlgLong plot_index, GlgLong point_index,
	DataPoint& XYdata_point)
{
	long month = currMonth();

	// Obtain one datasample for a given plot index:

	XYdata_point.Xvalue = point_index;
	if (windowTitle == "IndivsWealth")
	{
		XYdata_point.Yvalue = DEPData().getsorted_IndivsWealth().at(point_index)._qtty
			/ 1000000.0; // value
		XYdata_point.ID = agentsInfo()[point_index].ID
			= DEPData().getsorted_IndivsWealth().at(point_index)._ID;
		XYdata_point.type = agentsInfo()[point_index].type
			= DEPData().getsorted_IndivsWealth().at(point_index)._aTy;

		if (agentsInfo()[point_index].type == UndefAgentType) // Producer not-started or removed or not selected for plot
		{
			XYdata_point.value_valid = false;
			return GlgFalse;
		}
	}
	else if (windowTitle == "ProducersWealth")
	{
		XYdata_point.Yvalue = DEPData().getsorted_ProducersWealth().at(point_index)._qtty
			/ 1000000.0; // value
		XYdata_point.ID = agentsInfo()[point_index].ID
			= DEPData().getsorted_ProducersWealth().at(point_index)._ID;
		XYdata_point.type = agentsInfo()[point_index].type
			= DEPData().getsorted_ProducersWealth().at(point_index)._aTy;

		if (agentsInfo()[point_index].type == UndefAgentType) // Producer not-started or removed or not selected for plot
		{
			XYdata_point.value_valid = false;
			return GlgFalse;
		}
	}
	else
		XYdata_point.Yvalue = DEPData().PlotsData().at(windowTitle).at(plot_index).at(point_index);

	XYdata_point.value_valid = true;

	return GlgTrue;
}
void CDEPplotWindow::InitAfterH(void)
{
	nextMonth = currMonth() + 1;

	//  ----------------------- Definition parameters -----------------------

	title = PlotDefinition().plotTitle;

	XLabel = PlotDefinition().XLabel;
	XLow = currMonth() + 1;
	XHigh = XLow + getDEPData().getMonthsPerYear() * getDEPData().getNYears(); // months (months)

	YLabel = PlotDefinition().YLabel;
	YLow = PlotDefinition().YLow;
	YHigh = PlotDefinition().YHigh;

	PlotType = PlotDefinition().PlotType; // 0:line, 1:markers, 2:linemarkers

	// Plots labels source
	pLabel_array = PlotDefinition().pLabel_array;

	MarkerVisibility = 1; // 0: hide, 1: show
	MarkerSize = getInputParameter("PlotMarkerSize");
	MarkerType = 16; // 1:cross, 2:square, 4:filled square, 8: circle, 16: filled circle

	//  ----------------------- PlotVars values -----------------------

	PlotsVars().clear();
	long plotN = 0;
	for (plotN = 0; plotN < pLabel_array->size(); ++plotN)
	{
		CPlotVars pv;

		pv.PlotName = pLabel_array->at(plotN);
		pv.PlotType = 33; // 1:line, 32:markers, 33:linemarkers
		pv.LineWidth = getInputParameter("PlotLineWidth");
		pv.MarkerVisibility = MarkerVisibility; // 0: hide, 1: show
		pv.MarkerSize = MarkerSize;
		pv.MarkerType = MarkerType; // 1:cross, 2:square, 4:filled square, 8: circle, 16: filled circle
		pv.TooltipFormat = TooltipFormat;
		pv.fR = ColorDefinitions().at(plotN).dR;
		pv.fG = ColorDefinitions().at(plotN).dG;
		pv.fB = ColorDefinitions().at(plotN).dB;

		PlotsVars().push_back(pv);
	}

	NumPlots = plotN;

	// =========================  END OF DEFINITIONS ==============================

	// -----------------------  Create Plots using GlgAddPlot  ------------------------

	CreatePlots();

	//  ----------------------- Plot selection List  -----------------------

	GlgObject TypeList = GetResourceObject("TypeList");

	// 1. Delete init Item List

	long size = (GlgLong)GlgSendMessage(TypeList, "Handler", "GetItemCount", NULL, NULL, NULL, NULL);
	for (plotN = 0; plotN < (long)size; ++plotN)
		GlgSendMessage(TypeList, "Handler", "DeleteItem", NULL, NULL, NULL, NULL);// 
	GlgSendMessage(TypeList, "Handler", "UpdateItemList", NULL, NULL, NULL, NULL);

	// 2. Define labels selection list

	size = (long)pLabel_array->size();
	for (plotN = 0; plotN < size; ++plotN)
	{
		string name = PlotsVars(plotN).PlotName;
		GlgSendMessage(TypeList, "Handler", "AddItem", (GlgAnyType)name.c_str(), NULL, NULL, NULL);
	}

	GlgSendMessage(TypeList, "Handler", "UpdateItemList", NULL, NULL, NULL, NULL);

	// 3. Enable all plots initially

	GlgObject ItemStateList = (GlgObject)GlgSendMessage(TypeList, "Handler", "GetItemStateList", NULL, NULL, NULL, NULL);
	GlgLong nItems = GlgGetSize(ItemStateList);
	//vector<bool> state(nItems, false);
	for (long long plotN = 0; plotN < nItems; ++plotN)
	{
		GlgSetElement(ItemStateList, plotN, (GlgObject)True);
		//state[plotN] = GlgGetElement(ItemStateList, plotN);
	}

	//  ----------------------- LinePoint type List  -----------------------

	GlgObject LinePointList = GetResourceObject("Toolbar/LinePointList");

	// 1. Delete init Item List

	size = (GlgLong)GlgSendMessage(LinePointList, "Handler", "GetItemCount", NULL, NULL, NULL, NULL);
	for (plotN = 0; plotN < (long)size; ++plotN)
		GlgSendMessage(LinePointList, "Handler", "DeleteItem", NULL, NULL, NULL, NULL);// 
	GlgSendMessage(LinePointList, "Handler", "UpdateItemList", NULL, NULL, NULL, NULL);

	// 2. Define labels selection list

	GlgSendMessage(LinePointList, "Handler", "AddItem", "lines", NULL, NULL, NULL);
	GlgSendMessage(LinePointList, "Handler", "AddItem", "markers", NULL, NULL, NULL);
	GlgSendMessage(LinePointList, "Handler", "AddItem", "lineMarkers", NULL, NULL, NULL);

	GlgSendMessage(LinePointList, "Handler", "UpdateItemList", NULL, NULL, NULL, NULL);

	SetResource("Toolbar/LinePointList/SelectedIndex", PlotType);

	// -----------------------------------------------------------------------

	SetResource("ChartViewport/Title/String", title.c_str());
	SetResource("ChartViewport/Legend/Visibility", 0.);

	SetResource("ChartViewport/Chart/AutoScale", 1.);
	SetResource("ChartViewport/Chart/Background/Opacity", 1.);
	SetResource("ChartViewport/Chart/DrawCrossHair", 2.); // 0: none, 1:X, 2:Y, 3:XY
	SetResource("ChartViewport/Chart/NumLevels", 0.);

	SetResource("ChartViewport/Chart/TooltipMode", 3); // 0: none, 1:X, 3:Index
	SetResource("ChartViewport/Chart/TooltipFormat", PlotDefinition().TooltipFormat.c_str());

	SetResource("ChartViewport/Chart/BufferSize", 0.); // unlimited
	SetResource("ChartViewport/Chart/BufferXSpan", -1); // unlimited

	SetResource("ChartViewport/Chart/XAxis/XAxisType", 0.); // 0:range, 5:index scroll
	SetResource("ChartViewport/Chart/XAxis/LabelFormat", "%.0lf");
	SetResource("ChartViewport/Chart/XAxis/TooltipFormat", PlotDefinition().XAxisTooltipFormat.c_str());
	SetResource("ChartViewport/Chart/XAxis/AxisLabelAnchoring", 16.); // 0:centercenter 16:centertop
	SetResource("ChartViewport/Chart/XAxis/AxisLabelPosition", 32); // 32:centerbottom
	SetResource("ChartViewport/Chart/XAxis/AxisLabel/TextColor", 0., 0., 0.);
	SetResource("ChartViewport/Chart/XAxis/AxisLabel/String", PlotDefinition().XLabel.c_str());
	SetResource("ChartViewport/Chart/XAxis/MajorInterval", 120); // negative: number of
	SetResource("ChartViewport/Chart/XAxis/MinorInterval", 12);
	SetResource("ChartViewport/Chart/XAxis/Low", XLow);
	SetResource("ChartViewport/Chart/XAxis/High", XHigh);

	SetResource("ChartViewport/Chart/YAxis/YAxisType", 0.); // 0:range, 5:index scroll
	SetResource("ChartViewport/Chart/YAxis/LabelFormat", PlotDefinition().YAxisLabelFormat.c_str());
	SetResource("ChartViewport/Chart/YAxis/TooltipFormat", PlotDefinition().YAxisTooltipFormat.c_str());
	SetResource("ChartViewport/Chart/YAxis/AxisLabelAnchoring", 16); // 16:centertop
	SetResource("ChartViewport/Chart/YAxis/AxisLabelPosition", 16); // 16:centertop
	SetResource("ChartViewport/Chart/YAxis/AxisLabel/AnchorOffset", 0., -20., 0.);
	SetResource("ChartViewport/Chart/YAxis/AxisLabel/TextColor", 0., 0., 0.);
	SetResource("ChartViewport/Chart/YAxis/AxisLabel/String", PlotDefinition().YLabel.c_str());
	SetResource("ChartViewport/Chart/YAxis/MajorInterval", -10); // negative: number of
	SetResource("ChartViewport/Chart/YAxis/MinorInterval", -5);
	SetResource("ChartViewport/Chart/YAxis/Low", YLow);
	SetResource("ChartViewport/Chart/YAxis/High", YHigh);

	Update();
	Sync();
}
void CDEPplotWindow::CreatePlots()
{
	if (currMonth() < 0)
		return;

	// -----------------------  Create Plots using GlgAddPlot  ------------------------

	if (PlotDefinition().UsecurrMonthNPoints)
		PlotDefinition().NPoints = 1 + currMonth();
	else
	{
		long nPoints = 0;
		if (windowTitle == "IndivsWealth")
		{
			XLow = 0;
			XHigh = (long)DEPData().getsorted_IndivsWealth().size();
		}
		else if (windowTitle == "ProducersWealth")
		{
			XLow = 0;
			XHigh = (long)DEPData().getsorted_ProducersWealth().size();
		}

		SetResource("ChartViewport/Chart/XAxis/Low", XLow);
		SetResource("ChartViewport/Chart/XAxis/High", XHigh);
		PlotDefinition().NPoints = XHigh - XLow;
	}

	AgentInfo info;
	info.ID = info.type = -1;
	agentsInfo().clear();
	agentsInfo().resize(PlotDefinition().NPoints, info);

	GlgObject object;

	// 1. Delete current plots

	GlgObjectC plot_array = Chart.GetResourceObject("Plots");
	GlgObjectC clone = GlgCloneObject(plot_array, GLG_SHALLOW_CLONE);
	long size = GlgGetSize(clone);
	GlgSetStart(clone); // Initialize traversing.
	for (long i = 0; i < size; ++i)
	{
		object = GlgIterate(clone);
		GlgDeletePlot(Chart, NULL, object);
	}

	// 2. Create new plots and store the new objects IDs for each plot.

	delete[] Plots; // new: ...in case NumPlots has changed

	Chart.SetResource("NumPlots", NumPlots);
	Plots = new GlgObjectC[NumPlots];

	for (long plotN = 0; plotN < NumPlots; ++plotN)
	{
		Plots[plotN] = GlgAddPlot(Chart, NULL, NULL);
		auto& thisPlot = Plots[plotN];
		auto& plotVars = PlotsVars(plotN);

		thisPlot.SetResource("PlotType", plotVars.PlotType); // 1:line, 32:markers, 33:linemarkers
		thisPlot.SetResource("IncludeZero", 0.);
		thisPlot.SetResource("Annotation", plotVars.PlotName.c_str());
		thisPlot.SetResource("LineWidth", plotVars.LineWidth);
		// double-click to show (see Trace)
		thisPlot.SetResource("Marker/Visibility", plotVars.MarkerVisibility);
		// MarkerType 1:cross, 2:square, 4:filled square, 8: circle, 16: filled circle
		thisPlot.SetResource("Marker/MarkerType", plotVars.MarkerType);
		thisPlot.SetResource("Marker/MarkerSize", plotVars.MarkerSize);
		thisPlot.SetResource("Marker/FillColor", plotVars.fR, plotVars.fG, plotVars.fB);
		thisPlot.SetResource("EdgeColor", plotVars.fR, plotVars.fG, plotVars.fB);
	}
	Update();
}
void CDEPplotWindow::updatePlots()
{
	CreatePlots();

	double dd;
	GetResource("Toolbar/LinePointList/SelectedIndex", &dd);
	long idd = (long)dd, plottype = 1; // line
	if (idd == 1)
		plottype = 32; // marker
	else if (idd == 2)
		plottype = 33; // linemarker

	GlgObject TypeList = GetResourceObject("TypeList");
	GlgObject ItemStateList = (GlgObject)GlgSendMessage(TypeList, "Handler", "GetItemStateList", NULL, NULL, NULL, NULL);
	GlgLong size = GlgGetSize(ItemStateList);
	vector<bool> state(size, false);
	GlgSetStart(ItemStateList); // Initialize traversing.
	for (long i = 0; i < size; ++i)
	{
		GlgObject object = GlgIterate(ItemStateList);
		state[i] = (bool)object;
	}

	// Update plots with new data
	for (long ptype = 0; ptype < NumPlots; ++ptype)
	{
		Plots[ptype].SetResource("PlotType", plottype);
		Plots[ptype].SetResource("Enabled", (double)state[ptype]);

		//for (long point_index = 0; point_index < DEPData().getsorted_IndivsWealth().size(); ++point_index)
		for (long point_index = 0; point_index < PlotDefinition().NPoints; ++point_index)
		{
			if (GetPlotPoint(ptype, point_index, XYdata_point))
			{
				// Push a new data point into the chart's plot.
				if (windowTitle == "ProducersWealth")
				{
					if (XYdata_point.value_valid
						&& state[XYdata_point.type]) // && XYdata_point.Yvalue > 0
						PushPlotPoint(Plots[ptype], XYdata_point);
				}
				else if (XYdata_point.value_valid) // && XYdata_point.Yvalue > 0
					PushPlotPoint(Plots[ptype], XYdata_point);
			}
		}
	}

	Update();
}

// Define MarkDataSample instances

void CDEPplotWindow::MarkDataSample(double x, double y)
{
	// Obtain the closest data sample at the cursor position.
	GlgDataSample* data_sample = GetDataSample(x, y);

	if (!data_sample)
		return;

	string str;
	if (PlotDefinition().windowTitle == "IndivsWealth")
		str = "Indiv_" + to_string(agentsInfo().at(data_sample->time).ID);
	else if (PlotDefinition().windowTitle == "ProducersWealth")
		str = "Producer " + CProducer::getProducerLabelOfType(agentsInfo().at(data_sample->time).type)
		+ "_" + to_string(agentsInfo().at(data_sample->time).ID);
	else
		str = "x= " + to_string(x) + ", y= " + to_string(y);

	getWorld().ERRORmsg(str, false);

	GlgChangeObject(Chart, NULL);
	GlgUpdate(ChartVP);
}

//======================  CPlotsXYBase  ===================================

CPlotsXYBase::CPlotsXYBase(void)
{
	// default values

	UIname = "PlotsXY.glg";
	UpdateInterval = 100;  // Update interval in msec

	TimerID = 0;
	nextMonth = 0;
	Plots = NULL;
	YLow = 0;
	YHigh = 0;
	NumPlots = 1; // to be determined
	MarkerVisibility = 1; // 0: hide, 1: show
	MarkerType = 16; // 8: circle, 16: filled circle
	MarkerSize = 5;
	TooltipFormat = "<plot_string:%s>: x= <sample_x:%.0lf>, y= <sample_y:%.2lf>";
	XAxisTooltipFormat = "x= <axis_value:%.0lf>";
	YAxisTooltipFormat = "y= <axis_value:%.2lf>";
}
CPlotsXYBase::~CPlotsXYBase(void)
{
	//delete Plots;
	//delete Low;
	//delete High;
	delete[] Plots;
}

void CPlotsXYBase::Init(void)
{
	// Initialize Chart parameters before hierarchy is set up.
	InitBeforeH();

	EnableCallback(GLG_INPUT_CB, NULL);
	EnableCallback(GLG_TRACE_CB, NULL);

	SetupHierarchy();

	InitAfterH();

	// Display the chart.
	InitialDraw();

	// Start dynamic updates.
	StartUpdates();
}
// Initializes chart parameters before hierarchy is setup.
void CPlotsXYBase::InitBeforeH(void)
{
	// -------------------------  Setup(...)  -------------------------

	// Load GLG drawing from a specified file.
	LoadWidget(UIname.c_str());

	if (IsNull())
	{
		GlgError(GLG_USER_ERROR, (char*)"Can't load drawing.");
		exit(GLG_EXIT_ERROR);
	}

	// Set widget position and dimensions in screen coordinates.
	SetSize(x, y, width, hight);

	SetResource("ScreenName", windowTitle.c_str());

	NumPlots = 1;// The actual number will be given in InitializePlots()
	Chart.SetResource("NumPlots", NumPlots);

	ChartVP = GetResourceObject("ChartViewport");
	if (ChartVP.IsNull())
		error("Can't find ChartViewport", GlgTrue);

	Chart = ChartVP.GetResourceObject("Chart");
	if (Chart.IsNull())
		error("Can't find Chart object", GlgTrue);

	// Set Chart Zoom mode.
	ChartVP.SetZoomMode(NULL, &Chart, NULL, GLG_CHART_ZOOM_MODE);
}
void CPlotsXYBase::StartUpdates()
{
	TimerID = GlgAddTimeOut(AppContext(), UpdateInterval,
		(GlgTimerProc)UpdateXYChart, this);
}
void CPlotsXYBase::StopUpdates()
{
	if (TimerID)
	{
		GlgRemoveTimeOut(TimerID);
		TimerID = 0;
	}
}

void CPlotsXYBase::PushPlotPoint(GlgObjectC& plot, DataPoint& XYdata_point)
{
	// Supply plot value for the chart via ValueEntryPoint.
	plot.SetResource("ValueEntryPoint", XYdata_point.Yvalue);
	plot.SetResource("TimeEntryPoint", XYdata_point.Xvalue);

	if (!XYdata_point.value_valid)
	{
		// If the data point is not valid, set ValidEntryPoint resource to
		//  display holes for invalid data points. If the point is valid,
		//  it is automatically set to 1. by the chart.
		plot.SetResource("ValidEntryPoint", 0.);
	}
}
void CPlotsXYBase::updatePlots()
{
	double dd;
	GetResource("Toolbar/LinePointList/SelectedIndex", &dd);
	long idd = (long)dd, plottype = 1; // line
	if (idd == 1)
		plottype = 32; // marker
	else if (idd == 2)
		plottype = 33; // linemarker

	GlgObjectC TypeList = GetResourceObject("TypeList");
	GlgObject ItemStateList = (GlgObject)GlgSendMessage(TypeList, "Handler", "GetItemStateList", NULL, NULL, NULL, NULL);
	GlgLong nItems = GlgGetSize(ItemStateList);
	vector<bool> state(nItems, false);
	GlgSetStart(ItemStateList); // Initialize traversing.
	for (long plotN = 0; plotN < nItems; ++plotN)
	{
		//GlgObject object = GlgIterate(ItemStateList);
		//state[i] = (bool)object;
		state[plotN] = GlgGetElement(ItemStateList, plotN);
	}


	// Update plot lines with new data supplied by the DataFeed object.
	for (long ptype = 0; ptype < NumPlots; ++ptype)
	{
		Plots[ptype].SetResource("PlotType", plottype);

		Plots[ptype].SetResource("Enabled", (double)state[ptype]);

		if (GetPlotPoint(ptype, XYdata_point))
		{
			// Push a new data point into the chart's plot.
			if (XYdata_point.value_valid)
				PushPlotPoint(Plots[ptype], XYdata_point);
		}
	}

	Update();
}

void UpdateXYChart(CPlotsXYBase* chart, GlgLong* timer_id)
{
	// Start time for adjusting timer intervals.
	GlgULong sec, microsec;
	GlgGetTime(&sec, &microsec);

	if (currMonth() == chart->nextMonth)
	{
		chart->updatePlots();
		chart->nextMonth++;
	}

	// Set counter to next update month
	long maxMonth = getDEPData().getNYears() * getDEPData().getMonthsPerYear() - 1;
	while (
		chart->nextMonth < maxMonth
		&& (chart->nextMonth % (long)getInputParameter("MonthsBetweenUpdates") != 0))
		chart->nextMonth++;

	// Adjust timer intervals to have a constant update rate regardless 
	//   of the time it takes to redraw the chart.
	GlgLong timer_interval =
		GetAdjustedTimeoutXY(sec, microsec, chart->UpdateInterval);

	chart->TimerID =
		GlgAddTimeOut(AppContext, timer_interval,
			(GlgTimerProc)UpdateXYChart, chart);
}

void CPlotsXYBase::Input(GlgObjectC& viewport, GlgObjectC& message)
{
	CONST char
		* format,
		* action,
		* origin,
		* subaction;

	message.GetResource("Format", &format);
	message.GetResource("Action", &action);
	message.GetResource("Origin", &origin);
	message.GetResource("SubAction", &subaction);

	// Handle window closing. May use viewport's name.
	if (strcmp(format, "Window") == 0 &&
		strcmp(action, "DeleteWindow") == 0)
	{
		// Closing main window: exit.
		exit(GLG_EXIT_OK);
	}

	if (strcmp(format, "Button") == 0)         // Handle button clicks
	{
		if (strcmp(action, "Activate") != 0 &&     // Push button
			strcmp(action, "ValueChanged") != 0)   // Toggle button
			return;

		AbortZoomTo();

		if (strcmp(origin, "ZoomTo") == 0)
		{
			// Start ZoomTo operation
			ChartVP.SetZoom(NULL, 't', 0.);
		}
		else if (strcmp(origin, "initZoom") == 0)
		{
			// Set initial time span and reset initial Y ranges.
			SetChartSpan(XHigh);
			RestoreInitialYRanges();
		}
		else if (strcmp(origin, "allZoom") == 0) // 0420
		{
			double value;
			message.GetResource("OnState", &value);
			if (value)
			{
				SetResource("ChartViewport/Chart/AutoScale", 1.);
				SetChartSpan(-1);
				SetChartSpan(XHigh);
			}
			else
			{
				SetResource("ChartViewport/Chart/AutoScale", 0.);
			}
		}

		Update();
	}
	else if (strcmp(format, "Chart") == 0 &&
		strcmp(action, "CrossHairUpdate") == 0)
	{
		// To avoid slowing down real-time chart updates, invoke Update()
		//   to redraw cross-hair only if the chart is not updated fast
		//   enough by the timer.
		if (UpdateInterval > 100)
			Update();
	}

	// Handle Zoom/Pan events
	else if (strcmp(action, "Zoom") == 0)
	{
		if (strcmp(subaction, "ZoomRectangle") == 0)
		{
			// Stop scrolling when ZoomTo action is started. 
			//ChangeAutoScroll(0);
		}
		else if (strcmp(subaction, "End") == 0)
		{
			// No additional actions on finishing ZoomTo. The Value axis
			//   scrollbar appears automatically if needed
		}
		else if (strcmp(subaction, "Abort") == 0)
		{
			// Resume scrolling if it was on. 
			//ChangeAutoScroll(StoredScrollState);
		}

		Update();
	}
	else if (strcmp(action, "Pan") == 0)
	{
		// This code may be used to perform custom action when dragging the
		//   chart's data with the mouse.
		if (strcmp(subaction, "Start") == 0)   // Chart dragging start
		{
		}
		else if (strcmp(subaction, "Drag") == 0)    // Dragging
		{
		}
		else if (strcmp(subaction, "ValueChanged") == 0)   // Scrollbars
		{
		}
		// Dragging ended or aborted.
		else if (strcmp(subaction, "End") == 0 ||
			strcmp(subaction, "Abort") == 0)
		{
		}
	}

	else if (strcmp(format, "Option") == 0)
	{
		if (strcmp(action, "Select") == 0)
		{
			if (strcmp(origin, "LinePointList") == 0)
			{
				updatePlots();
			}
		}
	}
	else if (strcmp(format, "List") == 0)
	{
		if (strcmp(action, "Select") == 0)
		{
			if (strcmp(origin, "TypeList") == 0)
			{
				updatePlots();
			}
		}
	}

	// input text, long, double:
	else if (strcmp(format, "Text") == 0)
	{
		/*
			if (strcmp(origin, "NYearsText") == 0
				&& strcmp(action, "ValueChanged") == 0)
			{
				if (World().getCurrYear() == 0)
				{
					double value;
					GetResource("NYearsText/Value", &value);
					World().NYears() = (long)value;
				}
			}
		*/
	}
}
void CPlotsXYBase::Trace(GlgObjectC& callback_viewport,
	GlgTraceCBStruct* trace_data)
{
	long
		event_type = 0,
		x, y,
		button_index,
		width, height;

	GlgObjectC event_vp;
	event_vp = trace_data->viewport;

	// Process only events that occur in ChartViewport
	if (!event_vp.Same(ChartVP))
		return;

	switch (trace_data->event->message)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		x = GET_X_LPARAM(trace_data->event->lParam);
		y = GET_Y_LPARAM(trace_data->event->lParam);
		button_index = 1;
		event_type = BUTTON_PRESS;
		break;

	case WM_MOUSEMOVE:
		x = GET_X_LPARAM(trace_data->event->lParam);
		y = GET_Y_LPARAM(trace_data->event->lParam);
		event_type = MOUSE_MOVE;
		break;

	case WM_SIZE:
		width = LOWORD(trace_data->event->lParam);
		height = HIWORD(trace_data->event->lParam);
		event_type = RESIZE;
		break;

	default: return;
	}

	switch (event_type)
	{
	case RESIZE:
		break;

	case BUTTON_PRESS:
		if (ZoomToMode())
			return; // ZoomTo or dragging mode in progress.

		// Handle left mouse clicks.
		if (button_index == 1)
		{
			// Get click count to differentiate between double-click and single click.
			if (GetClickCount(x, y) == 2)
			{
				// Double-click: show/hide a marker for a selected data sample, if any.
				MarkDataSample(x, y);
			}
			else // Single click.
			{
				/* Start dragging with the mouse on a mouse click.
				   If the user clicked on an axis, the dragging will
				   be activated in the direction of that axis.
				   If the user clicked in the chart area, dragging
				   in both the X and the Y direction will be activated.
				*/
				GlgSetZoom(ChartVP, NULL, 's', 0.);
			}
		}
		break;

	case MOUSE_MOVE:
		break;

	default: return;
	}
}

void CPlotsXYBase::SetChartSpan(GlgLong span)
{
	if (span > 0)
	{
		Chart.SetResource("XAxis/High", (double)span);
		Chart.SetResource("XAxis/Low", (double)0.);
	}
	else  // Reset span to show all data accumulated in the buffer.
	{
		ChartVP.SetZoom(NULL, 'N', 0.);
		ChartVP.SetZoom(NULL, 'n', 0.);
	}
}

void CPlotsXYBase::UpdateXScale(double newNYears)
{
	// Update the X-axis scale when NYears changes dynamically
	XHigh = XLow + getDEPData().getMonthsPerYear() * newNYears;
	SetChartSpan((GlgLong)XHigh);
}

void CPlotsXYBase::RestoreInitialYRanges()
{
	GlgObjectC axis_array;
	GlgObjectC axis;

	axis_array = Chart.GetResourceObject("YAxisGroup");
	axis = axis_array.GetElement(0);
	axis.SetResource("Low", YLow);
	axis.SetResource("High", YHigh);
}
GlgLong CPlotsXYBase::ZoomToMode()
{
	// Returns true if the chart's viewport is in ZoomToMode.
	// ZoomToMode is activated on Dragging and ZoomTo operations.
	double zoom_mode;

	ChartVP.GetResource("ZoomToMode", &zoom_mode);
	if (zoom_mode)
		return (GlgLong)zoom_mode;

	return false;
}
void CPlotsXYBase::AbortZoomTo()
{
	if (ZoomToMode())
	{
		// Abort zoom mode in progress.
		ChartVP.SetZoom(NULL, 'e', 0.);
		Update();
	}
}

void CPlotsXYBase::SetSize(GlgLong x, GlgLong y,
	GlgLong width, GlgLong height)
{
	SetResource("Point1", 0., 0., 0.);
	SetResource("Point2", 0., 0., 0.);

	SetResource("Screen/XHint", (double)x);
	SetResource("Screen/YHint", (double)y);
	SetResource("Screen/WidthHint", (double)width);
	SetResource("Screen/HeightHint", (double)height);
}
double CPlotsXYBase::GetCurrTime()
{
	GlgULong sec, microsec;

	GlgGetTime(&sec, &microsec);
	return sec + microsec / 1000000.;
}
GlgLong GetAdjustedTimeoutXY(GlgULong sec1, GlgULong microsec1,
	GlgLong interval)
{
	GlgULong sec2, microsec2;
	GlgLong elapsed_time, adj_interval;

	GlgGetTime(&sec2, &microsec2);  // End time

	// Elapsed time in millisec
	elapsed_time =
		(sec2 - sec1) * 1000 + (GlgLong)(microsec2 - microsec1) / 1000;

	// Maintain constant update interval regardless of the system speed.
	if (elapsed_time + 20 >= interval)
		// Slow system: update as fast as we can, but allow a small interval for handling input events.
		adj_interval = 20;
	else
		// Fast system: keep constant update interval.
		adj_interval = interval - elapsed_time;

#if DEBUG_TIMER
	printf("sec= %ld, msec= %ld\n", sec2 - sec1, microsec2 - microsec1);
	printf("*** elapsed= %ld, requested= %ld, adjusted= %ld\n",
		(long)elapsed_time, (long)interval, (long)adj_interval);
#endif

	return adj_interval;
}

// Obtain a chart data sample at the cursor position.
GlgDataSample* CPlotsXYBase::GetDataSample(double x, double y)
{
	GlgObject
		plot,
		data_array;
	GlgDataSample* data_sample;
	long
		num_samples,
		i;
	char
		* name,
		* sample_x_string;
	double
		sample_x_value,
		sample_y_value;

	// Query chart selection at the cursor position.
	GlgObject selection =
		GlgCreateChartSelection(Chart, NULL, // NULL: query all plots
			x, y, dx, dy,
			True,    // x/y in screen coord
			True,// False,   // False: don't include invalid (hidden) points
			False);  // a point with smallest xy distance

	if (!selection)
	{
		error("No data sample at cursor.", False);
		return NULL;  // No valid data sample is selected.
	}

	// Query X/Y values of the selected data sample.
	plot = GlgGetResourceObject(selection, "SelectedPlot");
	GlgGetSResource(plot, "Name", &name);
	GlgGetDResource(selection, "SampleX", &sample_x_value);
	GlgGetDResource(selection, "SampleY", &sample_y_value);

	// Debugging info.
	GlgGetSResource(selection, "SampleXString", &sample_x_string);
	printf("Selection: %s time=%s value=%lf\n",
		name, sample_x_string, sample_y_value);

	// Dereference selection object, to prevent memory leak.
	GlgDropObject(selection);

	// Obtain a list of data samples from the selected plot.
	data_array = GlgGetResourceObject(plot, "Array");

	if (data_array && (num_samples = GlgGetSize(data_array)) != 0)
	{
		/* Traverse the array to find a data sample with a matching x/y value.
		   data_array is a linked list and should be traversed
		   using SetStart() and Iterate() for efficiency, as opposed to
		   using indices via GetElement(i);
		*/
		GlgSetStart(data_array);
		for (i = 0; i < num_samples; ++i)
		{
			data_sample = (GlgDataSample*)GlgIterate(data_array);

			// Debugging info.
			// printf( "DataSample info: time = %lf value = %lf\n", data_sample->time, data_sample->value );

			if (data_sample->valid &&
				data_sample->time == sample_x_value &&
				data_sample->value == sample_y_value)
				// found matching data sample
				return data_sample;
		}
	}

	return NULL;     // No matching samples found.
}
// Show/Hide a marker for the selected data sample at the cursor position.
void CPlotsXYBase::MarkDataSample(double x, double y)
{
	// Obtain the closest data sample at the cursor position.
	GlgDataSample* data_sample = GetDataSample(x, y);

	if (!data_sample)
		return;

	//data_sample->marker_vis = (data_sample->marker_vis == 0. ? /_*show*_/ 1.f : /_*hide*_/ 0.f);

	GlgChangeObject(Chart, NULL);
	GlgUpdate(ChartVP);
}
// Timer function used to identify a double click.
void dblclick_timer(GlgAnyType data, GlgIntervalID* id)
{
	// Reset flag.
	isSingleClick = False;
}
// For a double-click, return 2; otherwise, return 1 (single click).
long CPlotsXYBase::GetClickCount(double x, double y)
{
	if (isSingleClick)
	{
		isSingleClick = False; // Reset flag.
		return 2; // Double click occurred.
	}
	else
	{
		isSingleClick = True;

		// Start a timer to identify a double click.
		GlgAddTimeOut(AppContext, 500, (GlgTimerProc)dblclick_timer, NULL);
	}

	return 1;   // Single click.
}

void CPlotsXYBase::error(CONST char* str, GlgBoolean quit)
{
	GlgError(GLG_USER_ERROR, (char*)str);
	if (quit)
		exit(GLG_EXIT_ERROR);
}

#endif
// =========================================================================
