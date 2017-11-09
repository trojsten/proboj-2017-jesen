
Čo je proboj
------------

Proboj je počítačová hra, ktorej hráčmi nie ste vy, ale programy, čo napíšete.


Zdrojáky
--------

Štandardný hráč, čiže klient (v adresári `klienti/template`), sa skladá z jediného
zdrojáku `main.cpp`. Ale môžete ho rozdeliť aj na viacero.

V serveri (adresár `server`) je tiež zopár zdrojákov, čo vás bude zaujímať.

- `common.h` obsahuje základné štruktúry, čo váš klient dostane k dispozícii.
- `update.cpp` obsahuje všetky herné konštanty, a tiež
  implementáciu väčšiny herných pravidiel, takže ak v pravidlách nie je niečo
  jasné, pýtajte sa alebo sa skúste pozrieť tam.

Ak máte potrebu čítať ostatné zdrojáky, niekde nastala chyba. 
Pri kódení vášho klienta vám asi nepomôžu.

Ako kódiť klienta
-----------------

Skopírujte obsah `klienti/template` (alebo `klienti/hlupy`) do iného adresára a 
niečo v ňom nakóďte.

V priečinku proboja spustite `make`, čím všetko skompilujete. (Váš klient by 
mal byť v priečinku `klienti/meno_klienta`)

Potom spustite `./server/server zaznamy/01 mapy/symmetry.ppm klienti/vasklient klienti/vasklient klienti/hlupy` To spustí hru s troma hráčmi (vaším, druhým
vaším a hlúpym) a uloží záznam do `zaznamy/01`. Ten si môžete pozrieť tak,
že spustíte v priečinku proboja `python3 -m http.server` a v prehliadači 
otvoríte `http://localhost:8000/observer/observer.html`

Na začiatku hry dostane váš klient informácie o hre --- terén mapy.
Tiež má pritom viac času, aby sa mohol inicializovať --- server chvíľu počká, 
kým začne simulovať.

Server posiela klientovi každé kolo nový stav hry a čaká na jeho odpoveď. Časový
limit je dosť veľký, ale ak to bude naozaj dlho trvať, server nečaká. Ak klient
dlho neodpovedá, alebo program skončí (napríklad chybou), server ho znovu spustí
a pošle mu úvodné dáta (mapu).

Keď server spustíte u vás, je to len na testovanie. Na hlavnom počítači to beží na
ostro. Je tam aj webové rozhranie, cez ktoré môžete uploadovať vašich klientov.
Uploadujú sa zdrojáky a tie sa potom skompilujú (konkrétne sa spustí `make
naserveri SERVERDIR=/adresar/kde/je/server`).


Aký je proboj
-------------

Hra sa volá Tower Defense.

Každý hráč má (rovnakú) vlastnú mapu. 
Na mape je váša trofej, ku ktorej sa snažia dostať votrelci od ostatných hráčov.
Hrad môžete chrániť tým, že postavíte veže, ktoré na nich budú útočiť. Okrem toho, môžete 
ešte posaviť špeciálne laboratória, z ktorých viete posielať útočníkov ostatným hráčom.


Ako sa ťahá
-----------

V každom kole dostanete celý stav hry, to znamená o každom hráčovi, zistíte aké má 
veže, kde sú, akí útočníci od neho útočia

Template klienta je v `C++`, ak chcete použiť iný jazyk, tu budú technické 
podrobnosti (ak bude mať niekto záujem a povie mi to):

Pravidlá hry
------------

Váš program posiela 3 typi príkazov. Príkazy typu BUDUJ, pomocou ktorého staviate veže.
Existuje 5 typov obranných veží: TROLL, HYDRA, DRAK, MAG, RAPTOR. Každá z nich dokáže 
útočiť na všetkých útočníkov (okrem hydri, ktorá útočí len na lietajúce zajace).
postavenie veže vyžaduje nejaké množstvo energie, ktoré z každou ďaľšou postavenou vežou rastie.

Energiu získavati viacerímy spôsobmi. Nejaké množstvo energie máte na začiatku, niečo dostanete každé kolo,
ale hlavné je, že energiu získate aj za každé políčko ktoré prejde útočník ktorého vyšlete a ktorého zabijete.

Existujú 4 typi útočníkov, každý typ generuje jedna špeciálna veža. Útočníci sú: ZAJAC, ZOMBIE, KORYTNACKA, JEDNOROZEC.

Veža dokáže vyslať jedného útočníka každých niekoľko kôl.

Všetky vyššie spomenuté konštanty nájdete v súbore `update.cpp`.
