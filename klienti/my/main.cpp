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

#define For( i, n ) for( int i = 0 ; i < n ; i++ )
#define ff first
#define ss second

static int DX[] = { 0, 1, 0, -1 };
static int DY[] = { -1, 0, 1, 0 };

vector<int>dist2, dist;
Mapa mapa;
Stav stav;   // vzdy som hrac cislo 0
vector<Prikaz> prikazy;

int cielX, cielY;
int prveVolne, posledneVolne;
int akoBudemUtocit;


// grc

static void spravBFS(const Mapa& mapa, vector<int>& dist) {
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

// koniec grc



// pomocna funkcia. volajte zo zistiTah(), napr.:
// if (!vykonaj(Prikaz::stavaj(10, 10, VEZA_LASER))) { /* zeby malo dreva? */ }
bool vykonaj(const Prikaz& p) {
  bool uspech = vykonajPrikaz(mapa, stav, 0, p);
  if (uspech) prikazy.push_back(p);   // potom to posleme serveru
  return uspech;
}

vector<vector<double> > frekvencie;

void chodUtocnikom(pair<int,int> miesto, double vaha){
  // cerr << "chod " << miesto.ff << " " << miesto.ss << " vaha " << vaha
       // << " dist: " << dist2[miesto.ff*mapa.w+miesto.ss] << "\n"; 

  //nastav frekvencie
  for( int i=0; i<4; i++){
    if( !( miesto.ff == cielX && miesto.ss == cielY )){
      if( (0 <= miesto.ff+DX[i]) && (miesto.ff+DX[i] < mapa.h) ) 
        if( (0 <= miesto.ss+DY[i]) && (miesto.ss+DY[i] < mapa.w) ){
          if( mapa.zisti(miesto.ss+DY[i],miesto.ff+DX[i]) == POZEMOK )
            frekvencie[ miesto.ff+DX[i]][miesto.ss+DY[i] ]+=vaha;
            // cerr << "super\n";
          // else
            // cerr << "nie super\n";
        }
    }
    // else
    //   cerr << "#";
  }
  if( mapa.zisti(miesto.ss,miesto.ff) == CESTA )
    frekvencie[ miesto.ff][miesto.ss ]+=vaha;
  
  // zrataj pocet moznosti
  int moznosti=0;
  for( int i=0; i<4; i++){
    // cerr << "suradnice " << miesto.ff + DX[i] << "  " << miesto.ss+DY[i] << ", v dist2: " << (miesto.ff+DX[i])*mapa.w + miesto.ss+DY[i] << "\n";
    if( (0 <= miesto.ff+DX[i]) && (miesto.ff+DX[i] < mapa.h) ) 
      if( (0 <= miesto.ss+DY[i]) && (miesto.ss+DY[i] < mapa.w) ){

        if( dist2[ (miesto.ff+DX[i])*mapa.w + miesto.ss+DY[i] ]<dist2[ miesto.ff*mapa.w + miesto.ss ] )
          moznosti++;
      }
  }

  // rekurzia
  for( int i=0; i<4; i++){
    // cerr << i << " suradnice " << miesto.ff + DX[i] << "  " << miesto.ss+DY[i] << ", v dist2: " << (miesto.ff+DX[i])*mapa.w + miesto.ss+DY[i] << "\n";
    if( (0 <= miesto.ff+DX[i]) && (miesto.ff+DX[i] < mapa.h) ) 
      if( (0 <= miesto.ss+DY[i]) && (miesto.ss+DY[i] < mapa.w) )
        if( dist2[ (miesto.ff+DX[i])*mapa.w + miesto.ss+DY[i] ] < dist2[ miesto.ff*mapa.w + miesto.ss] )
          chodUtocnikom({miesto.ff+DX[i], miesto.ss+DY[i]}, vaha/moznosti);
  }
  // cerr << "chod koniec\n";
}
vector< pair<int,pair<int,int> > > stavby;
void najdiStavby(){
  for (int y = 0; y < mapa.h; y++)
    for (int x = 0; x < mapa.w; x++)
      if (mapa.zisti(y, x) == POZEMOK) 
        stavby.push_back({frekvencie[x][y], {y, x}});
  
  
  sort(stavby.rbegin(), stavby.rend());
  prveVolne=0;
  posledneVolne=stavby.size()-1;
}

void zistiFrekvencie(){
  frekvencie.resize(mapa.h);
  For(i, mapa.h)
    frekvencie[i].resize(mapa.w, 0);
  
  // spawnujeme prijdeneho utocnika na nahodnom spawne
  vector<pair<int,int> > spawnPolia;
  For(y, mapa.h)
    For(x, mapa.w)
      if (mapa.zisti(x, y) == SPAWN)
        spawnPolia.push_back(make_pair(x, y));

  // simuluj
  For(i, spawnPolia.size())
    chodUtocnikom({spawnPolia[i].ss, spawnPolia[i].ff},1);
  

  cerr << "plan: \n";
  for (int j = 0; j < frekvencie.size(); j++){
    for (int i=0; i < frekvencie[j].size(); i++ ){
      if (mapa.zisti(i,j)==CESTA) cerr << "0";
      if (mapa.zisti(i,j)==POZEMOK) cerr << "1";
      if (mapa.zisti(i,j)==VODA) cerr << "2";
      if (mapa.zisti(i,j)==CIEL) cerr << "7";
      if (mapa.zisti(i,j)==SPAWN) cerr << "9";
      cerr<< " ";
    }
    cerr << "\n";
  }

  For(i, dist2.size()){
    if (dist2[i]<10)
      cerr << " ";
    if (dist2[i]==392)
      cerr << "--";
    // else
      cerr << dist2[i];

    if( (i+1)%mapa.w == 0)
      cerr << "\n";
    else
      cerr << " ";
  }

  cerr << "frekvencie: \n";
  for (int j = 0; j < frekvencie.size(); j++){
    for (int i=0; i < frekvencie[j].size(); i++ )
      if (mapa.zisti(i,j)==POZEMOK)
        cerr << frekvencie[j][i] << " ";
      else
        cerr << "- ";
    cerr << "\n";
  }

  cerr << "frekvencie: \n";
  for (int j = 0; j < frekvencie.size(); j++){
    for (int i=0; i < frekvencie[j].size(); i++ )
      // if (mapa.zisti(i,j)==POZEMOK)
        cerr << frekvencie[j][i] << " ";
      // else
      //   cerr << "- ";
    cerr << "\n";
  }
}


// main() zavola tuto funkciu, ked nacita mapu
void inicializuj() {
  for (int j = 0; j < mapa.w; j++)
    for (int i=0; i < mapa.h; i++ )
      if (mapa.zisti(i,j)==CIEL){
        cielX = j;
        cielY = i;
      }
  cerr << "ciel "<< cielX << " " << cielY << "\n";
  spravBFS(mapa, dist2); 
  zistiFrekvencie();
  najdiStavby();
  srand(time(NULL) * getpid());
  // akoBudemUtocit = rand() % UTOCNIK_POCET_TYPOV;
}


int pocetVezi(int typ) {
  int result = 0;
  FOREACH(it, stav.hraci[0].veze) if (it->typ == typ) result++;
  return result;
}

double vyhodnotObranu(Hrac hrac){
  double sila=0;
  For(i, hrac.veze.size()){
    if(hrac.veze[i].typ == TROLL) sila += 5;
    if(hrac.veze[i].typ == HYDRA) sila += 3;
    if(hrac.veze[i].typ == DRAK) sila += 3;
    if(hrac.veze[i].typ == TEMNY_CARODEJNIK) sila += 10;
    if(hrac.veze[i].typ == LASER_RAPTOR) sila += 2;
  }
}

int najdiciel(){
  int ciel = 1;
  double minObrana=2000000000;
  double obrana;
  For(i,stav.hraci.size()){
    if (stav.hraci[i].umrel)
      continue;
    obrana = vyhodnotObranu(stav.hraci[i]);
    if( obrana < minObrana ){
      obrana = minObrana;
      ciel = i;
    }
  }
  return ciel;
}
void backPropagate(int x, int y, double vaha){
  if(mapa.zisti(x,y)!=CESTA && mapa.zisti(x,y)!= CIEL )
    return;
  //nastav prioritu
  frekvencie[x][y]*=1-vaha;
  
  // zrataj pocet moznosti
  int moznosti=0;
  for( int i=0; i<4; i++){
    if( (0 <= x+DX[i]) && (x+DX[i] < mapa.h) ) 
      if( (0 <= y+DY[i]) && (y+DY[i] < mapa.w) ){

        if( dist2[ (x+DX[i])*mapa.w + y+DY[i] ]<dist2[ x*mapa.w + y ] )
          moznosti += frekvencie[x][y];
      }
  }

  // rekurzia
  for( int i=0; i<4; i++){
    if( (0 <= x+DX[i]) && (x+DX[i] < mapa.h) ) 
      if( (0 <= y+DY[i]) && (y+DY[i] < mapa.w) )
        if( dist2[ (x+DX[i])*mapa.w + y+DY[i] ] > dist2[ x*mapa.w + y] )
          backPropagate( x+DX[i], y+DY[i], vaha*frekvencie[x+DX[i]][y+DY[i]]/moznosti  );
  }
}

// main() zavola tuto funkciu, ked chce vediet, ake prikazy chceme vykonat
void zistiTah() {
  // obranna veza
  if( rand()%6 ){
    if(stav.hraci[0].energia>=zistiCenuVeze(stav,0, TEMNY_CARODEJNIK) ){
      if( mapa.zisti(stavby[prveVolne].ss.ff, stavby[prveVolne].ss.ss) == POZEMOK )
      vykonaj(Prikaz::buduj(stavby[prveVolne].ss.ff, stavby[prveVolne].ss.ss, TEMNY_CARODEJNIK));
      prveVolne++;
    }
  }
  // utocna veza
  else{
    if( posledneVolne%2 == 0)
      vykonaj(Prikaz::buduj(stavby[posledneVolne].ss.ff, stavby[posledneVolne].ss.ss, LAB_ZAJAC));
    else
      vykonaj(Prikaz::buduj(stavby[posledneVolne].ss.ff, stavby[posledneVolne].ss.ss, LAB_JEDNOROZEC));

      backPropagate(stavby[prveVolne].ss.ff, stavby[prveVolne].ss.ss, 0.3);
      // fowardPropagate(stavby[prveVolne].ss.ff, stavby[prveVolne].ss.ss, 0.3);
    posledneVolne--;
  }


  int naKoho = najdiciel();
  vykonaj(Prikaz::utoc(akoBudemUtocit, naKoho));
}


int main() {
  // v tejto funkcii su vseobecne veci, nemusite ju menit (ale mozte).
  cerr << "main\n";

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

