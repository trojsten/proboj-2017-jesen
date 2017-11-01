#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <iostream>
#include <vector>
using namespace std;

#include "common.h"
#include "marshal.h"
#include "update.h"


Mapa mapa;
Stav stav;   // vzdy som hrac cislo 0
vector<Prikaz> prikazy;

int akoBudemUtocit;


// pomocna funkcia. volajte zo zistiTah(), napr.:
// if (!vykonaj(Prikaz::stavaj(10, 10, VEZA_LASER))) { /* zeby malo dreva? */ }
bool vykonaj(const Prikaz& p) {
  bool uspech = vykonajPrikaz(mapa, stav, 0, p);
  if (uspech) prikazy.push_back(p);   // potom to posleme serveru
  return uspech;
}


// main() zavola tuto funkciu, ked nacita mapu
void inicializuj() {
  srand(time(NULL) * getpid());
  akoBudemUtocit = rand() % UTOCNIK_POCET_TYPOV;
}


int pocetVezi(int typ) {
  int result = 0;
  FOREACH(it, stav.hraci[0].veze) if (it->typ == typ) result++;
  return result;
}


// main() zavola tuto funkciu, ked chce vediet, ake prikazy chceme vykonat
void zistiTah() {
  int mojLab = VEZA_LAB_PRVY + akoBudemUtocit;
  cerr<<"nieco\n";
  while (pocetVezi(mojLab) < 1) {
    if (!vykonaj(Prikaz::buduj(rand() % mapa.w, rand() % mapa.h, mojLab))) break;
  }
  while (true) {
    int naKoho = 1 + rand() % (mapa.pocetHracov-1);
    if (!vykonaj(Prikaz::utoc(akoBudemUtocit, naKoho))) break;
  }
  uloz(cerr, prikazy);
  // hmm este nejaku obranu by to chcelo...
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

