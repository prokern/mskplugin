#include "pch.h"
#include "Common.h"
#include "stdlib.h"
#include <openssl/sha.h>
#include <algorithm>
#include <iomanip>

using namespace std;

string formatTime(time_t rawtime) {

	struct tm ptm;
	gmtime_s(&ptm, &rawtime);

	char buffer[6];
	snprintf(buffer, 6, "%02d:%02d", ptm.tm_hour, ptm.tm_min);

	return string(buffer);
}

time_t getCurrentTime() {

	time_t rawtime;
	time(&rawtime);
	return rawtime;
}

bool isInside(GeoPoint pol[], int npol, GeoPoint p)
{
	bool c = false;

	for (int i = 0, j = npol - 1; i < npol; j = i++)
	{

		if ((((pol[i].y <= p.y) && (p.y < pol[j].y)) || ((pol[j].y <= p.y) && (p.y < pol[i].y))) &&
			(((pol[j].y - pol[i].y) != 0) && (p.x > ((pol[j].x - pol[i].x) * (p.y - pol[i].y) / (pol[j].y - pol[i].y) + pol[i].x))))
			c = !c;
	}
	return c;
}


void toClipboard(const string& s) {

	OpenClipboard(NULL);
	EmptyClipboard();
	HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, s.size() + 1);
	if (!hg) {
		CloseClipboard();
		return;
	}
	memcpy(GlobalLock(hg), s.c_str(), s.size() + 1);
	GlobalUnlock(hg);
	SetClipboardData(CF_TEXT, hg);
	CloseClipboard();
	GlobalFree(hg);
}

vector<string> SplitStringByChar(string S, char Delim)
{
	vector<string> res;
	istringstream iss(S);
	string token;
	while (getline(iss, token, Delim)) {
		res.push_back(token);
	}
	return res;
}

string precisionDouble(double value, int width, int prec)
{
	ostringstream out;
	out.width(width);
	out.fill('0');
	out.precision(prec);
	out << value;
	return out.str();
}
string FormatPressureAlt(int PA)
{
	if (PA == 0)	return "";
	else if (PA == 1)	return "CA";

	string s = to_string((int)(roundNearestHundredth(PA) / 100));	//	WARNING??? CHECK
	trim(s);
	if (s.length() < 2)	s = '0' + s;
	return s;
}
string getMach(int m) {
	ostringstream s;
	s << (m / 100.0);
	string str = s.str();
	return str;
}

CPosition BearingTo(CPosition Origin, double bearing, double dist)
{
	const double δ = (dist * 1852) / EARTH_RADIUS;
	const double θ = deg_to_rad(bearing);

	const double φ1 = deg_to_rad(Origin.m_Latitude);
	const double λ1 = deg_to_rad(Origin.m_Longitude);

	const double sinφ2 = sin(φ1) * cos(δ) + cos(φ1) * sin(δ) * cos(θ);
	const double φ2 = asin(sinφ2);
	const double y = sin(θ) * sin(δ) * cos(φ1);
	const double x = cos(δ) - sin(φ1) * sinφ2;
	const double λ2 = λ1 + atan2(y, x);

	const double lat = rad_to_deg(φ2);
	const double lon = rad_to_deg(λ2);

	CPosition ret;
	ret.m_Latitude = lat;
	ret.m_Longitude = lon;
	return ret;
}

string getPathName(const string& s) {

	char sep = '/';

#ifdef _WIN32
	sep = '\\';
#endif
	// TODO 
	size_t i = s.rfind(sep, s.length());
	if (i != string::npos) {
		return(s.substr(0, i));
	}

	return("");
}

void replaceAll(string& str, const string& from, const string& to) {
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

string sha256_string(string s)
{
	unsigned char hash[SHA256_DIGEST_LENGTH];
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, s.c_str(), s.length());
	SHA256_Final(hash, &sha256);
	
	stringstream ss;

	for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
	{
		ss << std::setfill('0') << std::setw(2) << hex << (unsigned int)hash[i];
	}
	return ss.str();
}

string base64_encode(char const* bytes_to_encode, unsigned int in_len) {

	string ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3] = "";
	unsigned char char_array_4[4] = "";

	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; (i < 4); i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];

		while ((i++ < 3))
			ret += '=';

	}

	return ret;

}
string base64_decode(string const& encoded_string) {
	int in_len = encoded_string.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	unsigned char char_array_4[4] = "", char_array_3[3] = "";
	string ret;

	while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i == 4) {
			for (i = 0; i < 4; i++)
				char_array_4[i] = (unsigned char)base64_chars.find(char_array_4[i]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
				ret += char_array_3[i];
			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 4; j++)
			char_array_4[j] = 0;

		for (j = 0; j < 4; j++)
			char_array_4[j] = (unsigned char)base64_chars.find(char_array_4[j]);

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
	}

	return ret;
}

inline string ints_to_hex(unsigned int data[], int len)
{
	std::stringstream stream;
	for (int i = 0; i < len; i++)
	{
		stream << std::hex << data[i];
	}
	return string(stream.str());
}

string builds(int cnt, ...)
{
	va_list ap;
	va_start(ap, cnt);
	string ret;
	for (int i = 0; i < cnt; i++)
	{
		ret += va_arg(ap, string);
	}
	va_end(ap);
	return ret;
}
