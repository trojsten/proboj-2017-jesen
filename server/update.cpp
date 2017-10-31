
#include <ostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <cmath>
using namespace std;

#include "common.h"
#include "update.h"

#define ERASE(v,x) (swap((x), *((v).end() - 1)), (v).pop_back())

static int DX[] = { 0, 1, 0, -1 };
static int DY[] = { -1, 0, 1, 0 };

const int kZdrzanieUtoku = 500;
const int kKoeficientCenyZburania = 2;
const int kVyhernaVzdialenost = 10;
const int kVyherneBodyZaklad = 50;
const int kVyherneBodyNasobokP = 2500;
const int kVydrzoveBody = 1;
const int kPeriodaPrirodnychUtokov = 60;
const int kVezaDamageBase = 35;
const int kVezaDamageRandom = 35;

const int kUvodneDrevo = 360;
const int kPrisunDreva = 1;
const int kPrisunDrevaZaklad = 4;
const int kNaboj = 1000;

const int kNasavacRange = 10;
const int kTrvanieLevelBonusu = 8;
const int kCaryChcuNaboj = 50;
const int kLienkyChcuNaboj = 70;
const int kLienkyKolkoSpomaluju = 5;

const int kUtocnikHp0[UTOCNIK_POCET_TYPOV] = { 100, 30, 80, 130, 400 };
const int kUtocnikHp1[UTOCNIK_POCET_TYPOV] = { 11, 3, 12, 20, 50 };
const int kUtocnikRychlost[UTOCNIK_POCET_TYPOV] = { 55, 80, 50, 25, 45 };

const int kUtokCena[UTOCNIK_POCET_TYPOV] = { 99, 96, 99, 99, 98 };
const int kUtokPocet0P[UTOCNIK_POCET_TYPOV] = { 1100, 3000, 900, 500, 260 };
const int kUtokPocet1P[UTOCNIK_POCET_TYPOV] = { 30, 200, 50, 36, 11 };
const int kUtokRozostupy[UTOCNIK_POCET_TYPOV] = { 80, 30, 110, 90, 250 };

// TODO kalibrovat!
const int kVezaCena0[VEZA_POCET_TYPOV] = { 159, 133, 147, 196, 141, 155, 128,   60, 42, 54,   72, 80, 61, 86, 75 };
const int kVezaCena1[VEZA_POCET_TYPOV] = { 10, 17, 7, 10, 5, 2, 4,   0, 11, 9,   0, 0, 0, 0, 0 };
const int kVezaCena2[VEZA_POCET_TYPOV] = { 0, 0, 0, 0, 0, 0, 0,   10, 0, 0,   0, 0, 0, 0, 0 };
const bool kVezaMaNaboj[VEZA_POCET_TYPOV] = { 0,0,1,1,0,1,0, 0,1,1, 0,0,0,0,0 };

const int kOdolnost[UTOCNIK_POCET_TYPOV][VEZA_POCET_BOJOVYCH] = {
  { 2, 1, 1, 1, 2, 4, 2 },
  { 2, 1, 4, 0, 1, 2, 4 },
  { 4, 2, 1, 4, 1, 2, 0 },
  { 1, 4, 2, 1, 1, 0, 1 },
  { 1, 1, 1, 4, 4, 1, 2 },
};


static ostream* g_observation;
void zapniObservation(ostream* observation) { g_observation = observation; }

#define OBSERVE(s,...) do {                                                    \
    if (!g_observation) break;                                                 \
    *g_observation << (s);                                                     \
    int __m[] = { __VA_ARGS__ };                                               \
    for (unsigned __i = 0; __i < sizeof(__m)/sizeof(*__m); __i++)              \
      *g_observation << " " << __m[__i];                                       \
    *g_observation << endl;                                                    \
  } while(0)


