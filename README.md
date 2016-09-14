# Single Button Writer (AVR)

![Overview](https://github.com/krassowski/single-button-writer-avr/blob/master/img/overview.jpg?raw=true)

## Opis projektu

Program umożliwia pisanie zdań za pomocą minimalnego zestawu elementów. Użytkownik poprzez naciśnięcie przycisku w odpowiednim momencie wybiera znak który chce napisać.
Aby maksymalnie skrócić czas potrzebny do pisania, na pierwszym ekranie dostępne są kolejno: spacja, odniesienia do 3 ekranów w zakresach alfabetu: `a-i`, `j-r`, `s-z` oraz odniesienie do ekranu ze znakami specjalnymi.
Funkcjonalność przycisku `backspace` jest dostępna pod symbolem strzałki wskazującej w lewo, a powrót do ekranu nadrzędnego możliwy jest przez wybranie strzałki w górę. Odpowiednie instrukcje wyświetlane są po inicjalizacji urządzenia; możliwe jest ich pominięcie przez wciśnięcie przycisku w trakcie ich wyświetlania.

Prędkość zmiany podświetlanej opcji (litery / polecenia) można regulować poprzez użycie potencjometru (w implementacji jest to konwersja odczytywanego sygnału analogowego - wyjściowego napięcia do sygnału cyfrowego i odpowiednie skalowanie opóźnień).

Jeżeli program został skompilowany z definicją 
```C
#define MODE MODE_FREQUENCY
```

litery w podekranach wyświetlane będą zgodnie z częstością ich występowania w danym alfabecie (domyślnie zaimplementowano alfabet angielski). Przy użyciu:

```C
#define MODE MODE_SIMPLE
```

litery w podekranach będą wyświetlane w kolejności alfabetycznej.

Domyślnym kontrolerem (sposobem sterowania) jest przycisk typu "push".


Możliwe jest jednak użycie urządzeń sterujacych. W ramach projektu przeprowadzono próby podłączenia elektrod do elektromiografi jako alternatywne źródło sterowania, wzorując się m.in. na [Muscle EMG Sensor for a Microcontroller](http://www.instructables.com/id/Muscle-EMG-Sensor-for-a-Microcontroller/?ALLSTEPS).

Wyciągnięto następujace wnioski:
- wymagane wzmacniacze różnicowe są trudno dostępne w kraju, trzeba je zamawiać zza granicy,
- niepowiodła się próba odczytywania sygnału przy użyciu ogólnodostępnych tranzystorów (zbyt małe napięcie?),
- skonstruowanie precyzyjnego wzmacniacza róznicowego jest stosunkowo trudne,
- warto zwrócić uwagę na dobór kabli podłączanych do elektrod, ponieważ mogą być pierwszym poważnym źródłem zakłoceń / miejscem zaniku sygnału,
- dużo prostszą alternatywą wydaje się być użycie czujnika nacisku lub sensora reagującego na zgięcie (flex bind sensor).

Program został napisany na zaliczenie przedmiotu ogólnouniwesyteckiego "Programowanie mikrokontrolerów AVR" na Uniwersytecie Warszawskim.

### Użyte podzespoły - zestaw minimalny:

- wyświetlacz HD44780 (16 x 2) z podłaczeniem 16 pinowym,
- mikroprocesor AVR ATMEGA8535,
- potencjometr obrotowy liniowy B100K
- przycisk typu "push button"
- 2 kondensatory 104 (100 nF),
- opornik 1 kO
- okablowanie
- źródło zasialania 5V DC, np. z USB

### Wymagane podpięcia - zestaw minimalny

- ADC0 (PA0) - napięcie regulowane przez potencjometr
- PB0 - push button
- ADC4 (PA4) - napięcie do sterowania przez alternatywne czujniki, np. sensor EMG.

Aby wybrać sensor należy zmienić definicje kontolera:
```C
#define CONTROLLER EMG // albo: 
// #define CONTROLLER BUTTON
```

wartość graniczna `THERESHOLD` z zakresu `1 - 1023` pozwala regulować próg reakcji na napięcie z alternatywnego czujnika:
```C
#define EMG_THERESHOLD 900
```

## Zdjęcie w zbliżeniu na układ podczas pracy:

![Zoomed](https://github.com/krassowski/single-button-writer-avr/blob/master/img/zoomed.jpg?raw=true)
