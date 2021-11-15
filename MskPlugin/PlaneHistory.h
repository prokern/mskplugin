#pragma once
#include "Global.h"
#include <atomic>
#include <mutex>
#include "Common.h"

using namespace std;

class PlaneHistory
{
private:
	map<string, time_t> __TrackedPlanes;
	mutex __histrory_locker;
public:
	// callsign, last_seen_time
	
	atomic<int> TrackedCount = 0;
	atomic<int> HandOff_In = 0;
	atomic<int> HandOff_Out = 0;

	bool Exists(string CS);
	void CleanUp();
	void AddPlane(string CS);
};

