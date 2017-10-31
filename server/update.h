
#ifndef UPDATE_H
#define UPDATE_H

#include <ostream>

#include "common.h"

extern const int kZdrzanieUtoku;
extern const int kKoeficientCenyZburania;
extern const int kVyhernaVzdialenost;
extern const int kVyherneBodyZaklad;
extern const int kVyherneBodyNasobokP;
extern const int kVydrzoveBody;
extern const int kPeriodaPrirodnychUtokov;
extern const int kVezaDamageBase;
extern const int kVezaDamageRandom;

extern const int kUvodneDrevo;
extern const int kPrisunDreva;
extern const int kPrisunDrevaZaklad;
extern const int kNaboj;

extern const int kNasavacRange;
extern const int kTrvanieLevelBonusu;
extern const int kCaryChcuNaboj;
extern const int kLienkyChcuNaboj;
extern const int kLienkyKolkoSpomaluju;

extern const int kUtocnikHp0[UTOCNIK_POCET_TYPOV];
extern const int kUtocnikHp1[UTOCNIK_POCET_TYPOV];
extern const int kUtocnikRychlost[UTOCNIK_POCET_TYPOV];

extern const int kUtokCena[UTOCNIK_POCET_TYPOV];
extern const int kUtokPocet0P[UTOCNIK_POCET_TYPOV];
extern const int kUtokPocet1P[UTOCNIK_POCET_TYPOV];
extern const int kUtokRozostupy[UTOCNIK_POCET_TYPOV];

extern const int kVezaCena0[VEZA_POCET_TYPOV];
extern const int kVezaCena1[VEZA_POCET_TYPOV];
extern const int kVezaCena2[VEZA_POCET_TYPOV];
extern const bool kVezaMaNaboj[VEZA_POCET_TYPOV];

extern const int kOdolnost[UTOCNIK_POCET_TYPOV][VEZA_POCET_BOJOVYCH];

void zapniObservation(std::ostream* observation);

Stav zaciatokHry(const Mapa& mapa);
int vzdialenost(int x1, int y1, int x2, int y2);
int zistiCenuVeze(const Stav& stav, int hrac, int typ);
void vysliUtocnikov(Stav& stav, int odKoho, int komu, int typ, int level);
void vezaZautoc(const Mapa& mapa, Stav& stav, int hrac, int veza, int utocnik, int special = -1);
bool vykonajPrikaz(const Mapa& mapa, Stav& stav, int hrac, const Prikaz& p);
void odsimulujUtocnika(const Mapa& mapa, Stav& stav, const std::vector<int>& dist, int hrac, int utocnik);
void odsimulujPrehru(const Mapa& mapa, Stav& stav, const std::vector<int>& dist, int hrac);
void odsimulujVezu(const Mapa& mapa, Stav& stav, const std::vector<int>& dist, int hrac, int veza);
void odsimulujKolo(const Mapa& mapa, Stav& stav, const std::vector<Odpoved>& akcie);
Stav zamaskujStav(const Mapa& mapa, const Stav& stav, int hrac);
void odmaskujOdpoved(const Mapa& mapa, const Stav& stav, int hrac, Odpoved& odpoved);
std::vector<int> ktoriZiju(const Mapa& mapa, const Stav& stav);
bool hraSkoncila(const Mapa& mapa, const Stav& stav);
std::vector<int> zistiRank(const Stav& stav);

#endif
