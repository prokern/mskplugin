#include "pch.h"
#include "AircraftExtendedData.h"
#include "CMskPlugin.h"
#include "Global.h"
#include "CCDLC.h"
#include <bitset>
#include "CMskPlugin.h"
#include <chrono>

int AircraftExtendedData::__own_state()
{
	// after lists
	if (ActualPlanInfo.IsCurrent)
	{
		return FP_OWN_STATE_CURRENT;
	}
	if (ActualControllerInfo.IsTransferFromMe)
	{
		return FP_OWN_STATE_TRANSFER_FROM_ME;
	}
	if (ActualControllerInfo.IsTransferToMe)
	{
		return FP_OWN_STATE_TRANSFER_TO_ME;
	}
	if (DisplayIn.any(DisplayIn))
	{
		return FP_OWN_STATE_EXPECTED;
	}
	return FP_OWN_STATE_NOT_CONCERNED;
}

bool AircraftExtendedData::CheckIfCCDLCActual(CCDLC_Message msg)
{
	if (msg.Author == Plugin->MyId)
	{
		return true;
	}
	if (ActualControllerInfo.IHaveControlledIt && ActualControllerInfo.IsTrackedByOther)
	{
		return false;
	}
	if (ActualPlanInfo.IsCurrent)
	{
		return true;
	}
	if (Plugin->MyFacility < ES_FACILITY_APPDEP)
	{
		return false;
	}
	// After route computation only
	auto presentPosition = ActualFlightData.PresentPosition;

	if (msg.Cmd == "DCT")
	{
		auto destinationPoint = Route.FindPoint(msg.Value);
		if (destinationPoint.IsValid())
		{
			auto destPos = destinationPoint.Position;
			auto areas = Route.GetCrossingAreaNames(presentPosition, destPos);

			return Plugin->Controllers->ImOwningAnyOfAreaNames(Plugin->MyCS, areas, 0, 0);
		}
	}
	else if (msg.Cmd == "SPD" || msg.Cmd == "ALT" || msg.Cmd == "TXT" || msg.Cmd == "CMT")
	{
		return Plugin->Controllers->ImOwningAnyOfAreaNames(Plugin->MyCS, Route.CrossAreasNames, 0, 0);
	}
	return false;
}

void AircraftExtendedData::Compute(CRadarTarget* rt, ComputeTask* task)
{
	auto fp = rt->GetCorrelatedFlightPlan();
	if (!fp.IsValid())	return;

	if (CreateTime == 0)
	{
		CreateTime = getCurrentTime();
	}

	__load_plan_info(&fp);
	__load_controller_info(&fp);
	__load_actual_flight_data(rt);
	__load_formular(rt, &fp);
	__load_auto_status();

	Route.__compute(rt, &ActualFormularData, &ActualFlightData, task->RebuildRoute, task->ResetRoute, task->DCT);

	__auto_manual_status();
	__calcTTL();
	__computeWarnings();
	__checkForAssigments(rt);

	DisplayIn = __get_req_display_list();
	ActualPlanInfo.OwnPlanState = __own_state();
	if (Plugin->hide_display_route)
	{
		DisplayOnRadar.DisplayRoute = false;
	}
	__request_cert_if_needed();
}

inline void AircraftExtendedData::__load_plan_info(CFlightPlan* fp)
{
	PreviousPlanInfo = ActualPlanInfo;

	ActualPlanInfo.PlanState = fp->GetState();
	ActualPlanInfo.PlanType = fp->GetFPState();
	ActualPlanInfo.IsCurrent = ActualPlanInfo.PlanState == FLIGHT_PLAN_STATE_ASSUMED;
	ActualPlanInfo.IsExpectedOrCurrent = (ActualPlanInfo.PlanState != FLIGHT_PLAN_STATE_NON_CONCERNED) && (ActualPlanInfo.PlanState != FLIGHT_PLAN_STATE_REDUNDANT);
	ActualPlanInfo.IsOnlyExpected = ActualPlanInfo.PlanState == FLIGHT_PLAN_STATE_NOTIFIED;
	ActualPlanInfo.IsOut = (ActualPlanInfo.PlanState == FLIGHT_PLAN_STATE_NON_CONCERNED) || (ActualPlanInfo.PlanState == FLIGHT_PLAN_STATE_REDUNDANT);
}

inline void AircraftExtendedData::__load_controller_info(CFlightPlan* fp)
{
	PreviousControllerInfo = ActualControllerInfo;

	ActualControllerInfo.ActiveControllerCS = check_str_create(fp->GetTrackingControllerCallsign());
	if (ActualControllerInfo.ActiveControllerCS != "")
	{
		auto ctrl = Plugin->ControllerSelect(ActualControllerInfo.ActiveControllerCS.c_str());
		if (ctrl.IsValid())
		{
			ActualControllerInfo.ActiveControllerFacility = ctrl.GetFacility();
		}

	}
	ActualControllerInfo.PreviousControllerCS = PreviousControllerInfo.ActiveControllerCS;
	ActualControllerInfo.ActiveControllerId = check_str_create(fp->GetTrackingControllerId());
	ActualControllerInfo.PreviousControllerId = PreviousControllerInfo.ActiveControllerId;
	ActualControllerInfo.NextControllerId = check_str_create(fp->GetCoordinatedNextController());
	ActualControllerInfo.TrackedByMe = fp->GetTrackingControllerIsMe();

	if ((!ActualControllerInfo.IHaveControlledIt && ActualPlanInfo.IsCurrent) || Plugin->History->Exists(check_str_create(fp->GetCallsign())))
	{
		ActualControllerInfo.IHaveControlledIt = true;
	}
	ActualControllerInfo.IsTrackedByOther = ActualControllerInfo.ActiveControllerId != "" && ActualControllerInfo.ActiveControllerId != Plugin->MyId;
	ActualControllerInfo.IsTransferToMe = ActualPlanInfo.PlanState == FLIGHT_PLAN_STATE_TRANSFER_TO_ME_INITIATED;
	ActualControllerInfo.IsTransferFromMe = ActualPlanInfo.PlanState == FLIGHT_PLAN_STATE_TRANSFER_FROM_ME_INITIATED;

	if (ActualControllerInfo.ActiveControllerCS != "")
	{
		ActualControllerInfo.OtherTrackerFacility = Plugin->Controllers->GetController(ActualControllerInfo.ActiveControllerCS).Facility;
	}
	else ActualControllerInfo.OtherTrackerFacility = -1;
}

