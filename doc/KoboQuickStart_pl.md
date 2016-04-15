Skrócona instrukcja obsługi Kobo - TopHat
=========================================

Ostrzeżenie
-----------

Użytkownik programu TopHat jest całkowicie odpowiedzialny za jego użycie. Oprogramowanie jest
jedynie pomocą nawigacyjną i nie zwalnia pilota w żadnym stopniu z odpowiedzialności za
przestrzeganie przepisów lotniczych.


Włączenie urządzenia
--------------------

Urządzenie Kobo z zainstalowanym oprogramowaniem TopHat włącza się tak samo jak oprogramowanie czytnika książek. 

Pierwsza metoda polega na przesunięciu przełącznika znajdującego się od góry obudowy. Przełącznik należy przesunąć
na kilka sekund w lewą stronę, a następnie puścić.

Drugą metodą włączenia urządzenia jest podłączenie ładowarki poprzez złącze USB.

Niezależnie od sposobu włączenia, proces uruchamiania oprogramowania sygnalizowany jest poprzez świecenie diody
obok włącznika. Po uruchomieniu na ekranie urządzenia pojawi się ekran startowy zawierający wyrysowane
przyciski sterujące dalszą pracą programu.

Powitalny ekran startowy pozwala wybrać tryb pracy urządzenia. Nieco poniżej środka ekranu znajdują się przyciski
`FLY` oraz `SIM`. Pierwszy tryb przeznaczony jest do faktycznego lotu. W tym trybie odczytywana jest pozycja z
odbiornika GPS oraz wykonywane są obliczenia nawigacyjne. Drugi tryb oznaczony jako `SIM` przeznaczony
jest do nauki posługiwania się nawigacją. W tym trybie program zamiast wczytywać faktyczne położenie z odbiornika
GPS symuluje lot szybowca.

Wyłączenie urządzenia
---------------------

Wyłączanie programu nawigacyjnego następuje poprzez czterokrotne naciśnięcie przycisku `M` (menu), a następnie
przyciśnięcie przycisku `Exit program`. Przed wyłączeniem program dodatkowo poprosi o potwierdzenie czy na
pewno chcemy przerwać nawigację.

Druga metoda polega na użyciu mechanicznego przycisku na górnej ramce obudowy. Przycisk należy na chwilę
przesunąć w lewo. W tym wypadku również program poprosi o potwierdzenie czy istotnie zamierzamy zakończyć
działanie. 

Całkowite wyłączenie urządzenia możliwe jest jedynie w chwili gdy wyświetlany jest główny ekran (powitalny).
Istnieją dwie możliwości wyłączenia urządzenia. Pierwsza polega na przyciśnięciu przycisku `Poweroff` w prawym
dolnym rogu ekranu. Drugi sposób wyłączenia polega na użyciu mechanicznego przycisku włącznika na górnej ramce 
urządzenia. Należy go na chwilę przesunąć w lewo.

Nawigacja przed wyłączeniem wyświetla listę ostatnio wykonanych lotów z datą i godziną wykonania lotu,
na samym dole ekranu po prawej stronie widoczny jest napis _powered off_. Ekran ten pozostaje widoczny
również po całkowitym wyłączeniu się urządzenia. Wyświetlacz typu elektroniczny papier nie zużywa energii
elektrycznej w tym stanie.

Ładowanie urządzenia
--------------------
Urządzenie posiada wewnętrzną baterię pozwalającą na pracę w trybie nawigacji przez kilka godzin. Ładowanie
odbywa się poprzez złącze USB, podobnie jak większość dzisiejszych telefonów. Podczas długiego lotu należy
zapewnić ładowanie z instalacji szybowca poprzez ładowarkę z napięciem wyjściowym 5V lub z innego źródła
np. przenośnej baterii _Power Bank_.

Warto wiedzieć, iż Kobo przy silnie rozładowanej baterii nie pozwala się włączyć. Nie jest to również możliwe
zaraz po podłączeniu zasilania. Trzeba cierpliwie zaczekać, aż wewnętrzna bateria naładuje się do poziomu 
pozwalającego na uruchomienie urządzenia, co może zająć około 1h.


Konfiguracja GPS 
----------------
Czytnik Kobo posiada wbudowany port szeregowy pozwalający na podłączenie odbiornika GPS lub elektronicznego
wariometru. Operacja taka wymaga otwarcia obudowy oraz przylutowania wyprowadzeń w przygotowanym przez
producenta urządzenia miejscu. Należy pamietać, iż wbudowany port szeregowy działa przy napięciu 3.3V, tak
wiec wymagany jest konwerter poziomów napięć.

Dodatkowo możliwe jest podłączenie GPS poprzez port USB za pomocą konwertera port szeregowy <-> USB. Kobo potrafi
funkcjonować jako USB host, jednak nie potrafi dostarczyć napięcia zasilającego do zewnętrznych urządzeń. 
Oznacza to konieczność podłączenia zasilania poprzez kabel rozgałęziający (nazywany UBS Y cable). Zasilanie
podłączone poprzez taki kabel pozwala jednocześnie zasilać urządzenie podłączone do USB,
jak i samo urządzenie Kobo.

Do wyboru źródła można dojść po trzykrotnym naciśnięciu przycisku `M` a następnie wybraniu symbolu zębatki.

Do konfiguracji urządzeń wejściowych wchodzi się poprzez wybranie `Device`. TopHat/XCSoar pozwala na
pobieranie danych jednocześnie z kilku urządzeń wejściowych. Lista wszystkich kanałów wejściowych widoczna
jest na ekranie. Po wybraniu urządzenia możliwa jest jego konfiguracja po naciśnięciu przycisku `Edycja`. 

