#include "pch.h"
#include "CMskPlugin.h"
#include "CMskRadarScreen.h"
#include "math.h"
#include "Common.h"
#include "Global.h"
#include "CCDLC.h"
#include "QR.h"
#include <mutex>
#include "AircraftExtendedData.h"


using namespace qrcodegen;
using namespace EuroScopePlugIn;
using namespace std;

CMskRadarScreen::CMskRadarScreen(CMskPlugin* plugin)
{
	this->mskPlugin = plugin;
	// Create all persistent objects

	auto area = this->GetRadarArea();
	auto center = ConvertCoordFromPixelToPosition({ (int)((area.right - area.left) / 2), (int)((area.bottom - area.top) / 2) });

	CreatePermanentScreenObject("ROUTECURSOR", center, true, "", "", &CMskRadarScreen::OnCursorEvent);
	CreatePermanentScreenObject("GEO", center, true, "", "", &CMskRadarScreen::OnGeoEvent);

	mskPlugin->SelectActiveSectorfile();
	for (auto p = mskPlugin->SectorFileElementSelectFirst(SECTOR_ELEMENT_FIX); p.IsValid(); p = mskPlugin->SectorFileElementSelectNext(p, SECTOR_ELEMENT_FIX))
	{

		CPosition pp;
		p.GetPosition(&pp, 0);

		CreatePermanentScreenObject("FIX_" + string(p.GetName()), pp, false, p.GetName(), "FIX", &CMskRadarScreen::OnMapWaypointEvent);
		CreatePermanentScreenObject("DCT_" + string(p.GetName()), pp, false, p.GetName(), "DCT", &CMskRadarScreen::OnRoutePointEvent);
	}
	for (auto p = mskPlugin->SectorFileElementSelectFirst(SECTOR_ELEMENT_VOR); p.IsValid(); p = mskPlugin->SectorFileElementSelectNext(p, SECTOR_ELEMENT_VOR))
	{

		CPosition pp;
		p.GetPosition(&pp, 0);

		CreatePermanentScreenObject("FIX_" + string(p.GetName()), pp, false, p.GetName(), "FIX", &CMskRadarScreen::OnMapWaypointEvent);
		CreatePermanentScreenObject("DCT_" + string(p.GetName()), pp, false, p.GetName(), "DCT", &CMskRadarScreen::OnRoutePointEvent);
	}
	for (auto p = mskPlugin->SectorFileElementSelectFirst(SECTOR_ELEMENT_NDB); p.IsValid(); p = mskPlugin->SectorFileElementSelectNext(p, SECTOR_ELEMENT_NDB))
	{

		CPosition pp;
		p.GetPosition(&pp, 0);

		CreatePermanentScreenObject("FIX_" + string(p.GetName()), pp, false, p.GetName(), "FIX", &CMskRadarScreen::OnMapWaypointEvent);
		CreatePermanentScreenObject("DCT_" + string(p.GetName()), pp, false, p.GetName(), "DCT", &CMskRadarScreen::OnRoutePointEvent);
	}
	for (auto p = mskPlugin->SectorFileElementSelectFirst(SECTOR_ELEMENT_AIRPORT); p.IsValid(); p = mskPlugin->SectorFileElementSelectNext(p, SECTOR_ELEMENT_AIRPORT))
	{

		CPosition pp;
		p.GetPosition(&pp, 0);

		CreatePermanentScreenObject("FIX_" + string(p.GetName()), pp, false, p.GetName(), "FIX", &CMskRadarScreen::OnMapWaypointEvent);
		CreatePermanentScreenObject("DCT_" + string(p.GetName()), pp, false, p.GetName(), "DCT", &CMskRadarScreen::OnRoutePointEvent);
	}
}

CMskRadarScreen::~CMskRadarScreen()
{
	mskPlugin->CurrentScreen = NULL;
	mskPlugin->ScreenPool.erase(remove(mskPlugin->ScreenPool.begin(), mskPlugin->ScreenPool.end(), this), mskPlugin->ScreenPool.end());
}