void AircraftExtendedData::__load_actual_flight_data(CRadarTarget* rt)
{
	bool isFirstSnapshot = PreviousFlightData.GroundSpeed == -1;
	PreviousFlightData = ActualFlightData;

	ActualFlightData.DataIsFirst = isFirstSnapshot;

	ActualFlightData.SnapshotTime = getCurrentTime();

	auto p = rt->GetPosition();
	auto ad = rt->GetCorrelatedFlightPlan().GetControllerAssignedData();
	auto fp = rt->GetCorrelatedFlightPlan();
	auto fpd = rt->GetCorrelatedFlightPlan().GetFlightPlanData();

	ActualFlightData.PresentPosition = p.GetPosition();
	ActualFlightData.TA = __getTA();
	ActualFlightData.__FlightLevel = p.GetFlightLevel();
	ActualFlightData.__PressureAlt = p.GetPressureAltitude();
	ActualFlightData.CurrentAltitude = (ActualFlightData.__PressureAlt <= ActualFlightData.TA) ? ActualFlightData.__PressureAlt : ActualFlightData.__FlightLevel;
	ActualFlightData.AFL_IsAltitude = (ActualFlightData.__PressureAlt <= ActualFlightData.TA) ? true : false;
	ActualFlightData.GroundSpeed = rt->GetGS();
	ActualFlightData.Track = (int)rt->GetTrackHeading();
	ActualFlightData.Heading = p.GetReportedHeading();
	ActualFlightData.TrueHeading = p.GetReportedHeadingTrueNorth();
	ActualFlightData.Pitch = p.GetReportedPitch();
	ActualFlightData.Bank = p.GetReportedBank();

	if (PreviousFlightData.IsValid())
	{
		ActualFlightData.CustomData = PreviousFlightData.CustomData;

		int deltaAlt = ActualFlightData.CurrentAltitude - PreviousFlightData.CurrentAltitude;
		time_t deltaT = ActualFlightData.SnapshotTime - PreviousFlightData.SnapshotTime;
		if (deltaT == 0) deltaT = 1;
		int vs = (deltaAlt / (int)deltaT) * 60;
		__VS_Snapshot_Add(vs);
	}

	ActualFlightData.CustomData.DepartureRwy = check_str_create(fpd.GetDepartureRwy());
	ActualFlightData.CustomData.DestinationRwy = check_str_create(fpd.GetArrivalRwy());
	ActualFlightData.CustomData.Sid = check_str_create(fpd.GetSidName());
	ActualFlightData.CustomData.Star = check_str_create(fpd.GetStarName());
	ActualFlightData.CustomData.DepartureICAO = check_str_create(fpd.GetOrigin());
	ActualFlightData.CustomData.DestinationICAO = check_str_create(fpd.GetDestination());

	ActualFlightData.VerticalSpeed = __GetAvgVS();
}

inline void AircraftExtendedData::__load_formular(CRadarTarget* rt, CFlightPlan* fp)
{
	PreviousFormularData = ActualFormularData;

	ActualFormularData.Callsign = check_str_create(fp->GetCallsign());

	auto rt_p = rt->GetPosition();
	if (rt_p.IsValid())
	{
		ActualFormularData.IsCharlie = rt_p.GetRadarFlags() >= 3;
	}
	else ActualFormularData.IsCharlie = false;

	__getFAFandCAPS();

	auto ad = fp->GetControllerAssignedData();

	ActualFormularData.ClearedAltitude = ad.GetClearedAltitude() != 0 ? ad.GetClearedAltitude() : fp->GetClearedAltitude();
	ActualFormularData.CFL_IsAltitude = ActualFormularData.ClearedAltitude <= ActualFlightData.TA;
	ActualFormularData.FinalAltitude = ad.GetFinalAltitude() != 0 ? ad.GetFinalAltitude() : fp->GetFinalAltitude();
	ActualFormularData.ActualSquawk = check_str_create(rt->GetPosition().GetSquawk());
	ActualFormularData.AssignedSquawk = check_str_create(ad.GetSquawk());
	ActualFormularData.HasClearance = fp->GetClearenceFlag();
	ActualFormularData.CommType = fp->IsTextCommunication() ? "T" : "V";
	ActualFormularData.AssignedMach = ad.GetAssignedMach();
	ActualFormularData.AssignedRate = ad.GetAssignedRate();
	ActualFormularData.AssignedSpeed = ad.GetAssignedSpeed();
	ActualFormularData.AssignedHeading = ad.GetAssignedHeading();
	auto acft_wt = fp->GetFlightPlanData().GetAircraftWtc();
	ActualFormularData.IsHeavy = acft_wt == 'H' || acft_wt == 'J';

	if (ActualFormularData.AcceptedTime == 0 && ActualPlanInfo.IsCurrent)
	{
		ActualFormularData.AcceptedTime = getCurrentTime();
	}

	ActualFormularData.ExtendedData.RVSM = fp->GetFlightPlanData().IsRvsm();
	auto caps = fp->GetFlightPlanData().GetCapibilities();
	//ActualFormularData.ExtendedData.Rnav = 
	//TODO: CONTINUE HERE WITH RNAV/RVSM CAPS
}

void AircraftExtendedData::__auto_manual_status()
{
	// 1. Auto set MAN modes (sq -> SQWK if set and differs)
	if (Plugin->MyFacility == ES_FACILITY_CTR && ActualFlightData.IsAir && ActualPlanInfo.IsCurrent && Route.IsInbound && !__star_cleared)
	{
		ActualFlightData.CustomData.ManualStatus = "STAR";
	}
	if (ActualFlightData.CustomData.ManualStatus != "" && ActualFlightData.IsAir && IsGroundStatus(ActualFlightData.CustomData.ManualStatus))
	{
		ActualFlightData.CustomData.ManualStatus = "";
		if (ActualControllerInfo.TrackedByMe && Plugin->ImController())
		{
			Plugin->Ccdlc->AddCCDLCMessageToQueue(ActualFormularData.Callsign, "STS", "CLEAR");
		}
	}
	if (ActualFlightData.CustomData.ManualStatus == "" && ActualFormularData.AssignedHeading != 0 && ActualPlanInfo.IsCurrent)
	{
		ActualFlightData.CustomData.ManualStatus = "VECT";
	}
	if (ActualFlightData.CustomData.ManualStatus == "VECT" && ActualFormularData.AssignedHeading == 0)
	{
		ActualFlightData.CustomData.ManualStatus = "";
	}
	if (ActualFlightData.CustomData.ManualStatus == "VECT" && ActualFlightData.CurrentAltitude < 2000)
	{
		ActualFlightData.CustomData.ManualStatus = "";
	}
	if (ActualPlanInfo.IsCurrent && ActualFlightData.IsAir && ActualFormularData.AssignedSquawk != ActualFormularData.ActualSquawk)
	{
		ActualFlightData.CustomData.ManualStatus = "SQWK";
	}
	if (ActualFlightData.CustomData.ManualStatus == "SQWK" && ActualFormularData.AssignedSquawk == ActualFormularData.ActualSquawk)
	{
		ActualFlightData.CustomData.ManualStatus = "";
	}
}

pair<bool, bool> AircraftExtendedData::__check_caps(CFlightPlanData* fpd)
{
	pair<bool, bool> ret = { false,false };

	return ret;

	auto acft = fpd->GetAircraftWtc();
	if (acft == 'H' || acft == 'J')
	{
		ret.first = true;
		ret.second = true;
		return ret;
	}
	//	string cs = fpd->Ca
}

inline void AircraftExtendedData::__computeWarnings()
{
	ActualFormularData.HighGrad = false;
	if (Route.RouteDistanceToDest <= 3)	return;
	ActualFormularData.HighGrad = (calcReqGrad(ActualFlightData.CurrentAltitude - MEAN_GROUND_LEVEL, Route.RouteDistanceToDest) > 4) && (ActualFlightData.GroundSpeed > 50);
}

