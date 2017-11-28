#include <cstdio>
#include<queue>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <unistd.h>

using namespace std;

#include "common.h"
#include "marshal.h"
#include "update.h"

struct Point{
    int x;
    int y;
    Point(){
        x=y=0;
    }
    Point(int a, int b){
        x=a;y=b;
    }
} mojaCasa;

Mapa mapa;
Stav stav;   // vzdy som hrac cislo 0
vector<Prikaz> prikazy;
bool prvy = true;
vector<Point> kdeChcem;
vector<Point> preLaby;
vector<vector<int> > budovy;
int ichcem = 0;
int jchcem;
int ilab = 0;

static int DX[] = {+2, +1, +1, +1, +0, +0, +0, +0, +0, -1, -1, -1, -2};
static int DY[] = {+0, -1, +0, +1, -2, -1, +0, +1, +2, -1, +0, +1, +0};
    
    
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
    vector<vector<int> > visited(mapa.h);
    budovy=vector<vector<int> > (mapa.h);
    for (int y=0; y<mapa.h; y++)
        for (int x=0; x<mapa.w; x++){
            if (mapa.zisti(x, y)==CIEL)
                mojaCasa = Point(x, y);
            visited[y].push_back(-1);
            budovy[y].push_back(-1);
            if (mapa.zisti(x, y)==POZEMOK && !mapa.priechodne(x, y+1) && !mapa.priechodne(x, y-1) && !mapa.priechodne(x+1, y) && !mapa.priechodne(x-1, y))
                preLaby.push_back(Point(x, y));
        }
    cerr << mojaCasa.x << ' ' << mojaCasa.y << endl;
    queue<pair<Point, int> > q;
    q.push(make_pair(mojaCasa, 0));
    while (!q.empty()){
        pair<Point, int> par = q.front();
        int x = par.first.x;
        int y = par.first.y;
        int vzd = par.second;
        q.pop();
        if (!mapa.priechodne(x, y)) cerr << "Som na nepriechodnom " << x << " " << y << endl;
        if (visited[y][x]!=-1)
            continue;
        visited[y][x]=vzd;
        cerr << "Prehladavam " << x << " " << y << endl;
        
        if (y>0){
            if (mapa.zisti(x, y-1)==POZEMOK && mapa.zisti(x, y)!=CIEL)
                kdeChcem.push_back(Point(x, y-1));
            else if (visited[y-1][x]==-1 && (mapa.zisti(x, y-1)==CESTA || mapa.zisti(x, y-1)==SPAWN))
                q.push(make_pair(Point(x, y-1), vzd+1));
        }
        if (x>0){
            if (mapa.zisti(x-1, y)==POZEMOK && mapa.zisti(x, y)!=CIEL)
                kdeChcem.push_back(Point(x-1, y));
            else if (visited[y][x-1]==-1 && (mapa.zisti(x-1, y)==CESTA || mapa.zisti(x-1, y)==SPAWN))
                q.push(make_pair(Point(x-1, y), vzd+1));
        }
        if (y<mapa.h-1){
            if (mapa.zisti(x, y+1)==POZEMOK && mapa.zisti(x, y)!=CIEL)
                kdeChcem.push_back(Point(x, y+1));
            else if (visited[y+1][x]==-1 && (mapa.zisti(x, y+1)==CESTA || mapa.zisti(x, y+1)==SPAWN))
                q.push(make_pair(Point(x, y+1), vzd+1));
        }
        if (x<mapa.w-1){
            if (mapa.zisti(x+1, y)==POZEMOK && mapa.zisti(x, y)!=CIEL)
                kdeChcem.push_back(Point(x+1, y));
            else if (visited[y][x+1]==-1 && (mapa.zisti(x+1, y)==CESTA || mapa.zisti(x+1, y)==SPAWN))
                q.push(make_pair(Point(x+1, y), vzd+1));
        }
    }
    cerr << "Vypisujem, kde chcem stavat" << endl;
    for (int i=0; i<kdeChcem.size(); i++)
        cerr << "tu " << kdeChcem[i].x << " " << kdeChcem[i].y << endl;
    cerr << "Vypisujem, kde chcem labaky" << endl;
    for (int i=0; i<preLaby.size(); i++)
        cerr << "tu " << preLaby[i].x << " " << preLaby[i].y << endl;
    jchcem = kdeChcem.size()-1;
}

