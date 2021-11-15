#include "pch.h"
#include "TagProcessor.h"
#include "AircraftExtendedData.h"
#include "CMskPlugin.h"
#include "CMskRadarScreen.h"
#include "Global.h"

TagProcessor::TagProcessor()
{
	// Initialize processor table
	AddTagProcessor(TAG_MSK_CONSTRAINT, false, false, "Current constraint (form)", &TagProcessor::ProcessConstraint);
	AddTagProcessor(TAG_MSK_DEPA_PORT, false, false, "Departure ICAO (form)", &TagProcessor::ProcessDeparturePort);
	AddTagProcessor(TAG_MSK_DEPA_PORT_TX, true, false, "Departure ICAO (list)", &TagProcessor::ProcessDeparturePort);
	AddTagProcessor(TAG_MSK_DEST_PORT, false, false, "Destination ICAO (form)", &TagProcessor::ProcessDestinationPort);
	AddTagProcessor(TAG_MSK_DEST_PORT_TX, true, false, "Destination ICAO (list)", &TagProcessor::ProcessDestinationPort);
	AddTagProcessor(TAG_MSK_STAR, false, false, "STAR (form)", &TagProcessor::ProcessSTAR);
	AddTagProcessor(TAG_MSK_STAR_TX, true, false, "STAR (list)", &TagProcessor::ProcessSTAR);
	AddTagProcessor(TAG_MSK_SID, false, false, "SID (form)", &TagProcessor::ProcessSID);
	AddTagProcessor(TAG_MSK_SID_TX, true, false, "SID (list)", &TagProcessor::ProcessSID);
	AddTagProcessor(TAG_MSK_XMT, true, false, "TX", &TagProcessor::ProcessLastXmt);
	AddTagProcessor(TAG_MSK_CURRENT_AREA, true, true, "Current Area (list)", &TagProcessor::ProcessArea);
	AddTagProcessor(TAG_MSK_CURRENT_STS, true, true, "Status Auto (list)", &TagProcessor::ProcessSts);
	AddTagProcessor(TAG_MSK_MANUAL_STS, true, true, "Status Manual (list)", &TagProcessor::ProcessManualStatus);
	AddTagProcessor(TAG_MSK_FINAL_FIX_ALT, true, false, "FAF alt (list)", &TagProcessor::ProcessFinalFixAlt);
	AddTagProcessor(TAG_MSK_TIME_TO_LAND, true, false, "TTL (list)", &TagProcessor::ProcessTTL);
	AddTagProcessor(TAG_MSK_AMAN_POINT, true, true, "AMAN Point (list)", &TagProcessor::ProcessAman);
	AddTagProcessor(TAG_MSK_AMAN_POINT_TIME, true, true, "AMAN Point Time (list)", &TagProcessor::ProcessAman);
	AddTagProcessor(TAG_MSK_AMAN_MILES, true, true, "AMAN Point Dist (list)", &TagProcessor::ProcessAman);
	AddTagProcessor(TAG_MSK_OWN_CS, false, false, "Callsign (form)", &TagProcessor::ProcessCallsign);
	AddTagProcessor(TAG_MSK_OWN_CS_TX, true, false, "Callsign (list)", &TagProcessor::ProcessCallsign);
	AddTagProcessor(TAG_MSK_DEPA_RWY, false, false, "Departure RWY (form)", &TagProcessor::ProcessRunway);
	AddTagProcessor(TAG_MSK_DEPA_RWY_TX, true, false, "Departure RWY (list)", &TagProcessor::ProcessRunway);
	AddTagProcessor(TAG_MSK_DEST_RWY, false, false, "Destination RWY (form)", &TagProcessor::ProcessRunway);
	AddTagProcessor(TAG_MSK_DEST_RWY_TX, true, false, "Destination RWY (list)", &TagProcessor::ProcessRunway);
	AddTagProcessor(TAG_MSK_OWN_AFL, false, false, "AFL (form)", &TagProcessor::ProcessFL);
	AddTagProcessor(TAG_MSK_OWN_AFL_TX, true, false, "AFL (list)", &TagProcessor::ProcessFL);
	AddTagProcessor(TAG_MSK_OWN_CFL, false, false, "CFL (form)", &TagProcessor::ProcessFL);
	AddTagProcessor(TAG_MSK_OWN_CFL_TX, true, false, "CRL (list)", &TagProcessor::ProcessFL);
	AddTagProcessor(TAG_MSK_OWN_FFL, false, false, "FFL (form)", &TagProcessor::ProcessFL);
	AddTagProcessor(TAG_MSK_OWN_FFL_TX, true, false, "FFL (list)", &TagProcessor::ProcessFL);
	AddTagProcessor(TAG_MSK_OWN_SPEED, false, false, "IAS/Mach (form)", &TagProcessor::ProcessSpeed);
	AddTagProcessor(TAG_MSK_OWN_SPEED_TX, true, true, "IAS/Mach (list)", &TagProcessor::ProcessSpeed);
	AddTagProcessor(TAG_MSK_OWN_GS, false, false, "GroundSpeed (form)", &TagProcessor::ProcessSpeed);
	AddTagProcessor(TAG_MSK_OWN_GS_TX, true, false, "GroundSpeed (list)", &TagProcessor::ProcessSpeed);
	AddTagProcessor(TAG_MSK_OWN_VS, false, false, "V/S (form)", &TagProcessor::ProcessSpeed);
	AddTagProcessor(TAG_MSK_OWN_VS_TX, true, false, "V/S (list)", &TagProcessor::ProcessSpeed);
	AddTagProcessor(TAG_MSK_LANG, false, false, "Language (form)", &TagProcessor::ProcessLang);
	AddTagProcessor(TAG_MSK_LANG_TX, true, false, "Language (list)", &TagProcessor::ProcessLang);
	AddTagProcessor(TAG_MSK_HIGH_GRAD, false, false, "High Grad (form)", &TagProcessor::ProcessHighGrad);
	AddTagProcessor(TAG_MSK_READY_TIME, true, false, "Ready time (list)", &TagProcessor::ProcessTimes);
	AddTagProcessor(TAG_MSK_OFFBLOCK_TIME, true, false, "Offblock time (list)", &TagProcessor::ProcessTimes);
	AddTagProcessor(TAG_MSK_REST_SPEED, false, false, "Restricted speed (form)", &TagProcessor::ProcessRestriction);
	AddTagProcessor(TAG_MSK_REST_ALT, false, false, "Restricted alt (form)", &TagProcessor::ProcessRestriction);
	AddTagProcessor(TAG_MSK_POSITION, false, false, "MSK List Position", &TagProcessor::ProcessAcftIndex);
	AddTagProcessor(TAG_MSK_RNAV, false, false, "RNAV indicator", &TagProcessor::ProcessExtendedData);
	AddTagProcessor(TAG_MSK_RVSM, false, false, "RVSM indicator", &TagProcessor::ProcessExtendedData);
	AddTagProcessor(TAG_MSK_NEWBIE, false, false, "Newbie indicator", &TagProcessor::ProcessExtendedData);
	AddTagProcessor(TAG_MSK_BADGUY, false, false, "Bad indicator", &TagProcessor::ProcessExtendedData);
	AddTagProcessor(TAG_MSK_ATC_RATING, false, false, "ATC rating indicator", &TagProcessor::ProcessExtendedData);
	AddTagProcessor(TAG_MSK_PILOT_RATING, false, false, "Pilot rating indicator", &TagProcessor::ProcessExtendedData);
	AddTagProcessor(TAG_MSK_PILOT_NAME, false, false, "Pilot name", &TagProcessor::ProcessExtendedData);
	AddTagProcessor(TAG_MSK_PILOT_FREETEXT1, false, false, "Freetext 1", &TagProcessor::ProcessExtendedData);
	AddTagProcessor(TAG_MSK_PILOT_FREETEXT2, false, false, "Freetext 2", &TagProcessor::ProcessExtendedData);
	AddTagProcessor(TAG_MSK_EXT_DATA_WARNING, false, false, "Extended data warning", &TagProcessor::ProcessExtendedData);
	AddTagProcessor(TAG_MSK_EXT_DATA_HOURS, false, false, "Pilot hours", &TagProcessor::ProcessExtendedData);
	AddTagProcessor(TAG_MSK_EXT_DATA_DEPA_ICAO, false, false, "Departure ICAO (extended)", &TagProcessor::ProcessExtendedData);
	AddTagProcessor(TAG_MSK_EXT_DATA_DEPA_RWY, false, false, "Departure RWY (extended)", &TagProcessor::ProcessExtendedData);
	AddTagProcessor(TAG_MSK_EXT_DATA_DEST_ICAO, false, false, "Destination ICAO (extended)", &TagProcessor::ProcessExtendedData);
	AddTagProcessor(TAG_MSK_EXT_DATA_DEST_RWY, false, false, "Destination RWY (extended)", &TagProcessor::ProcessExtendedData);
	AddTagProcessor(TAG_MSK_CCDLC_INITIATOR, false, false, "CCDLC Initiator", &TagProcessor::ProcessCCDLC);
	AddTagProcessor(TAG_MSK_CCDLC_REQUEST, false, false, "CCDLC Request", &TagProcessor::ProcessCCDLC);
	AddTagProcessor(TAG_MSK_CCDLC_REQUEST_EXT, false, false, "CCDLC Request2", &TagProcessor::ProcessCCDLC);
	AddTagProcessor(TAG_MSK_CCDLC_REACTIONS, false, false, "CCDLC Reactions", &TagProcessor::ProcessCCDLC);
	AddTagProcessor(TAG_MSK_CCDLC_FREETEXT, false, false, "CCDLC Freetext", &TagProcessor::ProcessCCDLC);
	AddTagProcessor(TAG_MSK_CCDLC_FREETEXT_EXT, false, false, "CCDLC Freetext2", &TagProcessor::ProcessCCDLC);
	AddTagProcessor(TAG_MSK_CCDLC_DISPLAY_DCT, false, false, "CCDLC DCT Button", &TagProcessor::ProcessCCDLC);
	AddTagProcessor(TAG_MSK_CCDLC_DISPLAY_ALT, false, false, "CCDLC ALT Button", &TagProcessor::ProcessCCDLC);
	AddTagProcessor(TAG_MSK_CCDLC_DISPLAY_SPD, false, false, "CCDLC SPD Button", &TagProcessor::ProcessCCDLC);
	AddTagProcessor(TAG_MSK_CCDLC_DISPLAY_TXT, false, false, "CCDLC TXT Button", &TagProcessor::ProcessCCDLC);
	AddTagProcessor(TAG_MSK_OWN_HDG, false, false, "Heading (form)", &TagProcessor::ProcessSpeed);

	// Functions
	AddFuncProcessor(FN_MSK_DISPLAY_ROUTE, "Display route", &TagProcessor::ProcessFN_DisplayRoute);
	AddFuncProcessor(FN_MSK_SHOW_INTERVAL, "Show interval", &TagProcessor::ProcessFN_ShowInterval);
	AddFuncProcessor(FN_MSK_AMAN_POINT, "Open AMAN point menu", &TagProcessor::ProcessFN_AmanPoint);
	AddFuncProcessor(FN_MSK_AMAN_POINT_SELECTED, "AMAN point selected", &TagProcessor::ProcessFN_AmanPointSelect);
	AddFuncProcessor(FN_MSK_TTL_CLICKED, "Open TTL menu", &TagProcessor::ProcessFN_TTL);
	AddFuncProcessor(FN_MSK_TTL_CLEARED_TO_LAND, "TTL menu selected", &TagProcessor::ProcessFN_TTL_Select);
	AddFuncProcessor(FN_MSK_MAN_STS_CLICKED, "Open Manual status menu", &TagProcessor::ProcessFN_ManualStatus);
	AddFuncProcessor(FN_MSK_MAN_STS_ASSIGNED, "Manual status selected", &TagProcessor::ProcessFN_ManualStatus_Selected);
	AddFuncProcessor(FN_MSK_CALLSIGN_MENU_OPEN, "Open callsign menu", &TagProcessor::ProcessFN_CallsignMenu);
	AddFuncProcessor(FN_MSK_LANGUAGE_SELECTED, "Language selected", &TagProcessor::ProcessFN_LangSelected);
	AddFuncProcessor(FN_MSK_REST_CLEAR_CONFIRMED, "Restriction selected", &TagProcessor::ProcessFN_RestSelected);
	AddFuncProcessor(FN_MSK_CENTER_TARGET, "Center radar target", &TagProcessor::ProcessFN_CenterTarget);
	AddFuncProcessor(FN_MSK_SWITCH_KMH, "Switch speed", &TagProcessor::ProcessFN_SwitchSpeed);
	AddFuncProcessor(FN_MSK_WAYPOINT_ROUTE_ACTION, "Waypoint route action applied", &TagProcessor::ProcessFN_RoutePointAction);
	AddFuncProcessor(FN_MSK_ROINT_RESET, "Route reset executed", &TagProcessor::ProcessFN_PointReset);
	AddFuncProcessor(FN_MSK_FREETEXT_ACTIVATE, "Freetext clicked", &TagProcessor::ProcessFN_Freetext);
	AddFuncProcessor(FN_MSK_FREETEXT_SET, "Freetext set", &TagProcessor::ProcessFN_Freetext);
	AddFuncProcessor(FN_MSK_TOGGLE_EXT, "Toggle extended data", &TagProcessor::ProcessFN_Freetext);
	AddFuncProcessor(FN_MSK_CCDLC_DCT_CLICKED, "CCDLC DCT clicked", &TagProcessor::ProcessFN_CCDLC);
	AddFuncProcessor(FN_MSK_CCDLC_ALT_CLICKED, "CCDLC ALT clicked", &TagProcessor::ProcessFN_CCDLC);
	AddFuncProcessor(FN_MSK_CCDLC_SPD_CLICKED, "CCDLC SPD clicked", &TagProcessor::ProcessFN_CCDLC);
	AddFuncProcessor(FN_MSK_CCDLC_SPD_M_CLICKED, "CCDLC MACH clicked", &TagProcessor::ProcessFN_CCDLC);
	AddFuncProcessor(FN_MSK_CCDLC_TXT_CLICKED, "CCDLC TXT clicked", &TagProcessor::ProcessFN_CCDLC);
	AddFuncProcessor(FN_MSK_CCDLC_DCT_SELECTED, "CCDLC DCT selected", &TagProcessor::ProcessFN_CCDLC);
	AddFuncProcessor(FN_MSK_CCDLC_ALT_SELECTED, "CCDLC ALT select", &TagProcessor::ProcessFN_CCDLC);
	AddFuncProcessor(FN_MSK_CCDLC_SPD_SELECTED, "CCDLC SPD selected", &TagProcessor::ProcessFN_CCDLC);
	AddFuncProcessor(FN_MSK_CCDLC_TXT_ENTERED, "CCDLC TXT entered", &TagProcessor::ProcessFN_CCDLC);
	AddFuncProcessor(FN_MSK_CCDLC_REQUEST_CLICKED, "CCDLC request clicked", &TagProcessor::ProcessFN_CCDLC);
	AddFuncProcessor(FN_MSK_CCDLC_REPLY_SELECTED, "CCDLC reply selected", &TagProcessor::ProcessFN_CCDLC);
	AddFuncProcessor(FN_MSK_CCDLC_FREETEXT_CLICKED, "CCDLC freetext clicked", &TagProcessor::ProcessFN_CCDLC);
	AddFuncProcessor(FN_MSK_CCDLC_FREETEXT_ENTERED, "CCDLC freetext entered", &TagProcessor::ProcessFN_CCDLC);
	AddFuncProcessor(FN_MSK_CCDLC_CUSTOM_REQ, "CCDLC custom request", &TagProcessor::ProcessFN_CCDLC);
	AddFuncProcessor(FN_MSK_SELECT_TEXT_MESSAGE, "Open text message selector", &TagProcessor::ProcessFN_TxtMsg);
	AddFuncProcessor(FN_MSK_SELECT_TEXT_MESSAGE_SELECTED, "Text message selected", &TagProcessor::ProcessFN_TxtMsg);
	AddFuncProcessor(FN_MSK_TOGGLE_FREETEXT, "Freetext toggled", &TagProcessor::ProcessFN_Freetext);

	for (auto& P : __tags)
	{
		Plugin->RegisterTagItemType(P.second.DisplayName.c_str(), P.second.ItemCode);
	}
	for (auto& P : __funcs)
	{
		Plugin->RegisterTagItemFunction(P.second.DisplayName.c_str(), P.second.ItemCode);
	}
}

