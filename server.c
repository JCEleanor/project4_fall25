
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

struct message
{
	char source[50];
	char target[50];
	char msg[200]; // message body
};

void terminate(int sig)
{
	printf("Exiting....\n");
	fflush(stdout);
	exit(0);
}

int main()
{
	int server;
	int target;
	int dummyfd;
	struct message req;
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, terminate);
	server = open("serverFIFO", O_RDONLY);
	if (server == -1) {
		perror("open serverFIFO");
		exit(1);
	}
	dummyfd = open("serverFIFO", O_WRONLY);
	if (dummyfd == -1) {
		perror("open serverFIFO for writing");
		exit(1);
	}

	while (1)
	{
		read(server, &req, sizeof(struct message));

		printf("Received a request from %s to send the message %s to %s.\n", req.source, req.msg, req.target);

		target = open(req.target, O_WRONLY);
		if (target == -1) {
			perror("open target FIFO");
		} else {
			write(target, &req, sizeof(struct message));
			close(target);
		}
	}
	close(server);
	close(dummyfd);
	return 0;
}
