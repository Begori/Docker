#include <iostream>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/mount.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define CGROUP_FOLDER "/sys/fs/cgroup/pids/container/"

void die(const char* msg)
{
	if (errno)
		perror(msg);
	else
		printf("ERROR: %s\n", msg);

	exit(1);
}

void write_rule(const char* path, const char* value)
{
	int fd = open(path, O_WRONLY | O_APPEND);
	if (fd == -1)
	{
		die("Failed to open the file");
	}

	if (write(fd, value, strlen(value)) == -1)
	{
		die("Failed to write the file");
	}

	close(fd);
}

void limit_processes_creation()
{
	mkdir(CGROUP_FOLDER, S_IRUSR | S_IWUSR);

	char pid[6];
	sprintf(pid, "%d", getpid());

	char path_cgroup_procs[50];
	sprintf(path_cgroup_procs, "%s%s", CGROUP_FOLDER, "cgroup.procs");

	char path_pids_max[50];
	sprintf(path_pids_max, "%s%s", CGROUP_FOLDER, "pids.max");

	char path_release[50];
	sprintf(path_release, "%s%s", CGROUP_FOLDER, "notify_on_release");

	write_rule(path_cgroup_procs, pid);
	write_rule(path_release, "1");
	write_rule(path_pids_max, "5");
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
		die("Memery error");
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
		die("Failed to clone the process");
	}

	wait(nullptr);
}

int runnable(void* args)
{
	run("/bin/sh");
}

int jail(void* args)
{
	limit_processes_creation();

	printf("Child PID: %d\n", getpid());
	
	setup_variables();
	setup_root("./root");

	mount("proc", "/proc", "proc", 0, 0);

	clone_process(runnable, SIGCHLD);

	umount("/proc");

	return 0;
}

int main(int argc, char** argv)
{
	printf("Parent PID: %d\n", getpid());

	clone_process(jail, CLONE_NEWPID | CLONE_NEWUTS | SIGCHLD);

	return 0;
}