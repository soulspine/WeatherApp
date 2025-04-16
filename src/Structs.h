#ifndef WEATHER_APP_DTO_H
#define WEATHER_APP_DTO_H

using namespace std;

namespace WeatherApp {
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
	};
}
#endif