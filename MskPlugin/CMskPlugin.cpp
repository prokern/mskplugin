#include "pch.h"
#include "framework.h"
#include "CMskRadarScreen.h"
#include "mycurl.h"
#include <iostream>
#include <fstream>
#include <locale>
#include <clocale>
#include <chrono>
#include <algorithm>
#include "HiddenWindow.h"

#define _WIN32_DCOM
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "Winmm.lib")
#include <comdef.h>
#include <Wbemidl.h>
#include "CCDLC.h"
#include "json.h"

#pragma warning( disable : 4244)

using namespace std;
using namespace EuroScopePlugIn;


void CMskPlugin::GetMousePosition_FN()
{
	while (!Terminating)
	{
		POINT p;
		GetCursorPos(&p);
		MouseX = p.x;
		MouseY = p.y;
		this_thread::sleep_for(50ms);
	}
	Plugin->ThreadRunning--;
}

LRESULT CALLBACK CallWndProc_Mouse(
	_In_ int    nCode,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
)
{
	if (nCode == HC_ACTION) {
		MOUSEHOOKSTRUCT* data = (MOUSEHOOKSTRUCT*)lParam;

		if (wParam == WM_LBUTTONDBLCLK)
		{
			if (Plugin != nullptr)
			{
				Plugin->OnLeftButtonDlbClicked();
			}
		}
	}
	return CallNextHookEx(0, nCode, wParam, lParam);
}

CMskPlugin::CMskPlugin() : EuroScopePlugIn::CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE,
	MY_PLUGIN_NAME.c_str(),
	MY_PLUGIN_VERSION.c_str(),
	MY_PLUGIN_DEVELOPER.c_str(),
	MY_PLUGIN_COPYRIGHT.c_str())
{
	Plugin = this;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	MyPath = GetMyPath();

	if (this->MyPath == "") {
		DisplayUrgent("Unable to resolve self path");
		return;
	}

	globalMouseHook = SetWindowsHookExA(WH_MOUSE, &CallWndProc_Mouse, AfxGetInstanceHandle(), 0);

	Ccdlc = new CCDLC();
	Lic = new License();

	if (Lic->check_file() == "XAZ")
	{
		thread s([] {
			this_thread::sleep_for(10000ms);
			Plugin->Ccdlc->AddCCDLCSystemMessageToQueue("ELHO:" + Plugin->MyCid);
		});
		s.detach();

		auto crl = mycurl();
		auto ccdlc_msg = crl.get("http://beta2.uuwv.ru/ccdlc.php?x=1&cid=" + MyCid);
		if (ccdlc_msg == "REJECT")
		{
			remove((MyPath + "lic.dat").c_str());
			DisplayUrgent("License rejected");
			return;
		}

		ExternalData = new ExternalDataProcessor();
		History = new PlaneHistory();
		Controllers = new ControllerList();
		Planes = new AircraftExtendedDataPool();
		ToCompute = new ComputationTaskPool();
		Aman = new AMAN();
		Afv = new AFV();
		Tags = new TagProcessor();

		RegisterDepartureFp();
		RegisterLandingFp();
		RegisterCCDLCFp();
		RegisterTransitFp();

		thread MOUSEPOS(&CMskPlugin::GetMousePosition_FN, this);
		ThreadRunning++;
		MOUSEPOS.detach();

		LoadExternalDataFiles();

		DisplaySilent(std::string("Version " + MY_PLUGIN_VERSION + " load completed"));
	}
}

bool CMskPlugin::LoadAreas()
{
	if (MyPath == "") return false;

	bool errors = false;
	SectorAreas.clear();

	try {
		ifstream file(MyPath + "AREAS.txt");
		string str;

		int lineNo = 0;
		while (std::getline(file, str))
		{
			lineNo++;
			std::vector<string> array = SplitStringByChar(str, '\t');
			if (array.size() < 2) continue;

			if (array[0] == "OWNER" && array.size() >= 5)
			{
				Controllers->Ownership.push_back({ array[1],atoi(array[2].c_str()),atoi(array[3].c_str()), array[4]=="R", array[5], array.size() > 6 ? array[6] : "" });
			}
			if (array[0] == "AREA" && array.size() == 5) {

				vector<string> Coords = SplitStringByChar(array[4], ' ');
				if (Coords.size() > 1) {

					vector<GeoPoint> pts;

					for (auto& Coord : Coords) {

						trim(Coord);
						if (Coord == "") continue;

						vector<string> LatLon = SplitStringByChar(Coord, ',');
						if (LatLon.size() != 2) {
							DisplaySilent("Syntax error at line " + to_string(lineNo) + ". LatLon format error");
							errors = true;
							break;
						}
						trim(LatLon[0]);
						trim(LatLon[1]);
						double lat = atof(LatLon[0].c_str());
						double lon = atof(LatLon[1].c_str());

						pts.push_back({ lat,lon });
					}
					if (pts.size() < 3) {
						DisplaySilent("Syntax error at line " + to_string(lineNo) + ". At least 3 coordinates required");
						errors = true;
					}
					else {
						SectorAreas.push_back({ array[1], atoi(array[3].c_str()), 0, pts });
					}
				}
			}
			else if (array[0] == "THR" && array.size() == 6) {

				vector<string> LatLon = SplitStringByChar(array[4], ',');
				if (LatLon.size() != 2) {
					DisplaySilent("Syntax error at line " + to_string(lineNo) + ". LatLon format error");
					errors = true;
					break;
				}
				double lat = atof(LatLon[0].c_str());
				double lon = atof(LatLon[1].c_str());

				Rwys.push_back({ array[1], array[2], {lat,lon}, array[5], array[3] });

			}
			else if (array[0] == "TRA" && array.size() == 3)
			{
				TRA.insert({ array[1], atoi(array[2].c_str()) });
			}
		}
		if (errors) {
			SectorAreas.clear();
			return false;
		}

		// Sort controller sector ownership
		Controllers->Sort();

		return true;
	}
	catch (...) {
		return false;
	}
}

