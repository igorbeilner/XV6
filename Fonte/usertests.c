#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "memlayout.h"

void teste(void) {
    int i;
    for(i=0; i<10; i++) {
        printf(1, "igor\n");
    }
}