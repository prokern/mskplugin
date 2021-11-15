#pragma once

#include <string>
#include <vector>
#include "EuroScopePlugIn.h"
#include "framework.h"
#include <atomic>

using namespace std;
using namespace EuroScopePlugIn;

const string MY_PLUGIN_NAME = "Moscow Plugin for ES";
const string MY_PLUGIN_VERSION = "3.1.2_142_rc2";
const string MY_PLUGIN_DEVELOPER = "Ilya N Pyankov";
const string MY_PLUGIN_COPYRIGHT = "All rights reserved";

const COLORREF DefaultColor = RGB(0, 0, 0);
const COLORREF CLR_TRX = RGB(249, 227, 22);
const COLORREF CLR_ALT = RGB(255, 160, 0);
const COLORREF TTL_RED = RGB(255, 78, 79);
const COLORREF TTL_WARN = RGB(255, 255, 0);
const COLORREF AMAN_WARN = RGB(255, 160, 0);
const COLORREF CLR_SPEED_ASSIGNED = RGB(0, 0, 255);
const COLORREF WHITE = RGB(255, 255, 255);
const COLORREF GREEN = RGB(153, 255, 51);
const COLORREF CLR_EXT = RGB(165, 251, 35);

const int FP_OWN_STATE_NOT_CONCERNED = 0;
const int FP_OWN_STATE_EXPECTED = 1;
const int FP_OWN_STATE_CURRENT = 2;
const int FP_OWN_STATE_TRANSFER_FROM_ME = 3;
const int FP_OWN_STATE_TRANSFER_TO_ME = 4;

const int MEAN_GROUND_LEVEL = 600;

const int PLUGIN_LIST_NOLIST = 0b0;
const int PLUGIN_LIST_DEPARTURES = 0b1;
const int PLUGIN_LIST_LANDINGS = 0b10;
const int PLUGIN_LIST_TRANSIT = 0b100;

const string slt = "___qw90op12";

class TagProcessor;

typedef void (TagProcessor::* TagItemParserFN)(int&, string&, string&, COLORREF&);
typedef void (TagProcessor::* TagFuncParserFN)(int&, string&, CFlightPlan&, CRadarTarget&, string&, POINT&, RECT&);

struct TagItemParser {

	int ItemCode = 0;
	bool HighLightWhenTX = false;
	bool OverrideColorEvenIfTX = false;
	string DisplayName;
	TagItemParserFN FN = NULL;

	bool IsValid()
	{
		return ItemCode != 0;
	}
};

struct TagFunctionParser {
	int ItemCode = 0;
	string DisplayName;
	TagFuncParserFN FN = NULL;

	bool IsValid()
	{
		return ItemCode != 0;
	}
};

