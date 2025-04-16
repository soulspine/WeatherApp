#pragma execution_character_set("utf-8")

#include "WeatherApp.h"

using namespace WeatherApp;

App::App() {
	cout << "Starting in: " << _getcwd(NULL, 0) << endl;

	// CHECKING FOR POTETO
	const wchar_t fileName[] = L"poteto.jpg";
	const UINT targetCheckSum = 39942;

	DWORD headerSum = 0, checkSum = 0;
	DWORD returnVal = MapFileAndCheckSumW(fileName, &headerSum, &checkSum);
	if ((returnVal != CHECKSUM_SUCCESS) or (checkSum != targetCheckSum)) {
		std::wstring part1 = L"Program encountered an error opening ";
		std::wstring part2 = L". Please make sure the file is in the same directory as the executable and it is the correct file.";

		// Convert the wchar_t[] array to std::wstring and concatenate
		std::wstring errorMsg = part1 + wstring(fileName)  + part2;

		// Pass the concatenated result to MessageBoxW
		MessageBoxW(NULL, errorMsg.c_str(), L"Essential file missing", MB_OK | MB_ICONERROR);
		exit(1);
	}

	fetchStationDataAndCacheIt();
}

cpr::Response App::requestGet(string endpoint, bool ignoreBaseUrl) {
	if (!ignoreBaseUrl) {
		//make sure there is no slash at the start of the endpoint
		while (endpoint[0] == '/') {
			endpoint = endpoint.substr(1);
		}

		endpoint = BASE_URL + endpoint;
	}
	
	return cpr::Get(cpr::Url{ endpoint });
}

string App::wcharToString(const wchar_t* wchar) {
	string str = "";
	for (int i = 0; wchar[i] != '\0'; i++) {
		str += wchar[i];
	}
	return str;
}

vector<string> App::removeVectorDuplicates(const vector<string>& input) {
	set<string> unique(input.begin(), input.end());
	return vector<std::string>(unique.begin(), unique.end());
}

string App::getJsonString(const nlohmann::json& obj, const std::string& key, const std::string& defaultValue) {
	if (obj.contains(key) && obj[key].is_string()) {
		return obj[key].get<std::string>();
	}
	return defaultValue;
}

void App::fetchStationDataAndCacheIt() {

	stations.clear();
	cachedMiasta.clear();
	cachedWojewodztwa.clear();
	cachedGminy.clear();
	cachedPowiaty.clear();

	//FETCHING STATIONS DATA
	int pages = INT_MAX;
	for (int i = 0; i < pages; i++) {
		auto response = requestGet("station/findAll?page=" + to_string(i) + "&size=500");
		if (response.status_code == 200) {

			json data = json::parse(response.text);

			//data = json::parse(data.dump(-1, ' ', true));

			pages = data["totalPages"];

			auto stationsList = data["Lista stacji pomiarowych"];

			for (const auto& station : stationsList) {
				Station s;
				s.id = station["Identyfikator stacji"];
				s.kodStacji = getJsonString(station, "Kod stacji");
				s.nazwa = getJsonString(station, "Nazwa stacji");
				s.idMiasta = station["Identyfikator miasta"];
				s.nazwaMiasta = getJsonString(station, "Nazwa miasta");
				s.gmina = getJsonString(station, "Gmina");
				s.powiat = getJsonString(station, "Powiat");
				s.wojewodztwo = getJsonString(station, "Województwo");
				s.ulica = getJsonString(station, "Ulica");
				s.szerokoscWGS84 = getJsonString(station, "WGS84 φ N");
				s.dlugoscWGS84 = getJsonString(station, "WGS84 λ E");

				stations.insert({ s.kodStacji, s });

				cachedMiasta.push_back(s.nazwaMiasta);
				cachedWojewodztwa.push_back(s.wojewodztwo);
				cachedGminy.push_back(s.gmina);
				cachedPowiaty.push_back(s.powiat);
			}
		}
		else {
			MessageBoxW(NULL, L"Unable to access API, make sure you have internet connection. API could be down. App will now exit.", L"Error", MB_OK | MB_ICONERROR);
			exit(2);
		};
	}

	cachedMiasta = removeVectorDuplicates(cachedMiasta);
	cachedWojewodztwa = removeVectorDuplicates(cachedWojewodztwa);
	cachedGminy = removeVectorDuplicates(cachedGminy);
	cachedPowiaty = removeVectorDuplicates(cachedPowiaty);

	cout << "Fetched " << stations.size() << " stations." << endl;
	cout << "Cached " << cachedPowiaty.size() << " powiaty." << endl;
	cout << "Cached " << cachedGminy.size() << " gminy." << endl;
	cout << "Cached " << cachedWojewodztwa.size() << " wojewodztwa." << endl;
	cout << "Cached " << cachedMiasta.size() << " miasta." << endl;
}