string CMskPlugin::GetMyPath() {

	try {
		char path[MAX_PATH];
		HMODULE hm = NULL;

		if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
			GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
			(LPCSTR) & "ReadFromSettings", &hm) != 0)
		{
			if (GetModuleFileName(hm, path, sizeof(path)) != 0)
			{
				return getPathName(string(path)) + '\\';
			}
		}
	}
	catch (const std::exception& exc) {
		DisplayUrgent(std::string("Exception when determine module path: " + string(exc.what())));
	}
	catch (const std::string& ex) {
		DisplayUrgent(std::string("Exception when determine module path: " + ex));
	}
	catch (...) {
		DisplayUrgent("Unknown exception when determine module path");
	}
	return "";
}

CMskPlugin::~CMskPlugin()
{
	Terminating = true;
	DisplaySilent("Waiting for thread to terminate");

	int remained = ThreadRunning;
	this_thread::sleep_for(500ms);
	DisplaySilent("Done. " + to_string(ThreadRunning));

	delete ExternalData;
	delete History;
	delete Controllers;
	delete Planes;
	delete ToCompute;
	delete Aman;
	delete Afv;
	delete Ccdlc;
	delete Tags;
}

CRadarScreen* CMskPlugin::OnRadarScreenCreated(const char* sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated)
{
	auto screen = new CMskRadarScreen(this);
	ScreenPool.push_back(screen);
	return screen;
}

void CMskPlugin::LoadExternalDataFiles()
{
	lock_guard<mutex> lock(__extern_data_locker);

	Constraints.Clear();

	auto cnt = Constraints.LoadFromFile(this->MyPath + "CST.txt");
	if (cnt > 0) {
		DisplaySilent(std::string("Loaded " + to_string(cnt) + " constraints"));
	}
	else {
		DisplayUrgent("Constraint load failed");
	}

	if (LoadAreas()) {
		DisplaySilent("Areas loaded. AREA: " + to_string(SectorAreas.size()) + ". RWY: " + to_string(Rwys.size()));
	}
	else {
		DisplayUrgent("Failed to load areas");
	}
}

void CMskPlugin::ManageAircraftFPList(string CS, MskListDisplay DisplayIn, ActiveCCDLCRequest CCDLC_Request)
{
	auto fp = FlightPlanSelect(CS.c_str());
	if (!fp.IsValid())	return;

	if (!DisplayIn.any(DisplayIn))
	{
		es_deplist.RemoveFpFromTheList(fp);
		es_ldglist.RemoveFpFromTheList(fp);
		es_trslist.RemoveFpFromTheList(fp);
	}
	else {
		if (DisplayIn.Depa)
		{
			es_deplist.AddFpToTheList(fp);
		}
		else if (!DisplayIn.Depa)
		{
			es_deplist.RemoveFpFromTheList(fp);
		}
		if (DisplayIn.Land)
		{
			es_ldglist.AddFpToTheList(fp);
		}
		else if (!DisplayIn.Land)
		{
			es_ldglist.RemoveFpFromTheList(fp);
		}
		if (DisplayIn.Trans)
		{
			es_trslist.AddFpToTheList(fp);
		}
		else if (!DisplayIn.Trans)
		{
			es_trslist.RemoveFpFromTheList(fp);
		}
	}
	if (CCDLC_Request.IsValid())
	{
		ccdlc_list.AddFpToTheList(fp);
	}
	else ccdlc_list.RemoveFpFromTheList(fp);
}

void CMskPlugin::AddToCCDLCList(string CS)
{
	auto fp = Plugin->FlightPlanSelect(CS.c_str());
	if (fp.IsValid())
		Plugin->ccdlc_list.AddFpToTheList(fp);
}

void CMskPlugin::RemoveFromCCDLCList(string CS)
{
	auto fp = Plugin->FlightPlanSelect(CS.c_str());
	if (fp.IsValid())
		Plugin->ccdlc_list.RemoveFpFromTheList(fp);
}

void CMskPlugin::OnGetTagItem(CFlightPlan FlightPlan, CRadarTarget RadarTarget, int ItemCode, int TagData, char sItemString[16], int* pColorCode, COLORREF* pRGB, double* pFontSize)
{
	string DisplayValue = "";
	COLORREF DisplayColor = RGB(0, 0, 0);
	COLORREF FinalColor = RGB(0, 0, 0);
	string CS = check_str_create(RadarTarget.GetCallsign());

	auto tag = Tags->GetTag(ItemCode);
	if (tag != NULL && tag->IsValid())
	{
		try {
			(Tags->*tag->FN)(ItemCode, CS, DisplayValue, DisplayColor);
		}
		catch (...)
		{
			DisplayUrgent("Calling on tag exception: " + to_string(ItemCode));
		}

		if (DisplayColor != DefaultColor)
		{
			FinalColor = DisplayColor;
		}
		if (Afv->IsTransmit(CS) && tag->HighLightWhenTX)
		{
			FinalColor = CLR_TRX;
		}
		if (tag->OverrideColorEvenIfTX && DisplayColor != DefaultColor && FinalColor == CLR_TRX)
		{
			FinalColor = DisplayColor;
		}
		if (FinalColor != DefaultColor)
		{
			*pRGB = FinalColor;
			*pColorCode = TAG_COLOR_RGB_DEFINED;
		}
		if (DisplayValue != "")
		{
			strncpy_s(sItemString, 16, DisplayValue.c_str(), -1);
		}
	}
}