int nahodnyHrac(){
    int h = 1 + rand() % (mapa.pocetHracov-1);
    while (stav.hraci[h].umrel)
        h = 1 + rand() % (mapa.pocetHracov-1);
    return h;
}


// main() zavola tuto funkciu, ked chce vediet, ake prikazy chceme vykonat
void zistiTah() {
    /*if (prvy){
        if (vykonaj(Prikaz::buduj(0, 0, LAB_ZOMBIE))) cerr << "Postavill som labak" << endl;
        prvy=false;
    }*/
    
    bool go = true;
    //cerr << "Cas je " << stav.cas << endl;
    while (go){
        while (ichcem<kdeChcem.size() && budovy[kdeChcem[ichcem].y][kdeChcem[ichcem].x]!=-1) ichcem++;
        while (jchcem>=0 && budovy[kdeChcem[ichcem].y][kdeChcem[ichcem].x]!=-1) jchcem--;
        if (ichcem>=kdeChcem.size())
            break;
        go = false;
        
        int x = kdeChcem[ichcem].x;
        int y = kdeChcem[ichcem].y;
        vector<int> pocet(4, 0);
        vector<Utocnik> utocnici = stav.hraci[0].utocnici;
        for (int i=0; i<utocnici.size(); i++){
            if (abs(utocnici[i].x-x)+abs(utocnici[i].y-y)<=2)
                pocet[utocnici[i].typ]++;
        }
        
        int max=0;
        int typ;
        int opt [] = {5, 6, 8};
        for (int i=0; i<pocet.size(); i++)
            if (pocet[i]>pocet[max])
                max = i;
        
        //cerr << pocet[max];
        if (stav.cas<=1)
            typ = LASER_RAPTOR;
        else if ((stav.cas>=30 && pocet[max]<=1) || (abs(x-mojaCasa.x)+abs(y-mojaCasa.y)>=6 && ichcem>=40 && rand()%3!=1))
            typ = opt[rand() % 3];
        else if (max==0 && pocet[0]>=3)
            typ = HYDRA;
        else if (rand()%8==1)
            typ = TEMNY_CARODEJNIK;
        else if (rand()%6==1){
            if (max==0)
                typ = pocet[0]>=3 ? HYDRA : DRAK;
            else if (max==1)
                typ = TROLL;
            else
                typ = DRAK;
        }
        else
            typ = LASER_RAPTOR;
        if (typ>=5){
            if (rand()%10==1)
                typ=7;
            cerr << "Ilab, kam idem stavat, je " << ilab << endl;
            if (ilab<preLaby.size()){
                if (vykonaj(Prikaz::buduj(preLaby[ilab].x, preLaby[ilab].y, typ))){
                    budovy[preLaby[ilab].y][preLaby[ilab].x]=typ;
                    ilab++;
                    go = true;
                }
                continue;
            }
            if (jchcem<0)
                break;
            if (vykonaj(Prikaz::buduj(kdeChcem[jchcem].x, kdeChcem[jchcem].y, typ))){
                budovy[kdeChcem[jchcem].y][kdeChcem[jchcem].x]=typ;
                jchcem--;
                go = true;
            }
        }
        else if (vykonaj(Prikaz::buduj(x, y, typ))){
            budovy[y][x]=typ;
            ichcem++;
            go = true;
        }
    }
    
    while (vykonaj(Prikaz::utoc(ZAJAC, nahodnyHrac())));
    while (vykonaj(Prikaz::utoc(ZOMBIE, nahodnyHrac())));
    while (vykonaj(Prikaz::utoc(KORITNACKA, nahodnyHrac())));
    while (vykonaj(Prikaz::utoc(JEDNOROZEC, nahodnyHrac())));
    
  //cerr << vykonaj(Prikaz::buduj(7, 4, TROLL)) << endl;
}


int main() {
  // v tejto funkcii su vseobecne veci, nemusite ju menit (ale mozte).
    cerr << "main" << endl;
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