//Defined TAGS
//Standard
const int TAG_MSK_CONSTRAINT = 556677;
const int TAG_MSK_DEPA_PORT = TAG_MSK_CONSTRAINT + 1;
const int TAG_MSK_DEPA_PORT_TX = TAG_MSK_CONSTRAINT + 2;
const int TAG_MSK_DEST_PORT = TAG_MSK_CONSTRAINT + 3;
const int TAG_MSK_DEST_PORT_TX = TAG_MSK_CONSTRAINT + 4;
const int TAG_MSK_STAR = TAG_MSK_CONSTRAINT + 5;
const int TAG_MSK_STAR_TX = TAG_MSK_CONSTRAINT + 6;
const int TAG_MSK_SID = TAG_MSK_CONSTRAINT + 7;
const int TAG_MSK_SID_TX = TAG_MSK_CONSTRAINT + 8;
const int TAG_MSK_XMT = TAG_MSK_CONSTRAINT + 9;
const int TAG_MSK_CURRENT_AREA = TAG_MSK_CONSTRAINT + 10;
const int TAG_MSK_CURRENT_STS = TAG_MSK_CONSTRAINT + 11;
const int TAG_MSK_MANUAL_STS = TAG_MSK_CONSTRAINT + 12;
const int TAG_MSK_FINAL_FIX_ALT = TAG_MSK_CONSTRAINT + 13;
const int TAG_MSK_TIME_TO_LAND = TAG_MSK_CONSTRAINT + 14;
const int TAG_MSK_AMAN_POINT = TAG_MSK_CONSTRAINT + 15;
const int TAG_MSK_AMAN_POINT_TIME = TAG_MSK_CONSTRAINT + 16;
const int TAG_MSK_AMAN_MILES = TAG_MSK_CONSTRAINT + 17;
const int TAG_MSK_OWN_CS = TAG_MSK_CONSTRAINT + 18;
const int TAG_MSK_OWN_CS_TX = TAG_MSK_CONSTRAINT + 19;
const int TAG_MSK_DEPA_RWY	= TAG_MSK_CONSTRAINT + 20;
const int TAG_MSK_DEPA_RWY_TX = TAG_MSK_CONSTRAINT + 21;
const int TAG_MSK_DEST_RWY = TAG_MSK_CONSTRAINT + 22;
const int TAG_MSK_DEST_RWY_TX = TAG_MSK_CONSTRAINT + 23;
const int TAG_MSK_OWN_AFL = TAG_MSK_CONSTRAINT + 24;
const int TAG_MSK_OWN_AFL_TX = TAG_MSK_CONSTRAINT + 25;
const int TAG_MSK_OWN_CFL = TAG_MSK_CONSTRAINT + 26;
const int TAG_MSK_OWN_CFL_TX = TAG_MSK_CONSTRAINT + 27;
const int TAG_MSK_OWN_FFL = TAG_MSK_CONSTRAINT + 28;
const int TAG_MSK_OWN_FFL_TX = TAG_MSK_CONSTRAINT + 29;
const int TAG_MSK_OWN_SPEED = TAG_MSK_CONSTRAINT + 30;
const int TAG_MSK_OWN_SPEED_TX = TAG_MSK_CONSTRAINT + 31;
const int TAG_MSK_OWN_GS = TAG_MSK_CONSTRAINT + 32;
const int TAG_MSK_OWN_GS_TX = TAG_MSK_CONSTRAINT + 33;
const int TAG_MSK_OWN_VS = TAG_MSK_CONSTRAINT + 34;
const int TAG_MSK_OWN_VS_TX = TAG_MSK_CONSTRAINT + 35;
const int TAG_MSK_LANG = TAG_MSK_CONSTRAINT + 36;
const int TAG_MSK_LANG_TX = TAG_MSK_CONSTRAINT + 37;
const int TAG_MSK_READY_TIME = TAG_MSK_CONSTRAINT + 38;
const int TAG_MSK_OFFBLOCK_TIME = TAG_MSK_CONSTRAINT + 39;
const int TAG_MSK_HIGH_GRAD = TAG_MSK_CONSTRAINT + 40;
const int TAG_MSK_REST_SPEED = TAG_MSK_CONSTRAINT + 41;
const int TAG_MSK_REST_ALT = TAG_MSK_CONSTRAINT + 42;
const int TAG_MSK_POSITION = TAG_MSK_CONSTRAINT + 43;
// Extended block
const int TAG_MSK_RNAV = TAG_MSK_CONSTRAINT + 44;
const int TAG_MSK_RVSM = TAG_MSK_CONSTRAINT + 45;
const int TAG_MSK_NEWBIE = TAG_MSK_CONSTRAINT + 46;
const int TAG_MSK_BADGUY = TAG_MSK_CONSTRAINT + 47;
const int TAG_MSK_ATC_RATING = TAG_MSK_CONSTRAINT + 48;
const int TAG_MSK_PILOT_RATING = TAG_MSK_CONSTRAINT + 49;
const int TAG_MSK_PILOT_NAME = TAG_MSK_CONSTRAINT + 50;
const int TAG_MSK_PILOT_FREETEXT1 = TAG_MSK_CONSTRAINT + 51;
const int TAG_MSK_PILOT_FREETEXT2 = TAG_MSK_CONSTRAINT + 52;
const int TAG_MSK_EXT_DATA_WARNING = TAG_MSK_CONSTRAINT + 53;
const int TAG_MSK_EXT_DATA_HOURS = TAG_MSK_CONSTRAINT + 54;

