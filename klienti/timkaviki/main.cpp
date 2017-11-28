#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <queue>
using namespace std;

#include "common.h"
#include "marshal.h"
#include "update.h"


Mapa mapa;
Stav stav;   // vzdy som hrac cislo 0
vector<Prikaz> prikazy;

vector <pair <int, int> > miesta;
vector <pair <int, int> > znova;
set <pair <int, int> > volne;
int vezicky[4]={0,0,0,0};
bool som=false, sme=false;
int dlzka=0;
bool setri=false;

int akoBudemUtocit;


// pomocna funkcia. volajte zo zistiTah(), napr.:
// if (!vykonaj(Prikaz::stavaj(10, 10, VEZA_LASER))) { /* zeby malo dreva? */ }
bool vykonaj(const Prikaz& p) {
  bool uspech = vykonajPrikaz(mapa, stav, 0, p);
  if (uspech) prikazy.push_back(p);   // potom to posleme serveru
  return uspech;
}


// main() zavola tuto funkciu, ked nacita mapu
void inicializuj() {
    srand(time(NULL)*getpid());
    vector <vector <bool> > bol(mapa.pole.size(), vector <bool> (mapa.pole[0].size(), false));
    queue <pair <pair <int, int>, int> > fronta;
    for (int i=0; i<mapa.pole.size(); ++i){
        for (int j=0; j<mapa.pole[0].size(); ++j){
            if(mapa.pole[i][j]==SPAWN){
                fronta.push({{i, j}, 0});
                bol[i][j]=true;
            }
            if(mapa.pole[i][j]==POZEMOK){
                volne.insert({i,j});
            }
        }
    }
    int dx[4]={0, 1, 0, -1};
    int dy[4]={1, 0, -1, 0};
    while(!fronta.empty()){
        //cerr<<fronta.front().first.first<< " "<<fronta.front().first.second<<endl;
        int x, y, cena;
        x=fronta.front().first.first;
        y=fronta.front().first.second;
        cena=fronta.front().second;
        fronta.pop();
        if(mapa.pole[x][y]==CIEL){
            dlzka=cena;
            continue;
        }
        for (int i=0; i<4; ++i){
            if(mapa.zisti(y+dy[i], x+dx[i])==CESTA && !bol[x+dx[i]][y+dy[i]]){
                fronta.push({{x+dx[i], y+dy[i]}, cena+1});
                bol[x][y]=true;
            }
            if(mapa.zisti(y+dy[i], x+dx[i])==POZEMOK){
                if(!bol[x+dx[i]][y+dy[i]]){
                    miesta.push_back({x+dx[i], y+dy[i]});
                    bol[x+dx[i]][y+dy[i]]=true;
                    volne.erase({x+dx[i], y+dy[i]});
                }
            }
        }
    }
    for (int i=0; i<miesta.size(); ++i){
        //cerr << miesta[i].first << " " << miesta[i].second << endl;
    }
}


int pocetVezi(int typ) {
  int result = 0;
  FOREACH(it, stav.hraci[0].veze) if (it->typ == typ) result++;
  return result;
}


