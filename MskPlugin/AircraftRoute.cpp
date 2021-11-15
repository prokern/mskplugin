#include "pch.h"
#include "AircraftRoute.h"
#include "Global.h"
#include "CMskPlugin.h"

using namespace EuroScopePlugIn;

RouteEntry AircraftRoute::__insert_es_points(ES_Route_Entry first_point, ES_Route_Entry last_point, RouteEntry where)
{
	while (distance(first_point, last_point) >= 0)
	{
		PointDetails pd;
		pd.Name = first_point->Name;
		pd.Position = first_point->Pos;

		if (isend(__find_name(pd.Name)))
			where = ++__route.insert(where, pd);
		first_point++;
	}
	return where;
}

RouteEntry AircraftRoute::__find_name(string Name)
{
	return find_if(__route.begin(), __route.end(), [&Name](const PointDetails& pd) {
		return pd.Name == Name;
		});
}

RouteEntry AircraftRoute::__find_coord(CPosition* ps)
{
	return find_if(__route.begin(), __route.end(), [&](const PointDetails& pd) {
		return __coords_same(pd.Position, *ps);
		});
}

RouteEntry AircraftRoute::__insert_point(string Name, CPosition pos, bool Manual, RouteEntry __where)
{
	auto pt = __find_name(Name);

	if (!isend(pt))	return pt;

	PointDetails pd;

	if (Name == "")
	{
		Plugin->ManualPointIndex++;
		pd.Free = true;
	}

	pd.Manual = Manual;
	pd.Name = Name == "" ? ("PT" + to_string(Plugin->ManualPointIndex)) : Name;
	pd.Position = pos;

	return __route.insert(__where, pd);
}

RouteEntry AircraftRoute::__append_point(string Name, CPosition pos, bool Manual)
{
	return __insert_point(Name, pos, Manual, __route.end());
}

RouteEntry AircraftRoute::__prepend_point(string Name, CPosition pos, bool Manual)
{
	return __insert_point(Name, pos, Manual, __route.begin());
}

RouteEntry AircraftRoute::__erase_point(RouteEntry what)
{
	if (isend(what))	return __route.end();
	return __route.erase(what);
}

RouteEntry AircraftRoute::__static_back(RouteEntry from)
{
	while (!isbegin(from))
	{
		--from;
		if (!from->Manual) return from;
	}
	return __route.end();
}

RouteEntry AircraftRoute::__erase_back_to_end(RouteEntry begin_from)
{
	__route.erase(__route.begin(), begin_from);
	return __route.begin();
}

void AircraftRoute::__pt_x_pt(RouteEntry start, RouteEntry end, bool remove_all)
{
	if (isend(start) || isend(end))	return;

	auto it = (++start)--;

	while (it != end && !isend(it))
	{
		if (!it->Manual || remove_all)
		{
			it = __route.erase(it);
		}
		else ++it;
	}
}

pair<ES_Route_Entry, RouteEntry> AircraftRoute::__find_common_back(ES_Route_Entry es, RouteEntry plg)
{
	if (ES_ROUTE.isend(es) || isend(plg))
	{
		return { ES_ROUTE.end(), __route.end() };
	}
	while (true)
	{
		auto es_pt = ES_ROUTE.find_back(plg->Name, es);

		if (!ES_ROUTE.isend(es_pt))
		{
			return { es_pt, plg };
		}
		else {
			if (isbegin(plg))
				break;
			else
				--plg;
		}
	}
	return { ES_ROUTE.end(), __route.end() };
}

bool AircraftRoute::__coords_same(CPosition p1, CPosition p2)
{
	return ((fabs(p1.m_Latitude - p2.m_Latitude) < 0.00001) && (fabs(p1.m_Longitude - p2.m_Longitude) < 0.00001));
}