void TagProcessor::AddTagProcessor(int TagId, bool HighLightWhenTX, bool OverrideColorEvenIfTX, string Name, TagItemParserFN Handler)
{
	__tags[TagId] = { TagId, HighLightWhenTX, OverrideColorEvenIfTX, Name, Handler };
}

void TagProcessor::AddFuncProcessor(int FuncId, string Name, TagFuncParserFN Handler)
{
	__funcs[FuncId] = { FuncId, Name, Handler };
}

TagItemParser* TagProcessor::GetTag(int TagId)
{
	return &__tags[TagId];
}

TagFunctionParser* TagProcessor::GetFN(int FnId)
{
	if (__funcs.count(FnId) > 0)
		return &__funcs[FnId];
	return NULL;
}

COLORREF TagProcessor::PortToColor(string Port) {

	if (Port == "UUEE")	return RGB(255, 188, 255);
	if (Port == "UUDD") return RGB(255, 194, 160);
	if (Port == "UUWW") return RGB(132, 207, 255);
	return RGB(0, 0, 0);
}

COLORREF TagProcessor::PortToColorForPointer(string Port) {

	if (Port == "UUEE")	return RGB(255, 188, 255);
	if (Port == "UUDD") return RGB(255, 194, 160);
	if (Port == "UUWW") return RGB(132, 207, 255);
	return RGB(255, 255, 255);
}

void TagProcessor::ProcessConstraint(int &ItemCode, string &CS, string& tag, COLORREF& color) {

	ExtendedDataRequest req;
	auto p = req.request(CS);
	tag = p->Route.ActiveConstraint;
}

void TagProcessor::ProcessDeparturePort(int &ItemCode, string &CS, string &tag, COLORREF &color)
{
	ExtendedDataRequest req;
	auto p = req.request(CS);
	tag = p->Route.DepartureICAO;
	color = PortToColor(tag);
}

void TagProcessor::ProcessDestinationPort(int &ItemCode, string &CS, string &tag, COLORREF &color) {

	ExtendedDataRequest req;
	auto p = req.request(CS);
	tag = p->Route.DestinationICAO;
	color = PortToColor(tag);
}

void TagProcessor::ProcessSTAR(int &ItemCode, string &CS, string &tag, COLORREF &color)
{
	string Dest;

	ExtendedDataRequest req;
	auto p = req.request(CS);
	tag = p->ActualFlightData.CustomData.Star;
	Dest = p->Route.DestinationICAO;
	color = PortToColor(Dest);
}

