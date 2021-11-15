#include "pch.h"
#include "mycurl.h"
#include <curl/curl.h>

size_t writeFunction(void* ptr, size_t size, size_t nmemb, std::string* data) {
	data->append((char*)ptr, size * nmemb);
	return size * nmemb;
}

string mycurl :: escape(string s)
{
	try {
		curl_global_init(CURL_GLOBAL_DEFAULT);
		string ret;

		auto curl = curl_easy_init();
		if (curl) {
			
			const char * f = curl_easy_escape(curl, s.c_str(), 0);
			if (f != NULL)
			{
				ret = string(f);
			}
			curl_easy_cleanup(curl);
			curl = NULL;
		}
		return ret;
	}
	catch (...) {
		return "";
	}
}

string mycurl::get(string url) {

	try {
		curl_global_init(CURL_GLOBAL_DEFAULT);

		auto curl = curl_easy_init();
		if (curl) {
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
			curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
			curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

			std::string response_string;
			//std::string header_string;
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
			//	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_string);

				//char* url;
				//long response_code;
			//	double elapsed;
				//curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
				//curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &elapsed);
			//	curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &url);

			curl_easy_perform(curl);
			curl_easy_cleanup(curl);
			curl = NULL;
			return response_string;
		}
		return "";
	}
	catch (...) {
		return "";
	}
}