const int TAG_MSK_EXT_DATA_DEPA_ICAO = TAG_MSK_CONSTRAINT + 55;
const int TAG_MSK_EXT_DATA_DEPA_RWY = TAG_MSK_CONSTRAINT + 56;
const int TAG_MSK_EXT_DATA_DEST_ICAO = TAG_MSK_CONSTRAINT + 57;
const int TAG_MSK_EXT_DATA_DEST_RWY = TAG_MSK_CONSTRAINT + 58;

const int TAG_MSK_OWN_HDG = TAG_MSK_CONSTRAINT + 59;
//CCDLC Tags
const int TAG_MSK_CCDLC_INITIATOR = TAG_MSK_CONSTRAINT + 1000;
const int TAG_MSK_CCDLC_REQUEST = TAG_MSK_CCDLC_INITIATOR + 1;
const int TAG_MSK_CCDLC_REQUEST_EXT = TAG_MSK_CCDLC_INITIATOR + 2;
const int TAG_MSK_CCDLC_REACTIONS = TAG_MSK_CCDLC_INITIATOR + 3;
const int TAG_MSK_CCDLC_FREETEXT = TAG_MSK_CCDLC_INITIATOR + 4;
const int TAG_MSK_CCDLC_FREETEXT_EXT = TAG_MSK_CCDLC_INITIATOR + 5;
const int TAG_MSK_CCDLC_DISPLAY_DCT = TAG_MSK_CCDLC_INITIATOR + 6;
const int TAG_MSK_CCDLC_DISPLAY_ALT = TAG_MSK_CCDLC_INITIATOR + 7;
const int TAG_MSK_CCDLC_DISPLAY_SPD = TAG_MSK_CCDLC_INITIATOR + 8;
const int TAG_MSK_CCDLC_DISPLAY_TXT = TAG_MSK_CCDLC_INITIATOR + 9;
// Defined functions
//Standard
const int FN_NOOP = TAG_MSK_CONSTRAINT + 2000;
const int FN_MSK_DISPLAY_ROUTE = FN_NOOP + 1;
const int FN_MSK_SHOW_INTERVAL = FN_NOOP + 2;
const int FN_MSK_REMOVE_SID = FN_NOOP + 3;	// not in use - no API ref
const int FN_MSK_AMAN_POINT = FN_NOOP + 4;
const int FN_MSK_AMAN_POINT_SELECTED = FN_NOOP + 5;
const int FN_MSK_TTL_CLICKED = FN_NOOP + 6;
const int FN_MSK_TTL_CLEARED_TO_LAND = FN_NOOP + 7;
const int FN_MSK_MAN_STS_CLICKED = FN_NOOP + 8;
const int FN_MSK_MAN_STS_ASSIGNED = FN_NOOP + 9;
const int FN_MSK_MAN_STS_CLEARED = FN_NOOP + 10;	// not in use
const int FN_MSK_CALLSIGN_MENU_OPEN = FN_NOOP + 11;
const int FN_MSK_LANGUAGE_SELECTED = FN_NOOP + 12;
const int FN_MSK_REST_CLEAR_CONFIRMED = FN_NOOP + 13;
const int FN_MSK_CENTER_TARGET = FN_NOOP + 14;
const int FN_MSK_SWITCH_KMH	= FN_NOOP + 15;
const int FN_MSK_SELECT_TEXT_MESSAGE = FN_NOOP + 16;
const int FN_MSK_SELECT_TEXT_MESSAGE_SELECTED = FN_NOOP + 17;
const int FN_MSK_WAYPOINT_ROUTE_ACTION = FN_NOOP + 18;
const int FN_MSK_ROINT_RESET = FN_NOOP + 19;
const int FN_MSK_FREETEXT_ACTIVATE = FN_NOOP + 20;
const int FN_MSK_FREETEXT_SET = FN_NOOP + 21;
const int FN_MSK_TOGGLE_EXT = FN_NOOP + 22;
const int FN_MSK_TOGGLE_FREETEXT = FN_NOOP + 23;
//CCDLC
const int FN_MSK_CCDLC_DCT_CLICKED = FN_NOOP + 3000;
const int FN_MSK_CCDLC_ALT_CLICKED = FN_MSK_CCDLC_DCT_CLICKED + 1;
const int FN_MSK_CCDLC_SPD_CLICKED = FN_MSK_CCDLC_DCT_CLICKED + 2;
const int FN_MSK_CCDLC_SPD_M_CLICKED = FN_MSK_CCDLC_DCT_CLICKED + 3;
const int FN_MSK_CCDLC_TXT_CLICKED = FN_MSK_CCDLC_DCT_CLICKED + 4;
const int FN_MSK_CCDLC_DCT_SELECTED = FN_MSK_CCDLC_DCT_CLICKED + 5;
const int FN_MSK_CCDLC_ALT_SELECTED = FN_MSK_CCDLC_DCT_CLICKED + 6;
const int FN_MSK_CCDLC_SPD_SELECTED = FN_MSK_CCDLC_DCT_CLICKED + 7;
const int FN_MSK_CCDLC_TXT_ENTERED = FN_MSK_CCDLC_DCT_CLICKED + 8;
const int FN_MSK_CCDLC_REQUEST_CLICKED = FN_MSK_CCDLC_DCT_CLICKED + 9;
const int FN_MSK_CCDLC_REPLY_SELECTED = FN_MSK_CCDLC_DCT_CLICKED + 10;
const int FN_MSK_CCDLC_FREETEXT_CLICKED = FN_MSK_CCDLC_DCT_CLICKED + 11;
const int FN_MSK_CCDLC_FREETEXT_ENTERED = FN_MSK_CCDLC_DCT_CLICKED + 12;
const int FN_MSK_CCDLC_CUSTOM_REQ = FN_MSK_CCDLC_DCT_CLICKED + 13;
const int FN_MSK_CCDLC_REQUEST_TRACK = FN_MSK_CCDLC_DCT_CLICKED + 14;	// not in use

