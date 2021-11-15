#include "pch.h"
#include "AFV.h"
#include "Global.h"
#include "Common.h"
#include "CMskPlugin.h"

LRESULT CALLBACK AFV_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_COPYDATA: {
		COPYDATASTRUCT* data = reinterpret_cast<COPYDATASTRUCT*>(lParam);

		if (data != nullptr && data->dwData == 666 && data->lpData != nullptr)
		{
			string msg = reinterpret_cast<const char*>(data->lpData);
			Plugin->Afv->AddAFVMessageToQueue(msg);
		}
		return TRUE;
	}
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK AFV_CallWndProc(
	_In_ int    nCode,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
)
{
	if (nCode == HC_ACTION) {
		//lets extract the data
		CWPSTRUCT* data = (CWPSTRUCT*)lParam;

	//	if (data->message == WM_CREATE) {
		//	if (mskPlugin == nullptr) {
		//		mskPlugin = reinterpret_cast<CMskPlugin*>(reinterpret_cast<CREATESTRUCT*>(data->lParam)->lpCreateParams);
		//	}
		//}
		if (data->message == WM_COPYDATA) {

			COPYDATASTRUCT* localdata = reinterpret_cast<COPYDATASTRUCT*>(data->lParam);

			if (localdata != nullptr && localdata->dwData == 666 && localdata->lpData != nullptr) {

				string msg = reinterpret_cast<const char*>(localdata->lpData);
				Plugin->Afv->AddAFVMessageToQueue(msg);
			}
		}
	}
	return CallNextHookEx(0, nCode, wParam, lParam);
}

void AFV::__FN_AFV()
{
	while (!Plugin->Terminating)
	{
		this_thread::sleep_for(200ms);

		lock_guard<mutex> lock(__afv_lock);

		while (__messages.size() != 0) {
			
			string message = __messages.front();

			if (message.empty()) {

				// Marks end of transmission
				if (!activeTransmittingPilots.empty())
				{
					previousActiveTransmittingPilots = activeTransmittingPilots;
					activeTransmittingPilots.clear();
				}
			}
			else {
				auto at = SplitStringByChar(message, ':');
				for (auto& x : at)
				{
					auto rt = Plugin->RadarTargetSelect(x.c_str());
					if (rt.IsValid())
					{
						activeTransmittingPilots.insert(x);
					}
					else
					{
						auto obs = safe_substr(x, 0, x.size() - 1);	// observers
						rt = Plugin->RadarTargetSelect(obs.c_str());
						if (rt.IsValid())
						{
							activeTransmittingPilots.insert(obs);
						}
					}
				}
			}
			__messages.pop();
		}
	}
}

void AFV::AddAFVMessageToQueue(string message)
{
	lock_guard<mutex> lock(__afv_lock);
	__messages.push(message);
}

bool AFV::IsTransmit(string CS)
{
	lock_guard<mutex> lock(__afv_lock);
	return (activeTransmittingPilots.find(CS) != activeTransmittingPilots.end()) || (previousActiveTransmittingPilots.find(CS) != previousActiveTransmittingPilots.end());
}

bool AFV::IsActiveTransmit(string CS)
{
	lock_guard<mutex> lock(__afv_lock);
	return activeTransmittingPilots.find(CS) != activeTransmittingPilots.end();
}

string AFV::GetPilotsForDraw()
{
	lock_guard<mutex> lock(__afv_lock);
	string ret;
	if (activeTransmittingPilots.size() > 0)
	{
		for (auto it = activeTransmittingPilots.begin(); it != activeTransmittingPilots.end(); it++)
		{
			ret += *it + " ";
		}
	}
	else {
		for (auto it = previousActiveTransmittingPilots.begin(); it != previousActiveTransmittingPilots.end(); it++)
		{
			ret += *it + " ";
		}
	}
	return ret;
}

AFV::AFV()
{
	WNDCLASS windowClass = {
	   NULL,
	   AFV_WndProc,
	   NULL,
	   NULL,
	   GetModuleHandle(NULL),
	   NULL,
	   NULL,
	   NULL,
	   NULL,
	   "RDFHiddenWindowClass"
	};

	RegisterClass(&windowClass);
	auto Wnd = FindWindow("RDFHiddenWindowClass", "RDFHiddenWindow");
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (Wnd != NULL) {

		__globalAFVHook = SetWindowsHookExA(WH_CALLWNDPROC, &AFV_CallWndProc, AfxGetInstanceHandle(), 0);
		if (__globalAFVHook == NULL) {
			auto err = GetLastError();
			Plugin->DisplayUrgent("Failed to setup hook for RDF window. Error: " + to_string(err));
		}
		else Plugin->DisplaySilent("Hook established");
	}
	else {
		__hiddenWindow = CreateWindow(
			"RDFHiddenWindowClass",
			"RDFHiddenWindow",
			NULL,
			0,
			0,
			0,
			0,
			NULL,
			NULL,
			GetModuleHandle(NULL),
			reinterpret_cast<LPVOID>(this)
		);

		if (GetLastError() != S_OK) {
			Plugin->DisplayUrgent("Unable to create window for AFV communication");
		}
	}
	thread __t(&AFV::__FN_AFV, this);
	Plugin->ThreadRunning++;
	__t.detach();
}

AFV::~AFV()
{
	if (__hiddenWindow != NULL) {
		DestroyWindow(__hiddenWindow);
	}
	if (__globalAFVHook != NULL) {
		UnhookWindowsHookEx(__globalAFVHook);
	}
}