Stav zaciatokHry(const Mapa& mapa) {
  Stav stav;
  stav.cas = 0;
  stav.hraci.clear();
  stav.hraci.resize(mapa.pocetHracov);
  for (int i = 0; i < mapa.pocetHracov; i++) {
    Hrac& h = stav.hraci[i];

    h.drevo = kUvodneDrevo;

    // zoberieme nahodnu permutaciu, kde mapovanie[i] == 0
    h.mapovanie.resize(mapa.pocetHracov);
    for (int j = 0; j < mapa.pocetHracov; j++) h.mapovanie[j] = j;
    random_shuffle(h.mapovanie.begin() + 1, h.mapovanie.end());
    swap(h.mapovanie[0], h.mapovanie[i]);
  }
  return stav;
}


static void spravBfs(const Mapa& mapa, vector<int>& dist) {
  dist.clear();
  dist.resize(mapa.w * mapa.h, mapa.w * mapa.h * 2);
  queue<int> Q;
  for (int y = 0; y < mapa.h; y++) for (int x = 0; x < mapa.w; x++) {
    if (mapa.zisti(x, y) == MAPA_CIEL) {
      Q.push(y * mapa.w + x);
      dist[y * mapa.w + x] = 0;
    }
  }
  while (!Q.empty()) {
    int p = Q.front(); Q.pop();
    int y = p / mapa.w, x = p % mapa.w;
    for (int d = 0; d < 4; d++) {
      int ny = y + DY[d], nx = x + DX[d];
      int np = ny * mapa.w + nx, nd = dist[p] + 1;
      if (!mapa.priechodne(nx, ny)) continue;
      if (dist[np] <= nd) continue;
      dist[np] = nd;
      Q.push(np);
    }
  }
}


int vzdialenost(int x1, int y1, int x2, int y2) {
  x1 -= x2; y1 -= y2;
  return x1*x1 + y1*y1;
}


template<class T> unsigned najdiXy(const vector<T>& v, int x, int y) {
  for (unsigned i = 0; i < v.size(); i++) if (v[i].x == x && v[i].y == y) return i;
  return v.size();
}


int zistiCenuVeze(const Stav& stav, int hrac, int typ) {
  int pocet = 0;
  FOREACH(it, stav.hraci[hrac].veze) if (it->typ == typ) pocet++;
  return kVezaCena0[typ] + pocet * kVezaCena1[typ] + pocet * pocet * kVezaCena2[typ];
}


void vysliUtocnikov(Stav& stav, int odKoho, int komu, int typ, int level) {
  OBSERVE("vysliUtocnikov", odKoho, komu, typ, level);
  int pocetSkupin = (kUtokPocet0P[typ] + level * kUtokPocet1P[typ]) / 100;
  for (int i = 0; i < pocetSkupin; i++) {
    Utocnik u;
    u.x = u.y = -1;
    u.typ = typ;
    u.hp = kUtocnikHp0[typ] + level * kUtocnikHp1[typ];
    u.ktorehoHraca = odKoho;
    u.level = level;
    u.pohybovyTimer = kZdrzanieUtoku + i * kUtokRozostupy[typ];
    u.kolkoSpomaleny = 0;
    OBSERVE("insert.prichadzajuci", komu, (int)stav.hraci[komu].prichadzajuci.size());
    stav.hraci[komu].prichadzajuci.push_back(u);
  }
}


void vezaZautoc(const Mapa& mapa, Stav& stav, int hrac, int veza, int utocnik, int special) {
  Hrac& h = stav.hraci[hrac];
  Veza& v = h.veze[veza];
  Utocnik& u = h.utocnici[utocnik];

  int damage = 0;
  for (int i = 0; i < v.getLevel(); i++) {
    if (special == -1) {
      damage += kVezaDamageBase + rand() % kVezaDamageRandom;
    }
    else {
      // veci, co sa deju aj na klientovi, musia byt deterministicke
      damage += kVezaDamageBase + kVezaDamageRandom / 2;
    }
  }
  damage *= kOdolnost[u.typ][v.typ];
  if (v.typ == VEZA_CARY) {
    if (v.energia >= kCaryChcuNaboj) {
      v.energia -= kCaryChcuNaboj;
    }
    else {
      damage = damage * 3 / 4;
    }
  }
  if (v.typ == VEZA_LIENKY) {
    if (v.energia >= kLienkyChcuNaboj) {
      v.energia -= kLienkyChcuNaboj;
      u.kolkoSpomaleny = max(u.kolkoSpomaleny, kLienkyKolkoSpomaluju);
      OBSERVE("vezaZautoc.spomal", hrac, v.x, v.y, utocnik);
    }
  }
  if (v.typ == VEZA_LUPA && special > 0) {
    damage = damage * 3 / 2 / special;
  }
  else if (v.typ == VEZA_LUPA) {
    damage = damage * 2 / 3;
  }

  OBSERVE("vezaZautoc", hrac, v.x, v.y, utocnik, damage);
  u.hp -= damage;
  if (u.hp <= 0) {
    OBSERVE("vezaZautoc.umrel", hrac, v.x, v.y, utocnik);
    OBSERVE("erase.utocnici", hrac, utocnik, (int)h.utocnici.size() - 1);
    ERASE(h.utocnici, h.utocnici[utocnik]);
  }
}


