#include "headers.h"

/* Modify this file as needed*/
int remainingtime;
int x,y;
void handler(int signum);
int main(int agrc, char * argv[])
{
    initClk();
    remainingtime=atoi(argv[1]);
    signal(SIGCONT,handler);
    handler(0);
    printf("process has terminated at time %d\n",x);
    destroyClk(false);
    return 0;
}
void processing()
{
    x=getClk();
    while(remainingtime>0)
    {
        y=getClk();
        if(x!=y)
        {
            remainingtime--;
            x=y;
        }
    }
    kill(getppid(),SIGUSR1);
    exit(0);
}
void handler(int signum)
{
    processing();

}
