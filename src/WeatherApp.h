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
	static int testsPassed = 0;
	static int testsFailed = 0;

	inline bool TestCompare(const auto& a, const auto& b, const string& name, const bool& print = true) {
		bool returnVal;
		if (a == b) {
			testsPassed++;
			returnVal = true;
		}
		else {
			testsFailed++;
			returnVal = false;
		}

		if (print) {
			cout << "Test \"" << name << "\": " << (returnVal ? "Passed" : "Failed") << endl;
		}

		return returnVal;
	}

	/// <summary>
	/// Usuwa duplikaty z vectora stringów.
	/// </summary>
	/// <param name="input">vector stringów</param>
	/// <returns>nowy vector bez powtórzeń</returns>
	inline vector<string> RemoveVectorDuplicates(const vector<string>& input) {
		set<string> unique(input.begin(), input.end());
		return vector<string>(unique.begin(), unique.end());
	}

	/// <summary>
	/// Wrapper do odczytywania wartości stringów z obiektów json. Jeśli nie mają wartości, zwraca defaultValue.
	/// </summary>
	/// <param name="obj">obiekt, z którego chcemy odczytać wartość</param>
	/// <param name="key">klucz wartości</param>
	/// <param name="defaultValue">domyślna wartość, która zostanie zwrócona w przypadku NULLa</param>
	/// <returns>string z wartością odczytaną lub defaultValue</returns>
	inline string GetSafeJsonString(const json& obj, const string& key, const string& defaultValue = "") {
		if (obj.contains(key) && obj[key].is_string()) {
			return obj[key].get<string>();
		}
		return defaultValue;
	}


	/// <summary>
	/// Tworzy string przedstawiający czas w formacie HH:MM z podanego timestampu.
	/// </summary>
	/// <param name="unixSeconds">timestamp w formacie unix bez milisekund</param>
	inline std::string FormatTimeHHMM(double unixSeconds) {
		using namespace std::chrono;
		std::time_t t = static_cast<std::time_t>(unixSeconds);
		std::tm      tm_local;
		localtime_s(&tm_local, &t);
		char buf[6];
		std::strftime(buf, sizeof(buf), "%H:%M", &tm_local);
		return std::string(buf);
	}

	/// <summary>
	/// Parsuje string w formacie "YYYY-mm-DD HH:MM" do timestampu unixowego.
	/// </summary>
	/// <param name="dateTime">string f formacie YYYY-mm-DD HH:MM</param>
	/// <returns>timestamp</returns>
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

		/// <summary>
		/// Pobiera dane sensora z API GIOS od teraz do daysCount dni i zapisuje je w bazie danych.
		/// </summary>
		/// <param name="sensorId">id sensora</param>
		/// <param name="out">obiekt json, do którego zapisane będą dane</param>
		/// <param name="daysCount">ilość dni wstecz</param>
		/// <returns>bool określający czy operacja się udała</returns>
		bool GetDataForSensorByDaysBack(const INT64& sensorId, json& out, const int& daysCount);
		
		/// <summary>
		/// Pobiera dane sensora z API GIOS w podanym przedziale czasowym i zapisuje je w bazie danych.
		/// </summary>
		/// <param name="sensorId">id sensora</param>
		/// <param name="out">obiekt json, do którego zapisane będą dane</param>
		/// <param name="dateFrom">data w formacie YYYY-mm-dd HH:MM otwierająca przedział czasowy</param>
		/// <param name="dateTo">data w formacie YYYY-mm-dd HH:MM zamykająca przedział czasowy</param>
		/// <returns>bool określający czy operacja się udała</returns>
		bool GetDataForSensorByTimeFrame(const INT64& sensorId, json& out, const string& dateFrom, const string& dateTo);

		/// <summary>
		/// Tworzy vector ze wszystkimi stacjami wczytanymi z pliku JSON.
		/// </summary>
		/// <returns>vector z obiektami Stations wszystkich wczytanych stacji</returns>
		vector<Station> GetCachedStations();

		/// <summary>
		/// Odczytuje ostatni zapisany odczyt dla sensora z bazy danych. Jeśli nie ma żadnego odczytu, timestamp będzie wynosić 0.
		/// </summary>
		/// <param name="sensorId">id sensora</param>
		/// <returns>obiekt SensorReading</returns>
		SensorReading GetLastSensorReading(INT64 sensorId);

		/// <summary>
		/// Ładuje wszystkie odczyty dla sensora w podanym przedziale czasowym i zapisuje je w vectorach outX i outY.
		/// </summary>
		/// <param name="sensorId">id sensora</param>
		/// <param name="dateFrom">data w formacie YYYY-mm-dd HH:MM otwierająca przedział czasowy</param>
		/// <param name="dateTo">data w formacie YYYY-mm-dd HH:MM zamykająca przedział czasowy</param>
		/// <param name="outX">vector, do którego będą zapisane timestampy odczytu</param>
		/// <param name="outY">vecotr, do którego będą zapisane wartości odczytu</param>
		void GetPlotPointsForSensorInTimeFrame(const INT64& sensorId, const string& dateFrom, const string& dateTo, vector<double>& outX, vector<double>& outY);

	private:
		StationCache stationCache;

		const LPCWSTR ERRORMSG_REQUEST_FAILED = L"Nie można zrealizować żądania. Sprawdź połączenie z internetem lub spróbuj ponownie później.";
		const LPCWSTR ERRORMSG_FILE_OPEN_FAILED = L"Nie można otworzyć pliku.";
		const string BASE_URL = "https://api.gios.gov.pl/pjp-api/v1/rest/";
		filesystem::path appDataPath;

		/// <summary>
		/// Ładuje dane stacji z pliku JSON i zapisuje je w bazie stationCache.
		/// </summary>
		void loadSavedStationData();

		/// <summary>
		/// Pobiera wszystkie stacje z API GIOS i zapisuje je w bazie stationCache oraz pliku JSON.
		/// </summary>
		void fetchStationDataAndSaveIt();

		/// <summary>
		/// Upewnia się, że folder appDataPath istnieje. Jeśli nie, tworzy go.
		/// </summary>
		void setUpAppDataFolder();

		/// <summary>
		/// Wrapper do cpr::Get, który dodaje BASE_URL do endpointu. Jeżeli ignoreBaseUrl jest ustawione na true, nie dodaje BASE_URL.
		/// </summary>
		/// <param name="endpoint">endpoint, który chcemy zapytać</param>
		/// <param name="ignoreBaseUrl">czy nie dodawać BASE_URL na początek zapytania</param>
		/// <returns>cpr::Response</returns>
		cpr::Response _requestGet(string endpoint, bool ignoreBaseUrl = false);

		/// <summary>
		/// Konwertuje wchar_t* do stringa.
		/// </summary>
		/// <param name="wchar">wejściowy wchar*</param>
		/// <returns>string z tym samym tekstem ale w innym formacie</returns>
		string _wcharToString(const wchar_t* wchar);

		/// <summary>
		/// Pokazuje errorBox z podanym komunikatem.
		/// </summary>
		/// <param name="message">wiadomość do pokazania</param>
		void _showErrorBox(LPCWSTR message);

		/// <summary>
		/// Helper funkcja do odczytywania pomiarów sensora.
		/// </summary>
		/// <param name="sensorId">id sensora</param>
		/// <param name="out">obiekt json, do którego ma być zapisany wynik zapytania</param>
		/// <param name="endpoint">endpoint, który ma być odpytany o dane</param>
		/// <returns>bool określający czy operacja się udała</returns>
		bool _sensorIteratorHelper(const INT64& sensorId, json& out, string const endpoint);
		
		/// <summary>
		/// Oddaje kod stacji na podstawie id stacji.
		/// </summary>
		/// <param name="stationId">id stacji</param>
		/// <returns>string zawierający kod stacji</returns>
		string _getStationCodeById(const INT64& stationId);
	};
}

#endif