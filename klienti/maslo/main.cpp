#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <math.h>
#include <queue>
using namespace std;

#include "common.h"
#include "marshal.h"
#include "update.h"

static int DX[] = { 0, 1, 0, -1 };
static int DY[] = { -1, 0, 1, 0 };

Mapa mapa;
Stav stav;   // vzdy som hrac cislo 0
vector<Prikaz> prikazy;

int akoBudemUtocit;


// pomocna funkcia. volajte zo zistiTah(), napr.:
// if (!vykonaj(Prikaz::stavaj(10, 10, VEZA_LASER))) { /* zeby malo dreva? */ }
bool vykonaj(const Prikaz& p) {
  bool uspech = vykonajPrikaz(mapa, stav, 0, p);
  if (uspech) prikazy.push_back(p);   // potom to posleme serveru
  return uspech;
}

struct Policko {
  Teren typ;
  int r;
  int s;
};

int zisti(int r, int s) {
    return (s >= 0 && s < mapa.w && r >= 0 && r < mapa.h ? mapa.pole[r][s] : VODA);
}

int min(int a, int b) {
  return a < b ? a : b;
}

Policko ciel;

// main() zavola tuto funkciu, ked nacita mapu
void inicializuj() {
  srand(time(NULL) * getpid());

  for (int r = 0; r < mapa.h; r++) 
    for (int s = 0; s < mapa.w; s++) 
      if (zisti(r,s) == CIEL) 
        ciel = (Policko){CIEL, r, s};
}


int pocetVezi(int typ) {
  int result = 0;
  FOREACH(it, stav.hraci[0].veze) if (it->typ == typ) result++;
  return result;
}

void najdi_lab(Policko &lab_volne, vector<vector<int>> &dist){
    for (int r=0; r<mapa.h; r++){
        for (int s=0; s<mapa.w; s++){
            bool is_empty = true;
            for (auto &v : stav.hraci[0].veze) {
                if (v.x == s && v.y == r) {
                    is_empty = false;
                    break;
                }
            }
            if (dist[r][s]==10000001 && zisti(r, s)==POZEMOK && is_empty){
                lab_volne={POZEMOK, r, s};
                return;
            }
        }
    }
    
    int najvacsia_vzdialenost =0;
    for (int r = 0; r < mapa.h; r++) {
        for (int s = 0; s < mapa.w; s++) {
            
            bool is_empty = true;
            for (auto &v : stav.hraci[0].veze) {
                if (v.x == s && v.y == r) {
                    is_empty = false;
                    break;
                }
            }
            if (dist[r][s] > najvacsia_vzdialenost && zisti(r, s) == POZEMOK && is_empty) {  		    		  
                najvacsia_vzdialenost = dist[r][s];
                lab_volne={POZEMOK, r, s};
            }
        }
    }  
}


TypBudovy co_stavat(vector<vector<int>> &dist){
    vector<int> pocty (9, 0); //TROLL HYDRA DRAK MAG RAPTOR  ZAJAC ZOMBIE KORYTNACKA JEDNOROZEC
    vector<vector<int>> A (mapa.w, vector<int>(mapa.h));
    int pocet_pozemok_susedi = 0;
    int pocet_pozemok_stred = 0;
    for (int i=0; i<mapa.w; i++){
        for (int j=0; j<mapa.h; j++){
            Teren povrch = mapa.zisti(i, j);            
            if(povrch == VODA){
                A[i][j] = 0;
            } else if (povrch == POZEMOK){
                if (dist[j][i] < 1000000) {
                  A[i][j] = 3;
                  pocet_pozemok_susedi++;
                } else {
                  A[i][j] = 2;
                  pocet_pozemok_stred++;
                }
            } else {
              A[i][j] = 1;
            }
        }
    }
    int pocet_pozemok = pocet_pozemok_susedi + pocet_pozemok_stred;
    
    int pocet_pozemok_susedi_volny = pocet_pozemok_susedi;
    int pocet_pozemok_stred_volny = pocet_pozemok_stred;
    
    for (int i = 0; i < stav.hraci[0].veze.size(); i++){
        Veza v = stav.hraci[0].veze[i]; // v.x  v.y  v.typ
        pocty[v.typ]++;
        if (v.typ < VEZA_POCET_BOJOVYCH || A[v.x][v.y] == 3) {
          pocet_pozemok_susedi_volny--;
        } else {
          pocet_pozemok_stred_volny--;          
        }
    }
    //TROLL HYDRA DRAK MAG RAPTOR  ZAJAC ZOMBIE KORYTNACKA JEDNOROZEC
    
    int limit_raptor = min(10, pocet_pozemok_susedi / 2);
    if (pocty[LASER_RAPTOR] < limit_raptor) {
        return LASER_RAPTOR;
    }   
    
    int limit_utocne_veze = 5;   // kolko miest nechat pre laboratoria 
    vector<int> cena (VEZA_POCET_TYPOV);
    int vezove_koef[] = {1000, 1000, 3, 1, 1000};
    if(pocet_pozemok_susedi_volny > 0 && pocet_pozemok_stred_volny + pocet_pozemok_susedi_volny > limit_utocne_veze){
        int index_najlacnejsej_veze = 0;
        int min = 10000009;
        
        for (int i = 0; i < 4; i++){
            cena[i] = kVezaCena2[i]*kVezaCena2[i]*pocty[i] + kVezaCena1[i]*pocty[i] + kVezaCena0[i];
            cena[i] *= vezove_koef[i];
            if (cena[i] < min){
                index_najlacnejsej_veze = i;
                min = cena[i];
            }
        }
        return (TypBudovy)index_najlacnejsej_veze;
    }    
    //TODO ine laby?
    return LAB_KORITNACKA;
}