// Aircraft statuses
const int ACFT_STATUS_GROUND = 0;
const int ACFT_STATUS_MOV = 1;
const int ACFT_STATUS_TAX = 2;
const int ACFT_STATUS_ROLL = 3;
const int ACFT_STATUS_CLB = 4;
const int ACFT_STATUS_ALT = 5;
const int ACFT_STATUS_CRZ = 6;
const int ACFT_STATUS_DES = 7;
const int ACFT_STATUS_HOLD = 8;

inline string safe_substr(string s, int start, int count)
{
	int len = s.length();
	if (len < (start + 1))
	{
		return "";
	}
	return s.substr(start, count);
}

inline bool AutoStsIsGround(int sts) {
	return (sts <= ACFT_STATUS_ROLL);
}

const string STS_GND[] = {
	"-CLR-", "STBY", "FREQ", "READY", "HOFF", "PUSH", "START", "TAXI", "DICE", "L/UP", "DEPA"
};

inline bool IsGroundStatus(string Sts)
{
	return find(begin(STS_GND), end(STS_GND), Sts) != end(STS_GND);
}

const string STS_AIR[] = {
	"-CLR-", "WAIT", "SQWK", "STAR", "AWAY", "NOREP", "VECT"
};
const vector<string> MOSCOW_PORTS = { "UUDD", "UUEE", "UUWW", "UUBW", "UUOO", "UWGG", "UUBP", "UUOK", "UUOB", "UUDL", "UUOL", "UUMU", "UUMO", "UUBK",
"UUBS", "UUOR", "UUOS", "UUBG", "UUBD", "UUBT", "UUWE", "UUBC", "UUEM", "UUEI", "UUMB", "UUBL", "UUBI", "UUBA",
"UUMT", "UUTO", "UUOT", "UUWH", "UUML", "UUOD", "UUEH", "XUEM", "XUMU", "XUBI", "UUMB", "XUBN", "UUDG", "UUBM", "XUMK", "UUML", "UUWH", "UUWE", "UUMW", "UUEY", "UUEM", "XUNB" };

