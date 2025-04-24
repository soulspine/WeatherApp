#include "imgui.h"
#include <ctime>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>

/// <summary>
/// Formatuje datę do stringa w formacie YYYY-mm-DD 00:00.
/// </summary>
/// <param name="t">obiekt tm zawierający wczytany odpowiedni czas</param>
/// <returns>odpowiednio zformatowany string</returns>
string FormatDate(const tm& t) {
    ostringstream oss;
    oss << setw(4) << setfill('0') << (t.tm_year + 1900) << "-"
        << setw(2) << setfill('0') << (t.tm_mon + 1) << "-"
        << setw(2) << setfill('0') << t.tm_mday
        << " 00:00";
    return oss.str();
}

/// <summary>
/// Element GUI wybierający zakres dat wstecz.
/// </summary>
/// <param name="fromDaysBack">zmienna przechowująca wartość pierwszego slidera</param>
/// <param name="toDaysBack">zmienna przechowująca wartość drugiego slidera</param>
/// <param name="startOut">string przechowujący zformatowany tekst z datą ustawioną na pierwszym sliderze</param>
/// <param name="endOut">string przechowujący zformatowany tekst z datą ustawioną na drugim sliderze</param>
/// <returns>bool określający czy użytkownik wszedł w interakcję z elementem w tej klatce</returns>
bool DateRangeBackwardsSelector(int* fromDaysBack, int* toDaysBack, string& startOut, string& endOut) {
    bool changed = false;


    if (ImGui::SliderInt("Od ilu dni wstecz (początek)", fromDaysBack, 0, 366)) {
        changed = true;
        if (*fromDaysBack <= *toDaysBack) {
            *toDaysBack = *fromDaysBack - 1;
        }
    }

    if (ImGui::SliderInt("Do ilu dni wstecz (koniec)", toDaysBack, -1, 365)) {
        changed = true;
		if (*toDaysBack >= *fromDaysBack) {
			*fromDaysBack = *toDaysBack + 1;
		}
    }

    time_t now = time(nullptr);
    tm today;
    localtime_s(&today, &now);
    today.tm_hour = 0; today.tm_min = 0; today.tm_sec = 0;
    time_t todayMidnight = mktime(&today);

    // Oblicz daty
    time_t startTime = todayMidnight - (*fromDaysBack * 86400);
    time_t endTime = todayMidnight - (*toDaysBack * 86400);

    if (toDaysBack == 0) {
		endTime += 86400; // dodajemy 1 dzień, aby obejmować dzisiejszą datę
    }

    tm start, end;
    localtime_s(&start, &startTime);
    localtime_s(&end, &endTime);

    startOut = FormatDate(start);
    endOut = FormatDate(end);

    ImGui::Text("Zakres dat:");
    ImGui::Text("Początek: %s", startOut.c_str());
    ImGui::Text("Koniec:   %s", endOut.c_str());

    return changed;
}
