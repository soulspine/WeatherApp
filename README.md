# WeatherApp
Aplikacja pozwalająca na pobieranie odczytów z sensorów jakości powietrza dostępnych przez API GIOS oraz prostą ich analizę na wykresie.

# Wymagania
Brak, wszystko jest zawarte w release i nie wymaga dodatkowych instalacji.

# Własna kompilacja
Aby skompilować aplikację należy otworzyć projekt w Visual Studio, upewnić się, że są zainstalowane wszystkie zależności NuGet (czyli tylko cpr lib) i użyć MS Build.

# Testy
Do programu dołączone są testy, które uruchamiane są poprzez dodanie flagi `--test` podczas uruchamiania aplikacji. Wyświetlone zostanie okno konsoli, w której będą wyświetlone wyniki testów, dodawanie nowych testów odbywa się w funkcji `main` poprzez uzycie `TestCompare()`, z załączonego namespace `WeatherApp`.

# Kody błędu aplikacji
- `1` - Nie można otworzyć poteto.png
- `2` - Błąd przy pobieraniu stacji z API GIOS