const vector<string> MUDR_PORTS = { "UUEE", "UUDD", "UUWW", "UUBW", "UUMU", "UUMO", "UUMB", "XUBN", "UUDG", "UUBM", "XUMK", "UUML", "UUWH", "UUWE", "UUMW", "UUEY", "UUEM", "XUBN" };

const string ATC_RATINGS[] = { "--", "OB", "S1", "S2", "S3", "C1", "C2", "C3", "I1", "I2", "I3", "SP", "AD" };

inline string GetPilotRating(int v)
{
	switch (v)
	{
		case 0: return "NPL";
		case 1:	return "PPL";
		case 3: return "IR";
		case 7: return "CMEL";
		case 15: return "ATPL";
	}
	return "---";
}

const int ES_FACILITY_OBS = 0;
const int ES_FACILITY_FSS = 1;
const int ES_FACILITY_DEL = 2;
const int ES_FACILITY_GND = 3;
const int ES_FACILITY_TWR = 4;
const int ES_FACILITY_APPDEP = 5;
const int ES_FACILITY_CTR = 6;

const int MAX_SHAPSHOT_LENGTH = 4;

const int	INT_MIN_LAT = 54;
const int	INT_MAX_LAT = 59;
const int	INT_MIN_LON = 30;
const int	INT_MAX_LON = 47;

struct ES_Route_Item {
	int Index = -1;
	string Name = "";
	CPosition Pos;
};

struct ControllerEntry {

	string Callsign;
	string Id;
	int Facility;
	string Port;
};

const int EXT_REQ_PILOT = 1;

struct ExternalRequest {

	int Type = -1;
	string ID = "";
	bool Deleted = false;
	void* Req = NULL;
};

struct ExternalRequest_Pilot {

	string Callsign;
	bool Fetched = false;

	string FullName;
	int PilotRating = 0;
	int ATCRating = 0;
	int BadGuyLevel = 0;
	int PilotHours = 0;
	int MoscowFlightCount = 0;
	string ExtendedType;
	int Cid = 0;
};

struct MskListDisplay {				// thread safe
	bool Depa = false;
	bool Land = false;
	bool Trans = false;

	static bool any(MskListDisplay e)
	{
		return e.Depa || e.Land || e.Trans;
	}
};

struct GeoPoint
{
	double x;
	double y;
};
struct ComputeTask {

	string Callsign = "";
	bool RebuildRoute = false;
	bool ResetRoute = false;
	string DCT = "";
};
struct OwnPlanState {

	bool del_concerned = false;
	bool gnd_in_concerned = false;
	bool gnd_out_concenred = false;
	bool twr_in_concerned = false;
	bool twr_out_concerned = false;
	bool app_in_concerned = false;
	bool app_out_concerned = false;
	bool ctr_in_concerned = false;
	bool ctr_out_concerned = false;
};
struct SectorArea {

	string Name;
	int BelowAlt = 0;
	int AboveAlt = 0;
	vector<GeoPoint> Coords;
};
struct AreaOwnership {

	string AreaName;
	int Above = 0;
	int Below = 0;
	bool IsRadar = false;
	string PrimaryCSPattern = "";
	string SecondaryCSPattern = "";