void TagProcessor::ProcessSID(int &ItemCode, string &CS, string &tag, COLORREF &color)
{
	ExtendedDataRequest req;
	auto p = req.request(CS);
	tag = p->ActualFlightData.CustomData.Sid;
}

void TagProcessor::ProcessLastXmt(int &ItemCode, string &CS, string &tag, COLORREF &color)
{
	if (Plugin->Afv->IsTransmit(CS)) {
		tag = "O";
	}
}

void TagProcessor::ProcessArea(int &ItemCode, string &CS, string &tag, COLORREF &color)
{
	ExtendedDataRequest req;
	auto p = req.request(CS);
	tag = p->Route.CurrentArea.Name;
	if (tag != "" && tag.length() >= 3 && safe_substr(tag, 0, 3) == "RWY")
	{
		color = TTL_RED;
	}
}

void TagProcessor::ProcessSts(int &ItemCode, string &CS, string &tag, COLORREF &color)
{
	ExtendedDataRequest req;
	auto p = req.request(CS);

	switch (p->ActualFormularData.AutoStatus) {

	case	ACFT_STATUS_ALT:	tag = "ALT"; if (p->Route.CurrentArea.Name != "") color = CLR_ALT; break;
	case ACFT_STATUS_CLB:		tag = "CLB"; break;
	case ACFT_STATUS_CRZ:		tag = "CRZ"; break;
	case ACFT_STATUS_DES:		tag = "DES"; break;
	case ACFT_STATUS_GROUND:	tag = "GND"; break;
	case ACFT_STATUS_HOLD:		tag = "HLD"; break;
	case ACFT_STATUS_MOV:		tag = "MOV"; break;
	case ACFT_STATUS_ROLL:		tag = "ROLL"; break;
	case ACFT_STATUS_TAX:		tag = "TAX"; break;
	}
}

void TagProcessor::ProcessFinalFixAlt(int &ItemCode, string &CS, string &tag, COLORREF &color)
{
	ExtendedDataRequest req;
	auto p = req.request(CS);

	tag = p->ActualFormularData.FAF;
}

void TagProcessor::ProcessTTL(int &ItemCode, string &CS, string &tag, COLORREF &color)
{
	int TTL = -1;
	bool CTL = false;

	ExtendedDataRequest req;
	auto p = req.request(CS);

	TTL = p->ActualFormularData.TTL;
	CTL = p->ActualFlightData.CustomData.CTL;

	tag = to_string(TTL);
	if (TTL == -1) tag = "";

	if (TTL <= 45)
	{
		color = TTL_RED;
	}
	else if (TTL <= 90)
	{
		color = TTL_WARN;
	}
	if (CTL)
	{
		tag += ".";
	}
}

void TagProcessor::ProcessAman(int &ItemCode, string &CS, string &tag, COLORREF &color)
{
	auto entry = Plugin->Aman->get(CS);
	if (entry.CS == "")	return;

	switch (ItemCode) {
		case	TAG_MSK_AMAN_POINT:	tag = entry.Point; break;
		case	TAG_MSK_AMAN_POINT_TIME: tag = entry.Time; break;
		case	TAG_MSK_AMAN_MILES: tag = to_string((int)entry.Dist); break;
	}

	if (entry.Warn)
	{
		color = AMAN_WARN;
	}
}

void TagProcessor::ProcessCallsign(int &ItemCode, string &CS, string &tag, COLORREF &color)
{
	ExtendedDataRequest req;
	auto p = req.request(CS);

	tag = p->ActualFormularData.Callsign;
}

void TagProcessor::ProcessRunway(int &ItemCode, string &CS, string &tag, COLORREF &color)
{
	ExtendedDataRequest req;
	auto p = req.request(CS);

	tag = (ItemCode == TAG_MSK_DEPA_RWY || ItemCode == TAG_MSK_DEPA_RWY_TX) ? p->ActualFlightData.CustomData.DepartureRwy : p->ActualFlightData.CustomData.DestinationRwy;
}

void TagProcessor::ProcessFL(int &ItemCode, string &CS, string &tag, COLORREF &color)
{
	ExtendedDataRequest req;
	auto p = req.request(CS);

	switch (ItemCode)
	{
	case TAG_MSK_OWN_AFL:
	case TAG_MSK_OWN_AFL_TX:	tag = (p->ActualFormularData.IsCharlie ? ((p->ActualFlightData.AFL_IsAltitude ? "A" : "") + FormatPressureAlt(p->ActualFlightData.CurrentAltitude)) : "AFL"); break;
	case TAG_MSK_OWN_CFL:
	case TAG_MSK_OWN_CFL_TX:	if (p->ActualFormularData.ClearedAltitude == 0) tag = "CFL";
						   else if (p->ActualFormularData.ClearedAltitude == 1)	tag = "CA";
						   else if (p->ActualFormularData.ClearedAltitude == 2)	tag = "VA";
						   else tag = (p->ActualFormularData.CFL_IsAltitude ? "A" : "") + FormatPressureAlt(p->ActualFormularData.ClearedAltitude); break;
	case TAG_MSK_OWN_FFL:
	case TAG_MSK_OWN_FFL_TX:	tag = FormatPressureAlt(p->ActualFormularData.FinalAltitude);
		if (p->Route.ExitBearing > -1 && p->Route.ExitBearing >= 0 && p->Route.ExitBearing < 180 && IsEvenLevel(p->ActualFormularData.FinalAltitude))
			color = CLR_ALT;
		else if (p->Route.ExitBearing > -1 && p->Route.ExitBearing >= 180 && p->Route.ExitBearing < 360 && !IsEvenLevel(p->ActualFormularData.FinalAltitude))
			color = CLR_ALT;
		break;
	}
}

void TagProcessor::ProcessSpeed(int &ItemCode, string &CS, string &tag, COLORREF &color)
{
	ExtendedDataRequest req;
	auto p = req.request(CS);

	if (ItemCode == TAG_MSK_OWN_HDG)
	{
		if (p->ActualFormularData.AssignedHeading > 0)
		{
			color = CLR_SPEED_ASSIGNED;
			tag = to_string(p->ActualFormularData.AssignedHeading);
			if (tag.length() == 2)
			{
				tag = "0" + tag;
			}
			tag = "H" + tag;
		}
	}
	else if (ItemCode == TAG_MSK_OWN_SPEED || ItemCode == TAG_MSK_OWN_SPEED_TX)
	{
		auto mach = p->ActualFormularData.AssignedMach;
		auto speed = p->ActualFormularData.AssignedSpeed;

		if (mach > 0)
		{
			tag = "M"+getMach(mach);
			color = CLR_SPEED_ASSIGNED;

			if (p->ActualFlightData.CurrentAltitude < 29000)
			{
				color = CLR_ALT;
			}
		}
		else if (speed > 0) {

			color = CLR_SPEED_ASSIGNED;
			if (!p->ActualFormularData.SpeedInNm)
			{
				tag = "K";
				tag += precisionDouble(roundNearestTenth((int)(speed * 1.852)), 3, 4);
			}
			else {
				tag = "S"+to_string(speed);
			}
			if (p->Route.RouteDistanceToDest < 20)
			{
				color = CLR_ALT;
			}
		}
	}
	else if (ItemCode == TAG_MSK_OWN_GS || ItemCode == TAG_MSK_OWN_GS_TX)
	{
		if (!p->ActualFormularData.SpeedInNm)
		{
			tag = "K" + precisionDouble(roundNearestTenth((int)(p->ActualFlightData.GroundSpeed * 1.852)), 0, 4);
		}
		else {
			tag = "N" + precisionDouble(p->ActualFlightData.GroundSpeed, 3, 4);
		}
	}
	else if (ItemCode == TAG_MSK_OWN_VS || ItemCode == TAG_MSK_OWN_VS_TX)
	{
		int vs = p->ActualFlightData.VerticalSpeed;
		if (abs(vs) < 100) return;
		if (p->ActualFormularData.VSInFm)
		{
			tag = "fm" + string(vs < 0 ? "-" : "") + precisionDouble(abs(vs) / 100, 2, 4);
		}
		else {
			auto ms = vs / 197;
			if (ms < 0)	return;
			tag = "ms" + string(vs < 0 ? "-" : "") + precisionDouble(abs(vs), 2, 4);
		}
	}
}

void TagProcessor::ProcessLang(int &ItemCode, string &CS, string &tag, COLORREF &color)
{
	ExtendedDataRequest req;
	auto p = req.request(CS);
	tag = p->ActualFlightData.CustomData.ImEnglish ? ">" : "";
}

void TagProcessor::ProcessHighGrad(int &ItemCode, string &CS, string &tag, COLORREF &color)
{
	color = TTL_WARN;

	ExtendedDataRequest req;
	auto p = req.request(CS);
	tag = p->ActualFormularData.HighGrad ? "G" : "";
}

void TagProcessor::ProcessTimes(int &ItemCode, string &CS, string &tag, COLORREF &color)
{
	ExtendedDataRequest req;
	auto p = req.request(CS);

	switch (ItemCode)
	{
	case TAG_MSK_READY_TIME:	tag = p->ActualFlightData.CustomData.ReadyTime; break;
	case TAG_MSK_OFFBLOCK_TIME:	tag = p->ActualFlightData.CustomData.ActualOffBlockTime; break;
	}
}

void TagProcessor::ProcessRestriction(int &ItemCode, string &CS, string &tag, COLORREF &color)
{
	color = GREEN;

	ExtendedDataRequest req;
	auto p = req.request(CS);
	switch (ItemCode)
	{
	case TAG_MSK_REST_SPEED:	tag = p->ActualFlightData.CustomData.SpeedUnrestricted ? "*" : " "; break;
	case TAG_MSK_REST_ALT:	tag = p->ActualFlightData.CustomData.AltUnrestricted ? "*" : " "; break;
	}
}

