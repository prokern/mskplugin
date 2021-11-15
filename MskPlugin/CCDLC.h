#pragma once
#include <mutex>
#include "Consts.h"
#include "AircraftExtendedData.h"
#include <queue>
#include <map>

class CMskPlugin;

class CCDLC
{
private:
	mutex __output;
	mutex __input;
public:
	CCDLC();

	int sss;

	mutex ccdlcLockInput;
	mutex ccdlcLockOutput;
	int ccdlcLastId = 0;
	queue<CCDLC_Message> ccdlcMessages;
	queue<string> ccdlcOutMessages;

	int localport = 0;
	string localip = "";

	inline bool IsRequestCmd(string Cmd)
	{
		return (Cmd == "DCT" || Cmd == "ALT" || Cmd == "SPD" || Cmd == "TXT");
	}
	inline bool IsSuplimentaryCmd(string Cmd)
	{
		return (Cmd == "STS" || Cmd == "LNG" || Cmd == "SDT" || Cmd == "RST");
	}
	inline bool IsSystemCmd(string Cmd)
	{
		return (Cmd == "EXIT" || Cmd == "WELCOME" || Cmd == "CCDLC" || Cmd == "CLIENT" || Cmd == "OFFLINE" || Cmd == "VALID" || Cmd == "EXPIRED");
	}
	inline bool IsAssigmentCmd(string Cmd)
	{
		return !IsRequestCmd(Cmd) && !IsSuplimentaryCmd(Cmd) && !IsSystemCmd(Cmd);
	}
	void AddCCDLCMessageToQueue(string Callsign, string RequestType, string Request);
	void AddCCDLCMessageToQueueIfTracking(string ActiveControllerId, string Callsign, string RequestType, string Request);
	void AddCCDLCSystemMessageToQueue(string Message);
	void AddCCDLCSystemMessageToQueueIfTracking(string ActiveControllerId, string Message);
	void ExecCCDLCMessage(CCDLC_Message msg);
	void InitCCDLCMessage(CCDLC_Message msg, AircraftExtendedData* plane);
	void CancelCCDLCMessage(AircraftExtendedData* plane);
	//Thread
	void CCDLC_Parser_FN();
	void CCDLC_Out_FN();
};

class InputCCDLCRequest
{
	friend class CCDLC;
private:
	mutex __lock;
public:
	InputCCDLCRequest();
	~InputCCDLCRequest();
};

class OutputCCDLCRequest
{
	friend class CCDLC;
private:
	mutex __lock;
public:
	OutputCCDLCRequest();
	void add(string request);
	~OutputCCDLCRequest();
};