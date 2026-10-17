#include "types.h"
#include "stat.h"
#include "user.h"

#define KIDS   4
#define UNIT   300
#define INNER  150000

static int units[KIDS] = {4, 1, 3, 2};

static void
spin(int u)
{
  volatile int sum = 0;
  int a, b;

  for(a = 0; a < u * UNIT; a++)
    for(b = 0; b < INNER; b++)
      sum += (a ^ b) & 7;
}

int
main(void)
{
  int pids[KIDS];
  int i, k, done;

  for(i = 0; i < KIDS; i++){
    setburst(units[i]);
    pids[i] = fork();
    if(pids[i] < 0){
      printf(2, "fork failed\n");
      exit();
    }
    if(pids[i] == 0){
      spin(units[i]);
      printf(1, "child %d (burst %d) pid %d done at tick %d\n",
             i, units[i], getpid(), uptime());
      exit();
    }
  }

  printf(1, "\nfinish order:\n");
  for(k = 1; k <= KIDS; k++){
    done = wait();
    for(i = 0; i < KIDS; i++)
      if(pids[i] == done)
        printf(1, "  #%d -> pid %d (burst %d)\n", k, done, units[i]);
  }
  exit();
}