inline void CMskRadarScreen::DrawGeo(HDC hDc) {

	if (mskPlugin->GeoPoints.size() > 0) {

		auto pen = CreatePen(PS_SOLID, 1, RGB(255, 255, 0));
		SelectObject(hDc, pen);
		MoveToEx(hDc, mskPlugin->GeoPoints.at(0).x, mskPlugin->GeoPoints.at(0).y, NULL);


		for (auto& P : mskPlugin->GeoPoints) {
			LineTo(hDc, P.x, P.y);
		}
		DeleteObject(pen);
	}

	if (mskPlugin->__hightlight != "") {

		for (auto& P : SectorAreas)
		{
			if (mskPlugin->__hightlight == P.Name || mskPlugin->__hightlight == "ALL")
			{
				auto pen = CreatePen(PS_SOLID, 1, RGB(255, 255, 0));
				SelectObject(hDc, pen);

				auto init = GeoPointToPixel(P.Coords.at(0));

				MoveToEx(hDc, init.x, init.y, NULL);

				for (size_t i = 1; i < P.Coords.size(); i++)
				{
					auto X = GeoPointToPixel(P.Coords.at(i));
					LineTo(hDc, X.x, X.y);
				}
				DeleteObject(pen);
			}
		}
	}

}

inline void CMskRadarScreen::DrawTransmitCircle(HDC dc, CPosition ps)
{
	HPEN mainPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
	HGDIOBJ oldBrush = SelectObject(dc, GetStockObject(HOLLOW_BRUSH));
	HGDIOBJ oldPen = SelectObject(dc, mainPen);

	SelectObject(dc, mainPen);
	POINT p_origin = ConvertCoordFromPositionToPixel(ps);

	Ellipse(dc, p_origin.x - 35, p_origin.y - 35, p_origin.x + 35, p_origin.y + 35);	// here to check circles (TODO)

	POINT oldPoint;
	MoveToEx(dc, (GetRadarArea().right - GetRadarArea().left) / 2, (GetRadarArea().bottom - GetRadarArea().top) / 2, &oldPoint);
	LineTo(dc, p_origin.x, p_origin.y);
	MoveToEx(dc, oldPoint.x, oldPoint.y, NULL);

	SelectObject(dc, oldBrush);
	SelectObject(dc, oldPen);

	DeleteObject(mainPen);

}

inline void CMskRadarScreen::DrawPointer(HDC dc, POINT center, COLORREF reqColor)
{
	auto pen = GetStockObject(DC_PEN);

	SetDCPenColor(dc, reqColor);

	auto oldPen = SelectObject(dc, pen);

	//4
	MoveToEx(dc, center.x - 4, center.y, NULL);
	LineTo(dc, center.x, center.y - 4);
	LineTo(dc, center.x + 4, center.y);
	LineTo(dc, center.x, center.y + 4);
	LineTo(dc, center.x - 4, center.y);
	//3
	MoveToEx(dc, center.x - 3, center.y, NULL);
	LineTo(dc, center.x, center.y - 3);
	LineTo(dc, center.x + 3, center.y);
	LineTo(dc, center.x, center.y + 3);
	LineTo(dc, center.x - 3, center.y);

	SelectObject(dc, oldPen);
}

inline void CMskRadarScreen::DrawTocTod(HDC dc, AircraftExtendedData* data)
{
	if (!data->DisplayOnRadar.DisplayRoute) return;

	CPosition usePoint;

	if (data->Route.TopOfClimbDist > 0)
	{
		if (data->ActualFormularData.AssignedHeading != 0)
			usePoint = data->Route.TopOfClimbDirectPoint;
		else usePoint = data->Route.TopOfClimbPoint;
	}
	else if (data->Route.TopOfDescendDist > 0)
	{
		if (data->ActualFormularData.AssignedHeading != 0)
			usePoint = data->Route.TopOfDescendDirectPoint;
		else usePoint = data->Route.TopOfDescendPoint;
	}
	else return;

	HPEN pen = CreatePen(PS_SOLID, 2, RGB(0, 102, 255));
	auto oldPen = SelectObject(dc, pen);

	auto point = ConvertCoordFromPositionToPixel(usePoint);

	if (data->Route.TopOfDescendDist > 0)
	{
		MoveToEx(dc, point.x, point.y, NULL);

		LineTo(dc, point.x - 10, point.y);
		LineTo(dc, point.x - 20, point.y - 10);

		MoveToEx(dc, point.x, point.y, NULL);
		LineTo(dc, point.x - 5, point.y - 5);
		MoveToEx(dc, point.x, point.y, NULL);
		LineTo(dc, point.x - 5, point.y + 5);
	}
	else if (data->Route.TopOfClimbDist > 0) {
		MoveToEx(dc, point.x, point.y, NULL);

		LineTo(dc, point.x - 10, point.y);
		LineTo(dc, point.x - 20, point.y + 10);

		MoveToEx(dc, point.x, point.y, NULL);
		LineTo(dc, point.x - 5, point.y - 5);
		MoveToEx(dc, point.x, point.y, NULL);
		LineTo(dc, point.x - 5, point.y + 5);
	}

	SelectObject(dc, oldPen);
	DeleteObject(pen);
}

