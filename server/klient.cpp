//universal proboj magic
#include <climits>
#include <unistd.h>
using namespace std;

#include "klient.h"
#include "util.h"

#define CAS_KLIENTA_NA_INICIALIZACIU 2000
#define CAS_KLIENTA_NA_ODPOVED 200
#define MAXIMUM_RESTARTOV 10


Klient::Klient(string _label, string _uvodneData, string cwd, string zaznamovyAdresar)
    : label(_label), uvodneData(_uvodneData), timeout(0),
      zostavaRestartov(MAXIMUM_RESTARTOV) {
  vector<string> command;
  command.push_back("./hrac");
  proces.setProperties(command, cwd,
                       zaznamovyAdresar + "/stderr." + label);
}


void Klient::restartuj() {
  if (zostavaRestartov > 0) {
    zostavaRestartov--;
    log("restartujem klienta %s", label.c_str());
    proces.restartuj();
    proces.write(uvodneData);
    timeout = max(timeout, gettime() + CAS_KLIENTA_NA_INICIALIZACIU);
  }
  else {
    if (zostavaRestartov == 0) {
      log("vzdavam restartovanie klienta %s", label.c_str());
    }
    zostavaRestartov = -1;
    proces.zabi();
  }
}


void Klient::zabi() {
  proces.zabi();
}


void Klient::vypniTimeout() {
  timeout = max(timeout, LLONG_MAX);
}


string Klient::komunikuj(string request) {
  return komunikujNaraz(vector<Klient*>(1, this), vector<string>(1, request))[0];
}


vector<string> Klient::komunikujNaraz(vector<Klient*> klienti, vector<string> requesty) {
  int pocet = klienti.size();
  requesty.resize(pocet);

  vector<string> odpovede(pocet, "");
  vector<bool> cakam(pocet, false);

  for (int i = 0; i < pocet; i++) {
    if (klienti[i]->zostavaRestartov != -1) {
      cakam[i] = true;
      klienti[i]->proces.write(requesty[i]);
      klienti[i]->timeout = max(klienti[i]->timeout, gettime() + CAS_KLIENTA_NA_ODPOVED);
    }
  }

  bool hotovo = false;
  while (!hotovo) {
    hotovo = true;
    long long now = gettime();
    for (int i = 0; i < pocet; i++) {
      if (cakam[i] && now < klienti[i]->timeout) {
        hotovo = false;
        odpovede[i] += klienti[i]->proces.nonblockRead();
        if (odpovede[i].size() >= 3 && odpovede[i].substr(odpovede[i].size()-3, 3) == "\n.\n") {
          cakam[i] = false;
        }
      }
    }
    usleep(1);
  }

  for (int i = 0; i < pocet; i++) {
    if (cakam[i]) {
      log("klient %s nestihol odpovedat", klienti[i]->label.c_str());
    }
  }
  return odpovede;
}

// //universal proboj magic
// // #include <string>
// using namespace std;
// 
// #include "klient.h"
// #include "util.h"
// 
// #define CAS_DO_RESTARTU 1000
// 
// Klient::Klient () {}
// 
// Klient::Klient (string _meno, string _uvodneData, string adresar, string execCommand, string zaznamovyAdresar)
//     : meno(_meno), uvodneData(_uvodneData), poslRestart(-1)
// {
//     vector<string> command;
//     command.push_back(execCommand);
//     proces.setProperties(command, adresar, zaznamovyAdresar + "/" + meno + ".log");
// }
// 
// Klient::Klient (string _meno, string _uvodneData, string adresar, string zaznamovyAdresar)
//     : Klient(_meno, _uvodneData, adresar, "./hrac", zaznamovyAdresar) {}
// 
// void Klient::restartuj () {
//     long long cas = gettime();
//     if (cas - poslRestart > CAS_DO_RESTARTU) {
//         loguj("restartujem klienta %s", meno.c_str());
//         precitane.clear();
//         proces.restartuj();
// 
//         posli(uvodneData);
//         poslRestart = cas;
//     }
// }
// 
// string Klient::citaj (unsigned cap) {
//     string nove = proces.read(cap);
//     int i = nove.size();
//     while (i > 0 && nove[i-1] != '\n') {
//         // vraciame iba hotovu odpoved, uzavretu znakom noveho riadku
//         i--;
//     }
//     if (i == 0) {
//         precitane += nove;
//         return "";
//     }
//     string res = precitane + nove.substr(0,i);
//     precitane = nove.substr(i);
//     return res;
// }
// 
// void Klient::posli (string data) {
//     proces.write(data);
// }
// 
// void Klient::zabi () {
//     proces.zabi();
// }
// 
// bool Klient::zije () {
//     return proces.zije();
// }
