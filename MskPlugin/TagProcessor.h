#pragma once
#include <map>
#include <string>
#include "Consts.h"
#include "EuroScopePlugIn.h"

using namespace std;
using namespace EuroScopePlugIn;

class CMskPlugin;
class TagProcessor;
class AircraftExtendedDataPool;

class TagProcessor
{
private:
	map<int, TagItemParser> __tags;
	map<int, TagFunctionParser> __funcs;
	CMskPlugin * plg = NULL;
public:
	TagProcessor();

	void AddTagProcessor(int TagId, bool HighLightWhenTX, bool OverrideColorEvenIfTX, string Name, TagItemParserFN Handler);
	void AddFuncProcessor(int FuncId, string Name, TagFuncParserFN Handler);
	
	COLORREF PortToColor(string Port);
	COLORREF PortToColorForPointer(string Port);

	TagItemParser* GetTag(int TagId);
	TagFunctionParser* GetFN(int FnId);

	// Tag handlers
	void ProcessConstraint(int &ItemCode, string &CS, string& tag, COLORREF& color);
	void ProcessDeparturePort(int &ItemCode, string &CS, string& tag, COLORREF& color);
	void ProcessDestinationPort(int &ItemCode, string &CS, string& tag, COLORREF& color);
	void ProcessSTAR(int &ItemCode, string &CS, string& tag, COLORREF& color);
	void ProcessSID(int &ItemCode, string &CS, string& tag, COLORREF& color);
	void ProcessLastXmt(int &ItemCode, string &CS, string& tag, COLORREF& color);
	void ProcessArea(int &ItemCode, string &CS, string& tag, COLORREF& color);
	void ProcessSts(int &ItemCode, string &CS, string& tag, COLORREF& color);
	void ProcessFinalFixAlt(int &ItemCode, string &CS, string& DisplayValue, COLORREF& DisplayColor);
	void ProcessTTL(int &ItemCode, string &CS, string& tag, COLORREF& color);
	void ProcessAman(int &ItemCode, string &CS, string& tag, COLORREF& color);
	void ProcessCallsign(int &ItemCode, string &CS, string& tag, COLORREF& color);
	void ProcessRunway(int &ItemCode, string &CS, string& tag, COLORREF& color);
	void ProcessFL(int &ItemCode, string &CS, string& tag, COLORREF& color);
	void ProcessSpeed(int &ItemCode, string &CS, string& tag, COLORREF& color);
	void ProcessLang(int &ItemCode, string &CS, string& tag, COLORREF& color);
	void ProcessHighGrad(int &ItemCode, string &CS, string& tag, COLORREF& color);
	void ProcessTimes(int &ItemCode, string &CS, string& tag, COLORREF& color);
	void ProcessRestriction(int &ItemCode, string &CS, string& tag, COLORREF& color);
	void ProcessCCDLC(int &ItemCode, string &CS, string& DisplayValue, COLORREF& DisplayColor);
	void ProcessManualStatus(int &ItemCode, string &CS, string& tag, COLORREF& color);
	void ProcessAcftIndex(int &ItemCode, string &CS, string& tag, COLORREF& color);
	void ProcessExtendedData(int &ItemCode, string &CS, string& tag, COLORREF& color);

	// Tag function handlers
	// Function processors
	void ProcessFN_DisplayRoute(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT  &area);
	void ProcessFN_ShowInterval(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT  &area);
	void ProcessFN_AmanPoint(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT  &area);
	void ProcessFN_AmanPointSelect(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT  &area);
	void ProcessFN_TTL(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT  &area);
	void ProcessFN_TTL_Select(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT  &area);
	void ProcessFN_ManualStatus(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT  &area);
	void ProcessFN_ManualStatus_Selected(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT  &area);
	void ProcessFN_CallsignMenu(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT  &area);
	void ProcessFN_LangSelected(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT  &area);
	void ProcessFN_RestSelected(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT  &area);
	void ProcessFN_CenterTarget(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT  &area);
	void ProcessFN_CCDLC(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT  &area);
	void ProcessFN_SwitchSpeed(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT  &area);
	void ProcessFN_TxtMsg(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT  &area);
	void ProcessFN_RoutePointAction(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT  &area);
	void ProcessFN_PointReset(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT  &area);
	void ProcessFN_Freetext(int &FunctionId, string &CS, CFlightPlan &fp, CRadarTarget &rt, string &ItemValue, POINT &pt, RECT  &area);
};

