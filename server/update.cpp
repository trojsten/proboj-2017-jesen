
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
const int kVezaDamageBase = 35;
const int kVezaDamageRandom = 35;

const int kUvodnaEnergia = 360;
const int kPrisunDreva = 1;
const int kPrisunDrevaZaklad = 4;

const int kNasavacRange = 10;
const int kTrvanieLevelBonusu = 8;
const int kCaryChcuNaboj = 50;
const int kLienkyChcuNaboj = 70;
const int kLienkyKolkoSpomaluju = 5;

const int kUtocnikHp[UTOCNIK_POCET_TYPOV] = { 100, 30, 80, 130, 400 };
const int kUtocnikHp1[UTOCNIK_POCET_TYPOV] = { 11, 3, 12, 20, 50 };
const int kUtocnikRychlost[UTOCNIK_POCET_TYPOV] = { 55, 80, 50, 25, 45 };

const int kUtokCena[UTOCNIK_POCET_TYPOV] = { 99, 96, 99, 99, 98 };

// TODO kalibrovat! TODO nastaviť od oka
const int kVezaCena0[VEZA_POCET_TYPOV] = { 159, 133, 147, 196, 141,   72, 80, 61, 86 };
const int kVezaCena1[VEZA_POCET_TYPOV] = { 10, 17, 7, 10, 5,   0, 0, 0, 0 };
const int kVezaCena2[VEZA_POCET_TYPOV] = { 0, 0, 0, 0, 0,    0, 0, 0, 0 };

const int kDamage[UTOCNIK_POCET_TYPOV][VEZA_POCET_BOJOVYCH] = {  //násobiteľ damage nepáči sa mi
  { 2, 1, 1, 1, 2, },
  { 2, 1, 4, 0, 1, },
  { 4, 2, 1, 4, 1, },
  { 1, 4, 2, 1, 1, },
};


static ostream* g_observation;
void zapniObservation(ostream* observation) { g_observation = observation; }


//este viac agicke makro by Tomi
#define OBSERVE(s,...) do {                                                    \
    if (!g_observation) break;                                                 \
    *g_observation << (s);                                                     \
    int __m[] = { __VA_ARGS__ };                                               \
    for (unsigned __i = 0; __i < sizeof(__m)/sizeof(*__m); __i++)              \
      *g_observation << " " << __m[__i];                                       \
    *g_observation << endl;                                                    \
  } while(0)


Stav zaciatokHry(const Mapa& mapa, int hracov) {
  Stav stav;
  stav.cas = 0;
  stav.hraci.clear();
  stav.hraci.resize(hracov);
  for (int i = 0; i < hracov; i++) {
    Hrac& h = stav.hraci[i];

    h.energia = kUvodnaEnergia;

    // zoberieme nahodnu permutaciu, kde mapovanie[i] == 0
    // netreba nahodnu, iba vymenime i a 0
    h.mapovanie.resize(mapa.pocetHracov);
    for (int j = 0; j < mapa.pocetHracov; j++) h.mapovanie[j] = j;
    //random_shuffle(h.mapovanie.begin() + 1, h.mapovanie.end());
    swap(h.mapovanie[0], h.mapovanie[i]);
  }
  return stav;
}