inline double CMskRadarScreen::GetPixelLen()
{
	auto ret = ConvertCoordFromPixelToPosition({ 10,10 }).DistanceTo(ConvertCoordFromPixelToPosition({ 10,11 }));
	return ret;
}

inline void CMskRadarScreen::DrawCircleIfHeavy(HDC dc, AircraftExtendedData* data, POINT center)
{
	if (!data->ActualFormularData.IsHeavy)	return;

	if (data->ActualFlightData.GroundSpeed < 100)	return;

	auto pen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
	auto oldPen = SelectObject(dc, pen);


	SelectObject(dc, pen);

	auto d = PointTo(center, 10, normAngle(data->ActualFlightData.Track - 180 - 135));
	MoveToEx(dc, d.x, d.y, NULL);
	d = PointTo(center, 10, normAngle(data->ActualFlightData.Track - 180 - 135 + 18));
	LineTo(dc, d.x, d.y);
	d = PointTo(center, 10, normAngle(data->ActualFlightData.Track - 180 - 135 + 18 * 2));
	LineTo(dc, d.x, d.y);
	d = PointTo(center, 10, normAngle(data->ActualFlightData.Track - 180 - 135 + 18 * 3));
	LineTo(dc, d.x, d.y);
	d = PointTo(center, 10, normAngle(data->ActualFlightData.Track - 180 - 135 + 18 * 4));
	LineTo(dc, d.x, d.y);
	d = PointTo(center, 10, normAngle(data->ActualFlightData.Track - 180 - 135 + 18 * 5));
	LineTo(dc, d.x, d.y);

	SelectObject(dc, oldPen);
	DeleteObject(pen);

}

inline void CMskRadarScreen::CenterDisplayIfRequired()
{
	if (mskPlugin->CenterRT.m_Latitude != 0)
	{
		CPosition l, r;
		GetDisplayArea(&l, &r);

		double half_width = abs(r.m_Longitude - l.m_Longitude) / 2;
		double half_height = abs(r.m_Latitude - l.m_Latitude) / 2;

		CPosition d_l, d_r;
		d_l.m_Latitude = mskPlugin->CenterRT.m_Latitude - half_height;
		d_l.m_Longitude = mskPlugin->CenterRT.m_Longitude - half_width;

		d_r.m_Latitude = mskPlugin->CenterRT.m_Latitude + half_height;
		d_r.m_Longitude = mskPlugin->CenterRT.m_Longitude + half_width;

		SetDisplayArea(d_l, d_r);

		mskPlugin->CenterRT.m_Latitude = 0;
		mskPlugin->CenterRT.m_Longitude = 0;
	}
}

