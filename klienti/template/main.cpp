#include <cstdio>
#include <iostream>
#include <vector>
using namespace std;

#include "common.h"
#include "marshal.h"
#include "update.h"


Mapa mapa;
Stav stav;   // vzdy som hrac cislo 0
vector<Prikaz> prikazy;


// pomocna funkcia. volajte zo zistiTah(), napr.:
// if (!vykonaj(Prikaz::stavaj(10, 10, DRAK))) { /* zeby malo energie? */ }
bool vykonaj(const Prikaz& p) {
  bool uspech = vykonajPrikaz(mapa, stav, 0, p);
  if (uspech) prikazy.push_back(p);   // potom to posleme serveru
  return uspech;
}


// main() zavola tuto funkciu, ked nacita mapu
void inicializuj() {
  // (sem patri vas kod)
}


// main() zavola tuto funkciu, ked chce vediet, ake prikazy chceme vykonat
void zistiTah() {
  // (sem patri vas kod)
}


int main() {
  // v tejto funkcii su vseobecne veci, nemusite ju menit (ale mozte).

  nacitaj(cin, mapa);
  inicializuj();

  while (cin.good()) {
    nacitaj(cin, stav);
    prikazy.clear();
    zistiTah();
    uloz(cout, prikazy);
    cout << ".\n" << flush;   // bodka a flush = koniec odpovede
  }

  return 0;
}