bool vykonajPrikaz(const Mapa& mapa, Stav& stav, int hrac, const Prikaz& p) {
  Hrac& h = stav.hraci[hrac];
  switch (p.typ) {
    case PRIKAZ_BUDUJ: {
      int typ = p.a;
      if (typ < 0 || typ >= VEZA_POCET_TYPOV) return false;
      if (najdiXy(h.veze, p.x, p.y) != h.veze.size()) return false;
      if (mapa.zisti(p.x, p.y) != MAPA_POZEMOK) return false;
      int cena = zistiCenuVeze(stav, hrac, typ);
      if (h.drevo < cena) return false;
      OBSERVE("vykonajPrikaz.buduj", hrac, p.x, p.y);
      h.drevo -= cena;
      Veza nova;
      nova.x = p.x;
      nova.y = p.y;
      nova.typ = typ;
      nova.energia = 0;
      nova.baseLevel = 1;
      nova.terazTahala = 1;
      h.veze.push_back(nova);
      return true;
    }

    case PRIKAZ_BURAJ: {
      unsigned i = najdiXy(h.veze, p.x, p.y);
      if (i == h.veze.size()) return false;
      if (h.veze[i].terazTahala) return false;
      int typ = h.veze[i].typ;
      int cena = zistiCenuVeze(stav, hrac, typ) / kKoeficientCenyZburania;
      if (h.drevo < cena) return false;
      OBSERVE("vykonajPrikaz.buraj", hrac, p.x, p.y);
      h.drevo -= cena;
      ERASE(h.veze, h.veze[i]);
      return true;
    }

    case PRIKAZ_AKTIVUJ: {
      int i = -1;
      FOREACH(it, h.veze) {
        if (it->x == p.x && it->y == p.y) i = it - h.veze.begin();
      }
      if (i == -1) return false;
      if (h.veze[i].terazTahala) return false;
      if (h.veze[i].typ != VEZA_NASAVAC && h.veze[i].energia < kNaboj) return false;
      switch (h.veze[i].typ) {
        case VEZA_LUPA: {
          if (vzdialenost(p.x, p.y, p.a, p.b) > h.veze[i].getLevel()) return false;
          vector<int> blizki;
          FOREACH(it, h.utocnici) {
            if (abs(it->x - p.a) <= 1 && abs(it->x - p.b) <= 1) {
              blizki.push_back(it - h.utocnici.begin());
            }
          }
          OBSERVE("vykonajPrikaz.aktivuj.lupa", hrac, p.x, p.y, p.a, p.b);
          FOREACH(it, blizki) {
            vezaZautoc(mapa, stav, hrac, i, *it, blizki.size());
          }
          h.veze[i].terazTahala = 1;
          return true;
        }

        case VEZA_NASAVAC: {
          unsigned ciel = najdiXy(h.veze, p.a, p.b);
          if (ciel == h.veze.size()) return false;
          if (p.a == p.x && p.b == p.y) return false;
          if (!kVezaMaNaboj[h.veze[ciel].typ]) return false;
          if (vzdialenost(p.x, p.y, p.a, p.b) > kNasavacRange) return false;
          OBSERVE("vykonajPrikaz.aktivuj.nasavac", hrac, p.x, p.y, p.a, p.b);
          if (h.veze[i].energia >= kNaboj) {
            h.veze[ciel].energia += kNaboj;
            h.veze[i].energia -= kNaboj;
          }
          else {
            h.veze[ciel].energia += h.veze[i].energia / 2;
            h.veze[i].energia = 0;
          }
          h.veze[i].terazTahala = 1;
          return true;
        }

        case VEZA_MOTIVATOR: {
          unsigned ciel = najdiXy(h.veze, p.a, p.b);
          if (ciel == h.veze.size()) return false;
          if (p.a == p.x && p.b == p.y) return false;
          if (vzdialenost(p.x, p.y, p.a, p.b) > kNasavacRange) return false;
          h.veze[i].energia -= kNaboj;
          h.veze[i].terazTahala = 1;
          if (h.veze[i].getLevel() > 1) {
            OBSERVE("vykonajPrikaz.aktivuj.motivator.trvaly", hrac, p.x, p.y, p.a, p.b);
            h.veze[ciel].baseLevel++;
            ERASE(h.veze, h.veze[i]);
          }
          else {
            OBSERVE("vykonajPrikaz.aktivuj.motivator", hrac, p.x, p.y, p.a, p.b);
            h.veze[ciel].levelBonusy.push_back(stav.cas + kTrvanieLevelBonusu);
          }
          return true;
        }
      }
      return false;
    }

    case PRIKAZ_UTOC: {
      int typ = p.a, ciel = p.b;
      if (typ < 0 || typ >= UTOCNIK_POCET_TYPOV) return false;
      if (ciel < 0 || ciel >= mapa.pocetHracov) return false;
      if (h.drevo < kUtokCena[typ]) return false;
      if (stav.hraci[ciel].umrel) return false;
      FOREACH(ph, stav.hraci) {
        if (ph->umrel) continue;
        FOREACH(pu, ph->utocnici) {
          if (pu->typ == typ && pu->ktorehoHraca == hrac) return false;
        }
        FOREACH(pu, ph->prichadzajuci) {
          if (pu->typ == typ && pu->ktorehoHraca == hrac) return false;
        }
      }
      bool moze = 0;
      int level = 1;
      FOREACH(it, h.veze) if (it->typ == VEZA_LAB_PRVY + typ) {
        moze = 1;
        if (it->getLevel() > 1) level++;
      }
      if (!moze) return false;
      h.drevo -= kUtokCena[typ];
      vysliUtocnikov(stav, hrac, ciel, typ, level);
      return true;
    }
  }
  return false;
}


