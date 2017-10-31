//universal proboj magic
#ifndef KLIENT_H
#define KLIENT_H

#include <vector>
#include <string>

#include "proces.h"

class Klient {
    
  private:
    std::string label;
    Proces proces;
    std::string uvodneData;
    long long timeout;
    int zostavaRestartov;
  public:
    Klient(std::string _label, std::string _uvodneData, std::string adresar, std::string logAdresar);
    std::string komunikuj(std::string request);
    void vypniTimeout();
    void restartuj();
    void zabi();
    static std::vector<std::string> komunikujNaraz(std::vector<Klient*> klienti, std::vector<std::string> requesty);
};

#endif