bool CMskPlugin::ICanUseCCDLC()
{
	auto c = ControllerMyself();
	if (!c.IsValid())	return false;
	return c.GetFacility() >= 4;
}

CSectorElement CMskPlugin::FindSectorElement(const int Type, string Name)
{
	SelectActiveSectorfile();
	for (auto p = SectorFileElementSelectFirst(Type); p.IsValid(); p = SectorFileElementSelectNext(p, Type))
	{
		if (p.GetName() == Name.c_str())
			return p;
	}
	return CSectorElement();
}

void CMskPlugin::OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area)
{
	auto fp = FlightPlanSelectASEL();
	if (!fp.IsValid()) return;

	auto rt = fp.GetCorrelatedRadarTarget();
	if (!rt.IsValid())	return;

	auto CS = check_str_create(fp.GetCallsign());
	if (CS == "") return;

	try {
		auto fn = Tags->GetFN(FunctionId);

		if (fn != NULL)
		{
			(Tags->*fn->FN)(FunctionId, CS, fp, rt, string(sItemString), Pt, Area);
		}
	}
	catch (...)
	{
		DisplayUrgent("Exception raised in OnFunctionCall. Id: " + to_string(FunctionId) + ", CS = " + CS);
	}
}

void CMskPlugin::DisplaySilent(string Msg) {

	lock_guard<mutex> lock(__disp);
	DisplayUserMessage("Moscow Plugin for ES", "Moscow Plugin for ES", Msg.c_str(), true, false, false, false, false);
}

void CMskPlugin::DisplayUrgent(string Msg) {
	lock_guard<mutex> lock(__disp);
	DisplayUserMessage("Moscow Plugin for ES", "Moscow Plugin for ES", Msg.c_str(), true, true, true, true, true);
}

void CMskPlugin::GetMyPort()
{
	auto im = ControllerMyself();
	MyPort = "";
	MyCS = "";

	if (im.IsValid()) {

		auto CS = check_str_create(im.GetCallsign());
		MyId = check_str_create(im.GetPositionId());
		MyCS = CS;
		MyFacility = im.GetFacility();
		MyName = im.GetFullName();
		if (CS != "") {
			try {
				MyPort = safe_substr(CS, 0, 4);
			}
			catch (...) {
				MyPort = "";
			}
			if (MyPort == "MSK_") {
				MyPort = "";
			}
			else if (MyPort == "UUWV")
			{
				MyPort = "";
			}
			else if (MyPort.find("_") != string::npos)
			{
				MyPort = "";
			}
		}
	}
}

bool CMskPlugin::ImController()
{
	auto c = ControllerMyself();
	if (!c.IsValid()) return false;
	return c.GetFacility() > 0;
}

void CMskPlugin::OnTimer(int Counter)
{
	GetMyPort();
	if (History != NULL)
		History->CleanUp();

	// For instructors: track plane
	if (TranslateASEL)
	{
		auto asel = RadarTargetSelectASEL();
		if (asel.IsValid())
		{
			auto current_asel_cs = check_str_create(asel.GetCallsign());
			if (current_asel_cs != LastASELCS)
			{
				Plugin->Ccdlc->AddCCDLCSystemMessageToQueue("ASEL:" + MyId + ":" + current_asel_cs);
			}
			LastASELCS = current_asel_cs;
		}
	}
	if (Planes != NULL)
	{
		while (true)
		{
			string computed_cs = Planes->GetComputedPlaneCS();
			if (computed_cs != "")
			{
				MskListDisplay DisplayIn;
				ActiveCCDLCRequest ccdlc_req;
				{
					ExtendedDataRequest r;
					auto plane = r.request(computed_cs);
					if (plane == NULL)
						continue;
					DisplayIn = plane->DisplayIn;
					ccdlc_req = plane->CCDLCRequest;
				}
				ManageAircraftFPList(computed_cs, DisplayIn, ccdlc_req);
			}
			else break;
		}
	}
}