void odsimulujUtocnika(const Mapa& mapa, Stav& stav, const vector<int>& dist, int hrac, int utocnik) {
  Utocnik& u = stav.hraci[hrac].utocnici[utocnik];

  u.pohybovyTimer -= kUtocnikRychlost[u.typ] * (u.kolkoSpomaleny ? 66 : 100) / 100;
  while (u.pohybovyTimer < 0) {
    u.pohybovyTimer += 100;

    int best = dist[u.y*mapa.w + u.x];
    for (int d = 0; d < 4; d++) {
      int nx = u.x + DX[d], ny = u.y + DY[d];
      if (mapa.priechodne(nx, ny)) {
        best = min(best, dist[ny*mapa.w + nx]);
      }
    }
    vector<int> smery;
    for (int d = 0; d < 4; d++) {
      int nx = u.x + DX[d], ny = u.y + DY[d];
      if (mapa.priechodne(nx, ny) && dist[ny*mapa.w + nx] == best) {
        smery.push_back(d);
      }
    }
    if (smery.size()) {
      int smer = smery[rand() % smery.size()];
      OBSERVE("odsimulujUtocnika", hrac, utocnik, u.x, u.y, u.x + DX[smer], u.y + DY[smer]);
      u.x += DX[smer];
      u.y += DY[smer];
    }
  }
}


