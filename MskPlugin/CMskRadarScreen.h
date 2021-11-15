#pragma once
#include "EuroScopePlugIn.h"
#include "CMskPlugin.h"

using namespace EuroScopePlugIn;

class CMskRadarScreen :
	public CRadarScreen
{
private:
	CMskPlugin* mskPlugin;
	mutex __lock;

	
public:
	
	map<string, ScreenObjectInfo> ScreenObjects;
	map<string,ScreenObjectInfo*> LastScreenObject;

	CMskRadarScreen(CMskPlugin* plugin);

	virtual ~CMskRadarScreen();
	
	void OnRefresh(HDC hDC, int Phase);
	void OnAsrContentToBeClosed(void);
	void OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area);
	void OnDoubleClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button);
	void OnClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button);
	void OnOverScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area);
	void OnMoveScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, bool Released);

	void ClearScreenObjects();
	ScreenObjectInfo * CreatePermanentScreenObject(string Name, CPosition Pos, bool Movable, string Param1, string Param2, ScreenObjectEvent OnEvent);
	inline void ActivateScreenObject(string Name, RECT area, ScreenObjectInfo * direct);

	inline void DrawGeo(HDC hDc);

	inline POINT GeoPointToPixel(GeoPoint gp);

	inline void DrawTransmitCircle(HDC dc, CPosition ps);
	inline void DrawTocTod(HDC dc, AircraftExtendedData *data);
	inline void DrawPointer(HDC dc, POINT center, COLORREF reqColor);
	inline void DrawCircleIfHeavy(HDC dc, AircraftExtendedData *data, POINT center);
	inline double GetPixelLen();

	inline void PaintQRCode(HDC dc, string data);
	inline void PaintMessage(HDC dc);
	inline void CenterDisplayIfRequired();

	void OnRoutePointEvent(ScreenObjectInfo obj, string Event);
	void OnMapWaypointEvent(ScreenObjectInfo obj, string Event);
	void OnGeoEvent(ScreenObjectInfo obj, string Event);
	void OnCursorEvent(ScreenObjectInfo obj, string Event);

	int tick = 0;
};