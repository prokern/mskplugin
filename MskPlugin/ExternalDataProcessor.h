#pragma once
#include <mutex>
#include <unordered_map>
#include <thread>
#include "mycurl.h"
#include "json.h"
#include "Consts.h"

using namespace std;

class ExternalDataProcessor
{
private:

	void __FN();
	mutex __lock;
	unordered_map<string,ExternalRequest> __pending_requests;
	unordered_map<string,ExternalRequest> __completed_requests;


	void __add_request(ExternalRequest req);
	void __erase_request(string req);
	void* __find_request(string req);
	

	void* __process_pilot_request(void* req);

public:
	bool Terminated = false;

	ExternalDataProcessor();
	~ExternalDataProcessor();

	void RequestCallsignInfo(string CS);
	void CancelCallsignRequest(string CS);
	ExternalRequest_Pilot GetCallsignInfo(string CS);
};

