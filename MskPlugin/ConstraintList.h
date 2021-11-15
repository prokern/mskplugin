#pragma once
#include <string>
#include <list>
#include <map>

using namespace std;
using sidstar = string;
using pointname = string;

struct Constraint {

	string ICAO;
	string Type;
	string Name;
	string Waypoint;
	string Value;
};

class ConstraintList
{
public:
	ConstraintList();
	~ConstraintList();

	list<string> Ports;
	map < sidstar, map<string,Constraint>> Constraints;

	int Count;

	int LoadFromFile(string FileName);
	bool SameRouteName(string V1, string V2);
	string FindConstraint(string SidStar, string Point);

	bool APInList(string AP);

	void Clear();

};

