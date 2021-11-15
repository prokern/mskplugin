#pragma once
#include "Consts.h"
#include "Global.h"
#include "ConstraintList.h"
#include "HiddenWindow.h"
#include "Common.h"
#include <mutex>
#include <set>
#include <queue>
#include <condition_variable>
#include "AircraftExtendedData.h"
#include <atomic>
#include "ComputationPool.h"
#include "ExternalDataProcessor.h"
#include "ControllerList.h"
#include "PlaneHistory.h"
#include "TagProcessor.h"
#include "AMAN.h"
#include "AFV.h"
#include "CCDLC.h"
#include "License.h"

using namespace EuroScopePlugIn;
using namespace std;

class CMskPlugin :
	public EuroScopePlugIn::CPlugIn
{
public:
	// system data
	bool Terminating = false;	// for threads
	atomic<int> ThreadRunning = 0;
	string MyPath;
	string GetMyPath();
	
	HHOOK globalMouseHook = NULL;
	HHOOK globalKeyboardHook = NULL;
	
	bool DisplayAllPorts = false;
	bool UseTimes = false;

	ExternalDataProcessor * ExternalData;
	PlaneHistory * History;
	ControllerList * Controllers;
	vector<CMskRadarScreen*> ScreenPool;
	CMskRadarScreen* CurrentScreen = NULL;
	// Extended data. Callsign,Data
	AircraftExtendedDataPool * Planes;
	ComputationTaskPool * ToCompute;
	TagProcessor * Tags;
	int ManualPointIndex = 100;
	AMAN * Aman;
	AFV * Afv;
	CCDLC * Ccdlc;
	License* Lic = NULL;

	mutex __extern_data_locker;
	void LoadExternalDataFiles();

	// QR and warnings
	bool QrShow = false;
	string QRAddr = "";
	bool ShowWarn = false;
	string WarnMsg = "";
	inline void __warn(string msg)
	{
		ShowWarn = msg != "";
		WarnMsg = msg;
	}
	// center target
	CPosition CenterRT;
	// global signals
	atomic<bool> hide_display_route = true;

	// local controller references
	string MyCid;
	string MyPort;
	string MyCS;
	string MyId;
	string MyName;
	int MyFacility = 0;
	
	bool ImController();

	// Registered plans
	CFlightPlanList	es_deplist;
	CFlightPlanList es_ldglist;
	CFlightPlanList es_trslist;
	CFlightPlanList ccdlc_list;
	void RegisterDepartureFp();
	void RegisterLandingFp();
	void RegisterCCDLCFp();
	void RegisterTransitFp();
	void RemoveFromAllLists(CFlightPlan fp);

	list<string> AllPorts;

	// Positioning areas
	bool LoadAreas();

	vector<string> FilterPlanPort;

	// draw functions for development
	bool GeoMode = false;
	vector<POINT> GeoPoints;
	vector<GeoPoint> GeoPointsCoords;
	string __hightlight;

	// CCDLC
	bool ICanUseCCDLC();
	mutex ccdlcMessages_locker;

	// ASEL follow
	string LastASELCS = "";
	bool TranslateASEL = false;
	string ReceiveASELFrom = "";

	CMskPlugin();
	virtual ~CMskPlugin();

	virtual CRadarScreen* OnRadarScreenCreated(const char* sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated);
	virtual void OnGetTagItem(CFlightPlan FlightPlan, CRadarTarget RadarTarget, int ItemCode, int TagData, char sItemString[16], int* pColorCode, COLORREF* pRGB, double* pFontSize);
	virtual void OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area);
	virtual bool OnCompileCommand(const char* sCommandLine);
	virtual void OnTimer(int Counter);
	void OnFlightPlanDisconnect(CFlightPlan FlightPlan);
	void OnFlightPlanFlightPlanDataUpdate(CFlightPlan FlightPlan);
	void OnFlightPlanControllerAssignedDataUpdate(CFlightPlan FlightPlan, int DataType);
	void OnRadarTargetPositionUpdate(CRadarTarget RadarTarget);
	void OnControllerPositionUpdate(CController Controller);
	void OnControllerDisconnect(CController Controller);
	//Mouse and keybaord events
	void OnLeftButtonDlbClicked();
	mutex __disp;
	void DisplaySilent(string Msg);
	void DisplayUrgent(string Msg);
	void GetMyPort();

	// Separated thread functions
	void RemoveRoutes();

	void AllScreenCreatePermanentObject(string Name, CPosition Pos, bool Movable, string Param1 = "", string Param2 = "", ScreenObjectEvent OnEvent = NULL);
	void ManageAircraftFPList(string CS, MskListDisplay DisplayIn, ActiveCCDLCRequest CCDLC_Request);
	void AddToCCDLCList(string CS);
	void RemoveFromCCDLCList(string CS);

	int MouseX = 0;
	int MouseY = 0;
	void GetMousePosition_FN();

	bool ShowRouteCursor = false;
	bool DisplayRadarWaypoint = false;
	string RouteCursorBeforePoint = "";
	string RouteCursorAfterPoint = "";
	string RouteCursorCS = "";
	POINT RouteCursorPos;
	RECT RouteCursorArea;
	string MarkedPoint = "";

	void RemoveAllScreenObjects();

	vector<CPosition> UserDefinedPointPositions;

	CSectorElement FindSectorElement(const int Type, string Name);
};