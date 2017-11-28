#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <set>
#include <iostream>
#include <queue>
#include <vector>
using namespace std;

#include "common.h"
#include "marshal.h"
#include "update.h"

#define ff first
#define ss second

Mapa mapa;
Stav stav;   // vzdy som hrac cislo 0
vector<Prikaz> prikazy;

set<pair<int,int> > veze;
queue<pair<int, int> > vybudovaneVeze;
vector<pair<int,int> > portaly;

int dx[] = {-1, 0, 0, 1};
int dy[] = {0, -1, 1, 0};

// pomocna funkcia. volajte zo zistiTah(), napr.:
// if (!vykonaj(Prikaz::stavaj(10, 10, DRAK))) { /* zeby malo energie? */ }
bool vykonaj(const Prikaz& p) {
  bool uspech = vykonajPrikaz(mapa, stav, 0, p);
  if (uspech) prikazy.push_back(p);   // potom to posleme serveru
  return uspech;
}


// main() zavola tuto funkciu, ked nacita mapu
vector<vector<vector<int> > > portalyPrePolicka;
vector<int> spawnDistance;
vector<int> spawnPokrytie;


void inicializuj() {

  portalyPrePolicka.resize(mapa.h, vector<vector<int> > (mapa.w));
  queue<pair<pair<int,int>, int> > Q;
  for(int i=0;i<mapa.h;++i){
    for(int j=0;j<mapa.w;++j){
      if(mapa.pole[i][j] == CIEL){
        Q.push({{i,j},0});
        break;
      }
    }
  }
  vector<vector<int> > V(mapa.h, std::vector<int> (mapa.w, -1));
  while(Q.size()){
    auto p = Q.front();
    Q.pop();
  if(p.ff.ff < 0 || p.ff.ff >= mapa.h || p.ff.ss < 0 || p.ff.ss >= mapa.w) continue;

    if( mapa.pole[p.ff.ff][p.ff.ss] != CESTA &&
        mapa.pole[p.ff.ff][p.ff.ss] != CIEL &&
        mapa.pole[p.ff.ff][p.ff.ss] != SPAWN
        ) continue;
    if(V[p.ff.ff][p.ff.ss] != -1 && mapa.pole[p.ff.ff][p.ff.ss] != SPAWN){
      continue;
    }
    V[p.ff.ff][p.ff.ss] = p.ss;
    if(mapa.pole[p.ff.ff][p.ff.ss] == SPAWN){
      int x = p.ff.ff, y = p.ff.ss;
      int distance = 0;
      while(mapa.pole[x][y] != CIEL){
     //   portalyPrePolicka[x][y].push_back(index);
        ++distance;
        x = x - dx[V[x][y]];
        y = y - dy[V[x][y]];
      }

      cerr << "B\n";

      int ddist = 1000000;
      for(int i = 0;i<portaly.size();++i){
        if(portaly[i] == make_pair(x,y)){
          ddist = spawnDistance[i];
        }
      }
      cerr << "A\n";
      if(ddist >= distance){
        int index = spawnDistance.size();
        spawnDistance.push_back(distance);
        int x = p.ff.ff, y = p.ff.ss;
        while(mapa.pole[x][y] != CIEL){
          portalyPrePolicka[x][y].push_back(index);
          x = x - dx[V[x][y]];
          y = y - dy[V[x][y]];
        }
      }
        continue;
    }
       
    for(int i=0;i<4;++i){
      Q.push({{p.ff.ff+dx[i],p.ff.ss+dy[i]},i});
    }
  } 
  spawnPokrytie.resize(spawnDistance.size(), 1<<25);

}

int ohodnotPolicko(int x, int y){
  if(x < 0 || x >= mapa.h || y < 0 || y >= mapa.w) return 0;
  if( mapa.pole[x][y] != CESTA) return 0;
  int hodnota = 0;
  for(auto p: portalyPrePolicka[x][y]){
    hodnota += spawnPokrytie[p];
  }
  return hodnota;
}

void znehodnotPolicko(int x, int y){
  cerr << "ZNEHODNOTVSTUP\n";
  if(x < 0 || x >= mapa.h || y < 0 || y >= mapa.w) return;
  set<int> znehodnoteneCesty;
  for (int i = 0; i < 4; i++){
    if(x +dx[i] < 0 || x +dx[i]>= mapa.h || y +dy[i]< 0 || y +dy[i]>= mapa.w) continue;
    for (auto p: portalyPrePolicka[x+dx[i]][y+dy[i]]){
      znehodnoteneCesty.insert(p);
    }
  }
  for(auto p: znehodnoteneCesty)
    spawnPokrytie[p] /= 2; 
  cerr << "ZNEHODNOTVYSTUP\n";
}