void odsimulujPrehru(const Mapa& mapa, Stav& stav, const vector<int>& dist, int hrac) {
  Hrac& h = stav.hraci[hrac];
  if (!h.umrel) {
    vector<int> hlasy(mapa.pocetHracov, 0);
    bool prehral = false;
    FOREACH(it, h.utocnici) {
      if (mapa.zisti(it->x, it->y) == MAPA_CIEL) prehral = true;
      if (dist[it->y*mapa.w+it->x] <= kVyhernaVzdialenost && it->ktorehoHraca != -1) {
        hlasy[it->ktorehoHraca] += it->hp;
      }
    }
    if (!prehral) return;

    int totalLevel = 0;
    FOREACH(it, h.veze) totalLevel += it->getLevel();
    int best = *max_element(hlasy.begin(), hlasy.end());
    for (int i = 0; i < mapa.pocetHracov; i++) {
      if (i != hrac && hlasy[i] && hlasy[i] > best/2) {
        OBSERVE("odsimulujPrehru", hrac, i, totalLevel);
        stav.hraci[i].body += kVyherneBodyZaklad + kVyherneBodyNasobokP * totalLevel / 100;
      }
    }
    h.umrel = true;
  }

  FOREACH(it, h.utocnici) {
    if (mapa.zisti(it->x, it->y) == MAPA_CIEL) {
      OBSERVE("odsimulujPrehru.vCieli", hrac, it - h.utocnici.begin());
      OBSERVE("erase.utocnici", hrac, it - h.utocnici.begin(), (int)h.utocnici.size() - 1);
      ERASE(h.utocnici, *it);
      --it;
    }
  }
}


void odsimulujVezu(const Mapa& mapa, Stav& stav, const vector<int>& dist, int hrac, int veza) {
  Hrac& h = stav.hraci[hrac];
  Veza& v = h.veze[veza];

  if (v.typ < VEZA_POCET_BOJOVYCH) {
    int best = -1, bestdist = -1;
    FOREACH(pu, h.utocnici) {
      // dostrel veze = level veze
      if (vzdialenost(v.x, v.y, pu->x, pu->y) <= v.getLevel()) {
        // mojdist == kolko zhruba tahov potrva, kym sa dostanem do ciela
        int mojdist = ((dist[pu->y*mapa.w + pu->x] - 1) * 100 + pu->pohybovyTimer) / kUtocnikRychlost[pu->typ];
        if (best == -1 || mojdist < bestdist) {
          best = pu - h.utocnici.begin();
          bestdist = mojdist;
        }
      }
    }
    if (best != -1) {
      vezaZautoc(mapa, stav, hrac, veza, best);
    }
  }

  if (v.typ == VEZA_ZBERAC) {
    h.drevo += kPrisunDreva;
  }

  if (v.typ == VEZA_NASAVAC) {
    int l = mapa.loziska[v.y][v.x];
    if (v.getLevel() > 1) l++;
    v.energia += 3*l*l*l;
  }
}


