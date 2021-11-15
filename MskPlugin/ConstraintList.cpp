#include "pch.h"
#include "ConstraintList.h"
#include <fstream>
#include <algorithm>
#include <vector>
#include <sstream>

using sidstar = string;
using pointname = string;

ConstraintList::ConstraintList()
{
	this->Count = 0;
}


ConstraintList::~ConstraintList()
{

}

bool ConstraintList::SameRouteName(string AIRAC, string ES) 
{
	//	OLMU1F (AIRAC) ->  OLMUN1KxILSY24L
	//return regex_match(ES, regex(AIRAC.substr(0,4)
	
	if (AIRAC.length() == 6) {
		// ARINC 424 6-length.
		return (ES.substr(0, 4) == AIRAC.substr(0, 4)) && (ES.substr(5, 2) == AIRAC.substr(4, 2));
	}
	else return AIRAC == ES;
}

string ConstraintList::FindConstraint(string SidStar, string Point)
{
	if (this->Constraints.count(SidStar) == 0) {

		// check for sid/star
		for (map < sidstar, map<string, Constraint>>::iterator it = Constraints.begin(); it != Constraints.end(); it++) {
			if (SameRouteName(it->first, SidStar)) {
				this->Constraints.insert({ SidStar, it->second });
				return FindConstraint(SidStar, Point);
			}
		}
		return "";
	}
	if (this->Constraints.at(SidStar).count(Point) == 0) {
		return "";
	}
	return this->Constraints.at(SidStar).at(Point).Value;
}

bool ConstraintList::APInList(string AP)
{
	return std::find(Ports.begin(), Ports.end(), AP) != Ports.end();
}

void ConstraintList::Clear()
{
	Count = 0;
	Ports.clear();
	Constraints.clear();
}

int ConstraintList::LoadFromFile(std::string FileName) 
{
	try {
		this->Count = 0;

		std::ifstream file(FileName);
		std::string str;

		while (std::getline(file, str))
		{
			std::replace(str.begin(), str.end(), ':', ' ');  // replace ':' by ' '

			std::vector<std::string> array;
			std::stringstream ss(str);
			std::string temp;

			while (ss >> temp) {
				array.push_back(temp);
			}
			if (array.size() != 5) {
				continue;
			}
			if (array[1] != "SID" && array[1] != "STAR") {
				continue;
			}
			
			auto cnst = Constraint{ array[0], array[1], array[2], array[3], array[4] };

			if (this->Constraints.count(array[2]) == 0) {	// no such sidstar

				this->Constraints.insert({ array[2], map<string,Constraint>{} });
			}
			if (this->Constraints.at(array[2]).count(array[3]) == 0) { // no such point
				this->Constraints.at(array[2]).insert({ array[3],cnst });
			}
			this->Ports.push_back(array[0]);
			this->Count++;
		}
		Ports.erase(std::unique(Ports.begin(), Ports.end()), Ports.end());

		return this->Count;
	}
	catch (const std::exception & exc) {
		(void)exc;
		return -1;
	}
	catch (const std::string & ex) {
		(void)ex;
		return -1;
	}
	catch (...) {
		return -1;
	}
}