void TagProcessor::ProcessAcftIndex(int &ItemCode, string &CS, string &tag, COLORREF &color)
{
	ExtendedDataRequest req;
	auto p = req.request(CS);
	tag = p->ActualFormularData.SortIndex;
}

void TagProcessor::ProcessExtendedData(int &ItemCode, string &CS, string &tag, COLORREF &color)
{
	ExtendedDataRequest req;
	auto p = req.request(CS);

	if (ItemCode == TAG_MSK_EXT_DATA_WARNING)
	{
		tag = p->ActualFormularData.ExtendedData.warn() ? ">>" : "";
		color = tag != "" ? CLR_EXT : color;
		return;
	}
	else if (ItemCode == TAG_MSK_PILOT_FREETEXT1)
	{
		tag = p->ActualFormularData.ExtendedData.ft1();
		color = WHITE;
		return;
	}
	else if (ItemCode == TAG_MSK_PILOT_FREETEXT2)
	{
		tag = p->ActualFormularData.ExtendedData.ft2();
		color = WHITE;
		return;
	}

	if (!p->ActualFormularData.ExtendedData.IsShown)	return;

	switch (ItemCode)
	{
	case TAG_MSK_EXT_DATA_DEPA_ICAO: tag = p->ActualFlightData.CustomData.DepartureICAO; break;
	case TAG_MSK_EXT_DATA_DEPA_RWY: tag = p->ActualFlightData.CustomData.DepartureRwy; break;
	case TAG_MSK_EXT_DATA_DEST_ICAO: tag = p->ActualFlightData.CustomData.DestinationICAO; break;
	case TAG_MSK_EXT_DATA_DEST_RWY: tag = p->ActualFlightData.CustomData.DestinationRwy; break;

	case TAG_MSK_RNAV:		tag = "RN";
		color = p->ActualFormularData.ExtendedData.Rnav ? color : CLR_ALT;
		break;
	case TAG_MSK_RVSM:		tag = "RV";
		color = p->ActualFormularData.ExtendedData.RVSM ? color : CLR_ALT;
		break;
	case TAG_MSK_NEWBIE:	tag = "NB";
		color = !p->ActualFormularData.ExtendedData.Newbie ? color : CLR_ALT;
		break;
	case TAG_MSK_BADGUY:	tag = "BG";
		color = !p->ActualFormularData.ExtendedData.Badguy ? color : CLR_ALT;
		break;
	case TAG_MSK_EXT_DATA_HOURS:
		tag = p->ActualFormularData.ExtendedData.Hours > 0 ? to_string(p->ActualFormularData.ExtendedData.Hours) : "---";
		break;
	case TAG_MSK_ATC_RATING:tag = ATC_RATINGS[p->ActualFormularData.ExtendedData.ATCRating];
		color = p->ActualFormularData.ExtendedData.ATCRating < 12 ? color : CLR_ALT;
		break;
	case TAG_MSK_PILOT_RATING:tag = GetPilotRating(p->ActualFormularData.ExtendedData.PilotRating);
		break;
	case TAG_MSK_PILOT_NAME:tag = p->ActualFormularData.ExtendedData.Name != "" ? safe_substr(p->ActualFormularData.ExtendedData.Name, 0, 15) : "----------"; break;
	}
}

void TagProcessor::ProcessCCDLC(int &ItemCode, string &CS, string& DisplayValue, COLORREF& DisplayColor)
{
	DisplayColor = WHITE;

	ExtendedDataRequest r;
	auto p = r.request(CS);

	if (!p->CCDLCRequest.IsValid())
	{
		if (ItemCode == TAG_MSK_CCDLC_DISPLAY_DCT)
			DisplayValue = "DCT";
		else if (ItemCode == TAG_MSK_CCDLC_DISPLAY_ALT)
			DisplayValue = "ALT";
		else if (ItemCode == TAG_MSK_CCDLC_DISPLAY_SPD)
			DisplayValue = "SPD";
		else if (ItemCode == TAG_MSK_CCDLC_DISPLAY_TXT)
			DisplayValue = "TEXTREQ";

		return;
	}

	auto req = p->CCDLCRequest;

	switch (ItemCode) {

	case	TAG_MSK_CCDLC_INITIATOR:				DisplayValue = req.Init; break;
	case	TAG_MSK_CCDLC_REQUEST:					DisplayValue = safe_substr(req.Request, 0, 15); break;
	case	TAG_MSK_CCDLC_REQUEST_EXT:				DisplayValue = safe_substr(req.Request, 15, 15); break;
	case	TAG_MSK_CCDLC_REACTIONS:				DisplayValue = safe_substr(req.Likes + req.Dislikes, 0, 15);	if (req.Dislikes.length() > 0) { DisplayColor = RGB(255, 0, 0); }; break;
	case	TAG_MSK_CCDLC_FREETEXT:					DisplayValue = safe_substr(req.Text, 0, 15); break;
	case	TAG_MSK_CCDLC_FREETEXT_EXT:				DisplayValue = safe_substr(req.Text, 15, 15); break;
	}

	if (req.Request == "REQ TRS")
	{
		DisplayColor = CLR_ALT;
	}
}

void TagProcessor::ProcessManualStatus(int &ItemCode, string &CS, string &tag, COLORREF &color)
{
	ExtendedDataRequest req;
	auto p = req.request(CS);

	tag = p->ActualFlightData.CustomData.ManualStatus;
	if (tag == "SQWK" || tag == "STBY" || tag == "VECT" || tag == "STAR")
	{
		color = CLR_ALT;
	}
	else if (tag == "HOFF")
	{
		color = CLR_SPEED_ASSIGNED;
	}
}

void TagProcessor::ProcessFN_SwitchSpeed(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT &area)
{
	ExtendedDataRequest req;
	auto p = req.request(CS);

	p->ActualFormularData.SpeedInNm = !p->ActualFormularData.SpeedInNm;
}

