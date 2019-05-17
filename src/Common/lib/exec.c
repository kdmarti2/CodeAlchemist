#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <openssl/md5.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

void failwith (char *msg) {
  perror (msg);
  exit (1);
}

int readFd (int fd, char *buf, int size){
  return read (fd, buf, size);
}

void closeFd (int fd) {
  close (fd);
}

int waitForExit (int pid){
  int status;
  while (waitpid (pid, &status, 0) != pid) ;
  if (WIFEXITED(status)) return WEXITSTATUS(status);
  else if (WIFSIGNALED(status)) return -WTERMSIG(status);
  else failwith ("waitForExit fail");
}

/**
 http://man7.org/tlpi/code/online/diff/fileio/copy.c.html
 **/
void copy_file(char* src,char *dst) {
	int fin, fout, openFlags;;
	mode_t filePerms;
	ssize_t numRead;
	char buf[1024];

	fin = open(src,O_RDONLY);

	if (fin == -1)
		return;

	openFlags = O_CREAT | O_WRONLY | O_TRUNC;
	filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

	fout = open(dst,openFlags,filePerms);
	while ((numRead = read(fin,buf,1024))>0) {
		if (write(fout,buf,numRead) != numRead)
			return;
	}
	close(fin);
	close(fout);
}

void exec (int argc, char **arg, char *dir, int timeout, int *ret) {
  int i, pid;
  char **argv = malloc (sizeof(char*) * (argc + 1));
  int stdoutPipe[2] = {-1, -1};
  int stderrPipe[2] = {-1, -1};

  for (i = 0; i< argc; i++) argv[i] = arg[i];
  argv[argc] = NULL;

  if (pipe (stdoutPipe) < 0 || pipe (stderrPipe) < 0)
    failwith ("pipe fail");
  

  /**
  *Save the generated testcase
  **/

  char* fout_name = calloc(1,256);

  char* fin_name = argv[argc-1];

  char * name = strrchr(fin_name,'/');

  /**
   *  Failed to find a directory probably local
   *  This is likely to cause a race_condiction here but hell got to go fast
   *  */

  if (getenv("RECORD"))
  {
  	time_t seconds = time(NULL);
  	if (!name) {
  	      snprintf(fout_name,255,"/codealc/gen/%s_%d",fin_name,(int)seconds);
  	} else {
  	      snprintf(fout_name,255,"/codealc/gen/%s_%d",name,(int)seconds);
  	}
  	copy_file(fin_name,fout_name);
  	free(fout_name);
  }

  pid = vfork ();
  
  if (pid < 0) failwith ("vfork fail");
  else if (pid == 0) {
    close (stdoutPipe[0]);
    close (stderrPipe[0]);

    dup2 (stdoutPipe[1], 1); close(stdoutPipe[1]);
    dup2 (stderrPipe[1], 2); close(stderrPipe[1]);

    alarm (timeout);
    if (chdir (dir) < 0) failwith ("chdir fail");
    execv (argv[0], argv);
  }
  
  close (stdoutPipe[1]);
  close (stderrPipe[1]);

  ret[0] = pid;
  ret[1] = stdoutPipe[0];
  ret[2] = stderrPipe[0];

}
