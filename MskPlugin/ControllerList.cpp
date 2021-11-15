#include "pch.h"
#include "ControllerList.h"
#include <algorithm>
#include "Common.h"
#include "CMskPlugin.h"


using namespace std;
using namespace EuroScopePlugIn;

void ControllerList::__build_ownership_map()
{
	for (auto& area : Ownership)
	{
		area.PrimaryOwners.clear();
		area.SecondaryOwners.clear();

		// Primary
		for (auto& ctrl : Controllers)
		{
			if (ctrl.second.Facility < ES_FACILITY_FSS) continue;
			if (!regex_string_match(area.PrimaryCSPattern, ctrl.first)) continue;
			area.PrimaryOwners.push_back(ctrl.second);
		}
		// Secondary
		if (area.PrimaryOwners.size() == 0 && area.SecondaryCSPattern != "")
		{
			for (auto& ctrl : Controllers)
			{
				if (ctrl.second.Facility < ES_FACILITY_FSS) continue;
				if (!regex_string_match(area.SecondaryCSPattern, ctrl.first)) continue;
				area.SecondaryOwners.push_back(ctrl.second);
			}
		}
	}
	if (Ownership.size() == 0) return;
	// After fulfilled - recycle again to fill TOP-DOWN

	try {
		auto it = end(Ownership);
		while (it != begin(Ownership))
		{
			--it;
			if (it->PrimaryOwners.size() == 0 && it->SecondaryOwners.size() == 0)
			{
				auto prev = (++it)--;
				if (prev != end(Ownership) && prev->AreaName == it->AreaName)
				{
					it->PrimaryOwners = prev->PrimaryOwners;
					it->SecondaryOwners = prev->SecondaryOwners;
				}
			}
		}
	}
	catch (...)
	{
		Plugin->DisplayUrgent("Exception raised in build ownership duing fill TOP-DOWN");
	}
	// Controller to zones
	__ctrl_to_zones.clear();
	for (auto& area : Ownership)
	{
		for (auto& PR : area.PrimaryOwners)
		{
			__ctrl_to_zones[PR.Callsign].push_back(area);
		}
		for (auto& SC : area.SecondaryOwners)
		{
			__ctrl_to_zones[SC.Callsign].push_back(area);
		}
	}
}

void ControllerList::Sort()
{
	lock_guard<mutex> lock(__ctrl_list_acc);
	sort(Ownership.begin(), Ownership.end());
}

void ControllerList::NewController(CController& ctrl)
{
	lock_guard<mutex> lock(__ctrl_list_acc);

	string CS = check_str_create(ctrl.GetCallsign());
	string Id = check_str_create(ctrl.GetPositionId());
	int Facility = ctrl.GetFacility();

	auto is_new = Controllers.count(CS) == 0;
	Controllers[CS] = { CS, Id, Facility, safe_substr(CS,0,4) };
	if (is_new)
	{
		__build_ownership_map();
	}
}

void ControllerList::ControllerGone(CController& ctrl)
{
	lock_guard<mutex> lock(__ctrl_list_acc);
	string CS = check_str_create(ctrl.GetCallsign());
	if (Controllers.count(CS) > 0)
	{	
		Controllers.erase(CS);
	}
	__build_ownership_map();
}

ControllerEntry ControllerList::GetController(string Callsign)
{
	lock_guard<mutex> lock(__ctrl_list_acc);

	if (Controllers.count(Callsign) > 0)
	{
		return Controllers[Callsign];
	}
	return ControllerEntry();
}

bool ControllerList::ImOwningAnyOfAreaNames(string MyCallsign, set<string> Names, int CurrentAlt, int ClearedAlt)
{
	for (auto& area : Names)
	{
		if (ImOwningArea(MyCallsign, area, CurrentAlt, ClearedAlt))
			return true;
	}
	return false;
}

set<string> ControllerList::GetOwningAreas(string MyCallsign, set<string> Names, int CurrentAlt, int ClearedAlt)
{
	set<string> ret;
	for (auto& area : Names)
	{
		if (ImOwningArea(MyCallsign, area, CurrentAlt, ClearedAlt))
			ret.insert(area);
	}
	return ret;
}

bool ControllerList::PortHasFacility(string PortICAO, int Facility)
{
	lock_guard<mutex> lock(__ctrl_list_acc);

	for (auto& C : Controllers)
	{
		if (C.second.Port == PortICAO && C.second.Facility == Facility)
			return true;
	}
	return false;
}

bool ControllerList::PortHasFacilityLowerThan(string PortICAO, int Facility)
{
	lock_guard<mutex> lock(__ctrl_list_acc);

	for (auto& C : Controllers)
	{
		if (C.second.Port == PortICAO && C.second.Facility < Facility && C.second.Facility > 2)	// excluding delivery
			return true;
	}
	return false;
}

bool ControllerList::PortHasFacilityHigherThan(string PortICAO, int Facility)
{
	lock_guard<mutex> lock(__ctrl_list_acc);

	for (auto& C : Controllers)
	{
		if (C.second.Port == PortICAO && C.second.Facility > Facility)
			return true;
	}
	return false;
}

bool ControllerList::ImOwningArea(string MyCallsign, string Name, int CurrentAlt, int TargetAlt)
{
	for (auto& a : Ownership)
	{
		if (Name != a.AreaName) continue;

		if (((CurrentAlt == 0) && (TargetAlt == 0)) || ((CurrentAlt >= a.Above) && (CurrentAlt <= a.Below)) || ((TargetAlt >= a.Above) && (TargetAlt <= a.Below)))
		{
			if (a.ImOwningIt(MyCallsign)) return true;
		}
	}
	return false;
}