void TagProcessor::ProcessFN_TxtMsg(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT &area)
{
	if (!Plugin->ImController())	return;

	ExtendedDataRequest req;
	auto p = req.request(CS);

	if (FunctionId == FN_MSK_SELECT_TEXT_MESSAGE)
	{
		Plugin->OpenPopupList(area, "TXT Message", 1);
		if (p->ActualFormularData.AutoStatus == ACFT_STATUS_GROUND || p->ActualFormularData.AutoStatus == ACFT_STATUS_MOV || p->ActualFormularData.AutoStatus == ACFT_STATUS_TAX)
		{
			if (!p->ActualFormularData.HasClearance)
			{
				Plugin->AddPopupListElement("CLEARANCE", NULL, FN_MSK_SELECT_TEXT_MESSAGE_SELECTED);
				Plugin->AddPopupListElement("CLEARANCE_OWNNAV", NULL, FN_MSK_SELECT_TEXT_MESSAGE_SELECTED);
				Plugin->AddPopupListElement("FP_WILL_AMEND", NULL, FN_MSK_SELECT_TEXT_MESSAGE_SELECTED);
				Plugin->AddPopupListElement("AIRAC", NULL, FN_MSK_SELECT_TEXT_MESSAGE_SELECTED);
				Plugin->AddPopupListElement("FLIGHTLEVEL", NULL, FN_MSK_SELECT_TEXT_MESSAGE_SELECTED);
				Plugin->AddPopupListElement("NOCHANGES", NULL, FN_MSK_SELECT_TEXT_MESSAGE_SELECTED);
			}
			else {
				Plugin->AddPopupListElement("SQUAWK", NULL, FN_MSK_SELECT_TEXT_MESSAGE_SELECTED);
				Plugin->AddPopupListElement("TAXI", NULL, FN_MSK_SELECT_TEXT_MESSAGE_SELECTED);
				Plugin->AddPopupListElement("HOLDSHORT", NULL, FN_MSK_SELECT_TEXT_MESSAGE_SELECTED);
				Plugin->AddPopupListElement("CROSS", NULL, FN_MSK_SELECT_TEXT_MESSAGE_SELECTED);
				Plugin->AddPopupListElement("LINEUP", NULL, FN_MSK_SELECT_TEXT_MESSAGE_SELECTED);
				Plugin->AddPopupListElement("LINEUP_BEHIND", NULL, FN_MSK_SELECT_TEXT_MESSAGE_SELECTED);
				Plugin->AddPopupListElement("LINEUP_SEQUENCE", NULL, FN_MSK_SELECT_TEXT_MESSAGE_SELECTED);
				Plugin->AddPopupListElement("TAKEOFF", NULL, FN_MSK_SELECT_TEXT_MESSAGE_SELECTED);
				Plugin->AddPopupListElement("-----", NULL, FN_NOOP);
				Plugin->AddPopupListElement("CROSS", NULL, FN_MSK_SELECT_TEXT_MESSAGE_SELECTED);
				Plugin->AddPopupListElement("TAXI_TO_GATE", NULL, FN_MSK_SELECT_TEXT_MESSAGE_SELECTED);
			}
			if (!p->ActualControllerInfo.IHaveControlledIt)
			{
				Plugin->AddPopupListElement("CONTACTME", NULL, FN_MSK_SELECT_TEXT_MESSAGE_SELECTED);
			}
		}
		else {
			if (p->ActualFormularData.AssignedSquawk != p->ActualFormularData.ActualSquawk)
			{
				Plugin->AddPopupListElement("SQUAWK", NULL, FN_MSK_SELECT_TEXT_MESSAGE_SELECTED);
			}
			Plugin->AddPopupListElement("AIRAC", NULL, FN_MSK_SELECT_TEXT_MESSAGE_SELECTED);
			Plugin->AddPopupListElement("STAR", NULL, FN_MSK_SELECT_TEXT_MESSAGE_SELECTED);
			if (p->ActualFlightData.CurrentAltitude < p->ActualFormularData.FinalAltitude)
			{
				Plugin->AddPopupListElement("CLIMB", NULL, FN_MSK_SELECT_TEXT_MESSAGE_SELECTED);
				Plugin->AddPopupListElement("CLIMB_MAXVS", NULL, FN_MSK_SELECT_TEXT_MESSAGE_SELECTED);
				Plugin->AddPopupListElement("STOP_CLIMB_AT", NULL, FN_MSK_SELECT_TEXT_MESSAGE_SELECTED);
			}
			if (p->ActualFlightData.CurrentAltitude > p->ActualFormularData.ClearedAltitude)
			{
				Plugin->AddPopupListElement("DESCEND", NULL, FN_MSK_SELECT_TEXT_MESSAGE_SELECTED);
				Plugin->AddPopupListElement("DES_MAXVS", NULL, FN_MSK_SELECT_TEXT_MESSAGE_SELECTED);
				Plugin->AddPopupListElement("STOP_DESCEND_AT", NULL, FN_MSK_SELECT_TEXT_MESSAGE_SELECTED);
			}
			Plugin->AddPopupListElement("DIRECT_TO", NULL, FN_MSK_SELECT_TEXT_MESSAGE_SELECTED);
			Plugin->AddPopupListElement("SPEED/MACH", NULL, FN_MSK_SELECT_TEXT_MESSAGE_SELECTED);
			Plugin->AddPopupListElement("HEADING", NULL, FN_MSK_SELECT_TEXT_MESSAGE_SELECTED);
			Plugin->AddPopupListElement("CLEARED_ILS", NULL, FN_MSK_SELECT_TEXT_MESSAGE_SELECTED);
			Plugin->AddPopupListElement("-----", NULL, FN_NOOP);
			Plugin->AddPopupListElement("CLEARED_LAND", NULL, FN_MSK_SELECT_TEXT_MESSAGE_SELECTED);
		}
	}
	else if (FunctionId == FN_MSK_SELECT_TEXT_MESSAGE_SELECTED)
	{
		string ru, en;
		if (ItemValue == "CLEARANCE" || ItemValue == "CLEARANCE_OWNNAV")
		{
			if (ItemValue == "CLEARANCE")
			{
				en = "Cleared to destination via flight plan route of runway " + p->ActualFlightData.CustomData.DepartureRwy + " via " + p->ActualFlightData.CustomData.Sid + " departure. ";
				ru = "Разрешено до аэродрома назначения. ВПП " + p->ActualFlightData.CustomData.DepartureRwy + ", выход по схеме " + p->ActualFlightData.CustomData.Sid + ". ";
			}
			else {
				en = "Cleared to destination via flight plan route of runway " + p->ActualFlightData.CustomData.DepartureRwy + ". After departure resume own navigation. ";
				ru = "Разрешено до аэродрома назначения. ВПП " + p->ActualFlightData.CustomData.DepartureRwy + ", после взлёта возобновите полёт по своим средствам навигации. ";
			}
			if (p->ActualFormularData.ClearedAltitude > 0)
			{
				en += "Initial climb " + (p->ActualFormularData.CFL_IsAltitude ? ("to altitude " + to_string(p->ActualFormularData.ClearedAltitude) + " feet. ") : ("to flight level " + FormatPressureAlt(p->ActualFormularData.ClearedAltitude) + ". "));
				ru += "Первоначальный набор " + (p->ActualFormularData.CFL_IsAltitude ? ("высота " + to_string(p->ActualFormularData.ClearedAltitude) + " футов. ") : ("эшелон " + FormatPressureAlt(p->ActualFormularData.ClearedAltitude) + ". "));
			}
			if (p->ActualControllerInfo.NextControllerId != "")
			{
				for (auto ctr = Plugin->ControllerSelectFirst(); ctr.IsValid(); ctr = Plugin->ControllerSelectNext(ctr))
				{
					string cs = check_str_create(ctr.GetPositionId());
					if (cs == p->ActualControllerInfo.NextControllerId)
					{
						en += "After departure contact " + precisionDouble(ctr.GetPrimaryFrequency(), 3, 6) + ". ";
						ru += "После взлёта работайте " + precisionDouble(ctr.GetPrimaryFrequency(), 3, 6) + ". ";

					}
				}
			}
			else {
				if (Plugin->ControllerMyself().GetFacility() <= 4)
				{
					en += "After departure monitor UNICOM on 122.8. ";
					ru += "После взлёта прослушивайте Юником 122.8. ";
				}
			}
			ru += "Сквок " + p->ActualFormularData.AssignedSquawk;
			en += "Squawk " + p->ActualFormularData.AssignedSquawk;
		}
		else if (ItemValue == "AIRAC")
		{
			ru = "Уважаемый пилот! Вы подали полётный план, содержащий старые точки и маршруты навигации. 3 декабря в Московской воздушной зоне произошли серьезные изменения в структуре, ";
			ru += "несовместимые с вашим полётным планом. Проверьте, что вы используете актуальную навигационную базу данных в вашем FMS/CDU и, при необходимости, установите свежий AIRAC. ";
			ru += "Корректный маршрут для полётов внутри РФ можно взять с сайта https://infogate.matfmc.ru/htme/routes.htme. Для международных полётов подойдет Route Finder";

			en = "Dear pilot! Your flight plan route is incompatible with huge Moscow Airspace structure changes occured 3rd of December 2020. ";
			en += "Please make sure you have the up-to-date AIRAC. ";
			en += "You can get correct route for domestic Russian flights from https://infogate.matfmc.ru/htme/routes.htme. For international flights Route Finder is available";
		}
		else if (ItemValue == "FLIGHTLEVEL")
		{
			if ((p->ActualFormularData.FinalAltitude % 2000) == 0)
			{
				ru = "Для полёта требуется нечётный эшелон (290, 310, 330 и т.п.). Исправльте пожалуйста и отправьте повторно ваш полётный план";
				en = "You are required to use odd flight level (290, 310, 330 etc.). Please fix and resend your flight plan";
			}
			else {
				ru = "Для полёта требуется чётный эшелон (280, 300, 320 и т.п.). Исправльте пожалуйста и отправьте повторно ваш полётный план";
				en = "You are required to use even flight level (280, 300, 320 etc.). Please fix and resend your flight plan";
			}

		}
		else if (ItemValue == "SQUAWK")
		{
			ru = "Установите сквок " + p->ActualFormularData.AssignedSquawk + ". Режим Чарли";
			en = "Set squawk " + p->ActualFormularData.AssignedSquawk + ". Mode Charlie";
		}
		else if (ItemValue == "TAXI")
		{
			ru = "Рулите на предварительный ВПП " + p->ActualFlightData.CustomData.DepartureRwy + " за машиной сопровождения. На предварительном доложите";
			en = "Taxi to holding point of runway " + p->ActualFlightData.CustomData.DepartureRwy + " behind follow-me car";
		}
		else if (ItemValue == "HOLDSHORT")
		{
			ru = "Ожидайте перед ВПП ";
			en = "Hold short of runway ";
		}
		else if (ItemValue == "CROSS")
		{
			ru = "Пересекайте ВПП ";
			en = "Cross runway ";
		}
		else if (ItemValue == "LINEUP")
		{
			ru = "Занимайте исполнительный ВПП " + p->ActualFlightData.CustomData.DepartureRwy;
			en = "Lineup runway " + p->ActualFlightData.CustomData.DepartureRwy;
		}
		else if (ItemValue == "LINEUP_BEHIND")
		{
			ru = "После посадки борта на короткой прямой занимайте исполнительный ВПП " + p->ActualFlightData.CustomData.DepartureRwy + " после посадки";
			en = "Behind landing traffic on short final lineup runway " + p->ActualFlightData.CustomData.DepartureRwy + " behind";
		}
		else if (ItemValue == "LINEUP_SEQUENCE")
		{
			ru = "Занимайте по очереди исполнительный ВПП " + p->ActualFlightData.CustomData.DepartureRwy;
			en = "Lineup in sequence runway " + p->ActualFlightData.CustomData.DepartureRwy;
		}
		else if (ItemValue == "LINEUP_SEQUENCE")
		{
			ru = "ВПП " + p->ActualFlightData.CustomData.DepartureRwy + " взлёт разрешаю";
			en = "Cleared for takeoff runway " + p->ActualFlightData.CustomData.DepartureRwy;
		}
		else if (ItemValue == "TAXI_TO_GATE")
		{
			ru = "Рулите на стоянку по вашему усмотрению за машиной сопровождения";
			en = "Taxi to gate of your choice behind follow-me car";
		}
		else if (ItemValue == "STAR")
		{
			ru = "Расчитывайте ВПП " + p->ActualFlightData.CustomData.DestinationRwy + " по СТАР " + p->ActualFlightData.CustomData.Star + ". Соблюдайте органичения СТАР. Готовность к снижению доложите";
			en = "Expect runway " + p->ActualFlightData.CustomData.DestinationRwy + " via STAR " + p->ActualFlightData.CustomData.Star + ". Respect STAR contraints. Report ready for descend";
		}
		else if (ItemValue == "CLIMB" || ItemValue == "CLIMB_MAXVS")
		{
			ru = "Набирайте " + (!p->ActualFlightData.CustomData.AltUnrestricted ? string("без ограничений ") : "") + (p->ActualFormularData.CFL_IsAltitude ? ("высоту " + to_string(p->ActualFormularData.ClearedAltitude) + " футов") : ("эшелон " + FormatPressureAlt(p->ActualFormularData.ClearedAltitude)));
			en = "Climb " + (!p->ActualFlightData.CustomData.AltUnrestricted ? string("unrestricted ") : "") + (p->ActualFormularData.CFL_IsAltitude ? ("to altitude " + to_string(p->ActualFormularData.ClearedAltitude) + " feet") : ("FL" + FormatPressureAlt(p->ActualFormularData.ClearedAltitude)));
			if (ItemValue == "CLIMB_MAXVS")
			{
				ru += " с максимальной вертикальной";
				en += " with maximum rate";
			}
		}
		else if (ItemValue == "STOP_CLIMB_AT")
		{
			ru = "Остановите набор на " + (p->ActualFormularData.CFL_IsAltitude ? ("высоте " + to_string(p->ActualFormularData.ClearedAltitude) + " футов") : ("эшелоне " + FormatPressureAlt(p->ActualFormularData.ClearedAltitude)));
			en = "Stop climb at " + (p->ActualFormularData.CFL_IsAltitude ? ("altitude " + to_string(p->ActualFormularData.ClearedAltitude) + " feet") : ("FL" + FormatPressureAlt(p->ActualFormularData.ClearedAltitude)));
		}
		else if (ItemValue == "DESCEND" || ItemValue == "DES_MAXVS")
		{
			ru = "Снижайтесь по СТАР " + (!p->ActualFlightData.CustomData.AltUnrestricted ? string("без ограничений ") : "") + (p->ActualFormularData.CFL_IsAltitude ? ("высота " + to_string(p->ActualFormularData.ClearedAltitude) + " футов") : (" эшелон " + FormatPressureAlt(p->ActualFormularData.ClearedAltitude)));
			en = "Descend via STAR " + (!p->ActualFlightData.CustomData.AltUnrestricted ? string("unrestricted ") : "") + (p->ActualFormularData.CFL_IsAltitude ? ("altitude " + to_string(p->ActualFormularData.ClearedAltitude) + " feet") : (" FL" + FormatPressureAlt(p->ActualFormularData.ClearedAltitude)));
			if (ItemValue == "DES_MAXVS")
			{
				ru += " с максимальной вертикальной";
				en += " with maximum rate";
			}
		}
		else if (ItemValue == "STOP_DESCEND_AT")
		{
			ru = "Остановите снижение на " + (p->ActualFormularData.CFL_IsAltitude ? ("высоте " + to_string(p->ActualFormularData.ClearedAltitude) + " футов") : ("эшелоне " + FormatPressureAlt(p->ActualFormularData.ClearedAltitude)));
			en = "Stop descend at " + (p->ActualFormularData.CFL_IsAltitude ? ("altitude " + to_string(p->ActualFormularData.ClearedAltitude) + " feet") : ("FL" + FormatPressureAlt(p->ActualFormularData.ClearedAltitude)));
		}
		else if (ItemValue == "DIRECT_TO" && p->Route.CurrentDirectPoint != NULL)
		{
			ru = "Разрешаю прямо на " + p->Route.CurrentDirectPoint->Name;
			en = "Cleared direct to " + p->Route.CurrentDirectPoint->Name;
		}
		else if (ItemValue == "SPEED/MACH")
		{
			if (p->ActualFormularData.AssignedMach > 0)
			{
				ru = "Установите мах " + getMach(p->ActualFormularData.AssignedMach);
				en = "Set mach number " + getMach(p->ActualFormularData.AssignedMach);
			}
			else if (p->ActualFormularData.AssignedSpeed > 0) {
				ru = "Установите приборную скорость " + to_string(p->ActualFormularData.AssignedSpeed) + " узлов";
				en = "Set indicated speed " + to_string(p->ActualFormularData.AssignedSpeed) + " knots";
			}
			else {
				ru = "Скорость без ограничений";
				en = "No speed restrictions";
			}
		}
		else if (ItemValue == "HEADING")
		{
			ru = "Следуйте на курсе " + to_string(p->ActualFormularData.AssignedHeading);
			en = "Fly heading " + to_string(p->ActualFormularData.AssignedHeading);
		}
		else if (ItemValue == "CLEARED_ILS")
		{
			ru = "ВПП " + p->ActualFlightData.CustomData.DestinationRwy + " заход ILS разрешаю. Захват курсового доложите";
			en = "Cleared for ILS approach runway " + p->ActualFlightData.CustomData.DestinationRwy + ". Report established";
		}
		else if (ItemValue == "CLEARED_LAND")
		{
			ru = "ВПП " + p->ActualFlightData.CustomData.DestinationRwy + " посадку разрешаю";
			en = "Cleared to land runway " + p->ActualFlightData.CustomData.DestinationRwy;
		}
		else if (ItemValue == "FP_WILL_AMEND")
		{
			ru = "Маршрут вашего полётного плана будет изменен в соответствии со свежим AIRAC. Через минуту вы можете получить его с сервера, нажав в вашем клиенте кнопку \"Fetch from server\". Сообщите получение и готовность записать условия на вылет";
			en = "Route flight plan route will be modified according to fresh AIRAC cycle. In one minute you can get it by pressing \"Fetch from server\" button in your client. Report got it and ready to copy clearance";
		}
		else if (ItemValue == "NOCHANGES")
		{
			ru = "Полётный план не изменился. Проверьте изменения, нажмите на кнопку \"File/send flight plan\" в клиенте. Тaкже попробуйте переподключиться";
			en = "Flight plan has not changed. Check editions, press \"File/send flight plan\" in your client. Also try to reconnect to network";
		}
		else if (ItemValue == "CONTACTME")
		{
			ru = "Свяжитесь со мной на частоте " + precisionDouble(Plugin->ControllerMyself().GetPrimaryFrequency(), 3, 6);
			en = "Please contact me on frequency " + precisionDouble(Plugin->ControllerMyself().GetPrimaryFrequency(), 3, 6);
		}

		if (p->ActualFlightData.CustomData.ImEnglish)
		{
			toClipboard(en);
		}
		else toClipboard(ru);
	}
}