bool AircraftExtendedData::RadarTargetFullCorrect(CRadarTarget rt)
{
	if (!rt.IsValid()) return false;
	auto fp = rt.GetCorrelatedFlightPlan();
	if (!fp.IsValid()) return false;
	if (!rt.GetPosition().IsValid()) return false;

	return true;
}

bool AircraftExtendedData::FPFullCorrect(CFlightPlan fp)
{
	if (!fp.IsValid()) return false;
	auto rt = fp.GetCorrelatedRadarTarget();
	if (!rt.IsValid()) return false;
	if (!rt.GetPosition().IsValid()) return false;

	return true;
}

void AircraftExtendedData::__calcTTL()
{
	ActualFormularData.TTL = -1;

	if (ActualFormularData.ClearedAltitude != 1) return;

	if (Route.DirectDistanceToDest > 20) return;

	if (ActualFlightData.CustomData.DestinationRwy.length() < 2) return;

	int rwy_hdg = atoi(ActualFlightData.CustomData.DestinationRwy.c_str()) * 10;	// even if 24L  -> 24

	int trk = ActualFlightData.Track;

	if (!heading_in_range(trk, rwy_hdg, 30)) return;

	int GS = ActualFlightData.GroundSpeed;

	for (auto& P : Rwys)
	{
		if (P.Port == Route.DestinationICAO && P.RWY == ActualFlightData.CustomData.DestinationRwy) {

			CPosition THR = CPosition();
			THR.m_Latitude = P.LatLon.x;
			THR.m_Longitude = P.LatLon.y;

			auto d_hdg = Route.PresentPosition.DirectionTo(THR);

			if (!heading_in_range(d_hdg, rwy_hdg, 20)) return;

			auto destanceToTHR = Route.PresentPosition.DistanceTo(THR);
			ActualFormularData.TTL = (int)round(destanceToTHR / GS * 3600);
		}
	}
}

inline void AircraftExtendedData::__load_auto_status()
{
	auto GS = ActualFlightData.GroundSpeed;
	auto ALT = ActualFlightData.CurrentAltitude;
	auto VS = ActualFlightData.VerticalSpeed;
	auto FIN = ActualFormularData.FinalAltitude;

	if (GS == 0) {
		ActualFormularData.AutoStatus = ACFT_STATUS_GROUND;
	}
	else if (GS <= 5)
	{
		ActualFormularData.AutoStatus = ACFT_STATUS_MOV;
	}
	else if (GS <= 30) {
		ActualFormularData.AutoStatus = ACFT_STATUS_TAX;
	}
	else if (GS > 30 && GS < 160 && ALT < 800 && (abs(VS) < 30)) {
		ActualFormularData.AutoStatus = ACFT_STATUS_ROLL;
	}
	else if (VS > 100) {
		ActualFormularData.AutoStatus = ACFT_STATUS_CLB;
	}
	else if (VS < -100)
	{
		ActualFormularData.AutoStatus = ACFT_STATUS_DES;
	}
	else if (GS > 100 && abs(VS) < 100 && ALT > 1000) {

		// ALT or CRZ
		if (abs(FIN - ALT) < 200)
		{
			ActualFormularData.AutoStatus = ACFT_STATUS_CRZ;
		}
		else {
			ActualFormularData.AutoStatus = ACFT_STATUS_ALT;
		}
	}
	ActualFlightData.IsAir = (ActualFormularData.AutoStatus == ACFT_STATUS_CLB || ActualFormularData.AutoStatus == ACFT_STATUS_ALT || ActualFormularData.AutoStatus == ACFT_STATUS_CRZ ||
		ActualFormularData.AutoStatus == ACFT_STATUS_DES || ActualFormularData.AutoStatus == ACFT_STATUS_HOLD);

	if (ActualFlightData.CustomData.CTL && AutoStsIsGround(ActualFormularData.AutoStatus))
	{
		ActualFlightData.CustomData.CTL = false;
	}
}

int AircraftExtendedData::__getTA()
{
	int TA = Plugin->GetTransitionAltitude();
	int useTA = 0;

	int LTA_Origin = TRA.count(Route.DepartureICAO) > 0 ? TRA.at(Route.DepartureICAO) : 0;
	int LTA_Dest = TRA.count(Route.DestinationICAO) > 0 ? TRA.at(Route.DestinationICAO) : 0;

	if (LTA_Origin > 0 && LTA_Dest > 0)
	{
		useTA = Route.DirectDistanceFromOrigin >= Route.DirectDistanceToDest ? LTA_Dest : LTA_Origin;
	}
	else if (LTA_Origin > 0)
	{
		useTA = LTA_Origin;
	}
	else if (LTA_Dest > 0)
	{
		useTA = LTA_Dest;
	}
	else useTA = TA;

	return useTA;
}

int AircraftExtendedData::__GetAvgGS()
{
	if (__GS_Snapshots.empty()) return 0;
	int ret = 0;
	for (auto it = __GS_Snapshots.begin(); it != __GS_Snapshots.end(); it++)
	{
		ret += *it;
	}
	return (int)(ret / __GS_Snapshots.size());
}

int AircraftExtendedData::__GetAvgVS()
{
	if (__VS_Snapshots.empty()) return 0;
	int ret = 0;
	for (auto it = __VS_Snapshots.begin(); it != __VS_Snapshots.end(); it++)
	{
		ret += *it;
	}
	return (int)roundNearestHundredth(ret / (int)__VS_Snapshots.size());
}

void AircraftExtendedData::__GS_Snapshot_Add(int V)
{
	if (__GS_Snapshots.size() == MAX_SHAPSHOT_LENGTH)
	{
		__GS_Snapshots.pop_back();
	}
	__GS_Snapshots.insert(__GS_Snapshots.begin(), V);
}

void AircraftExtendedData::__VS_Snapshot_Add(int G)
{
	if (__VS_Snapshots.size() == MAX_SHAPSHOT_LENGTH)
	{
		__VS_Snapshots.pop_back();
	}
	__VS_Snapshots.insert(__VS_Snapshots.begin(), G);
}

int AircraftExtendedData::__GetGSTrend()
{
	if (__GS_Snapshots.size() == 0)	return 0;
	return __GS_Snapshots.at(__GS_Snapshots.size() - 1) - __GS_Snapshots.at(0);
}

inline void AircraftExtendedData::__getFAFandCAPS()
{
	ActualFormularData.FAF = "";
	ActualFormularData.CAPS = "";

	for (auto& P : Rwys)
	{
		if (P.Port == Route.DestinationICAO) {
			ActualFormularData.FAF = P.FAF;
			ActualFormularData.CAPS = P.Caps;
			break;
		}
	}
}

