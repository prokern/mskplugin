#include "pch.h"
#include "ExternalDataProcessor.h"
#include "Global.h"

using namespace nlohmann;

void ExternalDataProcessor::__FN()
{
	while (!Terminated)
	{
		this_thread::sleep_for(10ms);

		unordered_map<string, ExternalRequest>::iterator it;

		{
			lock_guard<mutex> lock(__lock);

			it = __pending_requests.begin();
			if (it == __pending_requests.end())
				continue;
			if (it->second.Deleted)
			{
				__pending_requests.erase(it);
				continue;
			}
		}
		if (it->second.Type == EXT_REQ_PILOT)
		{
			ExternalRequest ext;
			ext.ID = it->second.ID;
			ext.Type = it->second.Type;
			ext.Req = it->second.Req;
			__process_pilot_request(ext.Req);
			{
				lock_guard<mutex> lock(__lock);
				__completed_requests[ext.ID] = ext;
				__pending_requests.erase(it);
			}
		}
	}
}

void ExternalDataProcessor::__add_request(ExternalRequest req)
{
	__pending_requests[req.ID] = req;
}

void ExternalDataProcessor::__erase_request(string req)
{
	if (__pending_requests.count(req) > 0)
		__pending_requests[req].Deleted = true;
	if (__completed_requests.count(req) > 0)
		__completed_requests.erase(req);
}

void* ExternalDataProcessor::__find_request(string req)
{
	if (__completed_requests.count(req) > 0)
		return __completed_requests[req].Req;
	return NULL;
}

void * ExternalDataProcessor::__process_pilot_request(void* req)
{
	ExternalRequest_Pilot* pr = (ExternalRequest_Pilot*)req;

	pr->Fetched = true;

	try {
		auto crl = mycurl();
		auto flight_data = crl.get("https://uuwv.ru/stats/scripts/who.php?cs=" + pr->Callsign);
		if (flight_data != "" && flight_data != "ERROR" && flight_data != "NOTFOUND")
		{
			json j = json::parse(flight_data);

			pr->ATCRating = j["rating"];
			pr->PilotRating = j["pilotrating"];
			pr->FullName = string(j["name_first"]) + " " + string(j["name_last"]);
			pr->PilotHours = atoi(string(j["pilottime"]).c_str());
		}
	}
	catch (...)
	{	}
	return req;
}

ExternalDataProcessor::ExternalDataProcessor()
{
	thread TH(&ExternalDataProcessor::__FN, this);
	TH.detach();
}

ExternalDataProcessor::~ExternalDataProcessor()
{
	Terminated = true;
	this_thread::sleep_for(50ms);
	__completed_requests.clear();
	__pending_requests.clear();
}

void ExternalDataProcessor::RequestCallsignInfo(string CS)
{
	lock_guard<mutex> lock(__lock);

	ExternalRequest_Pilot* p = new ExternalRequest_Pilot;
	p->Callsign = CS;
	__add_request({ EXT_REQ_PILOT, "PILOT_" + CS, false, p });
}

void ExternalDataProcessor::CancelCallsignRequest(string CS)
{
	lock_guard<mutex> lock(__lock);
	__erase_request("PILOT_" + CS);
}

ExternalRequest_Pilot ExternalDataProcessor::GetCallsignInfo(string CS)
{
	lock_guard<mutex> lock(__lock);

	try {
		void* req = __find_request("PILOT_" + CS);
		if (req != NULL)
		{
			return *(ExternalRequest_Pilot*)req;
		}
		return ExternalRequest_Pilot();
	}
	catch (...)
	{
		return ExternalRequest_Pilot();
	}

}