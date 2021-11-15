#pragma once
#include <string>
#include <vector>
#include "Consts.h"
#include <sstream>
#include <openssl/sha.h>
#include <algorithm>
#include <chrono>
#include <thread>
#include <regex>
#include "vector.h"

using namespace EuroScopePlugIn;
using namespace std;

static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

struct Point {
	double x = 0;
	double y = 0;
};

string formatTime(time_t rawtime);
time_t getCurrentTime();

inline bool regex_string_match(string pattern, string haystack)
{
	return regex_match(haystack, regex(pattern));
}

inline bool valid_icao(string icao)
{
	return regex_string_match("^[A-Z]{4}$", icao);
}

inline string check_str_create(const char* s)
{
	return s == NULL ? "" : string(s);
}

inline double normAngle(int x) {
	x = (int)fmod(x, 360);
	if (x < 0)
		x += 360;
	return x;
}

inline string bth(bool a)
{
	return a ? "1" : "0";
}

inline bool ccw(GeoPoint A, GeoPoint B, GeoPoint C)
{
	return (C.y - A.y) * (B.x - A.x) > (B.y - A.y) * (C.x - A.x);
}

inline int calcAltAt(int alt, double distance, int gs, int vs)
{
	double time_to_go_min = (distance / (gs + 0.001) * 60);
	auto ret = alt + (time_to_go_min * vs);
	return ret > 0 ? (int)ret : 0;
}
inline double calcAltDist(int current_alt, int req_alt, int gs, int vs)
{
	return gs / 60 * abs(current_alt - req_alt) / abs(vs);
}

inline double rad_to_deg(double rad)
{
	return rad * 180.0 / PI;
}

inline double deg_to_rad(double deg)
{
	return (deg * PI / 180.0);
}

inline double calcReqGrad(int alt, double distance)
{
	return rad_to_deg(atan((alt / (distance * 6076))));
}

inline int setbit(int a, int b)
{
	return a & b;
}

inline int clearbit(int a, int b)
{
	return a ^ b;
}

inline time_t time_diff_now(time_t from)
{
	return getCurrentTime() - from;
}

inline int isbit(int a, int b)
{

}

inline string get_str_vector(vector<string> src, int pos)
{
	return (src.size() - 1) >= (unsigned int)pos ? src[pos] : "---";
}

inline double getGrad(int vs, int gs)
{
	return rad_to_deg(asin(abs(vs) / (101 * gs)));
}
inline string uniq_id()
{
	chrono::milliseconds ms = chrono::duration_cast<chrono::milliseconds>(
		chrono::system_clock::now().time_since_epoch()
		);

	return to_string(ms.count());
}

inline bool IsEvenLevel(int level)
{
	return (level % 2000) == 0;
}

bool isInside(GeoPoint pol[], int npol, GeoPoint p);
void toClipboard(const std::string& s);

vector<string> SplitStringByChar(string S, char Delim);

static inline void ltrim(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
		return !std::isspace(ch);
		}));
}

static inline void rtrim(std::string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
		}).base(), s.end());
}

static inline void trim(std::string& s) {
	ltrim(s);
	rtrim(s);
}
static inline void rm_crlf(string& s)
{
	s.erase(remove_if(s.begin(), s.end(), [](const unsigned char t) { return t == '\n' || t == '\r'; }), s.end());
}
inline string remove_char(string s, char w)
{
	s.erase(remove(s.begin(), s.end(), w), s.end());
	return s;
}
inline int heading_diff(double actual_heading, double req_heading)
{
	return abs(((int)actual_heading - (int)req_heading + 180 + 360) % 360 - 180);
}
inline bool heading_in_range(double actual_heading, double req_heading, int range)
{
	int anglediff = abs(((int)actual_heading - (int)req_heading + 180 + 360) % 360 - 180);
	return heading_diff(actual_heading, req_heading) <= range;
}

inline double roundNearestHundredth(int d)
{
	int d_i = d;
	return ((d_i % 100) < 50) ? d_i - (d_i % 100) : d_i + (100 - (d_i % 100));
}

inline double roundNearestTenth(int d)
{
	int d_i = d;
	return ((d_i % 10) < 5) ? d_i - (d_i % 10) : d_i + (10 - (d_i % 10));
}

