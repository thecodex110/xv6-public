#include "types.h"
#include "stat.h"
#include "user.h"

#define NKIDS 4
#define WORK  300000000

int prio[NKIDS] = {1, 2, 3, 5};

void
busy_work(void)
{
  volatile int x = 0;
  int i;

  for(i = 0; i < WORK; i++)
    x = x + (i & 7);
}

void
run_child(int n)
{
  int start, stop;

  if(nice(prio[n]) < 0){
    printf(2, "kid %d: nice(%d) failed\n", n, prio[n]);
    exit();
  }

  start = uptime();
  busy_work();
  stop = uptime();

  printf(1, "kid %d pid %d prio %d: %d ticks\n",
         n, getpid(), prio[n], stop - start);
  exit();
}

int
main(void)
{
  int pids[NKIDS];
  int i, k, done;

  // parent sabse high priority pe, taaki saare kids fork ho jaayein
  // aur phir wait() mein sone ke baad kids priority pe compete karein
  nice(0);

  for(i = 0; i < NKIDS; i++){
    pids[i] = fork();
    if(pids[i] < 0){
      printf(2, "fork failed (kid %d)\n", i);
      exit();
    }
    if(pids[i] == 0)
      run_child(i);
  }

  printf(1, "Finish order:\n");
  for(k = 1; k <= NKIDS; k++){
    done = wait();
    for(i = 0; i < NKIDS; i++)
      if(pids[i] == done)
        printf(1, "  #%d -> pid %d (priority %d)\n", k, done, prio[i]);
  }
  exit();
}