_TopHat/XCSoar przyjmuje i przetwarza dane GPS tylko w trybie lotu (Fly). Żadne dane nie są wyświetlane gdy
program działa w trybie sumulacji. Warto o tym pamietać podczas konfiguracji portów wejściowych._


### Wewnętrzny port szeregowy

W przypadku Kobo Mini wbudowany port szeregowy widoczny jest w konfiguracji urządzania jako
`ttymxc0`. Prędkość należy wybrać w zależności od typu podłączonego odbiornika GPS. Standardową
prędkością GPS jest 4800 bodów, w nowszych typach odbiorników GPS można spotkać się również z większymi
prędkościami (często 9600). Po wybraniu poprawnej prędkości urządzenia stan urządzenia powinien
zmienić się na _Połączony_. 

### Port szeregowy USB 

Urządzenia szeregowe podłączone poprzez konwerter USB <-> port szeregowy widoczne są jako `ttyUSB0`.
Obowiązujące zasady dotyczące prędkości są takie same jak w przypadku wbudowanego portu szeregowego.


Synchronizacja danych
---------------------

Kobo TopHat umożliwia synchronizację danych z zewnętrznym komputerem. Istnieją dwie metody
synchronizacji danych: za pomocą pamięci USB lub poprzez bezpośrednie podłączenie do komputera. 

### Synchronizacja poprzez pamięć USB  

Podłączenie pamięci USB możliwe jest poprzez kabel USB Y, który pozwala na podłączenie zewnętrznego zasilania.
Jeśli nawigacja znajduje się na stronie startowej (tak jak zaraz po włączeniu) to chwilę po podłączeniu
pamięci USB powinno pojawić się menu składające się z kilku przycisków pozwalających na wybranie kierunku
przesłania danych.

Dla potrzeb synchronizacji używany jest katalog o nazwie XCSoarData znajdujący się na podłączonym dysku USB.

#### Przesłanie lotów na pamięć USB
_`Download flights to USB card`_ - powoduje skopiowanie wszystkich zapisanych przez nawigację lotów do
podkatalogu `XCSoarData/logs`.

#### Przesłanie zadań do Kobo
_`Upload tasks`_ - powoduje skopiowania zadań z katalogu `XCSoarData/tasks`
do nawigacji TopHat.
  
#### Skopiowanie wszystkiego na pamięć USB
_`Download everything to USB card`_ - powoduje skopiowanie
całej zawartości katalogów używanych przez TopHat Kobo do podłączonej pamięci USB. Obejmuje to
również kopię plików konfiguracyjnych.

#### Wyczyść wewnętrzne katalogi urządzenia i skopiuj wszystko z pamięci USB
_`Clean Kobo data directory and then upload everything to Kobo`_ - czyści urządzenie i kopiuje pamięć USB
do TopHat.


### Podłączenie do komputera

Nawigację TopHat należy podłączyć do urządzenia poprzez kabel USB. Następnie należy wybrać
opcję `PC connect`. Stosowny przycisk znajduje się na dole, po lewej stronie. Po zatwierdzeniu
ostrzeżeń urządzenie restartuje się i rozpoczyna działanie jako czytnik książek. Jeśli nie jest
zarejestrowany, wystarczy wybrać opcję `Don’t have WiFi network`. Po chwili w komputerze do którego
podłączone jest Kobo powinien być widoczny dysk o nazwie `KOBOeReader`. Na dysku powinien być
widoczny katalog XCSoarData zawierający wewnętrzne pliki nawigacyjne.


Uaktualnienie oprogramowania
----------------------------

Do wykonania uaktualnienia oprogramowania należy podłączyć do urządzenia dysk USB podobnie jak 
w przypadku transferu plików opisanego powyżej. Na dysku USB należy umieścić nową wersję 
instalacyjną `KoboRoot.tgz` w głównym katalogu dysku. Po podłączeniu dysku USB nawigacja
sprawdzi istnienie pliku i oprócz operacji sychronizacji plików pojawi się przycisk
_`Upgrade Top Hat`_ pozwalający na uaktualnienie oprogramowania.


Procedury awaryjne - reboot Kobo
--------------------------------

W przypadku problemu z nawigacją możliwe jest zrestartowanie nawigacji. Procedura jest następująca:
- przesunąć przycisk znajdujący się w górnej części obudowy w prawą stronę, odczekać 10 sekund, zwolnić przycisk
- odczekać kolejne 10 sekund
- przesunąć ponownie przycisk w prawo i zaczekać 2 sekundy a następnie znów zwolnić przycisk

Jeśli bateria jest naładowana Kobo powinno uruchomić się od nowa. 

Dodatkowo w tylnej części obudowy Kobo znajduje się dodatkowy, mechaniczny przycisk _`reset`_. 
Aby się do niego dostać trzeba zdjąć pierwszą część obudowy. W tym celu 'plecy' urządzenia można
podważyć zaczynając od prawego górnego rogu. To właśnie dlatego w prawym górnym rogu jest nieco
wycięty róg obudowy. Do przycisku _`reset`_ można się dostać poprzez niewielki, okrągły otwór
o średnicy około 1mm. Aby przycisnąć przycisk trzeba posłużyć się np. spinaczem biurowym lub innym
podobnym drutem. Pozostałe otwory w drugiej części tylnej pokrywy to otwory zawierające śruby oraz
inne technologiczne otwory o kształcie prostokątnym. 