string FormatPressureAlt(int PA);
string precisionDouble(double value, int width, int prec);
string getMach(int m);
string getPathName(const string& s);

inline string bts(bool v)
{
	return v ? "1" : "0";
}
inline bool stb(string s)
{
	return s == "1" ? true : false;
}

inline string pt_to_str(PointDetails * pt)
{
	return pt->Name + "@" + bts(pt->Manual) + "@" + bts(pt->Free) + "@" + precisionDouble(pt->Position.m_Latitude, 3, 8) + "@" + precisionDouble(pt->Position.m_Longitude, 3, 8);
}
inline bool str_to_pt(string s, PointDetails * pt)
{
	auto v = SplitStringByChar(s, '@');
	if (v.size() != 5)	return false;

	pt->Name = v[0];
	pt->Manual = stb(v[1]);
	pt->Free = stb(v[2]);
	pt->Position.m_Latitude = atof(v[3].c_str());
	pt->Position.m_Longitude = atof(v[4].c_str());

	return true;
}

inline string bth(bool a);

void replaceAll(std::string& str, const std::string& from, const std::string& to);

CPosition BearingTo(CPosition Origin, double bearing, double dist);

string sha256_string(string s);

static inline bool is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

string base64_encode(char const* bytes_to_encode, unsigned int in_len);
string base64_decode(std::string const& encoded_string);

inline void to_upper(string& s)
{
	std::transform(s.begin(), s.end(), s.begin(),
		[](unsigned char c) -> unsigned char { return std::toupper(c); });
}

inline string ints_to_hex(unsigned int data[], int len);

inline POINT PointTo(POINT origin, double distance, double deg_angle)
{
	POINT ret = POINT();
	ret.x = (int)(origin.x + distance * cos(deg_to_rad(deg_angle)));
	ret.y = (int)(origin.y + distance * sin(deg_to_rad(deg_angle)));

	return ret;
}

void inline padTo(std::string& str, const size_t num, const char paddingChar)
{
	if (num > str.size())
		str.insert(0, num - str.size(), paddingChar);
}

inline bool doIntersect(GeoPoint A, GeoPoint B, GeoPoint C, GeoPoint D)
{
	return (ccw(A, C, D) != ccw(B, C, D)) && (ccw(A, B, C) != ccw(A, B, D));
}

inline GeoPoint cp_to_p(CPosition p)
{
	return { p.m_Latitude, p.m_Longitude };
}

inline bool insidePolygon(vector<GeoPoint> polygon, GeoPoint testPoint)
{
	bool result = false;

	int j = polygon.size() - 1;
	for (size_t i = 0; i < polygon.size(); i++)
	{
		if (polygon[i].y < testPoint.y && polygon[j].y >= testPoint.y || polygon[j].y < testPoint.y && polygon[i].y >= testPoint.y)
		{
			if (polygon[i].x + (testPoint.y - polygon[i].y) / (polygon[j].y - polygon[i].y) * (polygon[j].x - polygon[i].x) < testPoint.x)
			{
				result = !result;
			}
		}
		j = i;
	}
	return result;
}

inline bool FindStrVector(vector<string>* v, string s)
{
	return (std::find(v->begin(), v->end(), s) != v->end());
}

inline string CPositionToStr(CPosition c)
{
	return to_string(c.m_Latitude) + " " + to_string(c.m_Longitude);
}

inline void StrToCPosition(CPosition* p, string s)
{
	auto v = SplitStringByChar(s, ' ');
	p->m_Latitude = stod(v[0]);
	p->m_Longitude = stod(v[1]);
}

inline bool face_180(double hdg, double target)
{
	auto anglediff = ((int)hdg - (int)target + 180 + 360) % 360 - 180;
	return (anglediff <= 90 && anglediff >= -90);
}

inline string __who()
{
	auto myid = this_thread::get_id();
	stringstream ss;
	ss << myid;
	string mystring = ss.str();

	return mystring;
}

inline void debug(string s)
{
	OutputDebugString(("Thread " + __who() + ": " + s + "\r\n").c_str());
}

inline bool DoubleIsZero(double d)
{
	return fabs(d) < 1e-10;
}

string builds(int cnt, ...);