void TagProcessor::ProcessFN_DisplayRoute(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT &area)
{
	ExtendedDataRequest req;
	auto p = req.request(CS);
	p->DisplayOnRadar.DisplayRoute = !p->DisplayOnRadar.DisplayRoute;
	Plugin->hide_display_route = false;
}

void TagProcessor::ProcessFN_ShowInterval(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT &area)
{
	ExtendedDataRequest req;
	auto p = req.request(CS);

	if (p->DisplayOnRadar.Display10km) {
		p->DisplayOnRadar.Display10km = false;
		p->DisplayOnRadar.Display5km = true;
	}
	else if (p->DisplayOnRadar.Display5km) {
		p->DisplayOnRadar.Display10km = false;
		p->DisplayOnRadar.Display5km = false;
	}
	else if (!p->DisplayOnRadar.Display10km) {
		p->DisplayOnRadar.Display10km = true;
	}
}

void TagProcessor::ProcessFN_AmanPoint(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT &area)
{
	ExtendedDataRequest req;
	auto p = req.request(CS);

	Plugin->OpenPopupList(area, "Point", 1);
	Plugin->AddPopupListElement("-Remove-", "", FN_MSK_AMAN_POINT_SELECTED);

	p->Route.EnumPoints([&](PointDetails* pd) {

		Plugin->AddPopupListElement(pd->Name.c_str(), "", FN_MSK_AMAN_POINT_SELECTED);
		});
}

void TagProcessor::ProcessFN_AmanPointSelect(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT &area)
{
	if (ItemValue == "-Remove-")
	{
		Plugin->Aman->del(CS);
	}
	else {
		Plugin->Aman->put(CS, ItemValue);
	}
}

void TagProcessor::ProcessFN_TTL(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT &area)
{
	ExtendedDataRequest req;
	auto p = req.request(CS);

	if (!p->ActualPlanInfo.IsExpectedOrCurrent)	return;

	Plugin->OpenPopupList(area, "C-T-L", 1);
	Plugin->AddPopupListElement("Cleared", "", FN_MSK_TTL_CLEARED_TO_LAND);
	Plugin->AddPopupListElement("Revoked", "", FN_MSK_TTL_CLEARED_TO_LAND);
}

void TagProcessor::ProcessFN_TTL_Select(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT &area)
{
	ExtendedDataRequest req;
	auto p = req.request(CS);

	if (ItemValue == "Cleared")
	{
		p->ActualFlightData.CustomData.CTL = true;
	}
	else p->ActualFlightData.CustomData.CTL = false;
}

void TagProcessor::ProcessFN_ManualStatus(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT &area)
{
	ExtendedDataRequest req;
	auto p = req.request(CS);

	if (!Plugin->ImController())	return;

	Plugin->OpenPopupList(area, "STATUS", 1);
	if (!p->ActualFlightData.IsAir)
	{
		for (auto& i : STS_GND)
		{
			Plugin->AddPopupListElement(i.c_str(), "", FN_MSK_MAN_STS_ASSIGNED);
		}
	}
	else {
		for (auto i : STS_AIR)
		{
			Plugin->AddPopupListElement(i.c_str(), "", FN_MSK_MAN_STS_ASSIGNED);
		}
	}
}

