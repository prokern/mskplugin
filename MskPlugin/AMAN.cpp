#include "pch.h"
#include "AMAN.h"
#include "Global.h"
#include "CMskPlugin.h"
#include <thread>

void AMAN::__AMAN_FN()
{
	int tick = 0;

	while (!Plugin->Terminating)
	{
		if (tick == 400)	// 4 seconds
		{
			tick = 0;
			lock_guard<mutex> lock(__aman_lock);

			time_t initTime = getCurrentTime();

			map<pair<string, string>, int> SameTime;

			auto it = __points.begin();

			while (it != __points.end())
			{
				auto& entry = *it;
				auto cs = entry.first;
				auto point = &entry.second;

				ExtendedDataRequest req;

				if (!req.check(cs))
				{
					// obsoletes
					it = __points.erase(it);
					continue;
				}
				else {
					auto data = req.request(cs);

					auto details = data->Route.FindPoint(it->second.Point);
					if (details.IsValid())
					{
						if (details.Distance < 1)
						{
							it = __points.erase(it);
							continue;
						}
						else {
							point->Dist = details.Distance;
							point->Time = formatTime(details.ExpectedPassTime);
							point->Warn = false;

							if (SameTime.count({ point->Point, point->Time }) == 0)
								SameTime.insert({ { point->Point, point->Time }, 1 });
							else SameTime.at({ point->Point, point->Time }) = SameTime.at({ point->Point, point->Time }) + 1;
						}
					}
				}
				it++;
			}
			// find same times
			for (auto it = __points.begin(); it != __points.end(); it++)
			{
				if (SameTime.count({ it->second.Point, it->second.Time }) > 0 && SameTime.at({ it->second.Point, it->second.Time }) > 1) {
					it->second.Warn = true;
				}
			}
		}
		this_thread::sleep_for(10ms);
		tick += 1;
	}
	Plugin->ThreadRunning--;
}

AMAN::AMAN()
{
	thread AMAN(&AMAN::__AMAN_FN, this);
	Plugin->ThreadRunning++;
	AMAN.detach();
}

AMANPointEntry AMAN::get(string cs)
{
	lock_guard<mutex> lock(__aman_lock);
	if (__points.count(cs) > 0)
		return __points[cs];
	return AMANPointEntry();
}

void AMAN::put(string CS, string PointName)
{
	lock_guard<mutex> lock(__aman_lock);
	__points[CS] = { CS, PointName, "WAIT", 0.0, false };
}

void AMAN::del(string CS)
{
	lock_guard<mutex> lock(__aman_lock);
	if (__points.count(CS) > 0)
		__points.erase(CS);
}