bool CMskPlugin::OnCompileCommand(const char* sCommandLine)
{
	string Cmd = string(sCommandLine);

	if (safe_substr(Cmd, 0, 5) == ".aman")
	{
		Aman->UseAman = !Aman->UseAman;
		DisplayUrgent("AMAN is " + (!Aman->UseAman ? string("NOT displayed") : string("displayed")));
		RegisterLandingFp();
		RegisterTransitFp();
		return true;
	}
	else if (safe_substr(Cmd, 0, 5) == ".time")
	{
		UseTimes = !UseTimes;
		DisplayUrgent("TIME is " + (!UseTimes ? string("NOT displayed") : string("displayed")));
		RegisterDepartureFp();
		return true;
	}
	else if (safe_substr(Cmd, 0, 7) == ".reload")
	{
		LoadExternalDataFiles();
	}
	else if (safe_substr(Cmd, 0, 3) == ".tx")
	{
		auto pieces = SplitStringByChar(Cmd, ' ');
		if (pieces.size() == 1)
		{
			Afv->AddAFVMessageToQueue("");
			return true;
		}
		string cs = pieces[1];
		Afv->AddAFVMessageToQueue(cs);

		return true;
	}
	else if (safe_substr(Cmd, 0, 6) == ".track")
	{
		if (!ImController())
			return true;

		auto pieces = SplitStringByChar(Cmd, ' ');
		if (pieces.size() != 2) return true;

		if (pieces[1] == "stop")
		{
			ReceiveASELFrom = "";
			DisplaySilent("Tracking stopped");
			Afv->AddAFVMessageToQueue("");
		}
		else {

			Ccdlc->AddCCDLCSystemMessageToQueue("TRACK:" + MyId + ":" + pieces[1]);
			ReceiveASELFrom = pieces[1];
			DisplaySilent("Requested " + pieces[1] + " to translate ASEL");
		}
	}
	else if (safe_substr(Cmd, 0, 4) == ".reg")
	{
		auto pieces = SplitStringByChar(Cmd, ' ');
		if (pieces.size() != 2) return true;

		string vatsim_id = pieces[1];
		string act_chg = Lic->create_challenge(vatsim_id);

		QrShow = true;
		QRAddr = "https://plugin.uuwv.ru/reg.php?c=" + vatsim_id + "&a=" + act_chg;
		return true;
	}
	if (Cmd == ".geo") {

		GeoMode = !GeoMode;
		GeoPoints.clear();

		DisplayUrgent("Geo mode is " + (this->GeoMode == true ? string("ON") : string("OFF")));

		return true;
	}
	else if (Cmd == ".displayall")
	{
		DisplayAllPorts = !DisplayAllPorts;
		DisplaySilent("OK");
		return true;
	}
	else if (Cmd == ".build") {

		string result = "";
		for (auto& P : GeoPointsCoords) {
			result = result + to_string(P.x) + "," + to_string(P.y) + " ";
		}
		DisplaySilent("Done. Total: " + to_string(GeoPointsCoords.size()) + " points");
		toClipboard(result);
		GeoPoints.clear();
		GeoPointsCoords.clear();
		GeoMode = false;

		return true;
	}
	else if (Cmd == ".myid") {
		auto ctr = ControllerMyself();
		if (!ctr.IsValid()) return false;
		DisplaySilent("My ID: " + string(ctr.GetFullName()));
		return true;
	}
	else if (Cmd == ".depa") {
		es_deplist.ShowFpList(true);
	}
	else if (Cmd == ".land") {
		es_ldglist.ShowFpList(true);
	}
	else if (Cmd == ".trans") {
		es_trslist.ShowFpList(true);
	}
	else if (Cmd == ".ccdlc") {
		ccdlc_list.ShowFpList(true);
	}
	else if (Cmd.substr(0, 6) == ".cccmd")
	{
		auto tokens = SplitStringByChar(Cmd.substr(7), ':');

		CCDLC_Message msg;

		msg.Author = tokens[1];
		msg.Cmd = tokens[0];
		msg.CS = tokens[2];
		msg.Id = 0;

		for (size_t i = 3; i < tokens.size(); i++)
		{
			msg.Value += tokens[i] + " ";
		}
		trim(msg.Value);
		msg.Raw = tokens;
		Plugin->Ccdlc->ExecCCDLCMessage(msg);
	}
	else if (Cmd.substr(0, 5) == ".show") {
		string zone = Cmd.substr(6);
		__hightlight = zone;
		if (zone == "ALL") {
			__hightlight = "ALL";
		}
	}
	else if (Cmd == ".debug")
	{
		DisplaySilent("Begin of FP list");

		for (auto fp = FlightPlanSelectFirst(); fp.IsValid(); fp = FlightPlanSelectNext(fp)) {

			if (!fp.IsValid()) continue;

			auto fsstate = to_string(fp.GetFPState());
			auto state = to_string(fp.GetState());
			auto cs = string(fp.GetCallsign());
			auto from = string(fp.GetFlightPlanData().GetOrigin());
			auto to = string(fp.GetFlightPlanData().GetDestination());

			DisplaySilent("FP: " + cs + " " + from + " " + to + " S:" + state + " FP:" + fsstate);
		}
		DisplaySilent("End of FP list");
	}
	else if (Cmd == ".owners")
	{
		string s = "";
		for (auto& Z : Controllers->Ownership)
		{
			s = Z.AreaName + " " + to_string(Z.Above) + "-" + to_string(Z.Below) + ". PRI: ";
			for_each(begin(Z.PrimaryOwners), end(Z.PrimaryOwners), [&s](const ControllerEntry& e) {	s += e.Callsign + " ";	});
			s += "SEC: ";
			for_each(begin(Z.SecondaryOwners), end(Z.SecondaryOwners), [&s](const ControllerEntry& e) {	s += e.Callsign + " ";	});

			DisplaySilent(s);
		}
		
		return true;
	}
	else if (Cmd == ".getzones") {

		auto ASEL = RadarTargetSelectASEL();
		if (!ASEL.IsValid()) return false;

		ExtendedDataRequest req;
		auto p = req.request(ASEL.GetCallsign());

		string r = "";
		for (auto& P : p->Route.CrossAreasNames)
		{
			r += P + " ";
		}
		DisplaySilent(r);

		return true;
	}
	return true;
}

