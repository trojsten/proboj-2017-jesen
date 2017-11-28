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
#include "konstanty.h"

#define POCET_SMEROV 4


Mapa mapa;
Stav stav;   // vzdy som hrac cislo 0
vector<Prikaz> prikazy;

int dx[POCET_SMEROV] = {0, 1, 0, -1};
int dy[POCET_SMEROV] = {1, 0, -1, 0};

vector<vector<int> > vzd_od_pohara;

int akoBudemUtocit;
int poc_stavbyhodnych_policok=0;
int pocet_postavenych_vezi = 0;
int pocet_postavenych_labov = 1;
vector<vector<double> > prav_prechodu;

vector <pair <int, int> >teleporty;


void suradnice_tel(){
  for (int i =0; i<mapa.w; i++){
    for (int j=0; j<mapa.h; j++){
      if (mapa.zisti(i,j)==SPAWN){
        teleporty.push_back(make_pair(i, j));
      }
    }
  }
}

bool porovnaj_pozicie(pair<int, int> poz1, pair<int, int> poz2) {
  return vzd_od_pohara[poz1.first][poz1.second] > vzd_od_pohara[poz2.first][poz2.second];
}

bool validny_index(int x, int y) {
  return (x < mapa.w && x >= 0 && y < mapa.h && y >= 0);
}

void pravdepodobnost(){
  double pocet_tel=teleporty.size();
  for (pair<int, int> tel : teleporty) {
    prav_prechodu[tel.first][tel.second]  = 1/pocet_tel;
  }

  vector<pair<int, int> > pozicie;
  for (int i = 0; i < mapa.w; i++) {
    for (int j = 0; j < mapa.h; j++) {
      pozicie.push_back(make_pair(i,j));
    }
  }
  sort(pozicie.begin(), pozicie.end(), porovnaj_pozicie);
  //sort(pozicie.begin(), pozicie.end(),
      //[&vzd_od_pohara](pair<int, int> poz1, pair<int, int> poz2)
      //{return vzd_od_pohara[poz1.first][poz1.second] > vzd_od_pohara[poz2.first][poz2.second];});

    for (pair<int, int> poz : pozicie){
      // Pre pohar nechceme rozsirit pravdepodobnost!
      if (vzd_od_pohara[poz.first][poz.second] < 2) break;

      int poc=0;
      for(int i=0;i<POCET_SMEROV;i++){
        if(validny_index(poz.first + dx[i], poz.second + dy[i]) && vzd_od_pohara[poz.first+dx[i]][poz.second+dy[i]]==vzd_od_pohara[poz.first][poz.second]-1)
          poc++;
      }

      for(int i=0;i<POCET_SMEROV;i++){
        if(validny_index(poz.first+dx[i], poz.second+dy [i]) && vzd_od_pohara[poz.first+dx[i]][poz.second+dy[i]]==vzd_od_pohara[poz.first][poz.second]-1)
          prav_prechodu[poz.first+dx[i]][poz.second+dy[i]]+=prav_prechodu[poz.first][poz.second]/poc;
      }

    }
    for (int i = 0; i < mapa.w; i++) {
      for (int j = 0; j < mapa.h; j++) {
        fprintf(stderr, "%.3f ", prav_prechodu [i][j]);
      }
      fprintf(stderr, "\n");
    }
}

// pomocna funkcia. volajte zo zistiTah(), napr.:
// if (!vykonaj(Prikaz::stavaj(10, 10, VEZA_LASER))) { /* zeby malo dreva? */ }
bool vykonaj(const Prikaz& p) {
  bool uspech = vykonajPrikaz(mapa, stav, 0, p);
  if (uspech) prikazy.push_back(p);   // potom to posleme serveru
  return uspech;
}

void bfs(){
  int pohar_x,pohar_y;
  queue<pair<int,int> > fronta;
  for(int i=0;i<mapa.w;i++)
    for(int j=0;j<mapa.h;j++){
      if(mapa.zisti(i,j)==CIEL){
        pohar_x=i;
        pohar_y=j;
      }
    }
  vzd_od_pohara[pohar_x][pohar_y]=0;
  fronta.push(make_pair(pohar_x,pohar_y));
  while(!fronta.empty()){
    pair<int,int> akt;
    akt=fronta.front();
    fronta.pop();
    for(int i=0;i<POCET_SMEROV;i++){
      if(mapa.priechodne(akt.first+dx[i],akt.second+dy[i]) && vzd_od_pohara[akt.first+dx[i]][akt.second+dy[i]]==-1) {
        fronta.push(make_pair(akt.first+dx[i],akt.second+dy[i]));
        vzd_od_pohara[akt.first+dx[i]][akt.second+dy[i]]=vzd_od_pohara[akt.first][akt.second]+1;
      }
    }
  }

}