void AircraftExtendedData::__set_init_climb_if_req()
{
	map<string, int> init_alts = {
		pair<string,int> { "3E", 7000},
		pair<string,int> { "3H", 7000},

		pair<string,int> { "3F", 4000},
		pair<string,int> { "3J", 4000},

		pair<string,int> { "3G", 4000},
		pair<string,int> { "3K", 4000},

		pair<string,int> { "3A", 7000},
		pair<string,int> { "3B", 7000},
		pair<string,int> { "3C", 7000},
		pair<string,int> { "3D", 7000},

		pair<string,int> { "3W", 5000},
		pair<string,int> { "3X", 5000},
		pair<string,int> { "3Y", 5000},
		pair<string,int> { "3Z", 5000},

		pair<string,int> { "3M", 5000},
		pair<string,int> { "3N", 5000},

		pair<string,int> { "3L", 4000},
		pair<string,int> { "3S", 4000},
		pair<string,int> { "3T", 4000},

		pair<string,int> { "3P", 4000},
		pair<string,int> { "3Q", 4000},
		pair<string,int> { "3R", 4000},

		pair<string,int> { "2Y", 2500},
		pair<string,int> { "2Z", 2500}
	};

	if (ActualControllerInfo.ActiveControllerId == Plugin->MyId && ActualFlightData.CustomData.Sid != "" && !ActualFormularData.HasClearance && !ActualFlightData.IsAir)
	{
		auto rt = Plugin->RadarTargetSelect(ActualFormularData.Callsign.c_str());
		if (!rt.IsValid())	return;

		string suffix = ActualFlightData.CustomData.Sid.substr(ActualFlightData.CustomData.Sid.length() - 2);
		if (init_alts.count(suffix) > 0)
		{
			rt.GetCorrelatedFlightPlan().GetControllerAssignedData().SetClearedAltitude(init_alts[suffix]);
		}

	}
}

MskListDisplay AircraftExtendedData::__list_delivery()
{
	MskListDisplay DISPLAY_IN;

	if (ActualFormularData.HasClearance)
	{
		DISPLAY_IN.Depa = false;
	}
	else
	{
	DISPLAY_IN.Depa = true;
	}
	if (Route.DepartureICAO == "" || Route.DepartureICAO != Plugin->MyPort)
	{
		DISPLAY_IN.Depa = DISPLAY_IN.Depa & false;
	}
	if (ActualPlanInfo.IsCurrent || ActualControllerInfo.IsTransferFromMe || ActualControllerInfo.IsTransferToMe)
	{
		DISPLAY_IN.Depa = DISPLAY_IN.Depa | true;
	}

	return DISPLAY_IN;
}

MskListDisplay AircraftExtendedData::__list_ground()
{
	MskListDisplay DISPLAY_IN;

	if (Route.DepartureICAO != "" && Route.DepartureICAO == Plugin->MyPort)
	{
		// GROUND DEPARTURES
		if (!ActualFlightData.IsAir)
		{
			DISPLAY_IN.Depa = true;
		}
		if (ActualPlanInfo.IsCurrent || ActualControllerInfo.IsTransferFromMe || ActualControllerInfo.IsTransferToMe)
		{
			DISPLAY_IN.Depa = true;
		}
	}
	if (Route.DestinationICAO != "" && Route.DestinationICAO == Plugin->MyPort)
	{
		// GROUND ARRIVALS
		if (!ActualFlightData.IsAir)
		{
			DISPLAY_IN.Land = true;
		}
		if (ActualControllerInfo.IHaveControlledIt && !ActualPlanInfo.IsCurrent)
		{
			DISPLAY_IN.Land = DISPLAY_IN.Land & false;
		}
		if (ActualPlanInfo.IsCurrent || ActualControllerInfo.IsTransferFromMe || ActualControllerInfo.IsTransferToMe)
		{
			DISPLAY_IN.Land = DISPLAY_IN.Land | true;
		}
	}
	return DISPLAY_IN;
}

MskListDisplay AircraftExtendedData::__list_tower()
{
	MskListDisplay DISPLAY_IN;

	if (Route.DepartureICAO != "" && Route.DepartureICAO == Plugin->MyPort)
	{
		// TOWER DEPARTURES
		if (!ActualFlightData.IsAir || (ActualFlightData.IsAir && (ActualPlanInfo.IsCurrent || ActualControllerInfo.IsTransferFromMe || ActualControllerInfo.IsTransferToMe)))
		{
			DISPLAY_IN.Depa = true;
		}
	}
	if (Route.DestinationICAO != "" && Route.DestinationICAO == Plugin->MyPort)
	{
		// TOWER ARRIVALS
		DISPLAY_IN.Land = true;

		if (ActualControllerInfo.IHaveControlledIt && !ActualPlanInfo.IsCurrent && ActualControllerInfo.IsTrackedByOther)
		{
			DISPLAY_IN.Land = DISPLAY_IN.Land & false;
		}
		if (ActualPlanInfo.IsCurrent || ActualControllerInfo.IsTransferFromMe || ActualControllerInfo.IsTransferToMe)
		{
			DISPLAY_IN.Land = DISPLAY_IN.Land | true;
		}
	}
	if (Route.DestinationICAO != "" && Route.DestinationICAO != Plugin->MyPort && Route.DepartureICAO != "" && Route.DepartureICAO != Plugin->MyPort)
	{
		// transit
		DISPLAY_IN.Trans = Plugin->Controllers->ImOwningAnyOfAreaNames(Plugin->MyCS, Route.CrossAreasNames, ActualFormularData.ClearedAltitude, ActualFormularData.ClearedAltitude);
	}
	return DISPLAY_IN;
}

MskListDisplay AircraftExtendedData::__list_radar()
{
	// Valid radar. UUEE, UUDD, UUWW, UWKD etc...
	MskListDisplay DISPLAY_IN;

	if (Route.DepartureICAO != "" && Route.DepartureICAO == Plugin->MyPort)
	{
		// RADAR DEPARTURES
		DISPLAY_IN.Depa = Plugin->Controllers->ImOwningAnyOfAreaNames(Plugin->MyCS, Route.CrossAreasNames, 0, 0);

		if (Plugin->Controllers->PortHasFacility(Route.DepartureICAO, ES_FACILITY_TWR))
		{
			if (ActualControllerInfo.OtherTrackerFacility == ES_FACILITY_TWR)
			{
				DISPLAY_IN.Depa = DISPLAY_IN.Depa & true;
			}
			else DISPLAY_IN.Depa = DISPLAY_IN.Depa & false;
		}
		else if (Plugin->Controllers->PortHasFacility(Route.DepartureICAO, ES_FACILITY_GND))
		{
			if (ActualControllerInfo.OtherTrackerFacility == ES_FACILITY_GND)
			{
				DISPLAY_IN.Depa = DISPLAY_IN.Depa & true;
			}
			else DISPLAY_IN.Depa = DISPLAY_IN.Depa & false;
		}
		else if (Plugin->Controllers->PortHasFacility(Route.DepartureICAO, ES_FACILITY_DEL))
		{
			if (ActualFormularData.HasClearance)
			{
				DISPLAY_IN.Depa = DISPLAY_IN.Depa & true;
			}
			else DISPLAY_IN.Depa = DISPLAY_IN.Depa & false;
		}

		if (ActualControllerInfo.IHaveControlledIt && !ActualPlanInfo.IsCurrent && ActualControllerInfo.IsTrackedByOther)
		{
			DISPLAY_IN.Depa = DISPLAY_IN.Depa & false;
		}
		if (ActualPlanInfo.IsCurrent || ActualControllerInfo.IsTransferFromMe || ActualControllerInfo.IsTransferToMe)
		{
			DISPLAY_IN.Depa = DISPLAY_IN.Depa | true;
		}
	}
	if (Route.DestinationICAO != "" && Route.DestinationICAO == Plugin->MyPort)
	{
		// RADAR ARRIVALS
		DISPLAY_IN.Land = Plugin->Controllers->ImOwningAnyOfAreaNames(Plugin->MyCS, Route.CrossAreasNames, 0, 0);

		if (ActualControllerInfo.IHaveControlledIt && !ActualPlanInfo.IsCurrent && ActualControllerInfo.IsTrackedByOther)
		{
			DISPLAY_IN.Land = DISPLAY_IN.Land & false;
		}
		if (ActualPlanInfo.IsCurrent || ActualControllerInfo.IsTransferFromMe || ActualControllerInfo.IsTransferToMe)
		{
			DISPLAY_IN.Land = DISPLAY_IN.Land | true;
		}
	}
	if (Route.DestinationICAO != "" && Route.DestinationICAO != Plugin->MyPort && Route.DepartureICAO != "" && Route.DepartureICAO != Plugin->MyPort)
	{
		// transit
		DISPLAY_IN.Trans = Plugin->Controllers->ImOwningAnyOfAreaNames(Plugin->MyCS, Route.CrossAreasNames, ActualFlightData.CurrentAltitude, ActualFormularData.ClearedAltitude);
		if (ActualPlanInfo.IsCurrent || ActualControllerInfo.IsTransferFromMe || ActualControllerInfo.IsTransferToMe)
		{
			DISPLAY_IN.Trans = DISPLAY_IN.Trans | true;
		}
	}
	return DISPLAY_IN;
}

