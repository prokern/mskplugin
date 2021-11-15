#pragma once
#include "Consts.h"
#include "Common.h"
#include <algorithm>
#include <mutex>
#include <queue>
#include "AircraftRoute.h"
#include "CCDLC.h"

using namespace std;
using namespace EuroScopePlugIn;

class CMskRadarScreen;
class CMskPlugin;

class AircraftExtendedData
{
public:

	AircraftFlightData PreviousFlightData;
	AircraftFlightData ActualFlightData;
	FormularData PreviousFormularData;
	FormularData ActualFormularData;
	ControllerInfo PreviousControllerInfo;
	ControllerInfo ActualControllerInfo;
	PlanInfo PreviousPlanInfo;
	PlanInfo ActualPlanInfo;
	AircraftRoute Route;
	MskListDisplay DisplayIn;
	AircraftRadarDisplayInfo DisplayOnRadar;
	time_t CreateTime = 0;
	ActiveCCDLCRequest CCDLCRequest;
	OwnPlanState OwnState;

	static bool RadarTargetFullCorrect(CRadarTarget rt);
	static bool FPFullCorrect(CFlightPlan fp);
	bool CheckIfCCDLCActual(CCDLC_Message msg);
	void Compute(CRadarTarget * rt, ComputeTask * task);

	inline void ClearStar()
	{
		__star_cleared = true;
	}
private:
	vector<int> __GS_Snapshots;
	vector<int> __VS_Snapshots;

	inline void __load_plan_info(CFlightPlan* fp);
	inline void __load_controller_info(CFlightPlan* fp);
	inline void __load_formular(CRadarTarget* rt, CFlightPlan* fp);
	inline void __load_actual_flight_data(CRadarTarget* rt);
	inline void __load_auto_status();
	inline void __getFAFandCAPS();
	inline void __computeWarnings();
	inline void __auto_manual_status();
	inline void __request_cert_if_needed();

	pair<bool, bool> __check_caps(CFlightPlanData* fpd);

	int __GetAvgGS();
	int __GetAvgVS();
	void __GS_Snapshot_Add(int V);
	void __VS_Snapshot_Add(int G);
	int __GetGSTrend();
	void __load(CRadarTarget * rt, ComputeTask * task);
	void __calcTTL();
	int __getTA();
	void __checkForAssigments(CRadarTarget * rt);
	void __set_init_climb_if_req();

	int __own_state();
	bool __star_cleared = false;

	// List processing
	MskListDisplay __get_req_display_list();
	MskListDisplay __list_delivery();
	MskListDisplay __list_ground();
	MskListDisplay __list_tower();
	MskListDisplay __list_radar();
	MskListDisplay __list_msk_app();
	MskListDisplay __list_uuwv_ctr();
	MskListDisplay __list_other_ctr();
};

class AircraftExtendedDataPool 
{
	friend class CCDLC;
private:
	unordered_map<string, AircraftExtendedData> __planes;
	mutex __pool_lock;
	atomic<bool> __locked = false;
	void __ExtendedDataComputation();
	mutex __computation_completed;
	queue<string> __computed_planes;
	void __add_computed_plane(string cs);
public:
	AircraftExtendedDataPool();
	void sortAcftList();
	vector<AircraftExtendedData*> SortedAcftList;
	AircraftExtendedData* operator [] (const string& cs);
	AircraftExtendedData* lastPlane = NULL;
	string lastPlaneCS = "";
	void lock();
	void unlock();
	bool alive(string cs);
	void remove(string CS);
	string GetComputedPlaneCS();

	template<typename Func>
	void Enum(Func f)
	{
		lock_guard<mutex> lock(__pool_lock);
		for (auto& P : __planes)
		{
			f(&P.second);
		}
	}
};

class ExtendedDataRequest {
private:
	atomic<bool> __released = false;
public:
	ExtendedDataRequest();
	~ExtendedDataRequest();
	AircraftExtendedData* request(string callsign);
	bool check(string callsign);
	void erase(string callsign);
	void release();
	AircraftExtendedData* check_req(string callsign);
	
};