void AircraftRoute::__es_process_route_modifications(CFlightPlanExtractedRoute* fp_route)
{
	ES_ROUTE = ES_Route(fp_route);

	ES_Route_Entry es_route_item = ES_ROUTE.empty() ? ES_ROUTE.end() : --ES_ROUTE.end();
	RouteEntry plg_route_item = __route.empty() ? __route.end() : __static_back(__route.end());

	// Iterating static point until they are different or one of iterators ended
	if (!isend(plg_route_item) && !ES_ROUTE.isend(es_route_item))
	{
		while (true)
		{
			if (es_route_item->Name == plg_route_item->Name)
			{
				if (!isbegin(plg_route_item) && !ES_ROUTE.isbegin(es_route_item))
				{
					--es_route_item;
					plg_route_item = __static_back(plg_route_item);
					if (isend(plg_route_item))
					{
						break;
					}
				}
				else break;
			}
			else {
				// Names are different
				auto common_points = __find_common_back(es_route_item, plg_route_item);
				if (!ES_ROUTE.isend(common_points.first))
				{
					// removing plg route points between common and last common
					RouteEntry remove_from = (++common_points.second)--;
					RouteEntry remove_to = plg_route_item;

					plg_route_item = __route.erase(remove_from, ++remove_to);
					// insert euroscope point
					auto add_from = (++common_points.first)--;
					auto add_to = es_route_item;
					plg_route_item = __insert_es_points(add_from, add_to, plg_route_item);
					// apply indices
					es_route_item = common_points.first;
					plg_route_item = common_points.second;
				}
				else {
					// no common points. drop ES route
					RouteEntry remove_from = __route.begin();
					RouteEntry remove_to = plg_route_item;

					plg_route_item = __route.erase(remove_from, ++remove_to);
					__insert_es_points(ES_ROUTE.begin(), es_route_item, plg_route_item);
					es_route_item = ES_ROUTE.begin();
					plg_route_item = __route.begin();
					break;
				}
			}
		}
	}

	if (ES_ROUTE.isbegin(es_route_item) && !isbegin(plg_route_item))
	{
		// ES route ended but some points in plg route exists
		// Remove all excessive points
		__route.erase(__route.begin(), plg_route_item);
	}
	else if (!ES_ROUTE.isbegin(es_route_item) && isbegin(plg_route_item))
	{
		// plugin route ends but still some points in es
		__insert_es_points(ES_ROUTE.begin(), es_route_item, plg_route_item);
	}
}

void AircraftRoute::__check_points_passed(AircraftFlightData* fdata)
{
	if (!fdata->IsAir) return;

	if (CurrentDirectPoint != NULL)
	{
		double dist = fdata->PresentPosition.DistanceTo(CurrentDirectPoint->Position);
		double dir = fdata->PresentPosition.DirectionTo(CurrentDirectPoint->Position);

		auto hdg_diff = heading_diff(fdata->Track, dir);

		if (dist < 0.3 || (dist < 2 && hdg_diff > 90))
		{
			__erase_point(begin(__route));
			if (__route.size() > 0)
			{
				CurrentDirectPoint = &*begin(__route);
			}
			else CurrentDirectPoint = NULL;
		}
	}

	/*
	auto it = __route.begin();

	while (!isend(it))
	{
		double dist = fdata->PresentPosition.DistanceTo(it->Position);
		double dir = fdata->PresentPosition.DirectionTo(it->Position);

		auto hdg_diff = heading_diff(fdata->Track, dir);

		if (it->Removed || dist < 0.5 || (dist < 5 && hdg_diff > 90) || (face_180(fdata->Track, TrackToDestination) && !face_180(fdata->Track, dir)))
		{
			it = __route.erase(it);
		}
		else it++;
	}
	*/
}

bool AircraftRoute::__pt_after_pt(string what, string after)
{
	auto pt = __find_name(after);
	if (isend(pt)) return false;

	return !isend(find_if(pt, __route.end(), [&what](const PointDetails& pd) {  return pd.Name == what; }));
}

