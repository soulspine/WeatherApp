#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <json.hpp>

using std::string;
using std::vector;
using std::unordered_map;
using std::optional;
using json = nlohmann::json;
using INT64 = int64_t;

/// <summary>
/// Struktura opisująca sensor.
/// </summary>
struct Sensor {
    /// <summary>
    /// Id sensora.
    /// </summary>
    INT64 id;
    /// <summary>
	/// Wzór chemiczny wartości mierzonej przez sensor.
    /// </summary>
    string formula;
    /// <summary>
	/// Kod wartości mierzonej przez sensor.
    /// </summary>
    string code;
    /// <summary>
	/// Mierzona wartość.
    /// </summary>
    string meteredValue;
    /// <summary>
	/// Id mierzonej wartości.
    /// </summary>
    INT64 meteredValueId;
};

/// <summary>
/// Struktura opisująca stację.
/// </summary>
struct Station {
    /// <summary>
	/// Id stacji.
    /// </summary>
    INT64 id;
    /// <summary>
	/// Kod stacji.
    /// </summary>
    string kodStacji;
    /// <summary>
	/// Nazwa stacji.
    /// </summary>
    string nazwa;
    /// <summary>
	/// Szerokość geograficzna WGS84.
    /// </summary>
    string szerokoscWGS84;
    /// <summary>
	/// Długość geograficzna WGS84.
    /// </summary>
    string dlugoscWGS84;
    /// <summary>
	/// Gmina, w której znajduje się stacja.
    /// </summary>
    string gmina;
    /// <summary>
	/// Id miasta, w którym znajduje się stacja.
    /// </summary>
    INT64 idMiasta;
    /// <summary>
	/// Nazwa miasta, w którym znajduje się stacja.
    /// </summary>
    string nazwaMiasta;
    /// <summary>
	/// Powiat, w którym znajduje się stacja.
    /// </summary>
    string powiat;
    /// <summary>
	/// Ulica, na której znajduje się stacja.
    /// </summary>
    string ulica;
    /// <summary>
	/// Województwo, w którym znajduje się stacja.
    /// </summary>
    string wojewodztwo;
    /// <summary>
	/// Wektor sensorów przypisanych do stacji.
    /// </summary>
    vector<Sensor> sensors;
	/// <summary>
	/// String dostosowany do wyświetlania w wyborze stacji w GUI.
	/// </summary>
	string comboLabel;
};

/// <summary>
/// Struktura opisująca cache stacji.
/// </summary>
struct StationCache {
    /// <summary>
	/// Słownik stacji, gdzie kluczem jest kod stacji, a wartością jest obiekt Station.
    /// </summary>
    unordered_map<string, Station> stations;
    /// <summary>
	/// Wektor nazw miast.
    /// </summary>
    vector<string> miasta;
    /// <summary>
	/// Wektor nazw województw.
    /// </summary>
    vector<string> wojewodztwa;
    /// <summary>
	/// Wektor nazw gmin.
    /// </summary>
    vector<string> gminy;
    /// <summary>
	/// Wektor nazw powiatów.
    /// </summary>
    vector<string> powiaty;

    /// <summary>
	/// Czyści cache stacji.
    /// </summary>
    void clear() {
        stations.clear();
        miasta.clear();
        wojewodztwa.clear();
        gminy.clear();
        powiaty.clear();
    }
};

/// <summary>
/// Struktura opisująca odczyt sensora.
/// </summary>
struct SensorReading {
    /// <summary>
	/// Timestamp odczytu w formacie UNIX (UTC).
    /// </summary>
    INT64 timestamp;
    /// <summary>
	/// Wartość odczytu sensora.
    /// </summary>
    double value;
};

/// <summary>
/// Dane sensora do wyświetlenia na wykresie.
/// </summary>
struct SensorPlotData {
    /// <summary>
	/// Nazwa sensora na wykresie.
    /// </summary>
    string plotName;
    /// <summary>
	/// Czy sensor ma być wyświetlany na wykresie.
    /// </summary>
    bool displayOnPlot = false;
	/// <summary>
	/// Ostatni odczyt sensora.
	/// </summary>
	SensorReading lastReading;
	/// <summary>
	/// Punkty X do wyświetlenia na wykresie.
	/// </summary>
	vector<double> plotXValues;
	/// <summary>
	/// Punkty Y do wyświetlenia na wykresie.
	/// </summary>
	vector<double> plotYValues;
};

// --- JSON serialization for Sensor ---
inline void to_json(json& j, const Sensor& s) {
    j = {
        {"id", s.id},
        {"formula", s.formula},
        {"code", s.code},
        {"meteredValue", s.meteredValue},
        {"meteredValueId", s.meteredValueId}
    };
}

inline void from_json(const json& j, Sensor& s) {
    j.at("id").get_to(s.id);
    j.at("formula").get_to(s.formula);
    j.at("code").get_to(s.code);
    j.at("meteredValue").get_to(s.meteredValue);
    j.at("meteredValueId").get_to(s.meteredValueId);
}


// --- JSON serialization for Station ---
inline void to_json(json& j, const Station& s) {
    j = {
        {"id", s.id},
        {"kodStacji", s.kodStacji},
        {"nazwa", s.nazwa},
        {"szerokoscWGS84", s.szerokoscWGS84},
        {"dlugoscWGS84", s.dlugoscWGS84},
        {"gmina", s.gmina},
        {"idMiasta", s.idMiasta},
        {"nazwaMiasta", s.nazwaMiasta},
        {"powiat", s.powiat},
        {"ulica", s.ulica},
        {"wojewodztwo", s.wojewodztwo},
        {"sensors", s.sensors},
        {"comboLabel", s.comboLabel}
    };
}

inline void from_json(const json& j, Station& s) {
    j.at("id").get_to(s.id);
    j.at("kodStacji").get_to(s.kodStacji);
    j.at("nazwa").get_to(s.nazwa);
    j.at("szerokoscWGS84").get_to(s.szerokoscWGS84);
    j.at("dlugoscWGS84").get_to(s.dlugoscWGS84);
    j.at("gmina").get_to(s.gmina);
    j.at("idMiasta").get_to(s.idMiasta);
    j.at("nazwaMiasta").get_to(s.nazwaMiasta);
    j.at("powiat").get_to(s.powiat);
    j.at("ulica").get_to(s.ulica);
    j.at("wojewodztwo").get_to(s.wojewodztwo);
    j.at("sensors").get_to(s.sensors);
	j.at("comboLabel").get_to(s.comboLabel);
}

// --- JSON serialization for StationCache ---
inline void to_json(json& j, const StationCache& c) {
    j = {
        {"stations", c.stations},
        {"miasta", c.miasta},
        {"wojewodztwa", c.wojewodztwa},
        {"gminy", c.gminy},
        {"powiaty", c.powiaty}
    };
}

inline void from_json(const json& j, StationCache& c) {
    j.at("stations").get_to(c.stations);
    j.at("miasta").get_to(c.miasta);
    j.at("wojewodztwa").get_to(c.wojewodztwa);
    j.at("gminy").get_to(c.gminy);
    j.at("powiaty").get_to(c.powiaty);
}
// --- JSON serialization for SensorReading ---
inline void to_json(json& j, const SensorReading& r) {
    j = {
        {"timestamp", r.timestamp},
        {"value", r.value}
    };
}

inline void from_json(const json& j, SensorReading& r) {
    j.at("timestamp").get_to(r.timestamp);
    j.at("value").get_to(r.value);
}