//universal proboj magic
#ifndef PROCES_H
#define PROCES_H

#include <vector>
#include <string>

class Proces {
    private:
        std::vector<std::string> command;
        std::string cwd;
        std::string errfile;
        int pid;
        int writefd;
        int readfd;

public:
    Proces() : cwd("."), pid(-1) { };
    void setProperties(std::vector<std::string> _command, std::string _cwd, std::string _errfile) {
      zabi();
      command = _command;
      cwd = _cwd;
      errfile = _errfile;
    }

    void write(std::string data);
    std::string nonblockRead();
    void zabi();
    void restartuj();
};

#endif

