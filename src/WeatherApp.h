#ifndef WEATHER_APP_H
#define WEATHER_APP_H

#include <windows.h>
#include <ImageHlp.h>
#pragma comment(lib, "Imagehlp.lib")
#include <iostream>
#include <direct.h>
#include <string>
//#include "restclient.h"

using namespace std;

class WeatherApp {
	public:
		WeatherApp();
	private:
		string wcharToString(const wchar_t* wchar);

};

#endif