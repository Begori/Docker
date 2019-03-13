#include <iostream>
#include <sys/wait.h>

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
	printf("Hello from child\n");

	return 0;
}

int main(int argc, char** argv)
{
	printf("Hello from parent\n");
	clone(jail, stack_memory(), SIGCHLD, 0);
	wait(nullptr);

	return 0;
}