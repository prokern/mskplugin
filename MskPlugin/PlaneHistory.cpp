#include "pch.h"
#include "PlaneHistory.h"

bool PlaneHistory::Exists(string CS)
{
	lock_guard<mutex> lock(__histrory_locker);
	return __TrackedPlanes.count(CS) > 0;
}

void PlaneHistory::CleanUp()
{
	lock_guard<mutex> lock(__histrory_locker);
	auto it = __TrackedPlanes.begin();
	while (it != __TrackedPlanes.end())
	{
		if (time_diff_now(it->second) > 1200)
		{
			// exceeded 20 minutes
			it = __TrackedPlanes.erase(it);
		}
		else ++it;
	}
}

void PlaneHistory::AddPlane(string CS)
{
	if (Exists(CS))
		return;
	lock_guard<mutex> lock(__histrory_locker);
	__TrackedPlanes[CS] = getCurrentTime();
	TrackedCount++;
}