void CMskRadarScreen::OnRefresh(HDC hDC, int Phase) {

	if (Phase != REFRESH_PHASE_AFTER_TAGS) return;

	mskPlugin->CurrentScreen = this;

	if (mskPlugin->ShowWarn)
		PaintMessage(hDC);
	else if (mskPlugin->QrShow)
		PaintQRCode(hDC, mskPlugin->QRAddr);

	if (mskPlugin->Afv == NULL) return;

	CenterDisplayIfRequired();

	DrawGeo(hDC);

	CMskPlugin* me = this->mskPlugin;

	for (auto rt = GetPlugIn()->RadarTargetSelectFirst(); rt.IsValid(); rt = GetPlugIn()->RadarTargetSelectNext(rt)) {

		string CS = check_str_create(rt.GetCallsign());

		if (CS == "") continue;

		auto radarTargetPosition = rt.GetPosition().GetPosition();

		// Draw circles for transmitting
		if (mskPlugin->Afv->IsActiveTransmit(CS)) {
			DrawTransmitCircle(hDC, radarTargetPosition);

		}

		ExtendedDataRequest req;

		auto AcftInfo = req.request(CS);

		if (AcftInfo->ActualFormularData.Callsign == "") continue;

		auto pos = AcftInfo->Route.PresentPosition;
		auto screen_pos = ConvertCoordFromPositionToPixel(pos);

		if (AcftInfo->ActualFormularData.IsCharlie)
			DrawPointer(hDC, screen_pos, mskPlugin->Tags->PortToColorForPointer(AcftInfo->Route.DestinationICAO));

		DrawCircleIfHeavy(hDC, AcftInfo, screen_pos);

		if (AcftInfo->DisplayOnRadar.DisplayRoute) {

			HPEN mainPen = CreatePen(PS_SOLID, 2, RGB(102, 255, 51));
			SelectObject(hDC, mainPen);
			MoveToEx(hDC, screen_pos.x, screen_pos.y, NULL);

			AcftInfo->Route.EnumPoints([&](PointDetails* Point) {

				POINT pt = ConvertCoordFromPositionToPixel(Point->Position);

				RECT rc = RECT();
				rc.left = pt.x + 5;
				rc.top = pt.y + 5;
				rc.bottom = rc.top + 50;
				rc.right = rc.left + 70;

				if (mskPlugin->ShowRouteCursor && mskPlugin->RouteCursorBeforePoint != "" && mskPlugin->RouteCursorBeforePoint == Point->Name)
				{
					LineTo(hDC, mskPlugin->RouteCursorPos.x, mskPlugin->RouteCursorPos.y);
				}

				LineTo(hDC, pt.x, pt.y);

				if (mskPlugin->ShowRouteCursor && mskPlugin->RouteCursorAfterPoint != "" && mskPlugin->RouteCursorAfterPoint == Point->Name)
				{
					LineTo(hDC, mskPlugin->RouteCursorPos.x, mskPlugin->RouteCursorPos.y);
				}

				// If CCDLC DCT in progress
				if (AcftInfo->CCDLCRequest.IsValid() && safe_substr(AcftInfo->CCDLCRequest.Request,0,3) == "DCT")
				{
					string dctName = safe_substr(AcftInfo->CCDLCRequest.Request, 4, 100);
					if (dctName == Point->Name)
					{
						POINT prev_pos;
						MoveToEx(hDC, screen_pos.x, screen_pos.y, &prev_pos);
						HPEN bluePen = CreatePen(PS_DASH, 1, RGB(0, 0, 255));
						auto oldPen = SelectObject(hDC, bluePen);
						LineTo(hDC, pt.x, pt.y);
						MoveToEx(hDC, prev_pos.x, prev_pos.y, NULL);
						SelectObject(hDC, oldPen);
						DeleteObject(bluePen);
					}
				}

				string caption = string(Point->Name + "\n");		//	PANZO
				caption += formatTime(Point->ExpectedPassTime) + "\n";	//	16:30

				auto obj = &ScreenObjects["DCT_" + Point->Name];

				if (obj->Name == "")
				{
					CreatePermanentScreenObject("FIX_" + Point->Name, Point->Position, false, Point->Name, "FIX", &CMskRadarScreen::OnMapWaypointEvent);
					obj = CreatePermanentScreenObject("DCT_" + Point->Name, Point->Position, false, Point->Name, "DCT", &CMskRadarScreen::OnRoutePointEvent);
				}

				if (obj->Over) {
					caption += ">" + formatTime(Point->ExpectedDirectPassTime) + "\n";
				}
				if (Point->StarConstraint != "")
				{
					try {
						caption += Point->StarConstraint + "\n";				// FL150+
					}
					catch (...)
					{
						int zzz = 0;
					}
				}
				else if (Point->SidConstraint != "")
				{
					caption += Point->SidConstraint + "\n";
				}

				HFONT fnt = CreateFont(12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, "EuroScope");
				auto oldFont = SelectObject(hDC, fnt);

				SetTextColor(hDC, RGB(255, 255, 255));

				if (obj->Over)
				{
					HBRUSH br = CreateSolidBrush(RGB(102, 255, 51));
					auto oldBrush = SelectObject(hDC, br);
					FrameRect(hDC, &rc, br);
					SelectObject(hDC, oldBrush);
					DeleteObject(br);
				}
				if (obj->Click && obj->Button == BUTTON_LEFT)
				{
					auto P = CreatePen(PS_DASH, 1, RGB(102, 255, 51));
					auto old = SelectObject(hDC, P);
					MoveToEx(hDC, screen_pos.x, screen_pos.y, NULL);
					LineTo(hDC, pt.x, pt.y);
					SelectObject(hDC, old);
					DeleteObject(P);
				}
				if (Plugin->MarkedPoint == obj->Param1)
				{
					HBRUSH br = CreateSolidBrush(RGB(0, 191, 255));
					auto oldBrush = SelectObject(hDC, br);
					FrameRect(hDC, &rc, br);
					SelectObject(hDC, oldBrush);
					DeleteObject(br);
				}
				DrawText(hDC, caption.c_str(), -1, &rc, DT_LEFT);
				ActivateScreenObject("", rc, obj);

				SelectObject(hDC, oldFont);
				DeleteObject(fnt);
				});
			// Track miles
			RECT tmiles = RECT();
			tmiles.left = screen_pos.x - 10;
			tmiles.top = screen_pos.y + 5;
			tmiles.right = screen_pos.x + 30;
			tmiles.bottom = screen_pos.y + 30;
			DrawText(hDC, to_string((int)AcftInfo->Route.RouteDistanceToDest).c_str(), -1, &tmiles, DT_LEFT);

			DeleteObject(mainPen);
			DrawTocTod(hDC, AcftInfo);
		}
		if (AcftInfo->DisplayOnRadar.Display10km || AcftInfo->DisplayOnRadar.Display5km) {

			double Range = AcftInfo->DisplayOnRadar.Display10km ? !AcftInfo->Route.CurrentAreasIsRadar ? 10 : 5 : !AcftInfo->Route.CurrentAreasIsRadar ? 5 : 2.5;
			COLORREF RangeColor = AcftInfo->DisplayOnRadar.Display10km ? RGB(255, 255, 0) : RGB(255, 0, 0);

			POINT p = ConvertCoordFromPositionToPixel(pos);

			CPosition rng_pos;
			rng_pos.m_Latitude = pos.m_Latitude + (Range / 1.852 / 60.0);
			rng_pos.m_Longitude = pos.m_Longitude;

			POINT rng_pt = ConvertCoordFromPositionToPixel(rng_pos);

			double len = sqrt(pow(rng_pt.x - p.x, 2) + pow(rng_pt.y - p.y, 2));

			SelectObject(hDC, GetStockObject(HOLLOW_BRUSH));
			HPEN pen = CreatePen(PS_SOLID, 1, RangeColor);
			SelectObject(hDC, pen);

			Ellipse(hDC, p.x - (int)len, p.y - (int)len, p.x + (int)len, p.y + (int)len);
			DeleteObject(pen);
		}
	}

	// If in Geo mode 
	if (mskPlugin->GeoMode) {
		ActivateScreenObject("GEO", GetRadarArea(), NULL);
	}

	// FInally, display active transmit pilots
	string Transmitting = mskPlugin->Afv->GetPilotsForDraw();

	if (Transmitting != "") {

		Transmitting = "Last CS: " + Transmitting;
		RECT display = GetRadarArea();
		RECT txt = RECT();
		txt.top = display.top + 50;
		txt.right = display.right - 20;
		txt.left = txt.right - 200;
		txt.bottom = txt.top + 20;

		SetTextColor(hDC, RGB(255, 255, 255));
		DrawText(hDC, Transmitting.c_str(), -1, &txt, DT_RIGHT);
	}
	if (mskPlugin->ShowRouteCursor)
	{
		// Draw cursor
		auto pen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
		auto oldPen = SelectObject(hDC, pen);
		MoveToEx(hDC, mskPlugin->RouteCursorPos.x - 20, mskPlugin->RouteCursorPos.y, NULL);
		LineTo(hDC, mskPlugin->RouteCursorPos.x + 20, mskPlugin->RouteCursorPos.y);
		MoveToEx(hDC, mskPlugin->RouteCursorPos.x, mskPlugin->RouteCursorPos.y - 20, NULL);
		LineTo(hDC, mskPlugin->RouteCursorPos.x, mskPlugin->RouteCursorPos.y + 20);
		SelectObject(hDC, oldPen);
		DeleteObject(oldPen);
		ActivateScreenObject("ROUTECURSOR", mskPlugin->RouteCursorArea, NULL);
	}
	if (mskPlugin->DisplayRadarWaypoint)
	{
		CPosition p1, p2;
		GetDisplayArea(&p1, &p2);

		HFONT fnt = CreateFont(12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, "EuroScope");
		auto oldFont = SelectObject(hDC, fnt);

		for (auto& P : ScreenObjects)
		{
			auto pt = P.second;
			if (pt.Param2 != "FIX" || pt.Pos.m_Latitude < p1.m_Latitude || pt.Pos.m_Latitude > p2.m_Latitude || pt.Pos.m_Longitude < p1.m_Longitude || pt.Pos.m_Longitude > p2.m_Longitude) continue;

			RECT pr = RECT();
			POINT ppp = ConvertCoordFromPositionToPixel(pt.Pos);
			pr.left = ppp.x;
			pr.top = ppp.y;
			pr.right = pr.left + 50;
			pr.bottom = pr.top + 30;
			SetTextColor(hDC, RGB(255, 255, 255));
			DrawText(hDC, pt.Param1.c_str(), -1, &pr, DT_LEFT);

			ActivateScreenObject("", pr, &P.second);
		}
		SelectObject(hDC, oldFont);
		DeleteObject(fnt);
	}
}