void TagProcessor::ProcessFN_ManualStatus_Selected(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT &area)
{
	ExtendedDataRequest req;
	auto p = req.request(CS);

	string sts = ItemValue;
	if (sts == "-CLR-")	 sts = "";

	if (p->ActualFlightData.CustomData.ManualStatus == "STAR")
	{
		p->ClearStar();
	}

	p->ActualFlightData.CustomData.ManualStatus = sts;

	if (sts == "")	sts = "CLEAR";

	Plugin->Ccdlc->AddCCDLCMessageToQueue(CS, "STS", sts);
}

void TagProcessor::ProcessFN_CallsignMenu(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT &area)
{
	ExtendedDataRequest req;
	auto p = req.request(CS);

	if (!p->ActualPlanInfo.IsExpectedOrCurrent || p->ActualPlanInfo.IsOnlyExpected)	return;

	Plugin->OpenPopupList(area, "Actions", 1);
	Plugin->AddPopupListElement("---Language---", "", FN_NOOP);
	Plugin->AddPopupListElement("ENGLISH", "", FN_MSK_LANGUAGE_SELECTED, p->ActualFlightData.CustomData.ImEnglish);
	Plugin->AddPopupListElement("RUSSIAN", "", FN_MSK_LANGUAGE_SELECTED, !p->ActualFlightData.CustomData.ImEnglish);
	Plugin->AddPopupListElement("---Restrictions---", "", FN_NOOP);
	Plugin->AddPopupListElement("CANCEL SPEED", "", FN_MSK_REST_CLEAR_CONFIRMED);
	Plugin->AddPopupListElement("CANCEL ALT", "", FN_MSK_REST_CLEAR_CONFIRMED);
	Plugin->AddPopupListElement("CANCEL ALL", "", FN_MSK_REST_CLEAR_CONFIRMED);
	Plugin->AddPopupListElement("REVOKE", "", FN_MSK_REST_CLEAR_CONFIRMED);
	Plugin->AddPopupListElement("---Route actions---", "", FN_NOOP);
	Plugin->AddPopupListElement("RESET ROUTE", "", FN_MSK_ROINT_RESET);
	Plugin->AddPopupListElement("---Extended data actions---", "", FN_NOOP);
	Plugin->AddPopupListElement("Show/hide freetext", "", FN_MSK_TOGGLE_FREETEXT);
}

void TagProcessor::ProcessFN_LangSelected(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT &area)
{
	ExtendedDataRequest req;
	auto p = req.request(CS);

	p->ActualFlightData.CustomData.ImEnglish = ItemValue == "ENGLISH";
	Plugin->Ccdlc->AddCCDLCMessageToQueue(CS, "LNG", ItemValue.substr(0, 3));
}

void TagProcessor::ProcessFN_RestSelected(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT &area)
{
	ExtendedDataRequest req;
	auto p = req.request(CS);

	if (ItemValue == "CANCEL SPEED")	p->ActualFlightData.CustomData.SpeedUnrestricted = true;
	else if (ItemValue == "CANCEL ALT")	p->ActualFlightData.CustomData.AltUnrestricted = true;
	else if (ItemValue == "CANCEL ALL") {
		p->ActualFlightData.CustomData.SpeedUnrestricted = true;
		p->ActualFlightData.CustomData.AltUnrestricted = true;
	}
	else if (ItemValue == "REVOKE")
	{
		p->ActualFlightData.CustomData.SpeedUnrestricted = false;
		p->ActualFlightData.CustomData.AltUnrestricted = false;
	}
	Plugin->Ccdlc->AddCCDLCMessageToQueue(CS, "RST", bth(p->ActualFlightData.CustomData.SpeedUnrestricted) + bth(p->ActualFlightData.CustomData.AltUnrestricted));
}

void TagProcessor::ProcessFN_CenterTarget(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT &area)
{
	Plugin->CenterRT = rt.GetPosition().GetPosition();
}

void TagProcessor::ProcessFN_CCDLC(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT &area)
{
	if (!Plugin->ICanUseCCDLC())	return;

	ExtendedDataRequest req;
	auto data = req.request(CS);

	if (FunctionId == FN_MSK_CCDLC_DCT_CLICKED)
	{
		Plugin->OpenPopupList(area, "CCDLC", 1);

		data->Route.EnumPoints([&](PointDetails* pd) {

			Plugin->AddPopupListElement(pd->Name.c_str(), "", FN_MSK_CCDLC_DCT_SELECTED);
			});
	}
	else if (FunctionId == FN_MSK_CCDLC_DCT_SELECTED) {

		Plugin->Ccdlc->AddCCDLCMessageToQueue(CS, "DCT", ItemValue);
	}
	else if (FunctionId == FN_MSK_CCDLC_ALT_CLICKED)
	{
		Plugin->OpenPopupList(area, "CCDLC", 1);
		for (int alt = 41000; alt >= 2000; alt = alt - 1000)
		{
			bool IsCurrent = abs(data->ActualFlightData.CurrentAltitude - alt) < 500;
			Plugin->AddPopupListElement(FormatPressureAlt(alt).c_str(), "", FN_MSK_CCDLC_ALT_SELECTED, IsCurrent);
		}
	}
	else if (FunctionId == FN_MSK_CCDLC_ALT_SELECTED)
	{
		Plugin->Ccdlc->AddCCDLCMessageToQueue(CS, "ALT", ItemValue);
	}
	else if (FunctionId == FN_MSK_CCDLC_SPD_CLICKED)
	{
		Plugin->OpenPopupList(area, "CCDLC", 1);
		for (int speed = 340; speed >= 140; speed = speed - 10)
		{
			bool IsCurrent = abs(data->ActualFlightData.GroundSpeed - speed) < 10;
			Plugin->AddPopupListElement(to_string(speed).c_str(), "", FN_MSK_CCDLC_SPD_SELECTED, IsCurrent);
		}
	}
	else if (FunctionId == FN_MSK_CCDLC_SPD_M_CLICKED)
	{
		Plugin->OpenPopupList(area, "CCDLC", 1);
		for (int mach = 89; mach >= 72; mach = mach - 1)
		{
			Plugin->AddPopupListElement(getMach(mach).c_str(), "", FN_MSK_CCDLC_SPD_SELECTED);
		}
	}
	else if (FunctionId == FN_MSK_CCDLC_SPD_SELECTED)
	{
		Plugin->Ccdlc->AddCCDLCMessageToQueue(CS, "SPD", ItemValue);
	}
	else if (FunctionId == FN_MSK_CCDLC_TXT_CLICKED)
	{
		Plugin->OpenPopupEdit(area, FN_MSK_CCDLC_TXT_ENTERED, "");
	}
	else if (FunctionId == FN_MSK_CCDLC_TXT_ENTERED)
	{
		trim(ItemValue);
		if (ItemValue != "")
		{
			Plugin->Ccdlc->AddCCDLCMessageToQueue(CS, "TXT", remove_char(ItemValue, ':'));
		}
	}
	else if (FunctionId == FN_MSK_CCDLC_FREETEXT_CLICKED)
	{
		// get previously entered text
		string prev_text = data->CCDLCRequest.Text;
		Plugin->OpenPopupEdit(area, FN_MSK_CCDLC_FREETEXT_ENTERED, prev_text.c_str());
	}
	else if (FunctionId == FN_MSK_CCDLC_FREETEXT_ENTERED)
	{
		trim(ItemValue);
		if (ItemValue == "") ItemValue = "REMOVE";
		Plugin->Ccdlc->AddCCDLCMessageToQueue(CS, "CMT", remove_char(ItemValue, ':'));
	}
	else if (FunctionId == FN_MSK_CCDLC_REQUEST_CLICKED)
	{
		Plugin->OpenPopupList(area, "CCDLC", 1);
		Plugin->AddPopupListElement("Agree", "", FN_MSK_CCDLC_REPLY_SELECTED);
		Plugin->AddPopupListElement("Refuse", "", FN_MSK_CCDLC_REPLY_SELECTED);
		Plugin->AddPopupListElement("Comment", "", FN_MSK_CCDLC_REPLY_SELECTED);
		Plugin->AddPopupListElement("---", "", FN_NOOP);
		Plugin->AddPopupListElement("Cancel", "", FN_MSK_CCDLC_REPLY_SELECTED);
	}
	else if (FunctionId == FN_MSK_CCDLC_REPLY_SELECTED)
	{
		if (ItemValue == "Agree")
		{
			Plugin->Ccdlc->AddCCDLCMessageToQueue(CS, "APP", "APP");
		}
		else if (ItemValue == "Refuse")
		{
			Plugin->Ccdlc->AddCCDLCMessageToQueue(CS, "REJ", "REJ");
		}
		else if (ItemValue == "Comment")
		{
			// get previously entered text
			string prev_text = data->CCDLCRequest.Text;
			Plugin->OpenPopupEdit(area, FN_MSK_CCDLC_FREETEXT_ENTERED, prev_text.c_str());
		}
		else if (ItemValue == "Cancel")
		{
			Plugin->Ccdlc->AddCCDLCMessageToQueue(CS, "CNL", "CNL");
		}
	}
	else if (FunctionId == FN_MSK_CCDLC_CUSTOM_REQ)
	{
		Plugin->OpenPopupList(area, "CCDLC", 1);
		Plugin->AddPopupListElement("TRS NOW", "", FN_MSK_CCDLC_TXT_ENTERED);
		Plugin->AddPopupListElement("IN CLB", "", FN_MSK_CCDLC_TXT_ENTERED);
		Plugin->AddPopupListElement("IN DES", "", FN_MSK_CCDLC_TXT_ENTERED);
		Plugin->AddPopupListElement("REQ HDG", "", FN_MSK_CCDLC_TXT_ENTERED);
		Plugin->AddPopupListElement("REQ RWY", "", FN_MSK_CCDLC_TXT_ENTERED);
		Plugin->AddPopupListElement("REQ APP", "", FN_MSK_CCDLC_TXT_ENTERED);
		Plugin->AddPopupListElement("REQ STAR", "", FN_MSK_CCDLC_TXT_ENTERED);
		Plugin->AddPopupListElement("UNRESTRICTED", "", FN_MSK_CCDLC_TXT_ENTERED);
		Plugin->AddPopupListElement("REQ TRS", "", FN_MSK_CCDLC_TXT_ENTERED);
		Plugin->AddPopupListElement("-----", "", FN_NOOP);
	}
}

