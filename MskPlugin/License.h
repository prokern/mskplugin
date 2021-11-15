#pragma once
#include <string>

using namespace std;

class License
{
public:
	License();
	wstring get_disk_ids();

	string check_file();
	int f;
	string create_challenge(string vatsim_id);
	bool check_response(string vatsim_id, string resp);
};

