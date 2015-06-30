#include "types.h"
#include "param.h"
#include "stat.h"
#include "user.h"

#define MAX_TESTE 9000
#define MAX_PROCESS 10

void teste(){
  int i, j;
    for(i=0; i<MAX_TESTE; i++)
      for(j=0; j<MAX_TESTE; j++);
}

int main() {
  int i, vet_pid[MAX_PROCESS];

  for(i=1; i<=MAX_PROCESS; i++) {

    vet_pid[i] = fork(i);

    if (vet_pid[i] == 0){
      teste();
      exit();
    }

  }

  printf(1, "   name   |   pid    |   step   |  stride  |   CPU\n");
  for(i=0; i<MAX_PROCESS; i++)
    wait();
  exit();

  return 0;
}