MskListDisplay AircraftExtendedData::__list_msk_app()
{
	// MSK_APP
	MskListDisplay DISPLAY_IN;

	auto land_or_depa = false;
	if (find(begin(MUDR_PORTS), end(MUDR_PORTS), Route.DepartureICAO) != end(MUDR_PORTS))
	{
		// Departure
		DISPLAY_IN.Depa = Plugin->Controllers->ImOwningAnyOfAreaNames(Plugin->MyCS, Route.CrossAreasNames, 0, 0);
		land_or_depa = true;

		if (Plugin->Controllers->PortHasFacility(Route.DepartureICAO, ES_FACILITY_APPDEP))
		{
			if (ActualControllerInfo.OtherTrackerFacility == ES_FACILITY_APPDEP)
			{
				DISPLAY_IN.Depa = DISPLAY_IN.Depa & true;
			}
			else DISPLAY_IN.Depa = DISPLAY_IN.Depa & false;
		}
		else if (Plugin->Controllers->PortHasFacility(Route.DepartureICAO, ES_FACILITY_TWR))
		{
			if (ActualControllerInfo.OtherTrackerFacility == ES_FACILITY_TWR)
			{
				DISPLAY_IN.Depa = DISPLAY_IN.Depa & true;
			}
			else DISPLAY_IN.Depa = DISPLAY_IN.Depa & false;
		}
		else if (Plugin->Controllers->PortHasFacility(Route.DepartureICAO, ES_FACILITY_GND))
		{
			if (ActualControllerInfo.OtherTrackerFacility == ES_FACILITY_GND)
			{
				DISPLAY_IN.Depa = DISPLAY_IN.Depa & true;
			}
			else DISPLAY_IN.Depa = DISPLAY_IN.Depa & false;
		}
		else if (Plugin->Controllers->PortHasFacility(Route.DepartureICAO, ES_FACILITY_DEL))
		{
			if (ActualFormularData.HasClearance)
			{
				DISPLAY_IN.Depa = DISPLAY_IN.Depa & true;
			}
			else DISPLAY_IN.Depa = DISPLAY_IN.Depa & false;
		}
		if (ActualControllerInfo.IHaveControlledIt && !ActualPlanInfo.IsCurrent && ActualControllerInfo.IsTrackedByOther)
		{
			DISPLAY_IN.Depa = DISPLAY_IN.Depa & false;
		}
		if (ActualPlanInfo.IsCurrent || ActualControllerInfo.IsTransferFromMe || ActualControllerInfo.IsTransferToMe)
		{
			DISPLAY_IN.Depa = DISPLAY_IN.Depa | true;
		}
	}
	if (find(begin(MUDR_PORTS), end(MUDR_PORTS), Route.DestinationICAO) != end(MUDR_PORTS))
	{
		// Landings
		DISPLAY_IN.Land = Plugin->Controllers->ImOwningAnyOfAreaNames(Plugin->MyCS, Route.CrossAreasNames, 0, 0);
		land_or_depa = true;

		if (ActualControllerInfo.IHaveControlledIt && !ActualPlanInfo.IsCurrent && ActualControllerInfo.IsTrackedByOther)
		{
			DISPLAY_IN.Land = DISPLAY_IN.Land & false;
		}
		if (ActualPlanInfo.IsCurrent || ActualControllerInfo.IsTransferFromMe || ActualControllerInfo.IsTransferToMe)
		{
			DISPLAY_IN.Land = DISPLAY_IN.Land | true;
		}
	}
	if (!land_or_depa)
	{
		DISPLAY_IN.Trans = Plugin->Controllers->ImOwningAnyOfAreaNames(Plugin->MyCS, Route.CrossAreasNames, ActualFlightData.CurrentAltitude, ActualFormularData.ClearedAltitude);
		if (ActualControllerInfo.IHaveControlledIt && !ActualPlanInfo.IsCurrent && ActualControllerInfo.IsTrackedByOther)
		{
			DISPLAY_IN.Trans = DISPLAY_IN.Trans & false;
		}
		if (ActualPlanInfo.IsCurrent || ActualControllerInfo.IsTransferFromMe || ActualControllerInfo.IsTransferToMe)
		{
			DISPLAY_IN.Trans = DISPLAY_IN.Trans | true;
		}
	}
	return DISPLAY_IN;
}