void CMskRadarScreen::OnCursorEvent(ScreenObjectInfo obj, string Event)
{
	if (Event == "CLICK" && obj.Button == BUTTON_RIGHT)
	{
		mskPlugin->OpenPopupList(obj.Area, "Cursor actions", 1);
		mskPlugin->AddPopupListElement("SET POINT", "", FN_MSK_WAYPOINT_ROUTE_ACTION);
		mskPlugin->AddPopupListElement("CANCEL", "", FN_MSK_WAYPOINT_ROUTE_ACTION);
	}
	else if (Event == "DBLCLICK")
	{
		CFlightPlan fp;
		CRadarTarget rt;
		int fid = FN_MSK_WAYPOINT_ROUTE_ACTION;
		mskPlugin->Tags->ProcessFN_RoutePointAction(fid, string(""), fp, rt, string("SET POINT"), obj.Point, obj.Area);
	}
}

void CMskRadarScreen::OnAsrContentToBeClosed(void) {

	delete this;
}

void CMskRadarScreen::OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area) {

	try {
		auto radarTarget = this->mskPlugin->RadarTargetSelectASEL();

		if (!radarTarget.IsValid()) return;

		string CS = string(radarTarget.GetCallsign());

	}
	catch (const std::exception& exc) {
		this->mskPlugin->DisplayUrgent("OnFuctionCall: General exception: " + string(exc.what()));
	}
	catch (const std::string& ex) {
		this->mskPlugin->DisplayUrgent("OnFuctionCall: General exception: " + ex);
	}
	catch (...) {
		this->mskPlugin->DisplayUrgent("OnFuctionCall: General exception");
	}
}

