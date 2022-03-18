#define _GNU_SOURCE
#include <fcntl.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


const int STACK_SIZE = 1024*1024;
const int BUF_SIZE = 1024;
const char *BC_CONTAINER_HOSTNAME = "bcdocker";
//const char *BC_CONTAINER_HOSTNAME = "docker_container";


void help_and_exit(char *progname) {
        printf ("Usage: %s run image application\n",progname);
        exit (EXIT_FAILURE);
}

void fail_and_exit(char *msg) {
        perror(msg);
        exit (EXIT_FAILURE);
}


void run_app(char *app) {
        char *argv[]={app,NULL};
        char *envp[]={"PATH=/bin:/usr/bin",NULL};
        pid_t pid;

        pid = fork();

        if (pid ==0) {           //child
                if (execve(app, argv, envp)== -1) {
                        fail_and_exit("execve(app ...)");
                }
        }
        else if (pid > 0) {         //parent
                if (waitpid(pid, NULL, 0) == -1) {
                        fail_and_exit("waitpid(child, ...)");
                }
        }
        else {                   //failure
                fail_and_exit("fork()");
        }

}



int run_setup_container(void *arg) {
        char hostname [BUF_SIZE];
        char **argv = (char **)arg;

        printf("child pid = %d\n", getpid());
        //gethostname(hostname,BUF_SIZE);
        //printf("hostname = %s\n", hostname);
        sethostname(BC_CONTAINER_HOSTNAME, strlen(BC_CONTAINER_HOSTNAME));
        gethostname(hostname,BUF_SIZE);
        printf("hostname = %s\n", hostname);

        if (chroot(argv[2]) == -1) {
                     fail_and_exit("cannot chroot(container_root..)");
             }
        
        //if (chroot("/home/brooklyn/pro2/cisc7310sys") == -1) {
        //        fail_and_exit("choot(/home/brooklyn/pro2/cisc7310sys)");
        //}
        //system("ls -l /");
        //system("ls -l");
        chdir("/");
        //system("ls -l .");

        if (mount("proc", "proc", "proc", 0, NULL) == -1) {
                fail_and_exit ("mount(proc ...)");
        }

        //run_app("/usr/bin/dash");
        run_app(argv[3]);
}

void run_container(int argc, char *argv[]) {
        pid_t pid;
        char *stack, *stackTop;

        stack = malloc(STACK_SIZE);
        if (stack == NULL) {
                fail_and_exit("malloc(STACK_SIZE)");

        }
        stackTop = stack + STACK_SIZE;


        //pid = clone(run_setup_container,stackTop,
                //CLONE_NEWNS | CLONE_NEWPID | CLONE_NEWUTS | SIGCHLD, (void *)argv);
        pid = clone(run_setup_container,stackTop,
                CLONE_NEWNS | CLONE_NEWPID | CLONE_NEWUTS | CLONE_NEWNET | CLONE_NEWCGROUP | SIGCHLD, (void *)argv);
        if (pid == -1) {
                fail_and_exit("clone(run_setup_container ...)");
        }
        if (waitpid(pid, NULL, 0) == -1) {
                fail_and_exit("waitpid(pid, NULL, NULL)");
        }
}

int main (int argc, char *argv[]){
//      prepareCgroups();
//		char buffer[256];
//		snprintf(buffer,256,"%d",getpid());
//      system("cat /sys/fd/cgroup/memory/bcdocker/memory.limit_in_bytes");
//		system("mkdir /sys/fs/cgroup/memory/bcdocker");
//		system("echo $s > /sys/fs/cgroup/memory/bcdocker/tasks");
/*		int fd = open("/sys/fs/cgroup/memory/bcdocker/memory.limit_in_bytes", O_CREAT | O_WRONLY);
		if(fd == -1){
			perror("cannot open(sys/fs/cgroup/memory/bcdocker...)");
			exit(EXIT_FAILURE);
		}
		if(write(fd, buf, strlen(buf)==-1){
			perror("cannot write(sys/fs/cgroup/memory/bcdocker...)");
			exit(EXIT_FAILURE);
		}
		close(fd);
*/


        if (argc < 4){
                help_and_exit(argv[0]);
        }

        if (argc >4){
        	if(strcmp(argv[4],"--memory=")==0){
        		printf("Doing memory\n");
/*        		char command[100];
        		snprintf(command,sizeof(command), "sh -c \"echo %s > tinysys/sy$);
        		if (system(command)==-1){
        			perror("cannot set memory");
        		}
*/        		int fd = open ("sys/fs/cgroup/memory/bcdocker/memory.limit_in_bytes", O_CREAT | O_WRONLY);
        		if (fd == -1){
        			perror("cannot open (sys/fs/cgroup/memory/bcdocker...)");
        		}
        		if (write(fd,argv[5],strlen(argv[5]))==-1){
        			perror("Cannot write (sys/fs/cgroup/memory/bcdocker...)");
        			exit(EXIT_FAILURE);
        		}
        		close(fd);
        	}

        	else if(strcmp(argv[4],"--pids-limit=")==0){
        		printf("Doing pid\n");
/*        		char command[100];
        		snprintf(command,sizeof(command),"sh -c \"echo %s = tinysys/sy$system(command);
*/	        	int fd = open("/sys/fs/cgroup/pids/bcdocker/tasks", O_CREAT | O_WRONLY);
		    	if (fd == -1) {
		    		perror("Cannot open pid ...");
		    		exit(EXIT_FAILURE);
		    	}
				if (write(fd, argv[5],strlen(argv[5]))==-1){
				perror("Cannot wirte pid ...");
				exit(EXIT_FAILURE);
				}
        	}
        }


        if (strcmp(argv[1],"run")==0){
                run_container(argc,argv);
        }

        else {
                help_and_exit(argv[0]);
        }

        exit (EXIT_SUCCESS);
}





