#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/mount.h>

#define CGROUP_FOLDER "/sys/fs/cgroup/pids/container/"

void limitProcessesCreation()
{
	mkdir(CGROUP_FOLDER, S_IRUSR | S_IWUSR);
}

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

void clone_process(int(*function)(void*), int flags)
{
	if (clone(function, stack_memory(), flags, 0) == -1)
	{
		printf("Clone\n");
	}

	wait(nullptr);
}

int runThis(void* args)
{
	run("/bin/sh");
}

int jail(void* args)
{
	printf("Child PID: %d\n", getpid());
	
	setup_variables();
	setup_root("./root");

	mount("proc", "/proc", "proc", 0, 0);

	clone_process(runThis, SIGCHLD);

	umount("/proc");

	return 0;
}

int main(int argc, char** argv)
{
	printf("Parent PID: %d\n", getpid());

	clone_process(jail, CLONE_NEWPID | CLONE_NEWUTS | SIGCHLD);

	return 0;
}