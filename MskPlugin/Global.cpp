#include "pch.h"
#include <map>
#include <vector>
#include "Global.h"
#include "ConstraintList.h"
#include "Consts.h"
#include <thread>
#include <mutex>


using namespace std;

std::map<string, int> TRA;
ConstraintList Constraints;
vector<SectorArea> SectorAreas;
vector<RwyThreshold> Rwys;

CMskPlugin * Plugin = NULL;
__DEBUG* __D;