void CMskRadarScreen::OnMoveScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, bool Released)
{
	string Obj = string(sObjectId);
	if (Obj == "ROUTECURSOR")
	{
		mskPlugin->RouteCursorArea = Area;
		mskPlugin->RouteCursorPos = Pt;
	}
}

void CMskRadarScreen::OnClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{
	string Obj = string(sObjectId);

	auto P = &ScreenObjects.at(Obj);

	if (LastScreenObject.count("CLICK") > 0)
		LastScreenObject.at("CLICK")->Click = false;


	P->Click = true;
	P->Button = Button;
	if (P->OnEvent != NULL) (this->*P->OnEvent)(*P, "CLICK");

	LastScreenObject["CLICK"] = P;
	RequestRefresh();
}

void CMskRadarScreen::OnDoubleClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{
	string Obj = string(sObjectId);

	auto P = &ScreenObjects.at(Obj);

	if (LastScreenObject.count("CLICK") > 0)
		LastScreenObject.at("CLICK")->Click = false;


	P->Click = true;
	P->Button = Button;
	P->Point = Pt;
	if (P->OnEvent != NULL) (this->*P->OnEvent)(*P, "DBLCLICK");

	LastScreenObject["CLICK"] = P;
	RequestRefresh();
}

void CMskRadarScreen::OnOverScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area)
{
	string Obj = string(sObjectId);

	auto P = &ScreenObjects[Obj];
	if (P->Name == "")
	{
		// dynamically created screen object -> ROUTE WAYPOINT
		P->Area = Area;
		P->Movable = false;
	}

	if (LastScreenObject.count("OVER") > 0)
		LastScreenObject.at("OVER")->Over = false;

	P->Over = true;

	if (P->OnEvent != NULL) (this->*P->OnEvent)(*P, "OVER");

	LastScreenObject["OVER"] = P;
	RequestRefresh();
}

