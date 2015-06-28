#include "types.h"
#include "param.h"
#include "stat.h"
#include "user.h"

#define N  20

void loop(){
  int x=0;
  while(x<999){
    x++;
  }
}

int main()
{
  int n,pid[N];
  for(n=0;n<N;n++){
    pid[n] = fork(N*n);
    if (pid[n] == 0){
      printf(1,"%d\n", n);
      loop();
      exit();
    }
  }
  for(n=0;n<N;n++)
    wait();
  exit();
  return 0;
}