//vyráta každému políčku cesty vzdialenost od ciela (na indexe y * mapa.w + x)
static void spravBfs(const Mapa& mapa, vector<int>& dist) {
  dist.clear();
  dist.resize(mapa.w * mapa.h, mapa.w * mapa.h * 2);
  queue<int> Q;
  for (int y = 0; y < mapa.h; y++) 
        for (int x = 0; x < mapa.w; x++) {
            if (mapa.zisti(x, y) == CIEL) {
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

//TODO bolo na nejaké veci s levelmi
int vzdialenost(int x1, int y1, int x2, int y2) {
  x1 -= x2; y1 -= y2;
  return x1*x1 + y1*y1;
}

//na hladanie možných obetí pre vežu
template<class T> unsigned najdiXy(const vector<T>& v, int x, int y) {
  for (unsigned i = 0; i < v.size(); i++) if (v[i].x == x && v[i].y == y) return i;
  return v.size();
}

//TODO zmeniť nech to nejde cez 3 konštanty
int zistiCenuVeze(const Stav& stav, int hrac, int typ) {
  int pocet = 0;
  FOREACH(it, stav.hraci[hrac].veze) if (it->typ == typ) pocet++;
  return kVezaCena0[typ] + pocet * kVezaCena1[typ] + pocet * pocet * kVezaCena2[typ];
}


void vysliUtocnika(Stav& stav, int odKoho, int komu, TypUtocnika typ) {
  OBSERVE("vysliUtocnikov", odKoho, komu, typ);
    Utocnik u;
    u.x = u.y = -1;
    u.typ = typ;
    u.hp = kUtocnikHp[typ];
    u.ktorehoHraca = odKoho;
    u.pohybovyTimer = kUtocnikRychlost[typ];
    stav.hraci[komu].prichadzajuci.push_back(u); //TODO nech to nehadze do prichadzajuci ale do utocnici
}


void vezaZautoc(const Mapa& mapa, Stav& stav, int hrac, int veza, int utocnik) {
  Hrac& h = stav.hraci[hrac];
  Veza& v = h.veze[veza];
  Utocnik& u = h.utocnici[utocnik];

  int damage = 0;
  damage += kDamage[v.typ][u.typ] + rand() % kVezaDamageRandom;
  

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
    case BUDUJ: {
      int typ = p.a;
      if (typ < 0 || typ >= VEZA_POCET_TYPOV) return false;            //TODO nejake warningy
      if (najdiXy(h.veze, p.x, p.y) != h.veze.size()) return false;  //policko nieje volne
      if (mapa.zisti(p.x, p.y) != POZEMOK) return false;
      int cena = zistiCenuVeze(stav, hrac, typ);
      if (h.energia < cena) return false;
      OBSERVE("vykonajPrikaz.buduj", hrac, p.x, p.y, typ);
      h.energia -= cena;
      Veza nova;
      nova.x = p.x;
      nova.y = p.y;
      nova.typ = typ;
      nova.terazTahala = 1;
      h.veze.push_back(nova);
      return true;
    }

    case BURAJ: {
      unsigned i = najdiXy(h.veze, p.x, p.y);
      if (i == h.veze.size()) return false;   //TODO warningy
      if (h.veze[i].terazTahala) return false;
      OBSERVE("vykonajPrikaz.buraj", hrac, p.x, p.y);
      ERASE(h.veze, h.veze[i]);
      return true;
    }

    case UTOC: {
      TypUtocnika typ = (TypUtocnika)p.a;
      int ciel = p.b;
      if (typ < 0 || typ >= UTOCNIK_POCET_TYPOV) return false;
      if (ciel <= 0 || ciel >= mapa.pocetHracov) return false;  //na seba neutocime
      //if (h.energia < kUtokCena[typ]) return false;
      if (stav.hraci[ciel].umrel) return false;
      int uz_poslal=0;
      FOREACH(ph, stav.hraci) {
        if (ph->umrel) continue;
//         FOREACH(pu, ph->utocnici) {
//           if (pu->typ == typ && pu->ktorehoHraca == hrac) return false;
//         }
        FOREACH(pu, ph->prichadzajuci) {
          if (pu->typ == typ && pu->ktorehoHraca == hrac) uz_poslal++;
        }
      }
      int moze = 0;
      FOREACH(it, h.veze) if (it->typ == VEZA_LAB_PRVY + typ) {
        moze++;
      }
      if (moze<=uz_poslal) return false;
//       h.drevo -= kUtokCena[typ];
      vysliUtocnika(stav, hrac, ciel, typ);
      return true;
    }
  }
  return false; //TODO warning
}


void odsimulujUtocnika(const Mapa& mapa, Stav& stav, const vector<int>& dist, int hrac, int utocnik) {
  Utocnik& u = stav.hraci[hrac].utocnici[utocnik];

  u.pohybovyTimer --;
  if (u.pohybovyTimer < 0) {
    u.pohybovyTimer = kUtocnikRychlost[u.typ];

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
    vector<int> hlasy(mapa.pocetHracov, 0); //kto ma zasluhy za prehru
    bool prehral = false;
    FOREACH(it, h.utocnici) {
      if (mapa.zisti(it->x, it->y) == CIEL) prehral = true;
      if (dist[it->y*mapa.w+it->x] <= kVyhernaVzdialenost && it->ktorehoHraca != -1) { //TODO naco je kVyhernaVzdialenost????
        hlasy[it->ktorehoHraca] += it->hp;
      }
    }
    if (!prehral) return;

//     int totalLevel = 0;
//     FOREACH(it, h.veze) totalLevel += it->getLevel();
    int best = *max_element(hlasy.begin(), hlasy.end());
    for (int i = 0; i < mapa.pocetHracov; i++) {
      if (i != hrac && hlasy[i] && hlasy[i] > best/2) {
        OBSERVE("odsimulujPrehru", hrac, i);
        stav.hraci[i].body += kVyherneBodyZaklad + kVyherneBodyNasobokP;// * totalLevel / 100;
      }
    }
    h.umrel = true;
  }

  FOREACH(it, h.utocnici) {
    if (mapa.zisti(it->x, it->y) == CIEL) {
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
      if (vzdialenost(v.x, v.y, pu->x, pu->y) <= 1) {
        // mojdist == kolko tahov potrva, kym sa dostanem do ciela
        int mojdist = ((dist[pu->y*mapa.w + pu->x] - 1) * kUtocnikRychlost[pu->typ] + pu->pohybovyTimer);
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
}


void odsimulujKolo(const Mapa& mapa, Stav& stav, const vector<Odpoved>& akcie) {
  vector<int> dist;
  spravBfs(mapa, dist);  //naco to robi kazde kolo?

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
    }


    FOREACH(pu, ph->prichadzajuci) {
      pu->pohybovyTimer--;
      if (pu->pohybovyTimer >= 0) continue; //TODO rovno do utoku
      pu->pohybovyTimer += 100;

      // spawnujeme prijdeneho utocnika na nahodnom spawne
      vector<pair<int,int> > spawnPolia;
      for (int y = 0; y < mapa.h; y++) for (int x = 0; x < mapa.w; x++) {
        if (mapa.zisti(x, y) == SPAWN) spawnPolia.push_back(make_pair(x, y));
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

    ph->energia += kPrisunDrevaZaklad; //TODO zisk energie za vsetko
    if (!ph->umrel) ph->body += kVydrzoveBody; //TODO body budeme ratat inak
  }

//   if (stav.cas && stav.cas % kPeriodaPrirodnychUtokov == 0) {
//     int typ = rand() % UTOCNIK_POCET_TYPOV;
//     int n = (stav.cas / kPeriodaPrirodnychUtokov);
//     int level = n * n + n - 1;
//     for (int i = 0; i < mapa.pocetHracov; i++) if (!stav.hraci[i].umrel) {
//       vysliUtocnikov(stav, -1, i, typ, level);
//     }
//   }

  OBSERVE("odsimulujKolo.koniec", stav.cas);
  stav.cas++;
}

//vela sa toho nemaskuje, len tolko, aby seba dostal ako 0
Stav zamaskujStav(const Mapa& mapa, const Stav& stav, int hrac) {
  const vector<int>& mapovanie = stav.hraci[hrac].mapovanie;

  Stav novy;
  novy.cas = stav.cas;
  novy.hraci.resize(stav.hraci.size());
  for (int i = 0; i < mapa.pocetHracov; i++) {
    novy.hraci[mapovanie[i]] = stav.hraci[i];
    Hrac& h = novy.hraci[mapovanie[i]];

    FOREACH(it, h.utocnici) {
      it->ktorehoHraca = mapovanie[it->ktorehoHraca];
    }

    h.prichadzajuci.clear();
    h.mapovanie.clear();
  }

  return novy;
}

//funny funkcia na inverzne maskovanie
static int indexHodnoty(const vector<int>& pole, int hodnota) {
  return find(pole.begin(), pole.end(), hodnota) - pole.begin();
}


void odmaskujOdpoved(const Mapa& mapa, const Stav& stav, int hrac, Odpoved& odpoved) {
  const Hrac& h = stav.hraci[hrac];
  FOREACH(it, odpoved) {
    if (it->typ == UTOC) it->b = indexHodnoty(h.mapovanie, it->b);
  }
}

//pouziva sa to niekde?
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

