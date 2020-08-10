#include <signal.h>
#include <sched.h>
#include <unistd.h>
#include <signal.h>

void hit(void* addr){

	for( unsigned int i=0; i < 10; i++){
		asm volatile(
			"movl (%0), %%eax\n"
			:
			: "r"(addr)
			: "eax"
		);
	}
}

void miss(void* addr){

	for( unsigned int i=0; i < 10; i++){
		asm volatile(
			"clflush 0(%0)\n"
			:
			: "r"(addr)
		);
	}
}

int main(){

	sigset_t sigset;
	siginfo_t siginfo;

	//create set of signals to wait for
	sigemptyset(&sigset);

	sigaddset(&sigset, SIGUSR1);
	sigaddset(&sigset, SIGUSR2);

	while(1){

		//wait for signal
		sigwaitinfo(&sigset, &siginfo);

		//signal for hit
		if( siginfo.si_signo == SIGUSR1 ) hit(siginfo.si_ptr);

		//signal for miss
		if( siginfo.si_signo == SIGUSR2 ) miss(siginfo.si_ptr);
	}

	return 0;
}