void AircraftRoute::__compute(CRadarTarget* rt, FormularData* formular, AircraftFlightData* data, bool RebuildRoute, bool ResetRoute, string DCT)
{
	auto currentTime = getCurrentTime();

	auto fp = rt->GetCorrelatedFlightPlan();
	if (!fp.IsValid())	return;

	auto fpd = fp.GetFlightPlanData();

	DepartureICAO = check_str_create(fpd.GetOrigin());
	DestinationICAO = check_str_create(fpd.GetDestination());
	DirectDistanceFromOrigin = fp.GetDistanceFromOrigin();
	DirectDistanceToDest = fp.GetDistanceToDestination();

	if (data->VerticalSpeed >= 400)
	{
		auto toc_alt = formular->ClearedAltitude > 0 ? formular->ClearedAltitude : formular->FinalAltitude;
		TopOfClimbDist = calcAltDist(data->CurrentAltitude, toc_alt, data->GroundSpeed, data->VerticalSpeed);
		TopOfDescendDist = 0;
	}
	else if (data->VerticalSpeed <= -400) {
		auto tod_alt = formular->ClearedAltitude > 0 ? formular->ClearedAltitude : 2000;
		TopOfDescendDist = calcAltDist(data->CurrentAltitude, tod_alt, data->GroundSpeed, data->VerticalSpeed);
		TopOfClimbDist = 0;
	}
	else {
		TopOfClimbDist = 0;
		TopOfDescendDist = 0;
	}

	PresentPosition = rt->GetPosition().GetPosition();

	CPosition* lastPosition = &PresentPosition;
	double totalDistance = 0.0;
	double distanceToLevelOff = 0.0;
	CPosition pointToLevelOff;
	CPosition directPointToLevelOff;

	if (TopOfClimbDist > 0 || TopOfDescendDist > 0)
	{
		distanceToLevelOff = TopOfClimbDist > 0 ? TopOfClimbDist : TopOfDescendDist;
		directPointToLevelOff = BearingTo(PresentPosition, data->Track, distanceToLevelOff);
	}
	else distanceToLevelOff = 0.0;

	CrossAreasNames.clear();
	CrossAreasIntersections.clear();

	auto rte = fp.GetExtractedRoute();
	TrackToDestination = PresentPosition.DirectionTo(rte.GetPointPosition(rte.GetPointsNumber() - 1));

	if (__route.empty() || RebuildRoute || ResetRoute)
	{
		if (ResetRoute)
			__route.clear();
		__es_process_route_modifications(&rte);
		ConstraintsLoaded = false;
		__check_depa_land_my_zone();
	}
	if (!__route.empty() && DCT != "")
	{
		SetFirstPoint(DCT);
	}
	__check_points_passed(data);

	IsInbound = LandInMyZone;

	for (auto& pd : __route)
	{
		pd.DirectDistance = PresentPosition.DistanceTo(pd.Position);
		pd.Distance = totalDistance + lastPosition->DistanceTo(pd.Position);
		pd.ExpectedAlt = calcAltAt(data->CurrentAltitude, pd.Distance, data->GroundSpeed, data->VerticalSpeed);
		pd.ExpectedDirectPassTime = currentTime + (time_t)(int)(PresentPosition.DistanceTo(pd.Position) / (data->GroundSpeed + 0.001) * 3600);
		pd.ExpectedPassTime = currentTime + (time_t)(int)((pd.Distance / (data->GroundSpeed + 0.001)) * 3600);

		if (distanceToLevelOff > 0) {

			if ((totalDistance <= distanceToLevelOff) && ((totalDistance + lastPosition->DistanceTo(pd.Position)) > distanceToLevelOff))
			{
				auto trk = lastPosition->DirectionTo(pd.Position);
				pointToLevelOff = BearingTo(*lastPosition, trk + 10, distanceToLevelOff - totalDistance);
				if (TopOfClimbDist > 0)
				{
					TopOfClimbDirectPoint = directPointToLevelOff;
					TopOfClimbPoint = pointToLevelOff;
				}
				else {
					TopOfDescendDirectPoint = directPointToLevelOff;
					TopOfDescendPoint = pointToLevelOff;
				}
			}
		}

		__addCrossingAreas2({ pd.Position.m_Latitude, pd.Position.m_Longitude }, { lastPosition->m_Latitude, lastPosition->m_Longitude });

		totalDistance += lastPosition->DistanceTo(pd.Position);

		lastPosition = &pd.Position;
	}
	if (CrossAreasNames.size() == 0)
	{
		// inside one?
		for (auto area = SectorAreas.begin(); area != SectorAreas.end(); area++)
		{
			if (insidePolygon(area->Coords, cp_to_p(PresentPosition)))
			{
				CrossAreasNames.insert(area->Name);
			}
		}
	}
	if (!__route.empty())
	{
		CurrentDirectPoint = &__route.front();
	}
	else CurrentDirectPoint = NULL;

	RouteDistanceToDest = totalDistance;

	if (!__getSectorArea(PresentPosition, data->CurrentAltitude, CurrentArea, CurrentAreasIsRadar))
	{
		CurrentArea.Name = "";
	}
	if (!ConstraintsLoaded)
	{
		__calcContraints(data->CustomData.Sid, data->CustomData.Star);
		ConstraintsLoaded = true;
	}

	if (!data->IsAir)
	{
		__getExitBearing(data->CustomData.Sid);
	}
}

