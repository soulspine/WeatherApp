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

		_showErrorBox(errorMsg.c_str());
		exit(1);
	}

	setUpAppDataFolder();
	loadSavedStationData();
}

cpr::Response App::_requestGet(string endpoint, bool ignoreBaseUrl) {
	if (!ignoreBaseUrl) {
		//make sure there is no slash at the start of the endpoint
		while (endpoint[0] == '/') {
			endpoint = endpoint.substr(1);
		}

		endpoint = BASE_URL + endpoint;
	}
	
	return cpr::Get(cpr::Url{ endpoint });
}

string App::_wcharToString(const wchar_t* wchar) {
	string str = "";
	for (int i = 0; wchar[i] != '\0'; i++) {
		str += wchar[i];
	}
	return str;
}

vector<string> App::_removeVectorDuplicates(const vector<string>& input) {
	set<string> unique(input.begin(), input.end());
	return vector<std::string>(unique.begin(), unique.end());
}

string App::_getJsonString(const json& obj, const string& key, const string& defaultValue) {
	if (obj.contains(key) && obj[key].is_string()) {
		return obj[key].get<std::string>();
	}
	return defaultValue;
}

void App::loadSavedStationData() {
	filesystem::path stationDataPath = appDataPath / "stationData.json";
	if (filesystem::exists(stationDataPath)) {
		//load
		ifstream stationDataFile(stationDataPath);
		if (stationDataFile.is_open()) {
			json j;
			stationDataFile >> j;
			stationCache.stations = j["stations"];
			stationCache.miasta = j["miasta"];
			stationCache.wojewodztwa = j["wojewodztwa"];
			stationCache.gminy = j["gminy"];
			stationCache.powiaty = j["powiaty"];
			stationDataFile.close();
		}
		else {
			_showErrorBox(L"Nie można otworzyć pliku z danymi stacji.");
		}
	}
	else {
		//fetch and save
		fetchStationDataAndSaveIt();
	}
	cout << "Loaded " << stationCache.stations.size() << " stations." << endl;
	cout << "Loaded " << stationCache.miasta.size() << " cities." << endl;
	cout << "Loaded " << stationCache.wojewodztwa.size() << " voivodeships." << endl;
	cout << "Loaded " << stationCache.gminy.size() << " communes." << endl;
	cout << "Loaded " << stationCache.powiaty.size() << " counties." << endl;
}

void App::fetchStationDataAndSaveIt() {

	stationCache.clear();

	//FETCHING STATIONS DATA
	int pages = INT_MAX;
	for (int i = 0; i < pages; i++) {
		auto response = _requestGet("station/findAll?page=" + to_string(i));
		if (response.status_code == 200) {

			json data = json::parse(response.text);

			//data = json::parse(data.dump(-1, ' ', true));

			pages = data["totalPages"];

			auto stationsList = data["Lista stacji pomiarowych"];

			for (const auto& station : stationsList) {
				Station s;
				s.id = station["Identyfikator stacji"];
				s.kodStacji = _getJsonString(station, "Kod stacji");
				s.nazwa = _getJsonString(station, "Nazwa stacji");
				s.idMiasta = station["Identyfikator miasta"];
				s.nazwaMiasta = _getJsonString(station, "Nazwa miasta");
				s.gmina = _getJsonString(station, "Gmina");
				s.powiat = _getJsonString(station, "Powiat");
				s.wojewodztwo = _getJsonString(station, "Województwo");
				s.ulica = _getJsonString(station, "Ulica");
				s.szerokoscWGS84 = _getJsonString(station, "WGS84 φ N");
				s.dlugoscWGS84 = _getJsonString(station, "WGS84 λ E");
				s.comboLabel = format("{} {}", s.nazwaMiasta, s.ulica);

				int sensorsPages = INT_MAX;

				for (int j = 0; j < sensorsPages; j++) {
					auto sensorsRes = _requestGet(format("station/sensors/{}?page={}", s.id, j));
					json sensorsData = json::parse(sensorsRes.text);
					sensorsPages = sensorsData["totalPages"];
					auto sensorsList = sensorsData["Lista stanowisk pomiarowych dla podanej stacji"];
					for (const auto& sensor : sensorsList) {
						Sensor sn;
						sn.id = sensor["Identyfikator stanowiska"];
						sn.formula = _getJsonString(sensor, "Wskaźnik - wzór");
						sn.code = _getJsonString(sensor, "Wskaźnik - kod");
						sn.meteredValue = _getJsonString(sensor, "Wskaźnik");
						sn.meteredValueId = sensor["Id wskaźnika"];
						s.sensors.push_back(sn);
					}
				}
				
				stationCache.stations[s.kodStacji] = s;
				stationCache.miasta.push_back(s.nazwaMiasta);
				stationCache.wojewodztwa.push_back(s.wojewodztwo);
				stationCache.gminy.push_back(s.gmina);
				stationCache.powiaty.push_back(s.powiat);
				cout << "Fetched station " << s.kodStacji << endl;
			}
		}
		else {
			_showErrorBox(ERRORMSG_REQUEST_FAILED);
			exit(2);
		};
	}

	stationCache.miasta = _removeVectorDuplicates(stationCache.miasta);
	stationCache.wojewodztwa = _removeVectorDuplicates(stationCache.wojewodztwa);
	stationCache.gminy = _removeVectorDuplicates(stationCache.gminy);
	stationCache.powiaty = _removeVectorDuplicates(stationCache.powiaty);

	//SAVE TO FILE
	filesystem::path stationDataPath = appDataPath / "stationData.json";
	ofstream stationDataFile(stationDataPath);
	if (!stationDataFile) {
		_showErrorBox(ERRORMSG_FILE_OPEN_FAILED);
	}

	json j = stationCache;

	stationDataFile << j.dump(4, ' ') << endl;
	stationDataFile.close();
}

