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


Mapa mapa;
Stav stav;   // vzdy som hrac cislo 0
vector<Prikaz> prikazy;

int akoBudemUtocit;
int poziciaCieluR;
int poziciaCieluS;
vector<pair<int,int> > volneObrannePolicka;
vector<pair<int,int> > volneUtocnePolicka;
int energia;
vector<vector<int> > b;
vector<pair<int,int> > pocetCiestObrannych[4];
int pocetOchrany;
int nakoho;


static int DX[] = { 0, 1, 0, -1 };
static int DY[] = { -1, 0, 1, 0 };


// pomocna funkcia. volajte zo zistiTah(), napr.:
// if (!vykonaj(Prikaz::stavaj(10, 10, VEZA_LASER))) { /* zeby malo dreva? */ }
bool vykonaj(const Prikaz& p) {
  bool uspech = vykonajPrikaz(mapa, stav, 0, p);
  if (uspech) prikazy.push_back(p);   // potom to posleme serveru
  return uspech;
}

vector<vector<int> > BFS() {
    vector<vector<int> > dist(mapa.h, vector<int>(mapa.w, -1));
    queue<pair<int,int> > Q;
    Q.push(make_pair(poziciaCieluR,poziciaCieluS));
    dist[poziciaCieluR][poziciaCieluS] = 0;
    while (!Q.empty()) {
        pair<int,int> p = Q.front(); Q.pop();
        int y = p.first, x = p.second;
        for (int d = 0; d < 4; d++) {
            int ny = y + DY[d], nx = x + DX[d];
            if (!mapa.priechodne(nx, ny)) continue;
            if (dist[ny][nx] != -1) continue;
            dist[ny][nx] = dist[y][x] +1;
            Q.push(make_pair(ny,nx));
        }
    }
    return dist;
}

void poziciaCielu() {
    for(int i=0; i < mapa.h; i++) {
        for(int j=0; j < mapa.w; j++) {
            if(mapa.pole[i][j] == CIEL) {
                poziciaCieluR = i;
                poziciaCieluS = j;
            }
        }
    }
}

void zistiVolneObrannePolicka() {
    int dx[4] = {1,-1,0,0};
    int dy[4] = {0,0,1,-1};
    for(int i=0; i < mapa.h; i++) {
        for(int j=0; j < mapa.w; j++) {
            if(mapa.pole[i][j] != POZEMOK) {
                continue;
            }
            
            bool ok = false;
            int pocetciest = 0;
            for(int k = 0; k < 4; k++) {
                if(mapa.zisti(j+dx[k], i+dy[k]) == CESTA) {
                    if(!ok)volneObrannePolicka.push_back(make_pair(i,j));
                    ok = true;
                    pocetciest++;
                }
            }

            if(pocetciest != 0) pocetCiestObrannych[pocetciest-1].push_back(make_pair(i,j));
            if(!ok) volneUtocnePolicka.push_back(make_pair(i,j));  //co robit ked dojdu utocne policka?
        }
    }
}

// main() zavola tuto funkciu, ked nacita mapu
void inicializuj() {
  srand(time(NULL) * getpid());
  akoBudemUtocit = rand() % UTOCNIK_POCET_TYPOV;
   
    poziciaCielu();
    zistiVolneObrannePolicka();
    
    b = BFS();
    
    pocetOchrany = 0;

}


int okHrac() {
    vector<int> pocetZlych(stav.hraci.size(), 0);
    int utok = 1;
    int najmenejzlych = 100000;
    for(int i = 1; i < stav.hraci.size(); i++) {
        if(stav.hraci[i].umrel == false) {
            for(int j = 0; j < stav.hraci[i].veze.size(); j++) {
                if(stav.hraci[i].veze[j].typ == HYDRA || stav.hraci[i].veze[j].typ == TEMNY_CARODEJNIK) {
                    pocetZlych[i]++;
                }
            }
            if(pocetZlych[i] < najmenejzlych) {
                najmenejzlych = pocetZlych[i];
                utok = i;
            }
        }
    }
 //   cerr << utok << endl;
    return utok;
}



int pocetVezi(int typ) {
  int result = 0;
  FOREACH(it, stav.hraci[0].veze) if (it->typ == typ) result++;
  return result;
}


// main() zavola tuto funkciu, ked chce vediet, ake prikazy chceme vykonat
void zistiTah() {
  //  cerr << stav.hraci.size() << endl;
  int mojaObrana = rand() % 5;
  int mojUtok = VEZA_LAB_PRVY + (rand() % 4);
  
  
  
    energia = stav.hraci[0].energia;

    int ktore;
    
    if(energia >= 200) {
        if(volneUtocnePolicka.size() > 0) {
            ktore = rand() % volneUtocnePolicka.size();
            if (vykonaj(Prikaz::buduj(volneUtocnePolicka[ktore].second, volneUtocnePolicka[ktore].first, LAB_ZAJAC))) {
                volneUtocnePolicka.erase(volneUtocnePolicka.begin()+ktore);
            }
        } else {
            for(int i = 0; i < 4; i++) {
                if(pocetCiestObrannych[i].size() > 0) {
                    ktore = rand() % pocetCiestObrannych[i].size();
                    if (vykonaj(Prikaz::buduj(pocetCiestObrannych[i][ktore].second, pocetCiestObrannych[i][ktore].first, LAB_ZAJAC))) {
                        pocetCiestObrannych[i].erase(pocetCiestObrannych[i].begin()+ktore);
                        break;
                    }
                }
            }
        }
    }
    
    nakoho = okHrac();
    while(vykonaj(Prikaz::utoc(ZAJAC, nakoho))) {}

    
 //   if(pocetOchrany < 4) {
    
    for(int i = 3; i >= 0; i--) {
        if(pocetCiestObrannych[i].size() > 0) {
            ktore = rand() % pocetCiestObrannych[i].size();
        
            if (vykonaj(Prikaz::buduj(pocetCiestObrannych[i][ktore].second, pocetCiestObrannych[i][ktore].first, LASER_RAPTOR))) {
                pocetCiestObrannych[i].erase(pocetCiestObrannych[i].begin()+ktore);
                pocetOchrany++;
                break;
            }
        }
    }
    
//    }
  
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

