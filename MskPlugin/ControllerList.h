#pragma once
#include "Consts.h"
#include <map>
#include <mutex>
#include <vector>
#include "Global.h"
#include <set>

using namespace EuroScopePlugIn;

class CMskPlugin;

class ControllerList
{
private:
	mutex __ctrl_list_acc;

	void __build_ownership_map();
	map<string, vector<AreaOwnership>> __ctrl_to_zones;

public:
	void Sort();

	vector<AreaOwnership> Ownership;	// <- loaded from external data
	map<string, ControllerEntry> Controllers;

	void NewController(CController& ctrl);
	void ControllerGone(CController& ctrl);
	ControllerEntry GetController(string Callsign);

	bool ImOwningArea(string MyCallsign, string Name, int CurrentAlt = 0, int TargetAlt = 0);
	bool ImOwningAnyOfAreaNames(string MyCallsign, set<string> Names, int CurrentAlt=0, int ClearedAlt=0);
	set<string> GetOwningAreas(string MyCallsign, set<string> Names, int CurrentAlt = 0, int ClearedAlt = 0);

	bool PortHasFacility(string PortICAO, int Facility);
	bool PortHasFacilityLowerThan(string PortICAO, int Facility);
	bool PortHasFacilityHigherThan(string PortICAO, int Facility);
	
};

