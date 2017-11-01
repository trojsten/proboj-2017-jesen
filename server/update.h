
#ifndef UPDATE_H
#define UPDATE_H

#include <ostream>

#include "common.h"

void zapniObservation(std::ostream* observation);


Stav zaciatokHry(const Mapa& mapa, int hracov);
int vzdialenost(int x1, int y1, int x2, int y2);
int zistiCenuVeze(const Stav& stav, int hrac, int typ);
void vysliUtocnikov(Stav& stav, int odKoho, int komu, int typ);
void vezaZautoc(const Mapa& mapa, Stav& stav, int hrac, int veza, int utocnik);
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
