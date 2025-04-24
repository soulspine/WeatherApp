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
#include <cstring>
#include <ctime>
#include <stdexcept>


using json = nlohmann::json;
using namespace std;

namespace WeatherApp {

	inline std::string FormatTimeHHMM(double unixSeconds) {
		using namespace std::chrono;
		std::time_t t = static_cast<std::time_t>(unixSeconds);
		std::tm      tm_local;
		localtime_s(&tm_local, &t);
		char buf[6];
		std::strftime(buf, sizeof(buf), "%H:%M", &tm_local);
		return std::string(buf);
	}

	inline double ParseTimestampYmdHHMM(const string& dateTime)
	{
		tm tm{};
		tm.tm_isdst = -1;

		istringstream ss(dateTime);
		ss >> get_time(&tm, "%Y-%m-%d %H:%M");
		if (ss.fail())
			throw invalid_argument("Niepoprawny format daty: " + dateTime);

		time_t t = mktime(&tm);
		if (t == -1)
			throw runtime_error("mktime() nie zdołał przeliczyć daty");

		return static_cast<double>(t);
	}

	class App {
	public:
		App();

		bool GetDataForSensorByDaysBack(const INT64& sensorId, json& out, const int& daysCount);
		bool GetDataForSensorByTimeFrame(const INT64& sensorId, json& out, const string& dateFrom, const string& dateTo);

		vector<Station> GetCachedStations();
		SensorReading GetLastSensorReading(INT64 sensorId);
		void GetPlotPointsForSensorInTimeFrame(const INT64& sensorId, const string& dateFrom, const string& dateTo, vector<double>& outX, vector<double>& outY);

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