void CMskPlugin::RegisterTransitFp()
{
	if (!es_trslist.IsValid())
	{
		es_trslist = RegisterFpList("MSK Transit");
	}
	es_trslist.DeleteAllColumns();

	es_trslist.AddColumnDefinition("RT", 1, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_XMT, NULL, TAG_ITEM_FUNCTION_NO, NULL, TAG_ITEM_FUNCTION_NO);
	es_trslist.AddColumnDefinition("T", 2, false, NULL, TAG_ITEM_TYPE_COMMUNICATION_TYPE, NULL, TAG_ITEM_FUNCTION_COMMUNICATION_POPUP, MY_PLUGIN_NAME.c_str(), FN_MSK_SELECT_TEXT_MESSAGE);
	es_trslist.AddColumnDefinition("L", 1, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_LANG_TX, NULL, TAG_ITEM_FUNCTION_NO, NULL, TAG_ITEM_FUNCTION_NO);
	es_trslist.AddColumnDefinition("C/S", 7, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_OWN_CS_TX, NULL, TAG_ITEM_FUNCTION_HANDOFF_POPUP_MENU, MY_PLUGIN_NAME.c_str(), FN_MSK_CALLSIGN_MENU_OPEN);
	es_trslist.AddColumnDefinition("DEPA", 5, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_DEPA_PORT_TX, NULL, TAG_ITEM_FUNCTION_OPEN_FP_DIALOG, NULL, TAG_ITEM_FUNCTION_NO);
	es_trslist.AddColumnDefinition("DEST", 5, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_DEST_PORT_TX, NULL, TAG_ITEM_FUNCTION_OPEN_FP_DIALOG, NULL, TAG_ITEM_FUNCTION_NO);
	es_trslist.AddColumnDefinition("SQ", 5, false, NULL, TAG_ITEM_TYPE_ASSIGNED_SQUAWK, NULL, TAG_ITEM_FUNCTION_SQUAWK_POPUP, NULL, TAG_ITEM_FUNCTION_NO);
	es_trslist.AddColumnDefinition("POS", 8, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_CURRENT_AREA, MY_PLUGIN_NAME.c_str(), FN_MSK_CENTER_TARGET, NULL, TAG_ITEM_FUNCTION_NO);
	es_trslist.AddColumnDefinition("MAN", 5, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_MANUAL_STS, MY_PLUGIN_NAME.c_str(), FN_MSK_MAN_STS_CLICKED, MY_PLUGIN_NAME.c_str(), FN_MSK_MAN_STS_CLEARED);
	es_trslist.AddColumnDefinition("STS", 4, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_CURRENT_STS, NULL, TAG_ITEM_FUNCTION_NO, NULL, TAG_ITEM_FUNCTION_NO);
	es_trslist.AddColumnDefinition("AFL", 4, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_OWN_AFL_TX, NULL, TAG_ITEM_FUNCTION_NO, NULL, TAG_ITEM_FUNCTION_NO);
	es_trslist.AddColumnDefinition("CFL", 4, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_OWN_CFL_TX, NULL, TAG_ITEM_FUNCTION_TEMP_ALTITUDE_POPUP, NULL, TAG_ITEM_FUNCTION_NO);
	es_trslist.AddColumnDefinition("FFL", 4, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_OWN_FFL_TX, NULL, TAG_ITEM_FUNCTION_COPN_ALTITUDE, NULL, TAG_ITEM_FUNCTION_NO);

	//TODO: MODIFY ACCORDING (LEFT -> DISPLAY ROUTE, RIGHT -> PLAN SETTINGS)

	if (Aman->UseAman)
	{
		es_trslist.AddColumnDefinition("POINT", 6, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_AMAN_POINT, MY_PLUGIN_NAME.c_str(), FN_MSK_AMAN_POINT, NULL, TAG_ITEM_FUNCTION_NO);
		es_trslist.AddColumnDefinition("PTIME", 6, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_AMAN_POINT_TIME, NULL, TAG_ITEM_FUNCTION_NO, NULL, TAG_ITEM_FUNCTION_NO);
		es_trslist.AddColumnDefinition("PDST", 4, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_AMAN_MILES, NULL, TAG_ITEM_FUNCTION_NO, NULL, TAG_ITEM_FUNCTION_NO);
	}

	es_trslist.AddColumnDefinition("I", 2, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_POSITION, NULL, TAG_ITEM_FUNCTION_NO, NULL, TAG_ITEM_FUNCTION_NO);
}

void CMskPlugin::RemoveFromAllLists(CFlightPlan fp)
{
	es_deplist.RemoveFpFromTheList(fp);
	es_ldglist.RemoveFpFromTheList(fp);
	es_trslist.RemoveFpFromTheList(fp);
	ccdlc_list.RemoveFpFromTheList(fp);
}

void CMskPlugin::RegisterCCDLCFp() {

	ccdlc_list = RegisterFpList("CCDLC");
	ccdlc_list.DeleteAllColumns();

	ccdlc_list.AddColumnDefinition("CS", 8, false, NULL, TAG_ITEM_TYPE_CALLSIGN, NULL, TAG_ITEM_FUNCTION_NO, NULL, TAG_ITEM_FUNCTION_NO);
	ccdlc_list.AddColumnDefinition("POS", 8, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_CURRENT_AREA, MY_PLUGIN_NAME.c_str(), FN_MSK_CENTER_TARGET, NULL, TAG_ITEM_FUNCTION_NO);
	ccdlc_list.AddColumnDefinition("SRC", 3, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_CCDLC_INITIATOR, NULL, TAG_ITEM_FUNCTION_NO, NULL, TAG_ITEM_FUNCTION_NO);
	ccdlc_list.AddColumnDefinition("REQUEST", 15, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_CCDLC_REQUEST, NULL, TAG_ITEM_FUNCTION_NO, NULL, TAG_ITEM_FUNCTION_NO);
	ccdlc_list.AddColumnDefinition("REQUEST2", 15, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_CCDLC_REQUEST_EXT, NULL, TAG_ITEM_FUNCTION_NO, NULL, TAG_ITEM_FUNCTION_NO);
	ccdlc_list.AddColumnDefinition("REACT", 15, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_CCDLC_REACTIONS, NULL, TAG_ITEM_FUNCTION_NO, NULL, TAG_ITEM_FUNCTION_NO);
	ccdlc_list.AddColumnDefinition("COMMENT", 15, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_CCDLC_FREETEXT, NULL, TAG_ITEM_FUNCTION_NO, NULL, TAG_ITEM_FUNCTION_NO);
	ccdlc_list.AddColumnDefinition("COMMENT2", 15, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_CCDLC_FREETEXT_EXT, NULL, TAG_ITEM_FUNCTION_NO, NULL, TAG_ITEM_FUNCTION_NO);
}