void najdi_najblizsiu_vezu(Policko &najblizsie_volne, vector<vector<int>> &dist) {
  queue<Policko> Q;
  Q.push(ciel);
  dist[ciel.r][ciel.s] = 0;
  
  while (!Q.empty()) {
    Policko p = Q.front(); Q.pop();
    for (int d = 0; d < 4; d++) {
      int nr = p.r + DY[d], ns = p.s + DX[d], nd = dist[p.r][p.s] + 1;
      Policko sused = {VODA, nr, ns};     
      if (zisti(nr, ns) == VODA) continue;
      if (dist[nr][ns] <= nd) continue;
      dist[nr][ns] = nd;
      if (!mapa.priechodne(ns,nr)) continue;
      Q.push(sused);
    }
  }
  
  int najmensia_vzdialenost = 10000001;

  for (int r = 0; r < mapa.h; r++) {
    for (int s = 0; s < mapa.w; s++) {
		  bool is_empty = true;
  	  for (auto &v : stav.hraci[0].veze) {
  		  if (v.x == s && v.y == r) {
  		    is_empty = false;
  		    break;
  		  }
  		}
  		if (dist[r][s] < najmensia_vzdialenost && zisti(r, s) == POZEMOK && is_empty) {  		    		  
  		  najmensia_vzdialenost = dist[r][s];
  		  najblizsie_volne = (Policko){VODA, r, s};
  		}
    }
  }  
  
}

// main() zavola tuto funkciu, ked chce vediet, ake prikazy chceme vykonat
int kolo = 1;
bool zajac_je = true;
int pocet_koryt = 1;

void zistiTah() {

  vector<vector<int> > dist (mapa.h, vector<int>(mapa.w, 10000001));
  Policko najblizsie_volne;  
  najdi_najblizsiu_vezu(najblizsie_volne, dist);
  Policko nejaky_lab;
  najdi_lab(nejaky_lab, dist);
  kolo++;
  
  int index_dalsej_veze = co_stavat(dist);
  if (!zajac_je) {
    index_dalsej_veze = LAB_ZAJAC;
    zajac_je = true;
  }
  if (pocet_koryt < kolo / 300) {
    index_dalsej_veze = LAB_KORITNACKA;
  }

	if (index_dalsej_veze >= VEZA_POCET_BOJOVYCH) {
		  if (vykonaj(Prikaz::buduj(nejaky_lab.s, nejaky_lab.r, index_dalsej_veze)) && index_dalsej_veze == LAB_KORITNACKA) {
		    pocet_koryt++;
		  }
	} else {
	  vykonaj(Prikaz::buduj(najblizsie_volne.s, najblizsie_volne.r, index_dalsej_veze));
	}
	
	int prvy_zivy = 1;
	for (int i = 1; i < stav.hraci.size(); i++) {
	  if (!stav.hraci[i].umrel) {
	    prvy_zivy = i;
	    break;
	  }
	}
	
  vykonaj(Prikaz::utoc(ZAJAC, prvy_zivy));
  while(vykonaj(Prikaz::utoc(KORITNACKA, prvy_zivy))) {}
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
/*
  int najmensie = 900000; 
  for (int r = 0; r < mapa.h; r++) {
    for (int s = 0; s < mapa.w; s++) { 
      bool isEmpty = true;
      for (int k = 0; k < stav.hraci[0].veze.size(); k++) {
        if (stav.hraci[0].veze[k].y == r && stav.hraci[0].veze[k].x == s) {
          isEmpty = false;
          break;
        }
      }
      if (dist[r][s] < najmensie && zisti(r, s) == POZEMOK && isEmpty) {
				cerr << "menim najmensie " << r << " " << s << " na " << dist[r][s] << endl;
        najmensie = dist[r][s];
        najmensie_pos.r = r; najmensie_pos.s = s;
      }
    }
  }
  
  for (int r = 0; r < mapa.h; r++) 
    for (int s = 0; s < mapa.w; s++) 
      dist[r][s] = abs(ciel.r - r) + abs(ciel.s - s);
      
      */