void App::setUpAppDataFolder() {
	char path[MAX_PATH];
	SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path);
	appDataPath = path;

	appDataPath /= "soulspine";
	filesystem::create_directories(appDataPath);

	appDataPath /= "WeatherApp";
	filesystem::create_directories(appDataPath);
	filesystem::create_directories(appDataPath / "database");
}

bool App::_sensorIteratorHelper(const INT64& sensorId, json& out, const string endpoint) {
	out = json::array();
	cout << "Fetching data for sensor " << sensorId << " | " << endpoint << endl;
	int pages = INT_MAX;
	for (int i = 0; i < pages; i++) {
		rateLimitRetry:
		auto response = _requestGet(format("{}&page={}&size=500", endpoint, i));
		cout << response.text << endl;
		if (response.status_code == 200) {
			cout << json::accept(response.text) << endl;
			json data = json::parse(response.text);
			pages = data["totalPages"];
			auto dataList = data["Lista archiwalnych wyników pomiarów"];
			for (const auto& item : dataList) {
				cout << item.dump(4, ' ') << endl << endl;
				json date = item["Data"];
				json value = item["Wartość"];
				if (!date.is_null() && !value.is_null()) {
					string dateString = date;
					tm tm = {};
					istringstream ss(dateString);
					ss >> get_time(&tm, "%Y-%m-%d %H:%M:%S");
					time_t timestamp = mktime(&tm);

					SensorReading sr;
					sr.timestamp = timestamp - 3600; // normalizing to UTC
					sr.value = value;
					out.push_back(sr);
					filesystem::path sensorDataPath = appDataPath / "database" / to_string(sensorId);
					filesystem::create_directories(sensorDataPath);
					filesystem::path readingPath = sensorDataPath / to_string(sr.timestamp) / ".json";
					if (!filesystem::exists(readingPath)) {
						ofstream file(sensorDataPath / (to_string(sr.timestamp) + ".json"));
						if (file.is_open()) {
							file << json(sr).dump(4, ' ');
							file.close();
						}
						else {
							_showErrorBox(ERRORMSG_FILE_OPEN_FAILED);
						}
					}
				}
			}
		}
		else {
			if (response.status_code == 429) {
				cout << "Rate limit reached. Sleeping for 5s." << endl;
				Sleep(5000);
				goto rateLimitRetry;
			}

			cout << response.text << endl;

			_showErrorBox(ERRORMSG_REQUEST_FAILED);
			return false;
		}
	}
	return true;
}

bool App::GetDataForSensorByDaysBack(const INT64& sensorId, json& out, const int& daysCount) {
	string endpoint = format("archivalData/getDataBySensor/{}?dayNumber={}", sensorId, daysCount);
	return _sensorIteratorHelper(sensorId, out, endpoint);
}

bool App::GetDataForSensorByTimeFrame(const INT64& sensorId, json& out, const string& dateFrom, const string& dateTo) {
	string endpoint = format("archivalData/getDataBySensor/{}?dateFrom={}&dateTo={}", sensorId, cpr::util::urlEncode(dateFrom), cpr::util::urlEncode(dateTo));
	return _sensorIteratorHelper(sensorId, out, endpoint);
}

void App::_showErrorBox(LPCWSTR message) {
	MessageBoxW(NULL, message, L"Error", MB_OK | MB_ICONERROR);
}

string App::_getStationCodeById(const INT64& stationId) {
	for (const auto& station : stationCache.stations) {
		if (station.second.id == stationId) {
			return station.first;
		}
	}
}

vector<Station> App::GetCachedStations() {
	auto out = vector<Station>();
	for (const auto& station : stationCache.stations) {
		out.push_back(station.second);
	}
	return out;
}

SensorReading App::GetLastSensorReading(INT64 sensorId) {
	filesystem::path sensorDataPath = appDataPath / "database" / to_string(sensorId);

	SensorReading out = {};
	out.timestamp = 0;
	out.value = 0;

	if (filesystem::exists(sensorDataPath)) {
		for (const auto& file : filesystem::directory_iterator(sensorDataPath)) {
			if (file.is_regular_file() and file.path().extension() == ".json") {
				string filename = file.path().filename().string();
				INT64 timestamp = stoll(filename.substr(0, filename.find('.')));
				if (timestamp > out.timestamp) {
					out.timestamp = timestamp;
				}
			}
		}

		if (out.timestamp != 0) {
			ifstream file(sensorDataPath / (to_string(out.timestamp) + ".json"));
			if (file.is_open()) {
				json j;
				file >> j;
				out = j;
				file.close();
			}
			else {
				_showErrorBox(ERRORMSG_FILE_OPEN_FAILED);
			}
		}
	}

	return out;
}