/*
inline void AircraftRoute::__addCrossingAreas(GeoPoint start, GeoPoint end)
{
	for (auto area = SectorAreas.begin(); area != SectorAreas.end(); area++)
	{
		for (size_t i = 0, j = 1; j < area->Coords.size(); i++, j++)
		{
			if (doIntersect({ area->Coords[i].x, area->Coords[i].y }, { area->Coords[j].x, area->Coords[j].y }, { start.x, start.y }, { end.x, end.y }))
			{
				CrossAreas.push_back(*area);
				Vector xc;
				if (Vector::LineSegementsIntersect({ area->Coords[i].x, area->Coords[i].y }, { area->Coords[j].x, area->Coords[j].y }, { start.x, start.y }, { end.x, end.y }, xc))
				{
					CPosition p;
					p.m_Latitude = xc.X;
					p.m_Longitude = xc.Y;
					CrossAreasIntersections[area->Name].push_back(p);
				}

			}
		}
		// and last segment
		if (doIntersect({ area->Coords[0].x, area->Coords[0].y }, { area->Coords[area->Coords.size() - 1].x, area->Coords[area->Coords.size() - 1].y }, { start.x, start.y }, { end.x, end.y }))
		{
			CrossAreas.push_back(*area);
		}
	}
}
*/

inline void AircraftRoute::__addCrossingAreas2(GeoPoint start, GeoPoint end)
{
	for (auto area = SectorAreas.begin(); area != SectorAreas.end(); area++)
	{
		for (size_t i = 0, j = 1; j < area->Coords.size(); i++, j++)
		{
			Vector xc;
			if (Vector::LineSegementsIntersect({ area->Coords[i].x, area->Coords[i].y }, { area->Coords[j].x, area->Coords[j].y }, { start.x, start.y }, { end.x, end.y }, xc))
			{
				CPosition p;
				p.m_Latitude = xc.X;
				p.m_Longitude = xc.Y;
				CrossAreasNames.insert(area->Name);
				CrossAreasIntersections[area->Name].push_back(p);
			}
		}
		// and last segment
		Vector xc;
		if (Vector::LineSegementsIntersect({ area->Coords[0].x, area->Coords[0].y }, { area->Coords[area->Coords.size() - 1].x, area->Coords[area->Coords.size() - 1].y }, { start.x, start.y }, { end.x, end.y }, xc))
		{
			CPosition p;
			p.m_Latitude = xc.X;
			p.m_Longitude = xc.Y;
			CrossAreasNames.insert(area->Name);
			CrossAreasIntersections[area->Name].push_back(p);
		}
	}
}

set<string> AircraftRoute::GetCrossingAreaNames(CPosition p1, CPosition p2)
{
	set<string> ret;

	for (auto area = SectorAreas.begin(); area != SectorAreas.end(); area++)
	{
		for (size_t i = 0, j = 1; j < area->Coords.size(); i++, j++)
		{
			if (doIntersect({ area->Coords[i].x, area->Coords[i].y }, { area->Coords[j].x, area->Coords[j].y }, cp_to_p(p1), cp_to_p(p2)))
			{
				ret.insert(area->Name);
			}
		}
		// and last segment
		if (doIntersect({ area->Coords[0].x, area->Coords[0].y }, { area->Coords[area->Coords.size() - 1].x, area->Coords[area->Coords.size() - 1].y }, cp_to_p(p1), cp_to_p(p2)))
		{
			ret.insert(area->Name);
		}
	}
	if (ret.size() == 0)
	{
		// inside one of?
		for (auto area = SectorAreas.begin(); area != SectorAreas.end(); area++)
		{
			if (insidePolygon(area->Coords, cp_to_p(p1)))
			{
				ret.insert(area->Name);
			}
		}
	}
	return ret;
}

