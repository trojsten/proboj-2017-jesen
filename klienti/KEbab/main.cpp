#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <queue>
#include <algorithm>
#include <random>

using namespace std;

#include "common.h"
#include "marshal.h"
#include "update.h"

typedef pair<int, int> bod;

Mapa mapa;
Stav stav;   // vzdy som hrac cislo 0
vector<Prikaz> prikazy;

// nase sracky
int obet;
int nasTah = 0;
int spawner = 0;
int pocetPortalov=0;
vector<bod> portaly;
vector<bod> *najkratsieCesty;
vector<pair<int, bod> > vaha;
vector<pair<int, bod> > topTriPolicka;

bool **zastavane;

vector<bod> smery;

//spocitaj portaly
void spocitajPortaly(){
  for(int i=0; i<mapa.h; i++)
    for(int j=0; j<mapa.w; j++){
      if(mapa.pole[i][j]==SPAWN) {
        pocetPortalov++;
        portaly.push_back(make_pair(i, j));
      }
    }
}

//komparator
bool penis(const pair<int, bod>& a, const pair<int, bod>& b) {
  return a.first < b.first;
}

//bfs od portalu ku cielu
void bfsCiel(bod zac, int portal){
  random_shuffle(smery.begin(), smery.end());

  bod **otec = new bod*[mapa.h];
  bool **videne = new bool*[mapa.h];
  vaha.resize(mapa.h*mapa.w, make_pair(0, make_pair(0, 0)));  

  for(int i = 0; i < mapa.h; i++) {
    otec[i] = new bod[mapa.w];
    videne[i] = new bool[mapa.w];
  
    for(int j = 0; j < mapa.w; j++) {
      videne[i][j] = false;
      vaha[i*mapa.h+j].second.first = i;
      vaha[i*mapa.h+j].second.second = j;
    }
  }

  queue<bod> q;
  bod cielPoz;

  q.push(zac);
  videne[zac.first][zac.second] = true;

  while(!q.empty()) {
    bod a = q.front();
    q.pop();
    
    if(mapa.pole[a.first][a.second]==CIEL){
      cielPoz=a;      
      break;
    }

    for(int i = 0; i < 4; i++) {
      int x = a.first - smery[i].first, y = a.second - smery[i].second;

      if(x < 0 or x >= mapa.h or y < 0 or y >= mapa.w)
        continue;

      if(mapa.priechodne(y, x) && !videne[x][y]) {
        q.push(make_pair(x, y));
        videne[x][y] = true;
        otec[x][y] = a;
      }
    }
  }
  
  bod a=cielPoz;
  while(a!=zac){
    najkratsieCesty[portal].push_back(a);
    a=otec[a.first][a.second];
  }
  najkratsieCesty[portal].push_back(zac);
    

  for(int i=0; i<najkratsieCesty[portal].size(); i++){
    bod b=najkratsieCesty[portal][i];    
    for(int c = 0; c < 4; c++) {
      int x = b.first - smery[c].first, y = b.second - smery[c].second;

      if(x < 0 or x >= mapa.h or y < 0 or y >= mapa.w)
        continue;

      if(mapa.pole[b.first][b.second] == CIEL)
        continue;

      if(mapa.pole[x][y]==POZEMOK)
        vaha[x*mapa.h+y].first++;
    }
  }

}


// pomocna funkcia. volajte zo zistiTah(), napr.:
// if (!vykonaj(Prikaz::stavaj(10, 10, DRAK))) { /* zeby malo energie? */ }
bool vykonaj(const Prikaz& p) {
  bool uspech = vykonajPrikaz(mapa, stav, 0, p);
  if (uspech) prikazy.push_back(p);   // potom to posleme serveru
  return uspech;
}


// main() zavola tuto funkciu, ked nacita mapu
void inicializuj() {
  srand(time(NULL) * getpid());

  smery.push_back(make_pair(0, 1));
  smery.push_back(make_pair(0, -1));
  smery.push_back(make_pair(1, 0));
  smery.push_back(make_pair(-1, 0));
  
  spocitajPortaly();
  najkratsieCesty = new vector<bod>[pocetPortalov];

  for(int i=0; i<pocetPortalov; i++)
    bfsCiel(portaly[i], i);
  
  for(int i = 0; i < mapa.h; i++){
    for(int j=0; j<mapa.w; j++){
      cerr << vaha[i*mapa.h+j].first << " ";
    }
    cerr << endl;
  }
  
  random_shuffle(vaha.begin(), vaha.end());
  sort(vaha.begin(), vaha.end(), penis);
  
  zastavane = new bool*[mapa.h];  
  
  for(int i=0; i < mapa.h; i++) {
    zastavane[i] = new bool[mapa.w];
    
    for(int j=0; j < mapa.w; j++)
      zastavane[i][j] = false;
  }

  for(int i=0; i<3; i++){
    topTriPolicka.push_back(vaha.back());
    vaha.pop_back();
  }

  obet = 1;
}


// main() zavola tuto funkciu, ked chce vediet, ake prikazy chceme vykonat
void zistiTah() { 
  if(stav.hraci[obet].umrel)
    for(int i = obet; i < mapa.pocetHracov; i++)
      if(!stav.hraci[i].umrel)
        obet = i;

  vykonaj(Prikaz::utoc(ZAJAC, obet));
  vykonaj(Prikaz::utoc(KORITNACKA, obet));
  vykonaj(Prikaz::utoc(ZOMBIE, obet));
  vykonaj(Prikaz::utoc(JEDNOROZEC, obet));  

  if(nasTah==11)
    for(int i=0; i<topTriPolicka.size(); i++){
      vaha.push_back(topTriPolicka.back());
      topTriPolicka.pop_back();
    }
//obrana
//HYDRA
   if(nasTah == 11 or nasTah == 12 or nasTah == 13) {
      bod kam;

    while(!vaha.empty() && zastavane[vaha.back().second.first][vaha.back().second.second])
      vaha.pop_back();

    if(!vaha.empty())
      kam = vaha.back().second;

    if((vykonaj(Prikaz::buduj(kam.second, kam.first, TEMNY_CARODEJNIK)))){
      zastavane[kam.first][kam.second] = true;
      nasTah++;
    
      vaha.pop_back();
    }
    } else if(nasTah < 10) {
     bod kam;

    while(!vaha.empty() && zastavane[vaha.back().second.first][vaha.back().second.second])
      vaha.pop_back();

    if(!vaha.empty())
      kam = vaha.back().second;

    if((vykonaj(Prikaz::buduj(kam.second, kam.first, LASER_RAPTOR)))){
      zastavane[kam.first][kam.second] = true;
      nasTah++;
    
      vaha.pop_back();  
    }
  } else if((nasTah - 5) % 5 != 0) {

    bod kam;

    while(!vaha.empty() && zastavane[vaha.back().second.first][vaha.back().second.second])
      vaha.pop_back();

    if(!vaha.empty())
      kam = vaha.back().second;

    if((vykonaj(Prikaz::buduj(kam.second, kam.first, DRAK)))){
      zastavane[kam.first][kam.second] = true;
      nasTah++;

      vaha.pop_back();  
    }
  } else {
    while(mapa.pole[vaha[spawner].second.first][vaha[spawner].second.second] != POZEMOK){
      spawner++;
    }
    
    if(vykonaj(Prikaz::buduj(vaha[spawner].second.second, vaha[spawner].second.first, random() % UTOCNIK_POCET_TYPOV + VEZA_LAB_PRVY))) {
      cerr << vaha[spawner].first << endl;
      spawner++;      
      nasTah++;
    }    
  }
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