void CMskPlugin::RegisterDepartureFp()
{
	if (!es_deplist.IsValid())
	{
		es_deplist = RegisterFpList("MSK Departures");
	}
	es_deplist.DeleteAllColumns();

	es_deplist.AddColumnDefinition("RT", 1, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_XMT, NULL, TAG_ITEM_FUNCTION_NO, NULL, TAG_ITEM_FUNCTION_NO);
	es_deplist.AddColumnDefinition("T", 2, false, NULL, TAG_ITEM_TYPE_COMMUNICATION_TYPE, NULL, TAG_ITEM_FUNCTION_COMMUNICATION_POPUP, MY_PLUGIN_NAME.c_str(), FN_MSK_SELECT_TEXT_MESSAGE);
	es_deplist.AddColumnDefinition("L", 1, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_LANG_TX, NULL, TAG_ITEM_FUNCTION_NO, NULL, TAG_ITEM_FUNCTION_NO);
	es_deplist.AddColumnDefinition("C/S", 7, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_OWN_CS_TX, NULL, TAG_ITEM_FUNCTION_HANDOFF_POPUP_MENU, MY_PLUGIN_NAME.c_str(), FN_MSK_CALLSIGN_MENU_OPEN);
	es_deplist.AddColumnDefinition("DEPA", 5, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_DEPA_PORT_TX, NULL, TAG_ITEM_FUNCTION_OPEN_FP_DIALOG, NULL, TAG_ITEM_FUNCTION_NO);
	es_deplist.AddColumnDefinition("DEST", 5, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_DEST_PORT_TX, NULL, TAG_ITEM_FUNCTION_OPEN_FP_DIALOG, NULL, TAG_ITEM_FUNCTION_NO);
	es_deplist.AddColumnDefinition("RWY", 4, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_DEPA_RWY_TX, NULL, TAG_ITEM_FUNCTION_ASSIGNED_RUNWAY, NULL, TAG_ITEM_FUNCTION_NO);
	es_deplist.AddColumnDefinition("SID", 8, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_SID_TX, NULL, TAG_ITEM_FUNCTION_ASSIGNED_SID, NULL, TAG_ITEM_FUNCTION_NO);
	es_deplist.AddColumnDefinition("NXT", 5, false, NULL, TAG_ITEM_TYPE_SECTOR_INDICATOR, NULL, TAG_ITEM_FUNCTION_NO, NULL, TAG_ITEM_FUNCTION_ASSIGNED_NEXT_CONTROLLER);
	es_deplist.AddColumnDefinition("SQ", 5, false, NULL, TAG_ITEM_TYPE_ASSIGNED_SQUAWK, NULL, TAG_ITEM_FUNCTION_SQUAWK_POPUP, NULL, TAG_ITEM_FUNCTION_NO);
	es_deplist.AddColumnDefinition("C", 2, false, NULL, TAG_ITEM_TYPE_CLEARENCE, NULL, TAG_ITEM_FUNCTION_SET_CLEARED_FLAG, NULL, TAG_ITEM_FUNCTION_NO);
	es_deplist.AddColumnDefinition("POS", 8, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_CURRENT_AREA, MY_PLUGIN_NAME.c_str(), FN_MSK_CENTER_TARGET, NULL, TAG_ITEM_FUNCTION_NO);
	es_deplist.AddColumnDefinition("MAN", 5, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_MANUAL_STS, MY_PLUGIN_NAME.c_str(), FN_MSK_MAN_STS_CLICKED, MY_PLUGIN_NAME.c_str(), FN_MSK_MAN_STS_CLEARED);
	es_deplist.AddColumnDefinition("STS", 4, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_CURRENT_STS, NULL, TAG_ITEM_FUNCTION_NO, NULL, TAG_ITEM_FUNCTION_NO);
	es_deplist.AddColumnDefinition("AFL", 4, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_OWN_AFL_TX, NULL, TAG_ITEM_FUNCTION_NO, NULL, TAG_ITEM_FUNCTION_NO);
	es_deplist.AddColumnDefinition("CFL", 4, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_OWN_CFL_TX, NULL, TAG_ITEM_FUNCTION_TEMP_ALTITUDE_POPUP, NULL, TAG_ITEM_FUNCTION_NO);
	es_deplist.AddColumnDefinition("FFL", 4, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_OWN_FFL_TX, NULL, TAG_ITEM_FUNCTION_COPN_ALTITUDE, NULL, TAG_ITEM_FUNCTION_NO);
	if (UseTimes)
	{
		es_deplist.AddColumnDefinition("RDTM", 6, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_READY_TIME, NULL, TAG_ITEM_FUNCTION_NO, NULL, TAG_ITEM_FUNCTION_NO);
		es_deplist.AddColumnDefinition("OBTM", 5, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_OFFBLOCK_TIME, NULL, TAG_ITEM_FUNCTION_NO, NULL, TAG_ITEM_FUNCTION_NO);
	}
	es_deplist.AddColumnDefinition("I", 2, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_POSITION, NULL, TAG_ITEM_FUNCTION_NO, NULL, TAG_ITEM_FUNCTION_NO);
}