void TagProcessor::ProcessFN_RoutePointAction(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT &area)
{
	auto screen = Plugin->CurrentScreen;
	if (screen == NULL) return;

	auto obj = screen->LastScreenObject["CLICK"];
	if (obj != NULL)
	{
		if (obj->Click)
		{
			if (ItemValue == "MARK POINT")
			{
				Plugin->MarkedPoint = obj->Param1;
			}
			else if (ItemValue == "UNMARK POINT")
			{
				Plugin->MarkedPoint = "";
			}
			else if (ItemValue == "MARKED PT THEN THIS")
			{
				string MP = Plugin->MarkedPoint;

				ExtendedDataRequest req;
				auto p = req.request(CS);

				Plugin->Ccdlc->AddCCDLCSystemMessageToQueue("MRT:" + Plugin->MyId + ":" + CS + ":DCT:" + MP + ":" + obj->Param1);
				p->Route.PointAfterPoint(MP, obj->Param1);

				Plugin->ToCompute->add({ CS, false, false });

				Plugin->MarkedPoint = "";
			}
			else if (ItemValue == "REMOVE WAYPOINT")
			{
				ExtendedDataRequest req;
				auto p = req.request(CS);

				Plugin->Ccdlc->AddCCDLCSystemMessageToQueue("MRT:" + Plugin->MyId + ":" + CS + ":RMP:" + obj->Param1);
				p->Route.DelPointByName(obj->Param1);

				Plugin->ToCompute->add({ CS, false, false });
			}
			else if (ItemValue == "INSERT EXISTING BEFORE")
			{
				Plugin->RouteCursorBeforePoint = obj->Param1;
				Plugin->RouteCursorAfterPoint = "";
				Plugin->DisplayRadarWaypoint = true;
				Plugin->RouteCursorCS = CS;
			}
			else if (ItemValue == "INSERT EXISTING AFTER")
			{
				Plugin->RouteCursorAfterPoint = obj->Param1;
				Plugin->RouteCursorBeforePoint = "";
				Plugin->DisplayRadarWaypoint = true;
				Plugin->RouteCursorCS = CS;
			}
			else if (ItemValue == "INSERT FREE BEFORE" || ItemValue == "INSERT FREE AFTER")
			{
				Plugin->RouteCursorArea = { pt.x - 30, pt.y - 30, pt.x + 30, pt.y + 30 };
				Plugin->ShowRouteCursor = true;
				Plugin->RouteCursorBeforePoint = ItemValue == "INSERT FREE BEFORE" ? obj->Param1 : "";
				Plugin->RouteCursorAfterPoint = ItemValue == "INSERT FREE AFTER" ? obj->Param1 : "";
				Plugin->RouteCursorCS = CS;
				Plugin->RouteCursorPos = pt;
			}
			else if (ItemValue == "ADD TO ROUTE")
			{
				ExtendedDataRequest req;
				auto p = req.request(CS);

				CPosition np = obj->Pos;

				PointDetails ins_pt;
				PointDetails rcp;

				if (Plugin->RouteCursorAfterPoint != "")
				{
					ins_pt = p->Route.AddPointAfterName(obj->Param1, np, Plugin->RouteCursorAfterPoint);
					rcp = p->Route.FindPoint(Plugin->RouteCursorAfterPoint);
				}
				else {
					ins_pt = p->Route.AddPointBeforeName(obj->Param1, np, Plugin->RouteCursorBeforePoint);
					rcp = p->Route.FindPoint(Plugin->RouteCursorBeforePoint);
				}

				if (ins_pt.IsValid() && rcp.IsValid())
				{
					Plugin->Ccdlc->AddCCDLCSystemMessageToQueue("MRT:" + Plugin->MyId + ":" + CS + (Plugin->RouteCursorAfterPoint != "" ? ":AFT:" : ":BEF:") + pt_to_str(&ins_pt) + ":" + pt_to_str(&rcp));
					Plugin->ToCompute->add({ CS, false, false });
				}
				Plugin->DisplayRadarWaypoint = false;
			}
			else if (ItemValue == "SET POINT")
			{
				ExtendedDataRequest req;
				auto p = req.request(CS);

				CPosition np = Plugin->CurrentScreen->ConvertCoordFromPixelToPosition(Plugin->RouteCursorPos);

				PointDetails ins_pt;
				PointDetails rcp;

				if (Plugin->RouteCursorAfterPoint != "")
				{
					ins_pt = p->Route.AddPointAfterName("", np, Plugin->RouteCursorAfterPoint);
					rcp = p->Route.FindPoint(Plugin->RouteCursorAfterPoint);
				}
				else {
					ins_pt = p->Route.AddPointBeforeName("", np, Plugin->RouteCursorBeforePoint);
					rcp = p->Route.FindPoint(Plugin->RouteCursorBeforePoint);
				}

				if (ins_pt.IsValid() && rcp.IsValid())
				{
					Plugin->AllScreenCreatePermanentObject("DCT_PT" + to_string(Plugin->ManualPointIndex), np, false, "PT" + to_string(Plugin->ManualPointIndex), "DCT", &CMskRadarScreen::OnRoutePointEvent);
					Plugin->AllScreenCreatePermanentObject("FIX_PT" + to_string(Plugin->ManualPointIndex), np, false, "PT" + to_string(Plugin->ManualPointIndex), "FIX", &CMskRadarScreen::OnRoutePointEvent);

					Plugin->Ccdlc->AddCCDLCSystemMessageToQueue("MRT:" + Plugin->MyId + ":" + CS + (Plugin->RouteCursorAfterPoint != "" ? ":AFT:" : ":BEF:") + pt_to_str(&ins_pt) + ":" + pt_to_str(&rcp));

					Plugin->ToCompute->add({ CS, false, false });
				}
				Plugin->DisplayRadarWaypoint = false;

				Plugin->ShowRouteCursor = false;
				Plugin->RemoveAllScreenObjects();
			}
			else if (ItemValue == "CANCEL")
			{
				Plugin->ShowRouteCursor = false;
				Plugin->DisplayRadarWaypoint = false;
				Plugin->RemoveAllScreenObjects();
			}
			else if (ItemValue == "RESET ROUTE")
			{
				if (rt.IsValid())
				{
					auto fp = rt.GetCorrelatedFlightPlan();
					if (fp.IsValid())
					{
						Plugin->Ccdlc->AddCCDLCSystemMessageToQueue("MRT:" + Plugin->MyId + ":" + CS + ":RST");

						fp.GetControllerAssignedData().SetAssignedHeading(0);
						fp.GetControllerAssignedData().SetScratchPadString("");
					}
				}
				Plugin->ToCompute->add({ CS, true, true });
			}
			else if (ItemValue == "CLEAR DIRECT")
			{
				if (rt.IsValid())
				{
					auto fp = rt.GetCorrelatedFlightPlan();
					if (fp.IsValid())
					{
						fp.GetControllerAssignedData().SetAssignedHeading(0);
						fp.GetControllerAssignedData().SetScratchPadString("");
					}
				}
				Plugin->ToCompute->add({ CS, false, false });
			}
		}
	}
	if (screen != NULL)
		screen->RequestRefresh();
}

void TagProcessor::ProcessFN_PointReset(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT &area)
{
	if (rt.IsValid())
	{
		auto fp = rt.GetCorrelatedFlightPlan();
		if (fp.IsValid())
		{
			Plugin->Ccdlc->AddCCDLCSystemMessageToQueue("MRT:" + Plugin->MyId + ":" + CS + ":RST");

			fp.GetControllerAssignedData().SetAssignedHeading(0);
			fp.GetControllerAssignedData().SetScratchPadString("");

			Plugin->ToCompute->add({ CS, true, true });
		}
	}
}

void TagProcessor::ProcessFN_Freetext(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT &area)
{
	ExtendedDataRequest req;
	auto p = req.request(CS);

	if (FunctionId == FN_MSK_FREETEXT_ACTIVATE)
	{
		if (Plugin->ImController())
		{
			Plugin->OpenPopupEdit(area, FN_MSK_FREETEXT_SET, p->ActualFormularData.ExtendedData.FreeText.c_str());
		}
	}
	else if (FunctionId == FN_MSK_TOGGLE_EXT)
	{
		p->ActualFormularData.ExtendedData.IsShown = !p->ActualFormularData.ExtendedData.IsShown;
	}
	else if (FunctionId == FN_MSK_TOGGLE_FREETEXT)
	{
		p->ActualFormularData.ExtendedData.ShowFreeText = !p->ActualFormularData.ExtendedData.ShowFreeText;
	}
	else {
		if (Plugin->ImController())
		{
			p->ActualFormularData.ExtendedData.FreeText = ItemValue;
			Plugin->Ccdlc->AddCCDLCMessageToQueue(CS, "FTT", remove_char(ItemValue, ':'));
		}
	}
}