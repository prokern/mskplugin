#pragma once
#include <mutex>
#include <map>

using namespace std;

struct AMANPointEntry {

	string CS = "";
	string Point = "";
	string Time = "";
	double Dist = 0;
	bool Warn = false;
};

class AMAN
{
private:
	mutex __aman_lock;
	map<string, AMANPointEntry> __points;
	void __AMAN_FN();
public:
	AMAN();
	AMANPointEntry get(string cs);
	void put(string CS, string PointName);
	void del(string CS);
	bool UseAman = false;
};

