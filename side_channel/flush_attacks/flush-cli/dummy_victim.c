#include <signal.h>
#include <sched.h>
#include <unistd.h>
#include <signal.h>

int hit(){

	for(unsigned int i=0; i < 1000; i++)
		asm volatile (
				"nop\n"\
				"nop\n"\
				"nop\n"\
			);
}

int main(){

	sigset_t sigset;
	int signum;

	//create set of signals to wait for
	sigemptyset(&sigset);

	sigaddset(&sigset, SIGUSR1);
	sigaddset(&sigset, SIGUSR2);

	while(1){

		//wait for signal
		sigwait(&sigset, &signum);

		//signal for hit
		if( signum == SIGUSR1 ) hit();
	}

	return 0;
}