bool vieme_stavat(int x, int y) {
  //cerr << x << ' ' << y << ' ' << mapa.zisti(x,y) << endl;
  if (mapa.zisti(x,y)!=POZEMOK)
    return false;

  for(int i = 0; i < (int) stav.hraci[0].veze.size(); i++){
    if(stav.hraci[0].veze[i].x == x && stav.hraci[0].veze[i].y == y)
      return false;
  }
  return true;
}

double spocitaj_prav_okolo(int i,int j){
  double sucet_stvorice=0;
  for (int k=0; k< POCET_SMEROV; k++){
    if (validny_index(i+dx[k], j+dy[k])) {
      sucet_stvorice+=prav_prechodu[i+dx[k]][j+dy[k]];
    }
  }
  return sucet_stvorice;
}

// main() zavola tuto funkciu, ked nacita mapu
void inicializuj() {
  srand(time(NULL) * getpid());
  akoBudemUtocit = rand() % UTOCNIK_POCET_TYPOV;
  vzd_od_pohara.resize(mapa.w, vector<int> (mapa.h,-1));
  bfs();
  prav_prechodu.resize(mapa.w, vector<double>(mapa.h, 0));
  suradnice_tel();
  pravdepodobnost();
  for (int i=0; i<mapa.w; i++){
    for (int j=0; j<mapa.h; j++){
      if (mapa.zisti(i,j) == POZEMOK && spocitaj_prav_okolo(i,j)>0.00001){
        cerr << "Zijem2.1\n";
        poc_stavbyhodnych_policok++;
        cerr << "Zijem2.2\n";
      }
      cerr << "ZIJEM3\n";
    }
  }
}


int pocetVezi(int typ) {
  int result = 0;
  FOREACH(it, stav.hraci[0].veze) if (it->typ == typ) result++;
  return result;
}

int spocitaj_cesty_okolo(int x, int y) {
  int pocet_ciest=0;

  for (int i = 0; i < POCET_SMEROV; i++) {
    if (mapa.zisti(x + dx [i], y + dy [i]) == CESTA ||
            mapa.zisti(x + dx [i], y + dy [i]) == SPAWN)
      pocet_ciest++;
  }

  return pocet_ciest;
}



//TODO: Osetrit pripad, ked nie je kde stavat
pair<int, int> suradnice_obrany() {
  if ( ((double)rand()) / RAND_MAX < PRAV_STAVAM_VEZU_NAHODNE) {
    int x = rand() % mapa.w;
    int y = rand() % mapa.h;

    int retries = 0;
    while (retries < MAX_RETRIES && (!vieme_stavat(x,y) || spocitaj_cesty_okolo(x,y) < 1 || spocitaj_prav_okolo(x, y)< 0.0000001)) {
      x = rand() % mapa.w;
      y = rand() % mapa.h;
      retries ++;

    }

    cerr << "Chceme postavit vezu na " << x << ' ' << y << endl;
    if (retries < MAX_RETRIES) return make_pair(x, y);
    else return make_pair(-1, -1);
  }
  else {
    double vysledna_stvorica=0;
    int x=-1, y=-1;
    for (int i=0; i<mapa.w; i++){
      for (int j=0; j<mapa.h; j++){
        if (vieme_stavat(i, j)){
          double sucet_stvorice=0;

          for (int k=0; k< POCET_SMEROV; k++){
            if (validny_index(i+dx[k], j+dy[k])) {
              sucet_stvorice+=prav_prechodu[i+dx[k]][j+dy[k]];
            }
          }

          if (vysledna_stvorica<sucet_stvorice){
             vysledna_stvorica=sucet_stvorice;
             x=i;
             y=j;
           }
        }
      }
    }
    if (vysledna_stvorica<0.000000001){
      return make_pair (-1, -1);
    }
    cerr << "Idem stavat na najpravdepodobnejsiu: " << vysledna_stvorica << ' ' << x << ' ' << y << endl;
    return make_pair(x,y);
  }


}