void CMskPlugin::OnFlightPlanDisconnect(CFlightPlan FlightPlan)
{
	string CS = check_str_create(FlightPlan.GetCallsign());

	es_deplist.RemoveFpFromTheList(FlightPlan);
	es_ldglist.RemoveFpFromTheList(FlightPlan);
	es_trslist.RemoveFpFromTheList(FlightPlan);
	ccdlc_list.RemoveFpFromTheList(FlightPlan);

	ExtendedDataRequest req;

	if (req.check(CS))
	{
		auto p = req.request(CS);

		Plugin->Ccdlc->AddCCDLCSystemMessageToQueue("ACFT:LOST:" + CS);
		try {
			Plugin->Planes->SortedAcftList.erase(remove(Plugin->Planes->SortedAcftList.begin(), Plugin->Planes->SortedAcftList.end(), p), Plugin->Planes->SortedAcftList.end());
		}
		catch (...)
		{
			DisplayUrgent("Exception raised duing sorting Acft list: " + CS);
		}
		try {
			req.erase(CS);
		}
		catch (...)
		{
			DisplayUrgent("Exception raised duing erasing Plane of CS: " + CS);
		}
		Plugin->Planes->sortAcftList();
	}
}

void CMskPlugin::RegisterLandingFp()
{
	if (!es_ldglist.IsValid())
	{
		es_ldglist = RegisterFpList("MSK Landings");
	}
	es_ldglist.DeleteAllColumns();

	es_ldglist.AddColumnDefinition("RT", 1, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_XMT, NULL, TAG_ITEM_FUNCTION_NO, NULL, TAG_ITEM_FUNCTION_NO);
	es_ldglist.AddColumnDefinition("T", 2, false, NULL, TAG_ITEM_TYPE_COMMUNICATION_TYPE, NULL, TAG_ITEM_FUNCTION_COMMUNICATION_POPUP, MY_PLUGIN_NAME.c_str(), FN_MSK_SELECT_TEXT_MESSAGE);
	es_ldglist.AddColumnDefinition("L", 1, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_LANG_TX, NULL, TAG_ITEM_FUNCTION_NO, NULL, TAG_ITEM_FUNCTION_NO);
	es_ldglist.AddColumnDefinition("C/S", 7, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_OWN_CS_TX, NULL, TAG_ITEM_FUNCTION_HANDOFF_POPUP_MENU, MY_PLUGIN_NAME.c_str(), FN_MSK_CALLSIGN_MENU_OPEN);
	es_ldglist.AddColumnDefinition("SQ", 5, false, NULL, TAG_ITEM_TYPE_ASSIGNED_SQUAWK, NULL, TAG_ITEM_FUNCTION_SQUAWK_POPUP, NULL, TAG_ITEM_FUNCTION_NO);
	es_ldglist.AddColumnDefinition("DEST", 5, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_DEST_PORT_TX, NULL, TAG_ITEM_FUNCTION_OPEN_FP_DIALOG, NULL, TAG_ITEM_FUNCTION_NO);
	es_ldglist.AddColumnDefinition("RWY", 4, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_DEST_RWY_TX, NULL, TAG_ITEM_FUNCTION_ASSIGNED_RUNWAY, NULL, TAG_ITEM_FUNCTION_NO);
	es_ldglist.AddColumnDefinition("STAR", 9, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_STAR_TX, NULL, TAG_ITEM_FUNCTION_ASSIGNED_STAR, NULL, TAG_ITEM_FUNCTION_NO);
	es_ldglist.AddColumnDefinition("POS", 8, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_CURRENT_AREA, MY_PLUGIN_NAME.c_str(), FN_MSK_CENTER_TARGET, NULL, TAG_ITEM_FUNCTION_NO);
	es_ldglist.AddColumnDefinition("MAN", 5, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_MANUAL_STS, MY_PLUGIN_NAME.c_str(), FN_MSK_MAN_STS_CLICKED, MY_PLUGIN_NAME.c_str(), FN_MSK_MAN_STS_CLEARED);
	es_ldglist.AddColumnDefinition("STS", 4, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_CURRENT_STS, NULL, TAG_ITEM_FUNCTION_NO, NULL, TAG_ITEM_FUNCTION_NO);
	es_ldglist.AddColumnDefinition("AFL", 4, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_OWN_AFL_TX, NULL, TAG_ITEM_FUNCTION_NO, NULL, TAG_ITEM_FUNCTION_NO);
	es_ldglist.AddColumnDefinition("CFL", 4, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_OWN_CFL_TX, NULL, TAG_ITEM_FUNCTION_TEMP_ALTITUDE_POPUP, NULL, TAG_ITEM_FUNCTION_NO);
	es_ldglist.AddColumnDefinition("FAF", 3, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_FINAL_FIX_ALT, NULL, TAG_ITEM_FUNCTION_NO, NULL, TAG_ITEM_FUNCTION_NO);
	es_ldglist.AddColumnDefinition("SPD", 4, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_OWN_SPEED_TX, NULL, TAG_ITEM_FUNCTION_ASSIGNED_SPEED_POPUP, NULL, TAG_ITEM_FUNCTION_ASSIGNED_MACH_POPUP);
	es_ldglist.AddColumnDefinition("GS", 4, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_OWN_GS_TX, NULL, TAG_ITEM_FUNCTION_NO, NULL, TAG_ITEM_FUNCTION_NO);
	es_ldglist.AddColumnDefinition("TTL", 4, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_TIME_TO_LAND, MY_PLUGIN_NAME.c_str(), FN_MSK_TTL_CLICKED, NULL, TAG_ITEM_FUNCTION_NO);
	if (Aman->UseAman)
	{
		es_ldglist.AddColumnDefinition("POINT", 6, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_AMAN_POINT, MY_PLUGIN_NAME.c_str(), FN_MSK_AMAN_POINT, NULL, TAG_ITEM_FUNCTION_NO);
		es_ldglist.AddColumnDefinition("PTIME", 6, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_AMAN_POINT_TIME, NULL, TAG_ITEM_FUNCTION_NO, NULL, TAG_ITEM_FUNCTION_NO);
		es_ldglist.AddColumnDefinition("PDST", 4, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_AMAN_MILES, NULL, TAG_ITEM_FUNCTION_NO, NULL, TAG_ITEM_FUNCTION_NO);
	}
	es_ldglist.AddColumnDefinition("I", 2, false, MY_PLUGIN_NAME.c_str(), TAG_MSK_POSITION, NULL, TAG_ITEM_FUNCTION_NO, NULL, TAG_ITEM_FUNCTION_NO);
}

