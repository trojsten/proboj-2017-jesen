#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
using namespace std;

#include "common.h"
#include "marshal.h"
#include "update.h"

#define VERYEARLYGAME 100
#define EARLYGAME 280
#define MIDGAME 888

Mapa mapa;
Stav stav;   // vzdy som hrac cislo 0
vector<Prikaz> prikazy;

int akoBudemUtocit;

vector<pair<int, int>> okolie = {{1,0},{-1,0},{0,-1},{0,1}};
vector<vector<int>> vzdial;
vector<vector<double>> frequency;
vector<pair<double,pair<int, int>>> greedy;
int usedPolF;
int usedPolB;
int target;
vector<int> randomHraci;
int shift = 0;

struct policko{
    int x, y;
    Teren typ;
};
policko casa;

void bfs(){
    vector<vector<bool>> pr(mapa.w);//prejdene policka
    for(int i = 0; i < mapa.w; i++){pr[i].resize(mapa.h);}
    queue<pair<int, int>> chod; // policka na kere pojdes
    queue<int> dist;//vzdialenost akt od case
    dist.push(0);
    chod.push(make_pair(casa.x, casa.y));
    while(!chod.empty()){
        auto akt = chod.front(); chod.pop(); int d = dist.front(); dist.pop();
        if(akt.first < 0 || akt.first > mapa.w-1 || akt.second < 0 || akt.second > mapa.h-1 || pr[akt.first][akt.second] == 1 || !mapa.priechodne(akt.first, akt.second)){
            continue;
        }
        else{
            vzdial[akt.first][akt.second] = d;
            pr[akt.first][akt.second] = 1;
            d++;
            for(int i = 0; i < 4; i++){
                chod.push(make_pair(akt.first+okolie[i].first,akt.second+okolie[i].second));
                dist.push(d);
            }
        }
    }
}

void frekvencie(){
    vector<pair<int, pair<int,int>>> vzdial_pol;
    for(int i = 0; i < mapa.w; i++){
      for(int j = 0; j < mapa.h; j++){
        vzdial_pol.push_back(make_pair(vzdial[i][j],make_pair(i,j)));
      }
    }
    sort(vzdial_pol.begin(), vzdial_pol.end());
    for(int i = vzdial_pol.size()-1; i >= 0; i--){
      int split = 0;
      pair<int, int> toto;
      toto = vzdial_pol[i].second;
      for(int o = 0; o < 4; o++){
        pair<int, int> akt(toto.first+okolie[o].first, toto.second+okolie[o].second);
        if(!(akt.first < 0 || akt.first > mapa.w-1 || akt.second < 0 || akt.second > mapa.h-1) && mapa.priechodne(akt.first, akt.second) && vzdial[akt.first][akt.second] < vzdial_pol[i].first) split++;
      }
      for(int o = 0; o < 4; o++){
        pair<int, int> akt(toto.first+okolie[o].first, toto.second+okolie[o].second);
        if(!(akt.first < 0 || akt.first > mapa.w-1 || akt.second < 0 || akt.second > mapa.h-1) && mapa.priechodne(akt.first, akt.second) && vzdial[akt.first][akt.second] < vzdial_pol[i].first) frequency[toto.first+okolie[o].first][toto.second+okolie[o].second] += (double)frequency[toto.first][toto.second]/(double)split;
      }
    }
    frequency[casa.x][casa.y] = 0;
}

void value(){
  for(int i = 0; i < mapa.w; i++){
    for(int j = 0; j < mapa.h; j++){
      if(mapa.zisti(i,j) == POZEMOK){
        double u = 0;
        for(int k = 0; k < 4; k++){
          pair<int, int> akt(i+okolie[k].first,j+okolie[k].second);
          if(!(akt.first < 0 || akt.first > mapa.w-1 || akt.second < 0 || akt.second > mapa.h-1)){
            u += frequency[akt.first][akt.second];
          }
        }
        greedy.push_back(make_pair(u,make_pair(i,j)));
      }
    }
  }
  sort(greedy.begin(), greedy.end());
}

// pomocna funkcia. volajte zo zistiTah(), napr.:
// if (!vykonaj(Prikaz::stavaj(10, 10, VEZA_LASER))) { /* zeby malo dreva? */ }
bool vykonaj(const Prikaz& p) {
  if(p.typ == BUDUJ) vykonajPrikaz(mapa, stav, 0, Prikaz::buraj(p.x, p.y));
  bool uspech = vykonajPrikaz(mapa, stav, 0, p);
  if (uspech) prikazy.push_back(p);   // potom to posleme serveru
  return uspech;
}