void CMskRadarScreen::OnRoutePointEvent(ScreenObjectInfo obj, string Event)
{
	auto rt = mskPlugin->RadarTargetSelectASEL();
	if (!rt.IsValid())	return;

	if (Event == "CLICK" && obj.Button == BUTTON_RIGHT)
	{
		ExtendedDataRequest req;
		auto plane = req.request(rt.GetCallsign());

		auto mskPlugin = Plugin;

		mskPlugin->OpenPopupList(obj.Area, "Route waypoint options", 1);

		if (mskPlugin->MarkedPoint == obj.Param1)
			mskPlugin->AddPopupListElement("UNMARK POINT", "", FN_MSK_WAYPOINT_ROUTE_ACTION);
		if (mskPlugin->MarkedPoint != obj.Param1)
		{
			if (mskPlugin->MarkedPoint != "")
				mskPlugin->AddPopupListElement("MARKED PT THEN THIS", "", FN_MSK_WAYPOINT_ROUTE_ACTION);
			else mskPlugin->AddPopupListElement("MARK POINT", "", FN_MSK_WAYPOINT_ROUTE_ACTION);
		}
		mskPlugin->AddPopupListElement("INSERT EXISTING BEFORE", "", FN_MSK_WAYPOINT_ROUTE_ACTION);
		mskPlugin->AddPopupListElement("INSERT EXISTING AFTER", "", FN_MSK_WAYPOINT_ROUTE_ACTION);
		mskPlugin->AddPopupListElement("INSERT FREE BEFORE", "", FN_MSK_WAYPOINT_ROUTE_ACTION);
		mskPlugin->AddPopupListElement("INSERT FREE AFTER", "", FN_MSK_WAYPOINT_ROUTE_ACTION);
		mskPlugin->AddPopupListElement("REMOVE WAYPOINT", "", FN_MSK_WAYPOINT_ROUTE_ACTION);
		mskPlugin->AddPopupListElement("CLEAR DIRECT", "", FN_MSK_WAYPOINT_ROUTE_ACTION);
		mskPlugin->AddPopupListElement("-----", "", FN_MSK_WAYPOINT_ROUTE_ACTION);
		mskPlugin->AddPopupListElement("RESET ROUTE", "", FN_MSK_WAYPOINT_ROUTE_ACTION);

	}
	else if (Event == "DBLCLICK" && obj.Button == BUTTON_LEFT)
	{
		auto mskPlugin = Plugin;
		string PointName = obj.Param1;
		auto FP = rt.GetCorrelatedFlightPlan();
		if (!FP.IsValid()) return;

		{
			ExtendedDataRequest req;
			auto p = req.request(rt.GetCallsign());
			auto man_pt = p->Route.FindPoint(PointName);

			if (!man_pt.IsValid() || man_pt.Free)
			{
				return;
			}
			p->DisplayOnRadar.DisplayRoute = false;
		}
		FP.GetControllerAssignedData().SetDirectToPointName(PointName.c_str());
		FP.GetControllerAssignedData().SetScratchPadString(PointName.c_str());

		Plugin->Ccdlc->AddCCDLCMessageToQueue(rt.GetCallsign(), "SDT", PointName);
		Plugin->ToCompute->add({ rt.GetCallsign(), false });
	}
}

void CMskRadarScreen::OnGeoEvent(ScreenObjectInfo obj, string Event)
{
	if (mskPlugin->GeoMode && Event == "DBLCLICK") {

		mskPlugin->GeoPoints.push_back(obj.Point);
		CPosition c = ConvertCoordFromPixelToPosition(obj.Point);
		mskPlugin->GeoPointsCoords.push_back({ c.m_Latitude, c.m_Longitude });

		int st = 1;
		for (auto& x : SectorAreas) {
			bool ins = isInside(&x.Coords[0], x.Coords.size(), { c.m_Latitude,c.m_Longitude });
			if (ins) {
				mskPlugin->DisplaySilent("INSIDE " + x.Name);
			}

		}
		mskPlugin->DisplaySilent("END OF ZONES");
		st = st + 1;
	}
}

void CMskRadarScreen::OnMapWaypointEvent(ScreenObjectInfo obj, string Event)
{
	if (Event == "CLICK" && obj.Button == BUTTON_LEFT)
	{
		mskPlugin->OpenPopupList(obj.Area, (mskPlugin->RouteCursorAfterPoint != "" ? (mskPlugin->RouteCursorAfterPoint + " THEN " + obj.Param1) : (obj.Param1 + " THEN " + mskPlugin->RouteCursorBeforePoint)).c_str(), 1);
		mskPlugin->AddPopupListElement("ADD TO ROUTE", "", FN_MSK_WAYPOINT_ROUTE_ACTION);
		mskPlugin->AddPopupListElement("CANCEL", "", FN_MSK_WAYPOINT_ROUTE_ACTION);
	}
}

