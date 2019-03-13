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

int jail(void* args)
{
	clearenv();

	run("/bin/sh");

	return 0;
}

int main(int argc, char** argv)
{
	printf("Hello from parent\n");
	clone(jail, stack_memory(), SIGCHLD, 0);
	wait(nullptr);

	return 0;
}