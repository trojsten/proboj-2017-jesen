#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <queue>
using namespace std;

#include "common.h"
#include "marshal.h"
#include "update.h"

const int dir[][2] = {{-1,0},{1,0},{0,-1},{0,1}};

Mapa mapa;
Stav stav;   // vzdy som hrac cislo 0
vector<Prikaz> prikazy;

int dalsiaVeza, target;

// pomocna funkcia. volajte zo zistiTah(), napr.:
// if (!vykonaj(Prikaz::stavaj(10, 10, VEZA_LASER))) { /* zeby malo dreva? */ }
bool vykonaj(const Prikaz& p) {
  bool uspech = vykonajPrikaz(mapa, stav, 0, p);
  if (uspech) prikazy.push_back(p);   // potom to posleme serveru
  return uspech;
}


// main() zavola tuto funkciu, ked nacita mapu
void inicializuj() {
  int seed = time(NULL) * getpid();
  cerr << "nulano START seed: " << seed << endl;
  srand(seed);
  
  dalsiaVeza = LAB_ZAJAC;
  target = 0;
}


int pocetVezi(int typ, int hrac) {
  int result = 0;
  FOREACH(it, stav.hraci[hrac].veze) if (it->typ == typ) result++;
  return result;
}


// main() zavola tuto funkciu, ked chce vediet, ake prikazy chceme vykonat
void zistiTah() {
    vector<int> targets(stav.hraci.size());
    int maxutocnici = 1;
    for (int i = 1; i < stav.hraci.size(); i++) {
        if (stav.hraci[i].umrel) continue;
        targets[i]++;
        FOREACH(it, stav.hraci[i].veze) switch (it->typ) {
            case TEMNY_CARODEJNIK: targets[i] += 20; break;
            case HYDRA: targets[i] += 8; break;
            case TROLL: case DRAK: targets[i] += 3; break;
            case LASER_RAPTOR: targets[i] += 1; break;
        }
        maxutocnici = max(maxutocnici,(int)stav.hraci[i].utocnici.size());
    }
    for (int i = 1; i < stav.hraci.size(); i++) {
        targets[i] *= 1.0-stav.hraci[i].utocnici.size()/2.0/maxutocnici;
        targets[i] *= targets[i];
        cerr << "priority " << i << " is " << targets[i] << endl;
        targets[i] += targets[i-1];
    }
    if (target == 0 || stav.hraci[target].umrel || stav.cas%100==0) {
        //do { target = rand()%(stav.hraci.size()-1)+1; } while (stav.hraci[target].umrel);
        int tgt = rand()%targets[targets.size()-1];
        target = 0;
        for (int i = 0; i < targets.size(); i++)
            target += targets[i] <= tgt;
        cerr << "new target: " << target << endl;
    }
    for (int i = 0; i < UTOCNIK_POCET_TYPOV; i++) while(vykonaj(Prikaz::utoc(i, target)));

    int xb, yb; for (xb = 0; xb < mapa.w; xb++) for (yb = 0; yb < mapa.h; yb++) if (mapa.zisti(xb,yb)==CIEL) goto foundbase; foundbase:;
    //int cntspawn = 0; for (int x = 0; x < mapa.w; x++) for (int y = 0; y < mapa.h; y++) cntspawn+=mapa.zisti(x,y)==SPAWN;
    //if (stav.hraci[0].veze.size()>4 && rand()&1 && cntspawn == 1) for (xb = 0; xb < mapa.w; xb++) for (yb = 0; yb < mapa.h; yb++) if (mapa.zisti(xb,yb)==SPAWN) goto foundspawn; foundspawn:;
    
    while (1) {
        int x = xb, y = yb;
        
        if (dalsiaVeza < VEZA_POCET_BOJOVYCH) {
            vector<vector<bool>> vis(mapa.h);
            for (int i = 0; i < mapa.h; i++) vis[i].resize(mapa.w, 0);
            queue<pair<int,int>> q; q.push(make_pair(xb,yb)); vis[yb][xb] = 1;
            while (!q.empty()) {
                int xx = q.front().first, yy = q.front().second; q.pop();
                for (int i = 0; i < 4; i++) {
                    int xxx = xx + dir[i][0], yyy = yy + dir[i][1];
                    if (mapa.priechodne(xxx,yyy)) {
                        if (!vis[yyy][xxx]) q.push(make_pair(xxx,yyy)); vis[yyy][xxx] = 1;
                    }
                    if (!(xx == xb && yy == yb) && mapa.zisti(xxx,yyy) == POZEMOK) {
                        bool f = 1; FOREACH (it, stav.hraci[0].veze) if (it->x == xxx && it->y == yyy) f = 0;
                        if (f) { x = xxx; y = yyy; goto foundtile; }
                    }
                }
            }
            dalsiaVeza = rand()%UTOCNIK_POCET_TYPOV+VEZA_LAB_PRVY;
        } else {
            int xx[4],yy[4]; for (int i = 0; i < 4; i++) xx[i] = yy[i] = -1;
            for (int yyy = 0; yyy < mapa.h; yyy++) {
                for (int xxx = 0; xxx < mapa.w; xxx++) {
                    if (mapa.zisti(xxx,yyy) != POZEMOK) continue;
                    bool f = 0; FOREACH (it, stav.hraci[0].veze) if (it->x == xxx && it->y == yyy) f = 1;
                    if (f) continue;
                    int susedia = 0;
                    for (int i = 0; i < 4; i++) {
                        int xxxx = xxx + dir[i][0], yyyy = yyy + dir[i][1];
                        if (mapa.priechodne(xxxx,yyyy) && mapa.zisti(xxxx,yyyy) != CIEL) susedia++;
                    }
                    xx[susedia] = xxx; yy[susedia] = yyy;
                }
            }
            for (int i = 0; i < 4; i++) {
                if (xx[i] != -1) { x = xx[i]; y = yy[i]; goto foundtile; }
            }
        }
        
        foundtile:;
        //cerr << "try build " << dalsiaVeza << " na " << x << " " << y << endl;
        if (vykonaj(Prikaz::buduj(x, y, dalsiaVeza))) {
            cerr << "staviam: " << x << "," << y << "; moj base: " << xb << "," << yb << endl;
            dalsiaVeza = LASER_RAPTOR;
            if (stav.hraci[0].veze.size() > 10) dalsiaVeza = HYDRA;
            if (stav.hraci[0].veze.size() > 12) dalsiaVeza = rand()%6;
            if (stav.hraci[0].veze.size() > 16) { dalsiaVeza = rand()%6; dalsiaVeza += VEZA_LAB_PRVY; if (dalsiaVeza >= VEZA_POCET_TYPOV) dalsiaVeza = TEMNY_CARODEJNIK; }
            cerr << "nasleduje " << dalsiaVeza << "(kolo: " << stav.cas << ")" << endl;
        } else break;
    }

  // hmm, este nejaku obranu by to chcelo...
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