// main() zavola tuto funkciu, ked chce vediet, ake prikazy chceme vykonat
void zistiTah() {
    int prisery[4]={0,0,0,0};
    for (int i=0; i<stav.hraci[0].utocnici.size(); ++i){
        if(stav.hraci[0].utocnici[i].typ==ZAJAC)++prisery[0];
        if(stav.hraci[0].utocnici[i].typ==ZOMBIE)++prisery[1];
        if(stav.hraci[0].utocnici[i].typ==KORITNACKA)++prisery[2];
        if(stav.hraci[0].utocnici[i].typ==JEDNOROZEC)++prisery[3];
    }
    if(!som){
        som=true;
        int x, y;
        if(volne.size()==0){
            x=miesta.back().first;
            y=miesta.back().second;
            miesta.pop_back();
        }
        else{
            x=(*volne.begin()).first;
            y=(*volne.begin()).second;
            volne.erase(volne.begin());
        }
        vykonaj(Prikaz::buduj(y, x, LAB_ZAJAC));
        ++vezicky[0];
    }
    if(!miesta.empty()){
        if(!sme){
                if(vykonaj(Prikaz::buduj(miesta.back().second, miesta.back().first, TEMNY_CARODEJNIK))){
                    miesta.pop_back();
                    sme=true;
                }
        }
        if(!miesta.empty()&&sme){
            //cerr << miesta.back().second << " " << miesta.back().first << endl;
            int q=rand()%(max(3, 20-((stav.cas)/100)));
            cerr << q << endl;
            if(((q==0)&& stav.cas%12==0)||setri){
                    setri=true;
                    if(!volne.empty()){
                        int f=(rand()%4)+5;
                        if(vykonaj(Prikaz::buduj((*volne.begin()).second, (*volne.begin()).first, f))){
                            volne.erase(volne.begin());
                            ++vezicky[f-5];
                            setri=false;
                        }            
                    }
                }
                else if(setri);
            else if(prisery[0]>prisery[1]+prisery[2]+prisery[3]&&stav.cas>200&&rand()%4==0){
                if(vykonaj(Prikaz::buduj(miesta.back().second, miesta.back().first, HYDRA))){
                    znova.push_back(miesta.back());
                    miesta.pop_back();
                }
            }
            else if(prisery[1]>prisery[0]+prisery[2]+prisery[3]&&stav.cas>200&&rand()%2==0){
                if(vykonaj(Prikaz::buduj(miesta.back().second, miesta.back().first, TROLL))){
                    znova.push_back(miesta.back());
                    miesta.pop_back();
                }
            }
            else {
                int veze[3]={0,0,0};
                veze[0]=zistiCenuVeze(stav, 0, TEMNY_CARODEJNIK);
                veze[1]=zistiCenuVeze(stav, 0, LASER_RAPTOR);
                veze[2]=zistiCenuVeze(stav, 0, DRAK); 
                
                if((veze[0]<veze[1])&&(veze[0]<veze[2])){
                    if(vykonaj(Prikaz::buduj(miesta.back().second, miesta.back().first, TEMNY_CARODEJNIK))){
                        miesta.pop_back();
                    }
                }
                else if((veze[1]<veze[2])&&(veze[1]<veze[0])){
                    if(vykonaj(Prikaz::buduj(miesta.back().second, miesta.back().first, LASER_RAPTOR))){
                        znova.push_back(miesta.back());
                        miesta.pop_back();
                    }
                }
                else if((veze[2]<veze[1])&&(veze[2]<veze[0])){
                    if(vykonaj(Prikaz::buduj(miesta.back().second, miesta.back().first, DRAK))){
                        miesta.pop_back();
                    }
                }
                /*if(vykonaj(Prikaz::buduj(miesta.back().second, miesta.back().first, LASER_RAPTOR))){
                    miesta.pop_back();
                }*/
            }
        }
    }
    else{
        if(rand()%2==0){
            if(!volne.empty()){
                int f=(rand()%4)+5;
                if(vykonaj(Prikaz::buduj((*volne.begin()).second, (*volne.begin()).first, f))){
                    volne.erase(volne.begin());
                    ++vezicky[f-5];
                }            
            }
        }
        else{
            if(zistiCenuVeze(stav, 0, TEMNY_CARODEJNIK)<zistiCenuVeze(stav, 0, DRAK)){
                if(stav.hraci[0].body>zistiCenuVeze(stav, 0, TEMNY_CARODEJNIK)){
                    vykonaj(Prikaz::buraj(znova.back().second, znova.back().first));
                    vykonaj(Prikaz::buduj(znova.back().second, znova.back().first, TEMNY_CARODEJNIK));
                    znova.pop_back();
                }
            }
            if(zistiCenuVeze(stav, 0, TEMNY_CARODEJNIK)>zistiCenuVeze(stav, 0, DRAK)){
                if(stav.hraci[0].body>zistiCenuVeze(stav, 0, DRAK)){
                    vykonaj(Prikaz::buraj(znova.back().second, znova.back().first));
                    vykonaj(Prikaz::buduj(znova.back().second, znova.back().first, DRAK));
                    znova.pop_back();
                }
            }
        }
    }
    for (int i=1; i<stav.hraci.size(); ++i){
        if(!stav.hraci[i].umrel){            
            for (int j=0; j<4; ++j){
                for (int c=0; c<vezicky[j]; ++c){
                    vykonaj(Prikaz::utoc(j, i));
                }
            }
            break;
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

