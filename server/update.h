
#ifndef UPDATE_H
#define UPDATE_H

#include <ostream>

#include "common.h"

using namespace std;

#include "common.h"
#include "update.h"
#include "marshal.h"



extern const int kVyhernaVzdialenost ;
extern const int kVyherneBodyZaklad ;
extern const int kVyherneBodyNasobokP ;
extern const int kVydrzoveBody ;

extern const int kVezaDamageRandom ;

extern const int kUvodnaEnergia ;
extern const int kPrisunEnergieZaklad ;
extern const int kPrisunEnergieKill ;
extern const int kPrisunEnergieDead ;

extern const int kUtocnikHp[UTOCNIK_POCET_TYPOV];
extern const int kUtocnikRychlost[UTOCNIK_POCET_TYPOV]; 

extern const int kVezaCena0[VEZA_POCET_TYPOV] ;
extern const int kVezaCena1[VEZA_POCET_TYPOV] ;
extern const int kVezaCena2[VEZA_POCET_TYPOV];

extern const int kVezaCooldown[UTOCNIK_POCET_TYPOV];

extern const int kDamage[VEZA_POCET_BOJOVYCH][UTOCNIK_POCET_TYPOV];

void zapniObservation(std::ostream* observation);
Stav zaciatokHry(const Mapa& mapa, int hracov);
static void spravBfs(const Mapa& mapa, std::vector<int>& dist);
int vzdialenost(int x1, int y1, int x2, int y2);
template<class T> unsigned najdiXy(const vector<T>& v, int x, int y);
int zistiCenuVeze(const Stav& stav, int hrac, int typ);
void vysliUtocnika(Stav& stav, int odKoho, int komu, TypUtocnika typ);
void vezaZautoc(const Mapa& mapa, Stav& stav, int hrac, int veza, int utocnik);
bool vykonajPrikaz(const Mapa& mapa, Stav& stav, int hrac, const Prikaz& p);
void odsimulujUtocnika(const Mapa& mapa, Stav& stav, const std::vector<int>& dist, int hrac, int utocnik);
void odsimulujPrehru(const Mapa& mapa, Stav& stav, const std::vector<int>& dist, int hrac);
void odsimulujVezu(const Mapa& mapa, Stav& stav, const std::vector<int>& dist, int hrac, int veza);
void odsimulujKolo(const Mapa& mapa, Stav& stav, const std::vector<Odpoved>& akcie);
Stav zamaskujStav(const Mapa& mapa, const Stav& stav, int hrac);
static int indexHodnoty(const std::vector<int>& pole, int hodnota) ;
void odmaskujOdpoved(const Mapa& mapa, const Stav& stav, int hrac, Odpoved& odpoved) ;
vector<int> ktoriZiju(const Mapa& mapa, const Stav& stav);
bool hraSkoncila(const Mapa& mapa, const Stav& stav);
vector<int> zistiRank(const Stav& stav);



#endif
