//universal proboj magic
#include <errno.h>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <cstdlib>
#include <cstdio>
#include <string>
using namespace std;

#include "proces.h"

void Proces::write(string data) {
  if(pid == -1) return;
  unsigned cur = 0;
  while (cur < data.size()) {
    int wlen = 1024;
    if (cur + wlen > data.size()) wlen = data.size() - cur;
    int status = ::write(writefd, data.c_str()+cur, wlen);
    if (status != wlen) {
      if (status == -1) fprintf(stderr, "write: pid %d: %s\n", pid, strerror(errno));
      else fprintf(stderr, "write: pid %d: zapisali sme len %d bajtov z %d!\n", pid, status, wlen);
      return;
    }
    cur += wlen;
  }
}

string Proces::nonblockRead() {
  if(pid == -1) return "";
  // nonblocking mame lebo uz pri vyrabani sme dali O_NONBLOCK
  char buf[1024];
  int len = read(readfd, buf, 1024);
  return string(buf, len < 0 ? 0 : len);
}

void Proces::zabi() {
  if(pid != -1) {
    kill(pid, SIGTERM);
    close(writefd);
    close(readfd);
  }
  pid = -1;
}

void Proces::restartuj() {
  zabi();

  const char *_cwd = cwd.c_str();
  const char *_errfile = errfile.c_str();
  char** _command = (char**)calloc(command.size() + 1, sizeof(char*));
  for (unsigned i = 0; i < command.size(); i++) _command[i] = (char*)command[i].c_str();

  int parent2child[2];
  int child2parent[2];
  if(pipe(parent2child) != 0) return;
  if(pipe(child2parent) != 0) return;

  int flags = fcntl(child2parent[0], F_GETFL, 0);
  fcntl(child2parent[0], F_SETFL, flags | O_NONBLOCK);

  int status = fork();
  if(status == -1) {
    perror("fork");
    free((void *)_command);
    return;   // nepodarilo sa
  }
  if(status == 0) {
    setsid();
    dup2(parent2child[0], 0);
    dup2(child2parent[1], 1);
    if (_errfile && _errfile[0]) {
      int errfd = open(_errfile, O_WRONLY | O_CREAT | O_APPEND, 0666);
      if (errfd == -1) {
        perror("open");
      }
      else {
        dup2(errfd, 2);
        close(errfd);
      }
    }
    close(parent2child[0]);
    close(parent2child[1]);
    close(child2parent[0]);
    close(child2parent[1]);
    if (chdir(_cwd) == -1) {
      fprintf(stderr, "chdir: %s: %s\n", _cwd, strerror(errno));
      exit(1);
    }
    execv(_command[0], _command);
    perror("execv");
    exit(1);
  }
  pid = status;
  writefd = parent2child[1];
  readfd = child2parent[0];
  close(parent2child[0]);
  close(child2parent[1]);
  free((void *)_command);
}

