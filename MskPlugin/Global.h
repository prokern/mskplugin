#pragma once
#include <map>
#include <vector>
#include "ConstraintList.h"
#include "Consts.h"
#include <fstream>
#include <ctime>
#include <iomanip>
#include <mutex>

class CCDLC;
class CMskPlugin;

class __DEBUG {
private:
	mutex __lock;
	ofstream outfile;
	map<string, string> __threads;

public:
	void reg(string name)
	{
		__threads[__who()] = name;
		out("THREAD STARTED");
	}
	void out(string s)
	{
		lock_guard<mutex> lock(__lock);
		auto z = outfile.is_open();
		outfile << tm() << " " << __threads[__who()] << ": " << s << "\n";
	}
	string __who()
	{
		auto myid = this_thread::get_id();
		stringstream ss;
		ss << myid;
		string mystring = ss.str();
		return mystring;
	}
	string tm()
	{

		auto t = std::time(nullptr);
		struct tm z;
		localtime_s(&z, &t);
		char buf[32];
		strftime(buf, 32, "%d.%m.%y %H:%M:%S", &z);
		return buf;
	}
	__DEBUG()
	{
		if (outfile.is_open())	return;
		int i = 0;
		while (true)
		{
			i++;
			ifstream f("C:\\Users\\proke\\Desktop\\VSProjects\\MskPlugin\\Debug\\logs\\" + to_string(i) + ".log");
			if (!f.good()) break;

		}
		outfile.open("C:\\Users\\proke\\Desktop\\VSProjects\\MskPlugin\\Debug\\logs\\" + to_string(i) + ".log", std::ios_base::app);
	}
};

extern map<string, int> TRA;
extern ConstraintList Constraints;
extern vector<SectorArea> SectorAreas;
extern vector<RwyThreshold> Rwys;
extern CMskPlugin* Plugin;
extern __DEBUG* __D;