MskListDisplay AircraftExtendedData::__list_uuwv_ctr()
{
	// UUWV_CTR
	MskListDisplay DISPLAY_IN;

	auto land_or_depa = false;
	if (find(begin(MOSCOW_PORTS), end(MOSCOW_PORTS), Route.DepartureICAO) != end(MOSCOW_PORTS))
	{
		// Departure
		DISPLAY_IN.Depa = Plugin->Controllers->ImOwningAnyOfAreaNames(Plugin->MyCS, Route.CrossAreasNames, 0, 0);
		land_or_depa = true;

		if (Plugin->Controllers->PortHasFacility(Route.DepartureICAO, ES_FACILITY_APPDEP))
		{
			if (ActualControllerInfo.OtherTrackerFacility == ES_FACILITY_APPDEP)
			{
				DISPLAY_IN.Depa = DISPLAY_IN.Depa & true;
			}
			else DISPLAY_IN.Depa = DISPLAY_IN.Depa & false;
		}
		else if (Plugin->Controllers->PortHasFacility(Route.DepartureICAO, ES_FACILITY_TWR))
		{
			if (ActualControllerInfo.OtherTrackerFacility == ES_FACILITY_TWR)
			{
				DISPLAY_IN.Depa = DISPLAY_IN.Depa & true;
			}
			else DISPLAY_IN.Depa = DISPLAY_IN.Depa & false;
		}
		else if (Plugin->Controllers->PortHasFacility(Route.DepartureICAO, ES_FACILITY_GND))
		{
			if (ActualControllerInfo.OtherTrackerFacility == ES_FACILITY_GND)
			{
				DISPLAY_IN.Depa = DISPLAY_IN.Depa & true;
			}
			else DISPLAY_IN.Depa = DISPLAY_IN.Depa & false;
		}
		else if (Plugin->Controllers->PortHasFacility(Route.DepartureICAO, ES_FACILITY_DEL))
		{
			if (ActualFormularData.HasClearance)
			{
				DISPLAY_IN.Depa = DISPLAY_IN.Depa & true;
			}
			else DISPLAY_IN.Depa = DISPLAY_IN.Depa & false;
		}
		if (ActualControllerInfo.IHaveControlledIt && !ActualPlanInfo.IsCurrent && ActualControllerInfo.IsTrackedByOther)
		{
			DISPLAY_IN.Depa = DISPLAY_IN.Depa & false;
		}
		if (ActualPlanInfo.IsCurrent || ActualControllerInfo.IsTransferFromMe || ActualControllerInfo.IsTransferToMe)
		{
			DISPLAY_IN.Depa = DISPLAY_IN.Depa | true;
		}
	}
	if (find(begin(MOSCOW_PORTS), end(MOSCOW_PORTS), Route.DestinationICAO) != end(MOSCOW_PORTS))
	{
		// Landings
		DISPLAY_IN.Land = Plugin->Controllers->ImOwningAnyOfAreaNames(Plugin->MyCS, Route.CrossAreasNames, 0, 0);
		land_or_depa = true;

		if (ActualControllerInfo.IHaveControlledIt && !ActualPlanInfo.IsCurrent && ActualControllerInfo.IsTrackedByOther)
		{
			DISPLAY_IN.Land = DISPLAY_IN.Land & false;
		}
		if (ActualPlanInfo.IsCurrent || ActualControllerInfo.IsTransferFromMe || ActualControllerInfo.IsTransferToMe)
		{
			DISPLAY_IN.Land = DISPLAY_IN.Land | true;
		}
	}
	if (!land_or_depa)
	{
		DISPLAY_IN.Trans = Plugin->Controllers->ImOwningAnyOfAreaNames(Plugin->MyCS, Route.CrossAreasNames, ActualFlightData.CurrentAltitude, ActualFormularData.ClearedAltitude);
		if (ActualControllerInfo.IHaveControlledIt && !ActualPlanInfo.IsCurrent && ActualControllerInfo.IsTrackedByOther)
		{
			DISPLAY_IN.Trans = DISPLAY_IN.Trans & false;
		}
		if (ActualPlanInfo.IsCurrent || ActualControllerInfo.IsTransferFromMe || ActualControllerInfo.IsTransferToMe)
		{
			DISPLAY_IN.Trans = DISPLAY_IN.Trans | true;
		}
	}
	return DISPLAY_IN;
}

MskListDisplay AircraftExtendedData::__list_other_ctr()
{
	// Other CTR
	MskListDisplay DISPLAY_IN;

	auto land_or_depa = false;
	if (Route.DepaInMyZone)
	{
		// Departure
		DISPLAY_IN.Depa = Plugin->Controllers->ImOwningAnyOfAreaNames(Plugin->MyCS, Route.CrossAreasNames, 0, 0);
		land_or_depa = true;

		if (Plugin->Controllers->PortHasFacility(Route.DepartureICAO, ES_FACILITY_APPDEP))
		{
			if (ActualControllerInfo.OtherTrackerFacility == ES_FACILITY_APPDEP)
			{
				DISPLAY_IN.Depa = DISPLAY_IN.Depa & true;
			}
			else DISPLAY_IN.Depa = DISPLAY_IN.Depa & false;
		}
		else if (Plugin->Controllers->PortHasFacility(Route.DepartureICAO, ES_FACILITY_TWR))
		{
			if (ActualControllerInfo.OtherTrackerFacility == ES_FACILITY_TWR)
			{
				DISPLAY_IN.Depa = DISPLAY_IN.Depa & true;
			}
			else DISPLAY_IN.Depa = DISPLAY_IN.Depa & false;
		}
		else if (Plugin->Controllers->PortHasFacility(Route.DepartureICAO, ES_FACILITY_GND))
		{
			if (ActualControllerInfo.OtherTrackerFacility == ES_FACILITY_GND)
			{
				DISPLAY_IN.Depa = DISPLAY_IN.Depa & true;
			}
			else DISPLAY_IN.Depa = DISPLAY_IN.Depa & false;
		}
		else if (Plugin->Controllers->PortHasFacility(Route.DepartureICAO, ES_FACILITY_DEL))
		{
			if (ActualFormularData.HasClearance)
			{
				DISPLAY_IN.Depa = DISPLAY_IN.Depa & true;
			}
			else DISPLAY_IN.Depa = DISPLAY_IN.Depa & false;
		}
		if (ActualControllerInfo.IHaveControlledIt && !ActualPlanInfo.IsCurrent && ActualControllerInfo.IsTrackedByOther)
		{
			DISPLAY_IN.Depa = DISPLAY_IN.Depa & false;
		}
		if (ActualPlanInfo.IsCurrent || ActualControllerInfo.IsTransferFromMe || ActualControllerInfo.IsTransferToMe)
		{
			DISPLAY_IN.Depa = DISPLAY_IN.Depa | true;
		}
	}
	if (Route.LandInMyZone)
	{
		// Landings
		DISPLAY_IN.Land = Plugin->Controllers->ImOwningAnyOfAreaNames(Plugin->MyCS, Route.CrossAreasNames, 0, 0);
		land_or_depa = true;

		if (ActualControllerInfo.IHaveControlledIt && !ActualPlanInfo.IsCurrent && ActualControllerInfo.IsTrackedByOther)
		{
			DISPLAY_IN.Land = DISPLAY_IN.Land & false;
		}
		if (ActualPlanInfo.IsCurrent || ActualControllerInfo.IsTransferFromMe || ActualControllerInfo.IsTransferToMe)
		{
			DISPLAY_IN.Land = DISPLAY_IN.Land | true;
		}
	}
	if (!land_or_depa)
	{
		DISPLAY_IN.Trans = Plugin->Controllers->ImOwningAnyOfAreaNames(Plugin->MyCS, Route.CrossAreasNames, ActualFlightData.CurrentAltitude, ActualFormularData.ClearedAltitude);
		if (ActualControllerInfo.IHaveControlledIt && !ActualPlanInfo.IsCurrent && ActualControllerInfo.IsTrackedByOther)
		{
			DISPLAY_IN.Trans = DISPLAY_IN.Trans & false;
		}
		if (ActualPlanInfo.IsCurrent || ActualControllerInfo.IsTransferFromMe || ActualControllerInfo.IsTransferToMe)
		{
			DISPLAY_IN.Trans = DISPLAY_IN.Trans | true;
		}
	}
	return DISPLAY_IN;
}