// main() zavola tuto funkciu, ked nacita mapu
void inicializuj() {
  cerr << "zijem a som debil" << endl;
  srand(time(NULL) * getpid());
  akoBudemUtocit = 0;
  frequency.resize(mapa.w);
  cerr << "heil debil" << endl;
  for(int i = 0; i < mapa.w; i++){
    frequency[i].resize(mapa.h, 0);
  }
  cerr << "segseg" << endl;
  for(int i = 0; i < mapa.h; i++){
    for(int j = 0; j < mapa.w; j++){
        //cerr << "i,j: " << i << " " << j << endl;
        if(mapa.zisti(j,i) == CIEL){
            casa.x = j; casa.y = i; casa.typ = CIEL;
        }
        else if(mapa.zisti(j,i) == SPAWN){
            //cerr << "ale som tu" << endl;
            frequency[j][i] = 1.0;
            //cerr << "nie som tu" << endl;
      }
    }
  }
  cerr << "iiii" << endl;
  vzdial.resize(mapa.w);
  for(int i = 0; i < mapa.w; i++){
    vzdial[i].resize(mapa.h);
  }
  cerr << "gegegeg" << endl;
  bfs();
  cerr << "heheheh" << endl;
  frekvencie();
  value();
  cerr << "vzdial: " << endl;
  for(int i = 0; i < mapa.w; i++){
    for(int j = 0; j < mapa.h; j++){
      cerr << vzdial[i][j] << " ";
    }
    cerr << endl;
  }
  cerr << "freq: " << endl;
  for(int i = 0; i < mapa.w; i++){
    for(int j = 0; j < mapa.h; j++){
      cerr << frequency[i][j] << " ";
    }
    cerr << endl;
  }
  usedPolF = 0;
  usedPolB = greedy.size() - 1;
  target = 0;
}


int pocetVezi(int typ) {
  int result = 0;
  FOREACH(it, stav.hraci[0].veze) if (it->typ == typ) result++;
  return result;
}


// main() zavola tuto funkciu, ked chce vediet, ake prikazy chceme vykonat
void zistiTah() {
  if(stav.cas == 0) 
  {
    for(int i=1; i<stav.hraci.size(); i++)
    {
      randomHraci.push_back(i);
    }
    random_shuffle(randomHraci.begin(), randomHraci.end());
  }
  if(stav.hraci[randomHraci[target]].umrel || !stav.cas%100)
  {
    target++;
    target %= randomHraci.size();
  }
  int mojLab = VEZA_LAB_PRVY + akoBudemUtocit;
  int naKoho = randomHraci[target];
  if(stav.cas == 0)
  {
    while(stav.hraci[0].energia > 500 - mapa.w*mapa.w*mapa.h*mapa.h/400)
    {
      if(vykonaj(Prikaz::buduj(greedy[usedPolB].second.first, greedy[usedPolB].second.second, 4)))usedPolB--;
    }
  }
  if(pocetVezi(mojLab) < 10 || stav.cas%5==0) while(vykonaj(Prikaz::utoc(akoBudemUtocit, naKoho)));
  if(vykonaj(Prikaz::buduj(greedy[usedPolF].second.first, greedy[usedPolF].second.second, mojLab))) 
  {
    usedPolF++;
    shift = stav.cas;
  }
  if(!usedPolF) return;
  if (stav.cas + shift < VERYEARLYGAME)
  {
    if(vykonaj(Prikaz::buduj(greedy[usedPolB].second.first, greedy[usedPolB].second.second, 1)))usedPolB--;
    if(rand()%1000 < mapa.h) if(vykonaj(Prikaz::buduj(greedy[usedPolB].second.first, greedy[usedPolB].second.second, 4)))usedPolB--;
  }
  else if(stav.cas + shift < EARLYGAME)
  {
    if(vykonaj(Prikaz::buduj(greedy[usedPolB].second.first, greedy[usedPolB].second.second, 1)))usedPolB--;
    if(rand()%max(150-stav.cas, 1)) if(vykonaj(Prikaz::buduj(greedy[usedPolB].second.first, greedy[usedPolB].second.second, 4)))usedPolB--;
  }
  else if(stav.cas + shift < MIDGAME)
    
  {
    if(vykonaj(Prikaz::buduj(greedy[usedPolB].second.first, greedy[usedPolB].second.second, 3)))usedPolB--;
    if(rand()%max(300-stav.cas, 1)) if(vykonaj(Prikaz::buduj(greedy[usedPolB].second.first, greedy[usedPolB].second.second, 2)))usedPolB--;
  }
  else
  {
    if(rand()%2) if(vykonaj(Prikaz::buduj(greedy[usedPolB].second.first, greedy[usedPolB].second.second, 3)))usedPolB--;
    if(!rand()%5) if(vykonaj(Prikaz::buduj(greedy[usedPolB].second.first, greedy[usedPolB].second.second, 2)))usedPolB--;
  }
  if(greedy[usedPolB].first*5 <= greedy.back().first || usedPolB <= usedPolF) usedPolB = greedy.size() - 1;
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