set<string> AircraftRoute::GetAreasOfPoint(CPosition p1)
{
	set<string> ret;
	for (auto area = SectorAreas.begin(); area != SectorAreas.end(); area++)
	{
		if (insidePolygon(area->Coords, cp_to_p(p1)))
		{
			ret.insert(area->Name);
		}
	}
	return ret;
}

void AircraftRoute::__calcContraints(string sid, string star)
{
	lock_guard<mutex> lock(Plugin->__extern_data_locker);

	if (CurrentDirectPoint == NULL)
	{
		return;
	}
	ActiveStarConstraint = "";
	ActiveSidContraint = "";
	ActiveConstraint = "";

	bool destInList = Constraints.APInList(DestinationICAO);
	bool originInList = Constraints.APInList(DepartureICAO);

	if (!destInList && !originInList)
	{
		return;
	}

	if (sid == "" && star == "")
	{
		return;
	}

	for (auto& p : __route)
	{
		if (star != "")
		{
			auto cst = Constraints.FindConstraint(star, p.Name);
			p.StarConstraint = cst;
			if (ActiveStarConstraint == "") ActiveStarConstraint = cst;
		}
		if (sid != "")
		{
			auto cst = Constraints.FindConstraint(sid, p.Name);
			p.SidConstraint = cst;
			if (ActiveSidContraint == "") ActiveSidContraint = cst;
		}
	}
	// Showing active contraint
	if (destInList && !originInList && RouteDistanceToDest > 250)
	{
		ActiveConstraint = CurrentDirectPoint->Name + " " + formatTime(CurrentDirectPoint->ExpectedPassTime);
	}
	else if (destInList && !originInList && RouteDistanceToDest <= 250)
	{
		if (ActiveStarConstraint != "")
			ActiveConstraint = ActiveStarConstraint;
		else ActiveConstraint = CurrentDirectPoint->Name + " " + formatTime(CurrentDirectPoint->ExpectedPassTime);
	}
	else if (!destInList && originInList && DirectDistanceFromOrigin <= 250)
	{
		if (ActiveSidContraint != "")
			ActiveConstraint = ActiveSidContraint;
		else ActiveConstraint = CurrentDirectPoint->Name + " " + formatTime(CurrentDirectPoint->ExpectedPassTime);
	}
	else if (!destInList && originInList && DirectDistanceFromOrigin > 250)
	{
		ActiveConstraint = CurrentDirectPoint->Name + " " + formatTime(CurrentDirectPoint->ExpectedPassTime);
	}
	else if (!destInList && !originInList)
	{
		ActiveConstraint = CurrentDirectPoint->Name + " " + formatTime(CurrentDirectPoint->ExpectedPassTime);
	}
}

bool AircraftRoute::__getSectorArea(CPosition p, int alt, SectorArea& s, bool& IsRadar)
{
	lock_guard<mutex> lock(Plugin->__extern_data_locker);

	int cur_alt = 500000;

	for (auto& A : SectorAreas) {

		if (isInside(&A.Coords[0], A.Coords.size(), { p.m_Latitude, p.m_Longitude })) {
			if (alt <= A.BelowAlt && A.BelowAlt <= cur_alt) {
				s = A;
				cur_alt = A.BelowAlt;
			}
		}
	}
	if (cur_alt == 500000) {
		return false;
	}
	for (auto& P : Plugin->Controllers->Ownership)
	{
		if (alt >= P.Above && alt <= P.Below && P.AreaName == s.Name)
		{
			IsRadar = P.IsRadar;
		}
	}
	return true;
}

void AircraftRoute::__getExitBearing(string sid)
{
	if (sid == "")
	{
		ExitBearing = -1;
		return;
	}

	CPosition* foundPos = NULL;
	for (auto& p : __route)
	{
		if (foundPos != NULL)
		{
			ExitBearing = (*foundPos).DirectionTo(p.Position);
			return;
		}
		string SidPreffix;
		if (p.Name.length() == 5)
		{
			SidPreffix = p.Name.substr(0, 4);
		}
		else SidPreffix = p.Name;

		auto f = sid.find(SidPreffix);
		if (f != string::npos)
		{
			foundPos = &p.Position;
		}
	}
}

