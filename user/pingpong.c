#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]){
	int p[2];
        int c[2];
        char buff[128];
	pipe(p);
	pipe(c);
	if (fork() == 0){
	    close(p[1]);
	    read(p[0], buff, 4);
	    close(p[0]);

	    printf("%d: received %s\n", getpid(), buff);

	    close(c[0]);
	    write(c[1], "pong", 4);
	    close(c[1]);

	}
	else {
	    close(p[0]);
	    write(p[1], "ping", 4);
	    close(p[1]);

	    close(c[1]);
	    read(c[0], buff, 4);
	    close(c[0]);

	    printf("%d: received %s\n", getpid(), buff);
	}
	exit(0);
}
