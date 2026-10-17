#include "types.h"
#include "stat.h"
#include "user.h"

#define KIDS   4
#define OUTER  1500
#define INNER  150000

// just keep the CPU busy
static void
spin(void)
{
  volatile int sum = 0;
  int a, b;

  for(a = 0; a < OUTER; a++)
    for(b = 0; b < INNER; b++)
      sum += (a ^ b) & 7;
}

int
main(void)
{
  int i, pid;

  for(i = 0; i < KIDS; i++){
    pid = fork();
    if(pid < 0){
      printf(2, "fork failed\n");
      exit();
    }
    if(pid == 0){
      spin();
      printf(1, "child %d (pid %d) done at tick %d\n", i, getpid(), uptime());
      exit();
    }
  }
  for(i = 0; i < KIDS; i++)
    wait();
  exit();
}