MskListDisplay AircraftExtendedData::__get_req_display_list()
{
	if (Plugin->MyFacility == ES_FACILITY_DEL)
	{
		return __list_delivery();
	}
	else if (Plugin->MyFacility == ES_FACILITY_GND)
	{
		return __list_ground();
	}
	else if (Plugin->MyFacility == ES_FACILITY_TWR)
	{
		return __list_tower();
	}
	else if (Plugin->MyFacility == ES_FACILITY_APPDEP)
	{
		if (Plugin->MyPort != "")
		{
			return __list_radar();
		}
		else {
			// Moscow
			return __list_msk_app();
		}
	}
	else if (Plugin->MyFacility == ES_FACILITY_CTR)
	{
		if (safe_substr(Plugin->MyCS, 0, 4) == "UUWV")
		{
			return __list_uuwv_ctr();
		}
		else {
			return __list_other_ctr();
		}
	}
	else return MskListDisplay();
}

void AircraftExtendedData::__checkForAssigments(CRadarTarget* rt)
{
	if (!PreviousFlightData.IsValid())	return;

	if (ActualFlightData.DataIsFirst)
	{
		Plugin->Ccdlc->AddCCDLCSystemMessageToQueue("ACFT:NEW:" + ActualFormularData.Callsign);
	}

	if (Plugin->MyId == "")	return;

	if ((PreviousControllerInfo.ActiveControllerId != ActualControllerInfo.ActiveControllerId) && (ActualPlanInfo.PlanState == FLIGHT_PLAN_STATE_ASSUMED))
	{
		Plugin->Ccdlc->AddCCDLCSystemMessageToQueue("GET:" + Plugin->MyId + ":" + ActualFormularData.Callsign + ":" + (ActualControllerInfo.ActiveControllerId != "" ? ActualControllerInfo.ActiveControllerId : "00"));
		Plugin->Planes->sortAcftList();
		__set_init_climb_if_req();
	}
	else if ((PreviousControllerInfo.ActiveControllerId != ActualControllerInfo.ActiveControllerId) && (PreviousControllerInfo.ActiveControllerId == Plugin->MyId) && (ActualPlanInfo.PlanState != FLIGHT_PLAN_STATE_ASSUMED))
	{
		Plugin->Ccdlc->AddCCDLCSystemMessageToQueue("RLZ:" + Plugin->MyId + ":" + ActualFormularData.Callsign);
		Plugin->Planes->sortAcftList();
	}

	if (ActualFormularData.AssignedHeading != PreviousFormularData.AssignedHeading)
	{
		Plugin->Ccdlc->AddCCDLCSystemMessageToQueueIfTracking(ActualControllerInfo.ActiveControllerId, "HDG:" + Plugin->MyId + ":" + ActualFormularData.Callsign + ":" + to_string(ActualFormularData.AssignedHeading));
	}
	if (ActualFormularData.AssignedRate != PreviousFormularData.AssignedRate)
	{
		Plugin->Ccdlc->AddCCDLCSystemMessageToQueueIfTracking(ActualControllerInfo.ActiveControllerId, "CRT:" + Plugin->MyId + ":" + ActualFormularData.Callsign + ":" + to_string(ActualFormularData.AssignedRate));
	}
	if (ActualFormularData.ClearedAltitude != PreviousFormularData.ClearedAltitude)
	{
		Plugin->Ccdlc->AddCCDLCSystemMessageToQueueIfTracking(ActualControllerInfo.ActiveControllerId, "CFL:" + Plugin->MyId + ":" + ActualFormularData.Callsign + ":" + to_string(ActualFormularData.ClearedAltitude));
	}
	if (ActualFormularData.FinalAltitude != PreviousFormularData.FinalAltitude)
	{
		Plugin->Ccdlc->AddCCDLCSystemMessageToQueueIfTracking(ActualControllerInfo.ActiveControllerId, "FFL:" + Plugin->MyId + ":" + ActualFormularData.Callsign + ":" + to_string(ActualFormularData.FinalAltitude));
	}
	if (ActualFormularData.HasClearance != PreviousFormularData.HasClearance)
	{
		Plugin->Ccdlc->AddCCDLCSystemMessageToQueueIfTracking(ActualControllerInfo.ActiveControllerId, "XLR:" + Plugin->MyId + ":" + ActualFormularData.Callsign + ":" + (ActualFormularData.HasClearance ? "1" : "0"));
	}
	if (ActualFormularData.AssignedMach != PreviousFormularData.AssignedMach)
	{
		Plugin->Ccdlc->AddCCDLCSystemMessageToQueueIfTracking(ActualControllerInfo.ActiveControllerId, "MCH:" + Plugin->MyId + ":" + ActualFormularData.Callsign + ":" + to_string(ActualFormularData.AssignedMach));
	}
	if (ActualFormularData.AssignedSpeed != PreviousFormularData.AssignedSpeed)
	{
		Plugin->Ccdlc->AddCCDLCSystemMessageToQueueIfTracking(ActualControllerInfo.ActiveControllerId, "ASP:" + Plugin->MyId + ":" + ActualFormularData.Callsign + ":" + to_string(ActualFormularData.AssignedSpeed));
	}
	if (ActualFlightData.CustomData.DepartureRwy != PreviousFlightData.CustomData.DepartureRwy)
	{
		Plugin->Ccdlc->AddCCDLCSystemMessageToQueueIfTracking(ActualControllerInfo.ActiveControllerId, "PRW:" + Plugin->MyId + ":" + ActualFormularData.Callsign + ":" + (ActualFlightData.CustomData.DepartureRwy != "" ? ActualFlightData.CustomData.DepartureRwy : "--"));
		Route.__compute(rt, &ActualFormularData, &ActualFlightData, true, false);
	}
	if (ActualFlightData.CustomData.DestinationRwy != PreviousFlightData.CustomData.DestinationRwy)
	{
		Plugin->Ccdlc->AddCCDLCSystemMessageToQueueIfTracking(ActualControllerInfo.ActiveControllerId, "LRW:" + Plugin->MyId + ":" + ActualFormularData.Callsign + ":" + (ActualFlightData.CustomData.DestinationRwy != "" ? ActualFlightData.CustomData.DestinationRwy : "--"));
		Route.__compute(rt, &ActualFormularData, &ActualFlightData, true, false);
	}
	if (ActualFlightData.CustomData.Sid != PreviousFlightData.CustomData.Sid)
	{
		Plugin->Ccdlc->AddCCDLCSystemMessageToQueueIfTracking(ActualControllerInfo.ActiveControllerId, "SID:" + Plugin->MyId + ":" + ActualFormularData.Callsign + ":" + (ActualFlightData.CustomData.Sid != "" ? ActualFlightData.CustomData.Sid : "--"));
		Route.__compute(rt, &ActualFormularData, &ActualFlightData, true, false);
		__set_init_climb_if_req();
	}
	if (ActualFlightData.CustomData.Star != PreviousFlightData.CustomData.Star)
	{
		Plugin->Ccdlc->AddCCDLCSystemMessageToQueueIfTracking(ActualControllerInfo.ActiveControllerId, "STAR:" + Plugin->MyId + ":" + ActualFormularData.Callsign + ":" + (ActualFlightData.CustomData.Star != "" ? ActualFlightData.CustomData.Star : "--"));
		Route.__compute(rt, &ActualFormularData, &ActualFlightData, true, false);
	}
	if (ActualFormularData.CommType != PreviousFormularData.CommType)
	{
		Plugin->Ccdlc->AddCCDLCSystemMessageToQueueIfTracking(ActualControllerInfo.ActiveControllerId, "CMM:" + Plugin->MyId + ":" + ActualFormularData.Callsign + ":" + ActualFormularData.CommType);
	}
	if (ActualFlightData.CustomData.DepartureICAO != PreviousFlightData.CustomData.DepartureICAO || ActualFlightData.CustomData.DestinationICAO != PreviousFlightData.CustomData.DestinationICAO)
	{
		Plugin->Ccdlc->AddCCDLCSystemMessageToQueueIfTracking(ActualControllerInfo.ActiveControllerId, "MRT:" + Plugin->MyId + ":" + ActualFormularData.Callsign + ":RST");
		Route.Clear();
		Route.__compute(rt, &ActualFormularData, &ActualFlightData, true, true);
	}
	if (ActualControllerInfo.IsTransferFromMe && !PreviousControllerInfo.IsTransferToMe)
	{
		Plugin->History->HandOff_In++;
	}
	if (ActualControllerInfo.IsTransferFromMe && !PreviousControllerInfo.IsTransferFromMe)
	{
		Plugin->History->HandOff_Out++;
	}
}