void postavVezu(int veza){
  cerr << "STAVIAM VEXU\n";
  pair<int,int> najlepsiePolicko;
  int hodnotaNajlepsieho = 0;
  for(int x=0;x<mapa.h;++x){
    for(int y=0;y<mapa.w;++y){
      if(mapa.pole[x][y] == POZEMOK && veze.find({x,y}) == veze.end()){
        cerr << x << " " << y << "\n";
        int h = 0;
        for(int i =0;i<4;++i){
          h += ohodnotPolicko(x+dx[i],y+dy[i]);
        }
        if(h > hodnotaNajlepsieho){
          najlepsiePolicko = {x,y};
          hodnotaNajlepsieho = h;
        }
      }
    }
  }
   int min = (1 << 25);
 /* for (int i = 0; i < 5;i++){
    int cena = zistiCenuVeze(stav, 0, i);
    if (cena < min){
     min = cena;
     veza = i;
    }
  }
  */
  if(vykonaj(Prikaz::buduj(najlepsiePolicko.ss, najlepsiePolicko.ff, veza))){
    if(veza == LASER_RAPTOR)
      vybudovaneVeze.push({najlepsiePolicko.ff, najlepsiePolicko.ss});
    veze.insert({najlepsiePolicko.ff, najlepsiePolicko.ss});
    znehodnotPolicko(najlepsiePolicko.ff,najlepsiePolicko.ss);
  }

}
//Momentalny typ
int momentalnyTyp = 3;
void postavLab(){
  int mojLab = momentalnyTyp % 4 + VEZA_LAB_PRVY;
  pair<int,int> najlepsiePolicko;
  int hodnotaNajlepsieho = (1<<30);
  for(int x=0;x<mapa.h;++x){
    for(int y=0;y<mapa.w;++y){
      if(mapa.pole[x][y] == POZEMOK && veze.find({x,y}) == veze.end()){
        int h = 0;
        for(int i =0;i<4;++i){
          h += ohodnotPolicko(x+dx[i],y+dy[i]);
        }
        if(h < hodnotaNajlepsieho){
          najlepsiePolicko = {x,y};
          hodnotaNajlepsieho = h;
        }
      }
    }
  }
  if(vykonaj(Prikaz::buduj(najlepsiePolicko.ss, najlepsiePolicko.ff, mojLab))){
    veze.insert({najlepsiePolicko.ff, najlepsiePolicko.ss});  
    momentalnyTyp++;  
  }
}

int ohodnotHraca(){
  vector <int> silaHraca(stav.hraci.size());
  for (int i = 1; i < stav.hraci.size(); i++){
    if(stav.hraci[i].umrel) silaHraca[i] = 1000000;
    for (int j = 0; j < stav.hraci[i].veze.size(); j++){
      int sila = 0;
      switch(stav.hraci[i].veze[j].typ){
        case TROLL: sila = 2;
          break;             
        case HYDRA: sila = 4;
          break;                   
        case DRAK: sila = 2;
          break;                      
        case TEMNY_CARODEJNIK: sila = 5;
          break;          
        case LASER_RAPTOR: sila = 1;
          break;       
        default: continue;
      }
      silaHraca[i] += sila;
    }
  }
  int silaNajslabsieho = 100000;
  int najslabsi = 1;
  for(int i = 1; i < silaHraca.size();i++){
    if (silaHraca[i] < silaNajslabsieho)
    {
      najslabsi = i;
      silaNajslabsieho = silaHraca[i];
    } 
  } 
  return najslabsi;
}


// main() zavola tuto funkciu, ked chce vediet, ake prikazy chceme vykonat
int pocetCarodej = 0;
int vezaKtoraSaMaStavat(){
  if(veze.size() <= 10) return 4;
  else if (veze.size()<=15) return 1;
  else if (veze.size()+pocetCarodej>19){
    if(veze.size()%3 == 0) return 2;
    else return 1;
  }
  else if (stav.hraci[0].energia > zistiCenuVeze(stav, 0, 3)) return 3;
  else return -1;
}

void zistiTah() {
  cerr << "$$$:"<< stav.hraci[0].energia << "\n";
  if((veze.size() % 5 == 0)&& (veze.size()>=10)) postavLab();
  else {
    if(vezaKtoraSaMaStavat() == -1){
      ;
    }
    else{
      if(vezaKtoraSaMaStavat() == 3){
        vykonaj(Prikaz::buraj(vybudovaneVeze.front().ss, vybudovaneVeze.front().ff));
        while(!vykonaj(Prikaz::buduj(vybudovaneVeze.front().ss, vybudovaneVeze.front().ff, 3))) ;
        ++pocetCarodej;
        vybudovaneVeze.pop();
      }
      else {
        postavVezu(vezaKtoraSaMaStavat());
      }
    }
  }

  int naKoho = ohodnotHraca();
  vykonaj(Prikaz::utoc(0, naKoho));
  vykonaj(Prikaz::utoc(1, naKoho));
  vykonaj(Prikaz::utoc(2, naKoho));
  vykonaj(Prikaz::utoc(3, naKoho));
  vykonaj(Prikaz::utoc(4, naKoho));
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

