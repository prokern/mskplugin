#include "pch.h"
#include "License.h"
#include "Global.h"
#include "CMskPlugin.h"
#pragma comment(lib, "wbemuuid.lib")
#include <Wbemidl.h>
#include <comdef.h>
#include <locale>
#include <codecvt>

string ws_to_s(wstring ws)
{
	//return string(ws.begin(), ws.end());
	std::wstring string_to_convert = ws;

	using convert_type = codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_type, wchar_t> converter;

	std::string converted_str = converter.to_bytes(string_to_convert);

	return  converted_str;
}

string License::check_file()
{
	ifstream keyfile;
	keyfile.open(Plugin->MyPath + "lic.dat");
	string vatsim_id;
	string key;
	getline(keyfile, vatsim_id);
	getline(keyfile, key);
	if (vatsim_id == "" || key == "")
	{
		Plugin->DisplayUrgent("All rights on usage of that plugin are reserved. Unauthorized usage is forbidden");
		Plugin->DisplayUrgent("Please register according to established order");
		return "XYZ";
	}
	else {
		if (check_response(vatsim_id, key))
		{
			Plugin->MyCid = vatsim_id;
			return "XAZ";
		}
		else {
			Plugin->DisplayUrgent("All rights on usage of that plugin are reserved. Unauthorized usage is forbidden");
			Plugin->DisplayUrgent("Invalid license file");
			return "XYZ";
		}
	}
}

string License::create_challenge(string vatsim_id)
{
	char buf[65] = "";
	wstring sid = get_disk_ids();
	string f = ws_to_s(sid) + vatsim_id + slt;
	auto res = sha256_string(f).substr(0, 20);
	to_upper(res);
	return res;
}

bool License::check_response(string vatsim_id, string resp)
{
	string correct = sha256_string(create_challenge(vatsim_id) + vatsim_id + slt).substr(0, 20);
	to_upper(correct);
	return resp == correct;
}

License::License()
{
	int zzz = 1;
}

wstring License::get_disk_ids()
{
	HRESULT hres;
	wstring cv;

	hres = CoInitialize(0);
	if (FAILED(hres))
	{
		return wstring();
	}

	IWbemLocator* pLoc = NULL;

	hres = CoCreateInstance(
		CLSID_WbemLocator,
		0,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, (LPVOID*)&pLoc);

	if (FAILED(hres))
	{
		CoUninitialize();
		return wstring();
	}

	IWbemServices* pSvc = NULL;

	hres = pLoc->ConnectServer(
		_bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
		NULL,                    // User name. NULL = current user
		NULL,                    // User password. NULL = current
		0,                       // Locale. NULL indicates current
		NULL,                    // Security flags.
		0,                       // Authority (for example, Kerberos)
		0,                       // Context object 
		&pSvc                    // pointer to IWbemServices proxy
	);

	if (FAILED(hres))
	{
		pLoc->Release();
		CoUninitialize();
		return wstring();
	}

	hres = CoSetProxyBlanket(
		pSvc,                        // Indicates the proxy to set
		RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
		RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
		NULL,                        // Server principal name 
		RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
		RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
		NULL,                        // client identity
		EOAC_NONE                    // proxy capabilities 
	);

	if (FAILED(hres))
	{
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return wstring();
	}

	IEnumWbemClassObject* pEnumerator = NULL;
	hres = pSvc->ExecQuery(
		bstr_t("WQL"),
		bstr_t("SELECT SerialNumber FROM Win32_PhysicalMedia"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL,
		&pEnumerator);

	if (FAILED(hres))
	{
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return wstring();
	}

	IWbemClassObject* pclsObj = NULL;
	ULONG uReturn = 0;

	while (pEnumerator)
	{
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
			&pclsObj, &uReturn);

		if (0 == uReturn)
		{
			break;
		}

		VARIANT vtProp;
		VariantInit(&vtProp);

		hr = pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0);
		auto bs = vtProp.bstrVal;

		//assert(bs != nullptr);
		cv += wstring(bs, SysStringLen(bs));
		VariantClear(&vtProp);
		pclsObj->Release();
	}

	pSvc->Release();
	pLoc->Release();
	pEnumerator->Release();
	CoUninitialize();

	return cv;
}