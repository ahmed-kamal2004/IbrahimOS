#include "headers.h"

/* Modify this file as needed*/
int remainingtime;
void synchornize(){
    remainingtime--;
    //printf("process remining time %d\n",remainingtime);
}
int main(int agrc, char * argv[])
{
    remainingtime = atoi(argv[1]);
    initClk();
    signal(SIGUSR1,synchornize);
    kill(getppid(),SIGCONT);

    while (remainingtime > 0);

    destroyClk(false);
    kill(getppid(),SIGUSR1);
    raise(SIGKILL);
    return 0;
}