void AircraftRoute::__check_depa_land_my_zone()
{
	auto depa = DepartureICAO;
	auto land = DestinationICAO;

	CPosition depa_ps;
	CPosition land_ps;

	LandInMyZone = false;
	DepaInMyZone = false;

	if (depa == "" || land == "")	return;

	for (auto se = Plugin->SectorFileElementSelectFirst(SECTOR_ELEMENT_AIRPORT); se.IsValid(); se = Plugin->SectorFileElementSelectNext(se, SECTOR_ELEMENT_AIRPORT))
	{
		if (se.GetName() == depa)
		{
			se.GetPosition(&depa_ps, 0);
		}
		if (se.GetName() == land)
		{
			se.GetPosition(&land_ps, 0);
		}
	}

	auto areas_depa = GetAreasOfPoint(depa_ps);
	auto areas_land = GetAreasOfPoint(land_ps);

	DepaInMyZone = Plugin->Controllers->ImOwningAnyOfAreaNames(Plugin->MyCS, areas_depa, 0, 0);
	LandInMyZone = Plugin->Controllers->ImOwningAnyOfAreaNames(Plugin->MyCS, areas_land, 0, 0);
}

void AircraftRoute::Clear()
{
	__route.clear();
}

PointDetails AircraftRoute::AddPointAfterName(string Name, CPosition pos, string AfterName)
{
	auto pt = __find_name(AfterName);
	if (isend(pt))	return PointDetails();
	return *__insert_point(Name, pos, true, ++pt);
}

PointDetails AircraftRoute::AddPointBeforeName(string Name, CPosition pos, string BeforeName)
{
	auto pt = __find_name(BeforeName);
	if (isend(pt))	return PointDetails();
	return *__insert_point(Name, pos, true, pt);
}

PointDetails AircraftRoute::FindPoint(string Name)
{
	auto pt = __find_name(Name);
	return !isend(pt) ? *pt : PointDetails();
}

PointDetails AircraftRoute::FindCoord(CPosition* ps)
{
	auto pt = __find_coord(ps);
	return !isend(pt) ? *pt : PointDetails();
}

bool AircraftRoute::DelPointByName(string Name)
{
	auto pt = __find_name(Name);
	if (isend(pt))	return false;
	__erase_point(pt);
	return true;
}

void AircraftRoute::SetFirstPoint(string PointName)
{
	auto pt = __find_name(PointName);
	if (!isend(pt) && !isbegin(pt))
	{
		__erase_back_to_end(pt);
	}
}

void AircraftRoute::PointAfterPoint(string Start, string End)
{
	auto spt = __find_name(Start);
	auto ept = __find_name(End);

	if (isend(spt) || isend(ept)) return;
	if (!__pt_after_pt(End, Start)) return;

	__pt_x_pt(spt, ept, true);
}

bool AircraftRoute::HasZone(string Zone)
{
	return find_if(begin(CrossAreasNames), end(CrossAreasNames), [&Zone](const string& zn) {
		return zn == Zone;
		}) != end(CrossAreasNames);
}

PointDetails* AircraftRoute::GetLastPoint()
{
	if (__route.size() == 0)	return NULL;
	return &__route.back();
}

ES_Route::ES_Route(CFlightPlanExtractedRoute* plan)
{
	if (plan == NULL)	return;

	__items.clear();

	int pointsCount = plan->GetPointsNumber();
	int closest_point = plan->GetPointsCalculatedIndex();
	int dir_to_index = plan->GetPointsAssignedIndex();
	int nextPoint = dir_to_index > -1 ? dir_to_index : closest_point;

	for (int i = nextPoint; i < pointsCount; i++)
	{
		__items.push_back({ i, check_str_create(plan->GetPointName(i)), plan->GetPointPosition(i) });
	}
}

ES_Route_Entry ES_Route::find(string Name)
{
	return ES_Route_Entry(); // TODO: check if needed
}

ES_Route_Entry ES_Route::find_back(string Name, ES_Route_Entry from)
{
	for (ES_Route_Entry it = from; it != __items.begin(); --it)
	{
		if (it->Name == Name)
		{
			return it;
		}
	}
	return __items.end();
}
