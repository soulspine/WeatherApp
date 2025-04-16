#ifndef WEATHER_APP_H
#define WEATHER_APP_H

#include <windows.h>
#include <ImageHlp.h>
#pragma comment(lib, "Imagehlp.lib")
#include <iostream>
#include <direct.h>
#include <string>
#include "Structs.h"
#include <cpr/cpr.h>
#include "json.hpp"
#include <iostream>


using json = nlohmann::json;
using namespace std;

namespace WeatherApp {

	class App {
		public:
			App();
		private:
			unordered_map<string, Station> stations;
			vector<string> cachedMiasta;
			vector<string> cachedWojewodztwa;
			vector<string> cachedGminy;
			vector<string> cachedPowiaty;

			const string BASE_URL = "https://api.gios.gov.pl/pjp-api/v1/rest/";

			void fetchStationDataAndCacheIt();
			cpr::Response requestGet(string endpoint, bool ignoreBaseUrl = false);
			string wcharToString(const wchar_t* wchar);
            vector<string> removeVectorDuplicates(const vector<string>& input);
			string getJsonString(const nlohmann::json& obj, const std::string& key, const std::string& defaultValue = "null");
	};
}

#endif