void odsimulujKolo(const Mapa& mapa, Stav& stav, const vector<Odpoved>& akcie) {
  vector<int> dist;
  spravBfs(mapa, dist);

  for (int i = 0; i < mapa.pocetHracov; i++) {
    FOREACH(it, akcie[i]) {
      vykonajPrikaz(mapa, stav, i, *it);
    }
  }

  for (int i = 0; i < mapa.pocetHracov; i++) {
    for (unsigned j = 0; j < stav.hraci[i].utocnici.size(); j++) {
      odsimulujUtocnika(mapa, stav, dist, i, j);
    }
  }

  for (int i = 0; i < mapa.pocetHracov; i++) {
    odsimulujPrehru(mapa, stav, dist, i);
  }

  for (int i = 0; i < mapa.pocetHracov; i++) {
    for (unsigned j = 0; j < stav.hraci[i].veze.size(); j++) {
      odsimulujVezu(mapa, stav, dist, i, j);
    }
  }

  FOREACH(ph, stav.hraci) {
    FOREACH(pv, ph->veze) {
      pv->terazTahala = 0;
      vector<int> noveBonusy;
      FOREACH(pb, pv->levelBonusy) (*pb)--;
      pv->levelBonusy.erase(remove(pv->levelBonusy.begin(), pv->levelBonusy.end(), 0), pv->levelBonusy.end());
    }

    FOREACH(pu, ph->utocnici) {
      if (pu->kolkoSpomaleny) pu->kolkoSpomaleny--;
    }

    FOREACH(pu, ph->prichadzajuci) {
      pu->pohybovyTimer -= kUtocnikRychlost[pu->typ];
      if (pu->pohybovyTimer >= 0) continue;
      pu->pohybovyTimer += 100;

      // spawnujeme prijdeneho ucastnika
      vector<pair<int,int> > spawnPolia;
      for (int y = 0; y < mapa.h; y++) for (int x = 0; x < mapa.w; x++) {
        if (mapa.zisti(x, y) == MAPA_SPAWN) spawnPolia.push_back(make_pair(x, y));
      }
      if (!spawnPolia.size()) return;
      pair<int,int> ktory = spawnPolia[rand() % spawnPolia.size()];
      pu->x = ktory.first;
      pu->y = ktory.second;
      OBSERVE("insert.utocnici", ph - stav.hraci.begin(), (int)ph->utocnici.size(), pu - ph->prichadzajuci.begin());
      ph->utocnici.push_back(*pu);
      OBSERVE("erase.prichadzajuci", ph - stav.hraci.begin(), pu - ph->prichadzajuci.begin(), (int)ph->prichadzajuci.size() - 1);
      ERASE(ph->prichadzajuci, *pu);
      --pu;
    }

    ph->drevo += kPrisunDrevaZaklad;
    if (!ph->umrel) ph->body += kVydrzoveBody;
  }

  if (stav.cas && stav.cas % kPeriodaPrirodnychUtokov == 0) {
    int typ = rand() % UTOCNIK_POCET_TYPOV;
    int n = (stav.cas / kPeriodaPrirodnychUtokov);
    int level = n * n + n - 1;
    for (int i = 0; i < mapa.pocetHracov; i++) if (!stav.hraci[i].umrel) {
      vysliUtocnikov(stav, -1, i, typ, level);
    }
  }

  OBSERVE("odsimulujKolo.koniec", stav.cas);
  stav.cas++;
}


Stav zamaskujStav(const Mapa& mapa, const Stav& stav, int hrac) {
  const vector<int>& mapovanie = stav.hraci[hrac].mapovanie;

  Stav novy;
  novy.cas = stav.cas;
  novy.hraci.resize(stav.hraci.size());
  for (int i = 0; i < mapa.pocetHracov; i++) {
    novy.hraci[mapovanie[i]] = stav.hraci[i];
    Hrac& h = novy.hraci[mapovanie[i]];

    FOREACH(it, h.utocnici) {
      if (it->ktorehoHraca != -1) it->ktorehoHraca = mapovanie[it->ktorehoHraca];
    }

    h.prichadzajuci.clear();
    h.mapovanie.clear();
  }

  return novy;
}


static int indexHodnoty(const vector<int>& pole, int hodnota) {
  return find(pole.begin(), pole.end(), hodnota) - pole.begin();
}


void odmaskujOdpoved(const Mapa& mapa, const Stav& stav, int hrac, Odpoved& odpoved) {
  const Hrac& h = stav.hraci[hrac];
  FOREACH(it, odpoved) {
    if (it->typ == PRIKAZ_UTOC) it->b = indexHodnoty(h.mapovanie, it->b);
  }
}


vector<int> ktoriZiju(const Mapa& mapa, const Stav& stav) {
  vector<int> result;
  FOREACH(it, stav.hraci) {
    if (!it->umrel) result.push_back(it - stav.hraci.begin());
  }
  return result;
}


bool hraSkoncila(const Mapa& mapa, const Stav& stav) {
  return ktoriZiju(mapa, stav).size() == 0;
}


vector<int> zistiRank(const Stav& stav) {
  vector<int> rank;
  FOREACH(it, stav.hraci) {
    rank.push_back(it->body);
  }
  return rank;
}