inline void AircraftExtendedData::__request_cert_if_needed()
{
	if (ActualFormularData.ExtendedData.Name == "" && time_diff_now(CreateTime) > 70)
	{
		auto external_info = Plugin->ExternalData->GetCallsignInfo(ActualFormularData.Callsign);
		if (external_info.Fetched)
		{
			ActualFormularData.ExtendedData.ATCRating = external_info.ATCRating;
			ActualFormularData.ExtendedData.Hours = external_info.PilotHours;
			ActualFormularData.ExtendedData.Name = external_info.FullName;
			ActualFormularData.ExtendedData.Newbie = external_info.PilotHours > 0 && external_info.PilotHours < 100;
			ActualFormularData.ExtendedData.PilotRating = external_info.PilotRating;

			if (external_info.FullName == "")
			{
				ActualFormularData.ExtendedData.Name = "UNRESOLVED";
			}
			Plugin->ExternalData->CancelCallsignRequest(ActualFormularData.Callsign);
		}
		else {
			Plugin->ExternalData->RequestCallsignInfo(ActualFormularData.Callsign);
		}
	}

}

AircraftExtendedData* AircraftExtendedDataPool::operator [] (const string& cs)
{
	if (lastPlaneCS == cs)
	{
		return lastPlane;
	}
	auto p = &__planes[cs];
	lastPlane = p;
	lastPlaneCS = cs;
	return p;
}

void AircraftExtendedDataPool::lock()
{
	__pool_lock.lock();
}

string AircraftExtendedDataPool::GetComputedPlaneCS()
{
	lock_guard<mutex> lock(__computation_completed);
	if (__computed_planes.size() > 0)
	{
		string cs = __computed_planes.front();
		__computed_planes.pop();
		return cs;
	}
	return "";
}

void AircraftExtendedDataPool::__add_computed_plane(string cs)
{
	lock_guard<mutex> lock(__computation_completed);
	__computed_planes.push(cs);
}

AircraftExtendedDataPool::AircraftExtendedDataPool()
{
	thread COMPUTE(&AircraftExtendedDataPool::__ExtendedDataComputation, this);
	Plugin->ThreadRunning++;
	COMPUTE.detach();
}

void AircraftExtendedDataPool::__ExtendedDataComputation()
{
	while (!Plugin->Terminating)
	{
		this_thread::sleep_for(5ms);

		ComputeTask cs = Plugin->ToCompute->get();
		if (cs.Callsign == "")
		{
			continue;
		}

		auto rt = Plugin->RadarTargetSelect(cs.Callsign.c_str());
		if (!rt.IsValid()) continue;

		auto fp = rt.GetCorrelatedFlightPlan();

		if (!AircraftExtendedData::FPFullCorrect(fp)) {
			continue;
		}
		if (fp.GetSimulated())	continue;

		{
			ExtendedDataRequest req;
			auto plane = req.request(cs.Callsign);

			try {
				plane->Compute(&rt, &cs);
			}
			catch (...)
			{
				Plugin->DisplayUrgent("Faild to compute CS: " + cs.Callsign);
			}

			if (find(Plugin->Planes->SortedAcftList.begin(), Plugin->Planes->SortedAcftList.end(), plane) == Plugin->Planes->SortedAcftList.end())
			{
				Plugin->Planes->SortedAcftList.push_back(plane);
			}
			Plugin->Planes->sortAcftList();
			__add_computed_plane(cs.Callsign);
		}
	}
	Plugin->ThreadRunning--;
}

void AircraftExtendedDataPool::sortAcftList()
{
	try {
		std::sort(begin(SortedAcftList), end(SortedAcftList), [](AircraftExtendedData* const& a, AircraftExtendedData* const& b) -> bool {

			if (a->ActualPlanInfo.IsCurrent && b->ActualPlanInfo.IsCurrent)
			{
				return a->ActualFormularData.Callsign < b->ActualFormularData.Callsign;
			}
			else if (a->ActualPlanInfo.IsCurrent && !b->ActualPlanInfo.IsCurrent)
			{
				return true;
			}
			else if (!a->ActualPlanInfo.IsCurrent && b->ActualPlanInfo.IsCurrent)
			{
				return false;
			}
			else if (!a->ActualPlanInfo.IsCurrent && !b->ActualPlanInfo.IsCurrent)
			{
				return a->ActualFormularData.Callsign < b->ActualFormularData.Callsign;
			}
			else return false;
			});

		int i = 0;
		for (auto& P : SortedAcftList)
		{
			P->ActualFormularData.SortIndex = to_string(i);
			padTo(P->ActualFormularData.SortIndex, 2, '0');
			i++;
		}
	}
	catch (...)
	{
		Plugin->DisplayUrgent("Exception raised in sortAcftList");
	}
}

void AircraftExtendedDataPool::unlock()
{
	__pool_lock.unlock();
}

bool AircraftExtendedDataPool::alive(string cs)
{
	return __planes.count(cs) > 0;
}

void AircraftExtendedDataPool::remove(string CS)
{
	if (alive(CS))
	{
		lastPlane = NULL;
		lastPlaneCS = "";
		__planes.erase(CS);
	}
}

ExtendedDataRequest::ExtendedDataRequest()
{
	Plugin->Planes->lock();
}

ExtendedDataRequest::~ExtendedDataRequest()
{
	if (!__released)
		Plugin->Planes->unlock();
}

void ExtendedDataRequest::release()
{
	Plugin->Planes->unlock();
	__released = true;
}

AircraftExtendedData* ExtendedDataRequest::request(string callsign)
{
	return (*Plugin->Planes)[callsign];
}
bool ExtendedDataRequest::check(string callsign)
{
	return Plugin->Planes->alive(callsign);
}
void ExtendedDataRequest::erase(string callsign)
{
	Plugin->Planes->remove(callsign);
}
AircraftExtendedData* ExtendedDataRequest::check_req(string callsign)
{
	if (check(callsign))
		return request(callsign);
	return NULL;
}