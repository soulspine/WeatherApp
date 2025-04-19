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

struct Sensor {
    INT64 id;
    string formula;
    string code;
    string meteredValue;
    INT64 meteredValueId;
};

struct Station {
    INT64 id;
    string kodStacji;
    string nazwa;
    string szerokoscWGS84;
    string dlugoscWGS84;
    string gmina;
    INT64 idMiasta;
    string nazwaMiasta;
    string powiat;
    string ulica;
    string wojewodztwo;
    vector<Sensor> sensors;
	string comboLabel;
};

struct StationCache {
    unordered_map<string, Station> stations;
    vector<string> miasta;
    vector<string> wojewodztwa;
    vector<string> gminy;
    vector<string> powiaty;

    void clear() {
        stations.clear();
        miasta.clear();
        wojewodztwa.clear();
        gminy.clear();
        powiaty.clear();
    }
};

struct SensorReading {
    INT64 timestamp; // UNIX timestamp (UTC)
    double value;
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