	vector<ControllerEntry> PrimaryOwners;
	vector<ControllerEntry> SecondaryOwners;

	bool operator < (const AreaOwnership& other) const
	{
		if (AreaName == other.AreaName)
			return Above < other.Above;
		else return AreaName < other.AreaName;
	}

	bool ImOwningIt(string MyCallsign)
	{
		return find_if(begin(PrimaryOwners), end(PrimaryOwners), [&MyCallsign](const ControllerEntry& e) {	return e.Callsign == MyCallsign; }) != end(PrimaryOwners) ||
			find_if(begin(SecondaryOwners), end(SecondaryOwners), [&MyCallsign](const ControllerEntry& e) {	return e.Callsign == MyCallsign; }) != end(SecondaryOwners);
	}
};
struct RwyThreshold {
	string Port;
	string RWY;
	GeoPoint LatLon;
	string Caps;
	string FAF;
};
struct Int_Cell_Entry {
	CPosition Pos;
	int Altitide = -1;
};
struct AircraftRadarDisplayInfo {

	bool Display10km = false;
	bool Display5km = false;
	bool DisplayRoute = false;
};
struct GeoPosition {

};
struct ManualPoint {
	string Name = "";
	CPosition Pos;
	string AfterPoint = "";
};
struct PointDetails {

	string Name = "";
	int ExpectedAlt = -1;
	time_t ExpectedPassTime = -1;
	time_t ExpectedDirectPassTime = -1;
	CPosition Position;
	double Distance = -1;
	double DirectDistance = -1;
	bool Removed = false;
	bool Manual = false;
	bool Free = false;
	string StarConstraint = "";
	string SidConstraint = "";

	bool IsValid()
	{
		return Name != "";
	}
};
struct CCDLC_Message {
	int Id = 0;
	string Author;
	string CS;
	string Cmd;
	string Value;
	vector<string> Raw;
	string Raw_S;
};
struct ActiveCCDLCRequest {
	string CS;
	string Init;
	string Request;
	string Text;
	string Likes;
	string Dislikes;

	bool IsValid()
	{
		return CS != "";
	}
};
struct CCDLC_Menu {
	bool Show = false;
	RECT rect;
	CPosition ps;
};
struct AircraftRouteEntry {

	PointDetails point;
	AircraftRouteEntry* prev = NULL;
	AircraftRouteEntry* next = NULL;
};
struct ControllerInfo {

	string ActiveControllerCS = "";
	int ActiveControllerFacility = -1;
	string PreviousControllerCS = "";
	string ActiveControllerId = "";
	string PreviousControllerId = "";
	string NextControllerId = "";
	bool TrackedByMe = false;
	bool IHaveControlledIt = false;
	bool IsTrackedByOther = false;
	bool IsTransferToMe = false;
	bool IsTransferFromMe = false;
	int OtherTrackerFacility = -1;
};
struct PlanInfo {
	int PlanState = -1;
	int PlanType = -1;
	bool IsExpectedOrCurrent = false;
	bool IsCurrent = false;
	bool IsOnlyExpected = false;
	bool IsOut = false;

	int OwnPlanState = FLIGHT_PLAN_STATE_NON_CONCERNED;

	bool IsValid()
	{
		return PlanState > -1;
	}
};
struct ExtendedFormularData {

	bool IsShown = false;

	bool Rnav = true;
	bool RVSM = true;
	bool Newbie = false;
	bool Badguy = false;
	int Hours = 0;
	int ATCRating = 0;
	int PilotRating = 0;
	bool LocalMarker = false;
	bool GlobalMarker = false;
	string FreeText = "";
	string Name = "";
	bool ShowFreeText = false;

	bool warn()
	{
		return Newbie || Badguy || ATCRating >= 11 || LocalMarker || GlobalMarker;
	}
	string ft1()
	{
		if (!ShowFreeText)	return "";
		return FreeText != "" ? safe_substr(FreeText, 0, 15) : "FREETEXT";
	}
	string ft2()
	{
		if (!ShowFreeText) return "";
		return FreeText != "" ? safe_substr(FreeText, 15, 100) : "";
	}
};
struct FormularData {