ScreenObjectInfo* CMskRadarScreen::CreatePermanentScreenObject(string Name, CPosition Pos, bool Movable, string Param1 = "", string Param2 = "", ScreenObjectEvent OnEvent = NULL)
{
	ScreenObjectInfo ret;

	bool exists = false;

	lock_guard<mutex> lock(__lock);

	if (ScreenObjects.count(Name) > 0)
	{
		(&ScreenObjects[Name])->Name = Name;
		(&ScreenObjects[Name])->Pos = Pos;
		(&ScreenObjects[Name])->Movable = Movable;
		(&ScreenObjects[Name])->Param1 = Param1;
		(&ScreenObjects[Name])->Param2 = Param2;
		(&ScreenObjects[Name])->OnEvent = OnEvent;
		return &ScreenObjects[Name];
	}

	ret = ScreenObjectInfo{ Name, false, false, Movable, Pos, {0,0}, {0,0,0,0}, 0, Param1, Param2, OnEvent, true, false };
	ScreenObjects.insert({ Name, ret });

	return &ScreenObjects[Name];
}

inline void CMskRadarScreen::ActivateScreenObject(string Name, RECT area, ScreenObjectInfo* direct = NULL)
{
	ScreenObjectInfo* obj;
	if (direct == NULL)
		obj = &ScreenObjects.at(Name);
	else obj = direct;
	obj->Area = area;
	AddScreenObject(1, obj->Name.c_str(), area, obj->Movable, NULL);

}

inline POINT CMskRadarScreen::GeoPointToPixel(GeoPoint gp)
{
	CPosition p = CPosition();
	p.m_Latitude = gp.x;
	p.m_Longitude = gp.y;
	return ConvertCoordFromPositionToPixel(p);
}

inline void CMskRadarScreen::PaintQRCode(HDC dc, string data)
{
	std::vector<QrSegment> segs =
		QrSegment::makeSegments(data.c_str());

	int seg_size = 8;

	auto rv = GetRadarArea();

	QrCode qr1 = QrCode::encodeSegments(segs, QrCode::Ecc::HIGH, 1, 10, -1, false);


	int pos_left = (int)((rv.right - rv.left) / 2) - (int)(qr1.getSize() * seg_size / 2);
	int pos_top = (int)((rv.bottom - rv.top) / 2) - (int)(qr1.getSize() * seg_size / 2);

	for (int y = 0; y < qr1.getSize(); y++) {
		for (int x = 0; x < qr1.getSize(); x++) {

			auto a = qr1.getModule(x, y);

			RECT r;
			r.left = pos_left + x * seg_size;
			r.top = pos_top + y * seg_size;
			r.bottom = r.top + seg_size;
			r.right = r.left + seg_size;

			auto br = GetStockObject(a ? BLACK_BRUSH : WHITE_BRUSH);

			FillRect(dc, &r, (HBRUSH)br);
		}
	}
}

void CMskRadarScreen::ClearScreenObjects()
{
	lock_guard<mutex> lock(__lock);
	auto it = ScreenObjects.begin();
	while (it != ScreenObjects.end())
	{
		if (!it->second.Permanent)
		{
			it = ScreenObjects.erase(it);
		}
		else ++it;
	}
}

inline void CMskRadarScreen::PaintMessage(HDC dc)
{
	auto brush = CreateSolidBrush(RGB(255, 0, 0));
	auto oldBrush = SelectObject(dc, brush);

	auto rv = GetRadarArea();
	int center_x = (int)((rv.right - rv.left) / 2);
	int center_y = (int)((rv.bottom - rv.top) / 2);



	int len = mskPlugin->WarnMsg.length();
	int msg_size = len * 10;

	RECT r;
	r.left = center_x - (int)(msg_size / 2);
	r.top = center_y - 20;
	r.right = r.left + 20 + msg_size + 20;
	r.bottom = r.top + 20 + 20;

	RECT t;
	CopyRect(&t, &r);
	r.top -= 20;

	FillRect(dc, &r, brush);

	SetTextColor(dc, RGB(0, 0, 0));
	DrawText(dc, mskPlugin->WarnMsg.c_str(), -1, &t, DT_CENTER);

	SelectObject(dc, oldBrush);
	DeleteObject(brush);

}