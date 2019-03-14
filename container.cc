#include <iostream>
#include <sys/wait.h>
#include <unistd.h>

int run(const char* name)
{
	char* args[] = { (char*)name, (char*)0 };
	execvp(name, args);
}

char* stack_memory()
{
	int stackSize = 65536;
	char* stack = new (std::nothrow) char[stackSize];

	if (stack == nullptr)
	{
		printf("Cannot allocate memory \n");
		exit(1);
	}

	return stack + stackSize;
}

void setup_variables()
{
	clearenv();
	setenv("TERM", "xterm", 0);
	setenv("PATH", "/bin/:/sbin/:/usr/bin:/usr/sbin", 0);
}

void setup_root(const char* folder)
{
	chroot(folder);
	chdir("/");
}

int jail(void* args)
{
	printf("Child PID: %d\n", getpid());
	
	setup_variables();
	setup_root("./root");

	run("/bin/sh");

	return 0;
}

int main(int argc, char** argv)
{
	printf("Parent PID: %d\n", getpid());

	if (clone(jail, stack_memory(), CLONE_NEWPID | CLONE_NEWUTS | SIGCHLD, 0) == -1)
	{
		printf("Clone\n");
	}

	wait(nullptr);

	return 0;
}