void CMskPlugin::RemoveRoutes()
{
	hide_display_route = true;
	if (Planes != NULL)
	{
		Planes->Enum([](AircraftExtendedData* p) {
			p->DisplayOnRadar.DisplayRoute = false;
			});
	}
	thread __t([&]() {
		this_thread::sleep_for(300ms);
		RemoveAllScreenObjects();
		});
	__t.detach();
}

void CMskPlugin::AllScreenCreatePermanentObject(string Name, CPosition Pos, bool Movable, string Param1, string Param2, ScreenObjectEvent OnEvent)
{
	for (auto& s : ScreenPool)
	{
		s->CreatePermanentScreenObject(Name, Pos, Movable, Param1, Param2, OnEvent);
	}
}

void CMskPlugin::RemoveAllScreenObjects()
{
	DisplayRadarWaypoint = false;
	for (auto& screen : ScreenPool)
	{
		screen->ClearScreenObjects();
	}
}

void CMskPlugin::OnLeftButtonDlbClicked()
{
	RemoveRoutes();
}

void CMskPlugin::OnFlightPlanFlightPlanDataUpdate(CFlightPlan FlightPlan)
{
	// THREAD CONTEXT: MAIN
	if (!FlightPlan.IsValid())	return;
	auto rt = FlightPlan.GetCorrelatedRadarTarget();
	if (!rt.IsValid())	return;

	ToCompute->add({ check_str_create(rt.GetCallsign()), true, false });
}

void CMskPlugin::OnRadarTargetPositionUpdate(CRadarTarget RadarTarget)
{
	// THREAD CONTEXT: MAIN
	if (!RadarTarget.IsValid()) return;
	auto CS = check_str_create(RadarTarget.GetCallsign());
	auto fp = RadarTarget.GetCorrelatedFlightPlan();
	if (!fp.IsValid()) return;

	ToCompute->add({ CS, false, false, "" });
}

void CMskPlugin::OnFlightPlanControllerAssignedDataUpdate(CFlightPlan FlightPlan, int DataType)
{
	// THREAD CONTEXT: MAIN
	if (!FlightPlan.IsValid())	return;
	auto rt = FlightPlan.GetCorrelatedRadarTarget();
	if (!rt.IsValid())	return;

	ToCompute->add({ check_str_create(rt.GetCallsign()), false, false,
		DataType == CTR_DATA_TYPE_DIRECT_TO ? FlightPlan.GetExtractedRoute().GetPointName(FlightPlan.GetExtractedRoute().GetPointsAssignedIndex()) : "" });

}

void CMskPlugin::OnControllerPositionUpdate(CController Controller)
{
	// THREAD CONTEXT: MAIN
	try {
		if (Controller.IsValid() && ControllerMyself().GetPositionId() == Controller.GetPositionId())
		{
			GetMyPort();
		}
		if (Controller.GetPositionIdentified() && ControllerMyself().IsValid() && ControllerMyself().GetPositionId() == Controller.GetPositionId())
		{
			GetMyPort();
			Plugin->Ccdlc->AddCCDLCSystemMessageToQueue("HELLO:" + MyId + ":" + MyCS + ":" + to_string(MyFacility) + ":" + (MyName == "" ? MyCS : ""));
		}
		Controllers->NewController(Controller);
	}
	catch (...)
	{
		DisplayUrgent("Exception raised in OnControllerPositionUpdate");
	}
}

void CMskPlugin::OnControllerDisconnect(CController Controller)
{
	//THREAD CONTEXT:	MAIN
	try {
		if (ControllerMyself().IsValid() && ControllerMyself().GetPositionId() == Controller.GetPositionId())
		{
			// im disconnected
			Plugin->Ccdlc->AddCCDLCSystemMessageToQueue("DSCN:" + MyId + ":" + MyCS);
		}
		Controllers->ControllerGone(Controller);
	}
	catch (...)
	{
		DisplayUrgent("Exception raised in OnControllerDisconnect");
	}
}

CMskPlugin* gpMyPlugin = NULL;

void    __declspec (dllexport)    EuroScopePlugInInit(EuroScopePlugIn::CPlugIn** ppPlugInInstance)
{
	// AFX_MANAGE_STATE(AfxGetStaticModuleState())

		// create the instance
	*ppPlugInInstance = gpMyPlugin = new CMskPlugin();
}


//---EuroScopePlugInExit-----------------------------------------------

void    __declspec (dllexport)    EuroScopePlugInExit(void)
{
	// AFX_MANAGE_STATE(AfxGetStaticModuleState())

		// delete the instance
	delete gpMyPlugin;
}