	string Callsign;
	int AutoStatus = ACFT_STATUS_GROUND;
	bool IsCharlie = false;
	string FAF = "";
	string CAPS = "";
	bool HighGrad = false;
	bool IsHeavy = false;
	int TTL = -1;
	int SecondsToLand = -1;
	time_t SecondsToLandControlTime = -1;
	bool SpeedInNm = true;
	bool VSInFm = true;
	time_t AcceptedTime = 0;
	string SortIndex = "00";
	string MarkerType = "";
	string CommType = "";
	string ActualSquawk = "2000";
	string AssignedSquawk = "2000";
	bool HasClearance = false;
	int FinalAltitude = -1;
	int ClearedAltitude = -1;
	bool CFL_IsAltitude = false;
	int AssignedSpeed = -1;
	int AssignedMach = -1;
	int AssignedRate = -1;
	int AssignedHeading = -1;

	ExtendedFormularData ExtendedData;

	bool IsValid()
	{
		return Callsign != "";
	}
};

struct CustomAssignedData {

	string DepartureRwy;
	string DestinationRwy;
	string Sid;
	string Star;
	string DepartureICAO;
	string DestinationICAO;
	bool CTL = false;
	bool ImEnglish = false;
	bool SpeedUnrestricted = false;
	bool AltUnrestricted = false;
	string ManualStatus = "";
	string ReadyTime = "";
	string ActualOffBlockTime = "";
	string ScheduledOffBlockTime = "";
	bool TrackRequested = false;
};
struct AircraftFlightData {

	CPosition PresentPosition;
	int GroundSpeed = -1;
	int Pitch = -1;
	int Bank = -1;
	int VerticalSpeed = -1;
	int CurrentAltitude = -1;	
	bool AFL_IsAltitude = false;
	int Track = -1;
	int Heading = -1;
	int TrueHeading = -1;
	bool IsAir = false;
	CustomAssignedData CustomData;

	time_t SnapshotTime = 0;
	int __PressureAlt = 0;
	int __FlightLevel = 0;
	int TA = 0;
	bool DataIsFirst = true;

	bool IsValid()
	{
		return this->GroundSpeed > -1;
	}
};

class CMskPlugin;
class CMskRadarScreen;
class AircraftExtendedData;

typedef void (CMskPlugin::* PluginTagFunctionParserFN)(int, string, CFlightPlan, CRadarTarget, string, POINT, RECT);
typedef void (CMskPlugin::* RadarTagFunctionParserFN)(int, string, CFlightPlan, CRadarTarget, string, POINT, RECT);
typedef void (*AircraftExtDataInvokator)(AircraftExtendedData*, void*, void*, void*, void*);
typedef void RoutePointEnumFN(PointDetails*);



struct PluginTagFunctionParser {
	int ItemCode = 0;
	string DisplayName;
	PluginTagFunctionParserFN Parser;
};

struct RadarTagFunctionParser
{
	int ItemCode = 0;
	string DisplayName;
	RadarTagFunctionParserFN Parser;
};

struct ScreenObjectInfo;

typedef void (CMskRadarScreen::* ScreenObjectEvent)(ScreenObjectInfo, string);
typedef void (CMskRadarScreen::* ScreenObjectDrawEvent)(ScreenObjectInfo, HDC);

struct ScreenObjectInfo {

	string Name;
	bool Over = false;
	bool Click = false;
	bool Movable = false;
	CPosition Pos;
	POINT Point;
	RECT Area;
	int Button = 0;
	string Param1;
	string Param2;
	ScreenObjectEvent OnEvent;
	bool Permanent = false;
	bool Visible = false;
};

const double EARTH_RADIUS = 6371000.0;
const double PI = 3.14159265358979323846;

