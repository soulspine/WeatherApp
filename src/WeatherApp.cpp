#include "WeatherApp.h"

WeatherApp::WeatherApp() {
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
}

string WeatherApp::wcharToString(const wchar_t* wchar) {
	string str = "";
	for (int i = 0; wchar[i] != '\0'; i++) {
		str += wchar[i];
	}
	return str;
}