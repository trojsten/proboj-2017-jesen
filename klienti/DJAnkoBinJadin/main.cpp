#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cmath>
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
static int DX[] = { 0, 1, 0, -1 };
static int DY[] = { -1, 0, 1, 0 };

int akoBudemUtocit;
int naKoho;
vector<int> dist2;
vector<vector<float>> priorita;
vector<vector<bool>> zastavane;
bool agresia=false;
int cyklus=0;
float minplan;
const float drsnost_raptora=sqrt(7);
const float drsnost_draka=sqrt(11);
const float drsnost_maga=sqrt(40);

int pocetVezi(int typ);

float sila(){
  return drsnost_raptora*pocetVezi(LASER_RAPTOR)+drsnost_draka*pocetVezi(DRAK)+drsnost_maga*pocetVezi(TEMNY_CARODEJNIK);
}

float lidl_raptor(){
  return drsnost_raptora/(24+5*pocetVezi(LASER_RAPTOR));
}
float lidl_drak(){
  return drsnost_draka/(147+7*pocetVezi(DRAK));
}
float lidl_mag(){
  return drsnost_maga/(296+10*pocetVezi(TEMNY_CARODEJNIK)+5*pocetVezi(TEMNY_CARODEJNIK)*pocetVezi(TEMNY_CARODEJNIK));
}

static void Bfs(vector<int>& dist) {
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

void prior(int x,int y,float k){
  priorita[x][y]+=k;
  vector<int>smery;
  for (int d = 0; d < 4; d++) {
    int ny = y + DY[d], nx = x + DX[d];
    if(mapa.priechodne(nx,ny) && dist2[y * mapa.w + x]>dist2[ny * mapa.w + nx]){
      smery.push_back(d);
    }
  }
  k/=smery.size();
  for (int i=0;i<smery.size();i++){
    prior(x+DX[smery[i]],y+DY[smery[i]],k);
  }
}

float plan(int x,int y){
  float A=0;
  for (int d = 0; d < 4; d++) {
    int ny = y + DY[d], nx = x + DX[d];
    if(mapa.priechodne(nx,ny)){
      A+=priorita[nx][ny];
    }
  }
  return A;
}
// pomocna funkcia. volajte zo zistiTah(), napr.:
// if (!vykonaj(Prikaz::stavaj(10, 10, VEZA_LASER))) { /* zeby malo dreva? */ }
bool vykonaj(const Prikaz& p) {
  bool uspech = vykonajPrikaz(mapa, stav, 0, p);
  if (uspech) prikazy.push_back(p);   // potom to posleme serveru
  return uspech;
}

// main() zavola tuto funkciu, ked nacita mapu
void inicializuj() {
  srand(time(NULL) * getpid());
  priorita.resize(mapa.pole.size(),vector<float>(mapa.pole[0].size(),0));
  zastavane.resize(mapa.pole.size(),vector<bool>(mapa.pole[0].size(),false));
  Bfs(dist2);
  for(int i=0;i<mapa.w;i++){
    for (int j=0;j<mapa.h;j++){
      if(mapa.zisti(i,j)==SPAWN){
        prior(i,j,1);
      }
    }
  }
  for(int i=0;i<mapa.w;i++){
    for (int j=0;j<mapa.h;j++){
      if(mapa.zisti(i,j)==CIEL){
        priorita[i][j]=0;
      }
    }
  }
  for(int i=0;i<mapa.w;i++){
    for (int j=0;j<mapa.h;j++){
      if(mapa.zisti(i,j)==POZEMOK && minplan>plan(i,j)){
        minplan=plan(i,j);
      }
    }
  }
  akoBudemUtocit = rand() % UTOCNIK_POCET_TYPOV;
  naKoho =  1 + rand() % (mapa.pocetHracov-1);
}


int pocetVezi(int typ) {
  int result = 0;
  FOREACH(it, stav.hraci[0].veze) if (it->typ == typ) result++;
  return result;
}


// main() zavola tuto funkciu, ked chce vediet, ake prikazy chceme vykonat
void zistiTah() {
  int oplati_sa;
  int G=0;
  if(lidl_mag()>=lidl_drak()&&lidl_mag()>=lidl_raptor())oplati_sa=TEMNY_CARODEJNIK;
  if(lidl_drak()>=lidl_mag()&&lidl_drak()>=lidl_raptor())oplati_sa=DRAK;
  if(lidl_raptor()>=lidl_drak()&&lidl_raptor()>=lidl_mag())oplati_sa=LASER_RAPTOR;
  FOREACH(it, stav.hraci[0].utocnici)if(mapa.zisti(it->x,it->y)==SPAWN)G++;
  //int mojLab = VEZA_LAB_PRVY + akoBudemUtocit;
  //while(vykonaj(Prikaz::utoc(akoBudemUtocit, naKoho))){}
  //    vykonaj(Prikaz::buduj(rand() % mapa.w, rand() % mapa.h, mojLab));
  //vykonaj(Prikaz::buduj(rand() % mapa.w, rand() % mapa.h, rand()%VEZA_POCET_TYPOV));
  while(stav.hraci[naKoho].umrel)naKoho =  1 + rand() % (mapa.pocetHracov-1);
  bool B=true;
  vector<int> RX;
  vector<int> RY;
  while(sila()<G*13 &&B){
    RX.clear();
    RY.clear();
    float maxi=0;
    for (int i=0;i<mapa.w;i++){
      for (int j=0;j<mapa.h;j++){
        if(mapa.zisti(i,j)==POZEMOK && !zastavane[i][j] && plan(i,j)>maxi){
          maxi=plan(i,j);
        }
      }
    }
    if(maxi==0){
      agresia=true;
      break;
    }
    int maxi2=0;
    for (int i=0;i<mapa.w;i++){
      for (int j=0;j<mapa.h;j++){
        int p=j*mapa.w+i;
        if(mapa.zisti(i,j)==POZEMOK && !zastavane[i][j] && plan(i,j)==maxi && dist2[p]>maxi2){
          maxi2=dist2[p];
        }
      }
    }
    for (int i=0;i<mapa.w;i++){
      for (int j=0;j<mapa.h;j++){
        int p=j*mapa.w+i;
        if(mapa.zisti(i,j)==POZEMOK && !zastavane[i][j] && plan(i,j)==maxi && dist2[p]==maxi2){
          RX.push_back(i);
          RY.push_back(j);
        }
      }
    }
    int sur=rand()%RX.size();
    if(vykonaj(Prikaz::buduj(RX[sur], RY[sur], oplati_sa))){
      zastavane[RX[sur]][RY[sur]]=true;
    }else B=false;
  }
  bool done=false;
  if(agresia||stav.hraci[0].energia>1000){
    for (int i=0;i<mapa.w && !done;i++){
      for (int j=0;j<mapa.h && !done;j++){
        if(plan(i,j)==minplan && vykonaj(Prikaz::buduj(i, j, LAB_KORITNACKA))){
          zastavane[i][j]=true; done=true;
        }
      }
    }
  }
  cyklus++;
  cyklus%=19;
  if(cyklus==0){
    while(vykonaj(Prikaz::utoc(KORITNACKA, naKoho))){}
  }
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