pair <int, int> suradnice_labu() {
  for (int i = 0; i < mapa.w; i++){
    for (int j = 0; j < mapa.h; j++){
      if (vieme_stavat(i,j) && spocitaj_cesty_okolo(i,j) == 0){
        cerr << "Chceme stavat lab na " << i << ' ' << j << endl;
        return make_pair(i,j);
      }
      else {
        int BIG_NUMBER = 999;
        double vysledna_stvorica=BIG_NUMBER;
        int x=-1, y=-1;
        for (int i=0; i<mapa.w; i++){
          for (int j=0; j<mapa.h; j++){
            if (vieme_stavat(i, j)){
              double sucet_stvorice=0;

              for (int k=0; k< POCET_SMEROV; k++){
                if (validny_index(i+dx[k], j+dy[k])) {
                  sucet_stvorice+=prav_prechodu[i+dx[k]][j+dy[k]];
                }
              }

              if (vysledna_stvorica>sucet_stvorice){
                 vysledna_stvorica=sucet_stvorice;
                 x=i;
                 y=j;
               }
            }
          }
        }
        if (vysledna_stvorica==BIG_NUMBER){
          cerr << "Nevieme postavit lab, nemame na to policko\n";
          return make_pair(-1,-1);
        }
        return make_pair(x, y);
      }//
    }
  }
}



// main() zavola tuto funkciu, ked chce vediet, ake prikazy chceme vykonat
void zistiTah() {
  cerr << "TAHAM!\n";
  int mojLab = VEZA_LAB_PRVY + akoBudemUtocit;
  if (stav.cas == 0){
    pair <int, int> suradnice_lab = suradnice_labu();
    vykonaj (Prikaz::buduj(suradnice_lab.first, suradnice_lab.second, 5));
  }

  if( ((double)rand()) / RAND_MAX < PRAV_STAVAM_LAB || pocet_postavenych_vezi*4 > poc_stavbyhodnych_policok*pocet_postavenych_labov) {
      pair<int, int> suradnice_lab = suradnice_labu();
      int x_buduj_lab = suradnice_lab.first;
      int y_buduj_lab = suradnice_lab.second;
      if (zistiCenuVeze(stav, 0, mojLab) <= stav.hraci[0].energia && vieme_stavat(x_buduj_lab, y_buduj_lab)){
        cerr << "Podarilo sa postavit!\n";
        pocet_postavenych_labov++;
      }
      vykonaj(Prikaz::buduj(x_buduj_lab, y_buduj_lab, mojLab));
  } else {

      pair<int, int> suradnice = suradnice_obrany();
      int x_buduj_obrana = suradnice.first;
      int y_buduj_obrana = suradnice.second;
      int buduj_veza = rand() % VEZA_LAB_PRVY;
      if (zistiCenuVeze(stav, 0, buduj_veza) <= stav.hraci[0].energia && vieme_stavat(x_buduj_obrana, y_buduj_obrana)){
        cerr << "Podarilo sa postavit!";
        pocet_postavenych_vezi++;
        for (int i = 0; i < POCET_SMEROV; i++) {
          if (validny_index(x_buduj_obrana+dx[i],y_buduj_obrana+dy[i])) {
            prav_prechodu[x_buduj_obrana + dx[i]][y_buduj_obrana + dy[i]] -= 0.000001;
          }
        }
      }
      vykonaj(Prikaz::buduj(x_buduj_obrana, y_buduj_obrana, buduj_veza));
  }
  for (int i = 0; i < 100; i++){

    int naKoho = 1 + rand() % (mapa.pocetHracov-1);
    vykonaj(Prikaz::utoc(akoBudemUtocit, naKoho));
    vykonaj(Prikaz::utoc(0,naKoho));
  }
  // hmm este nejaku obranu by to chcelo...
}




int main() {
  // v tejto funkcii su vseobecne veci, nemusite ju menit (ale mozte).

  nacitaj(cin, mapa);
  inicializuj();

  cerr << "Lepsi klient, verzia 1.1\n";

  while (cin.good()) {
    nacitaj(cin, stav);
    prikazy.clear();
    zistiTah();
    uloz(cout, prikazy);
    cout << ".\n" << flush;   // bodka a flush = koniec odpovede
  }

  return 0;
}
