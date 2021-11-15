#pragma once
#include <unordered_map>
#include <map>
#include "Consts.h"
#include <set>

using namespace std;
using namespace EuroScopePlugIn;

class CMskPlugin;

typedef list<PointDetails>::iterator RouteEntry;

typedef vector<ES_Route_Item>::iterator ES_Route_Entry;

class AircraftExtendedData;

class ES_Route {
private:
	vector<ES_Route_Item> __items;
public:
	int ActiveIndex = -1;

	inline bool empty()
	{
		return __items.size() == 0;
	}
	inline bool isbegin(ES_Route_Entry r)
	{
		return r == __items.begin();
	}
	inline ES_Route_Entry end()
	{
		return __items.end();
	}
	inline ES_Route_Entry begin()
	{
		return __items.begin();
	}
	inline bool isend(ES_Route_Entry r)
	{
		return r == __items.end();
	}

	ES_Route(CFlightPlanExtractedRoute * plan = NULL);
	ES_Route_Entry find(string Name);
	ES_Route_Entry find_back(string Name, ES_Route_Entry from);
};

class AircraftRoute
{
	friend class AircraftExtendedData;
private:

	list<PointDetails> __route;
	ES_Route ES_ROUTE;

	inline bool isend(RouteEntry r)
	{
		return r == __route.end();
	}
	inline bool isbegin(RouteEntry r)
	{
		return r == __route.begin();
	}
	inline bool isempty(RouteEntry r)
	{
		return isbegin(r) && isend(r);
	}

	RouteEntry __insert_es_points(ES_Route_Entry first_point, ES_Route_Entry last_point, RouteEntry where);
	RouteEntry __find_name(string Name);
	RouteEntry __find_coord(CPosition* ps);
	RouteEntry __insert_point(string Name, CPosition pos, bool Manual, RouteEntry __where);
	RouteEntry __append_point(string Name, CPosition pos, bool Manual);
	RouteEntry __prepend_point(string Name, CPosition pos, bool Manual);
	RouteEntry __erase_point(RouteEntry what);
	RouteEntry __static_back(RouteEntry from);
	RouteEntry __erase_back_to_end(RouteEntry begin_from);
	void __pt_x_pt(RouteEntry start, RouteEntry end, bool remove_all);
	pair< ES_Route_Entry, RouteEntry> __find_common_back(ES_Route_Entry es, RouteEntry plg);

	bool __coords_same(CPosition p1, CPosition p2);
	void __es_process_route_modifications(CFlightPlanExtractedRoute* fp_route);
	void __check_points_passed(AircraftFlightData* fdata);
	bool __pt_after_pt(string what, string after);

	void __compute(CRadarTarget* rt, FormularData* formular, AircraftFlightData * data, bool RebuildRoute, bool ResetRoute, string DCT = "");

	//void inline __addCrossingAreas(GeoPoint start, GeoPoint end);
	void inline __addCrossingAreas2(GeoPoint start, GeoPoint end);
	void __calcContraints(string sid, string star);
	bool __getSectorArea(CPosition p, int alt, SectorArea& s, bool& IsRadar);
	void __getExitBearing(string sid);
	void __check_depa_land_my_zone();

public:
	string DepartureICAO;
	string DestinationICAO;
	SectorArea CurrentArea;
	bool CurrentAreasIsRadar = false;
	set<string> CrossAreasNames;
	map<string, vector<CPosition>> CrossAreasIntersections;
	CPosition PresentPosition;
	double TopOfDescendDist = 0;
	double TopOfClimbDist = 0;
	CPosition TopOfDescendPoint;
	CPosition TopOfClimbPoint;
	CPosition TopOfClimbDirectPoint;
	CPosition TopOfDescendDirectPoint;
	PointDetails* CurrentDirectPoint = NULL;
	double TrackToDestination = 0.0;
	bool ConstraintsLoaded = false;
	string ActiveStarConstraint = "";
	string ActiveSidContraint = "";
	string ActiveConstraint = "";
	double DirectDistanceFromOrigin = -1;
	double DirectDistanceToDest = -1;
	double RouteDistanceToDest = -1;
	double ExitBearing = 0.0;
	bool IsInbound = false;
	bool DepaInMyZone = false;
	bool LandInMyZone = false;

	void Clear();

	PointDetails AddPointAfterName(string Name, CPosition pos, string AfterName);
	PointDetails AddPointBeforeName(string Name, CPosition pos, string BeforeName);
	PointDetails FindPoint(string Name);
	PointDetails FindCoord(CPosition* ps);
	bool DelPointByName(string Name);
	void SetFirstPoint(string PointName);
	void PointAfterPoint(string Start, string End);
	bool HasZone(string Zone);
	PointDetails* GetLastPoint();

	set<string> GetCrossingAreaNames(CPosition p1, CPosition p2);
	set<string> GetAreasOfPoint(CPosition p1);

	template <typename Pred>
	void EnumPoints(Pred fn)
	{
		for (auto& p : __route)
		{
			fn(&p);
		}
	}
};

