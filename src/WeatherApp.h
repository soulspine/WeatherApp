#pragma execution_character_set("utf-8")
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
#include <shlobj.h>
#include <filesystem>


using json = nlohmann::json;
using namespace std;

namespace WeatherApp {

	class App {
	public:
		App();

		bool GetDataForSensorByDaysBack(const INT64& sensorId, json& out, const int& daysCount);

		bool GetDataForStationByDaysBack(const INT64& stationId, json& out, const int& daysCount);
		bool GetDataForStationByDaysBack(const string& stationCode, json& out, const int& daysCount);

		vector<Station> GetCachedStations();
		SensorReading GetLastSensorReading(INT64 sensorId);

	private:
		StationCache stationCache;

		const LPCWSTR ERRORMSG_REQUEST_FAILED = L"Nie można zrealizować żądania. Sprawdź połączenie z internetem lub spróbuj ponownie później.";
		const LPCWSTR ERRORMSG_FILE_OPEN_FAILED = L"Nie można otworzyć pliku z danymi stacji.";
		const string BASE_URL = "https://api.gios.gov.pl/pjp-api/v1/rest/";
		filesystem::path appDataPath;

		void loadSavedStationData();
		void fetchStationDataAndSaveIt();
		void setUpAppDataFolder();


		cpr::Response _requestGet(string endpoint, bool ignoreBaseUrl = false);
		string _wcharToString(const wchar_t* wchar);
		vector<string> _removeVectorDuplicates(const vector<string>& input);
		string _getJsonString(const json& obj, const string& key, const string& defaultValue = "");
		void _showErrorBox(LPCWSTR message);
		bool _sensorIteratorHelper(const INT64& sensorId, json& out, string const endpoint);
		string _getStationCodeById(const INT64& stationId);
	};
}

#endif