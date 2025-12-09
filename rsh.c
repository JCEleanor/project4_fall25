#include <stdio.h>
#include <stdlib.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h> // Required for mkfifo

#define N 13

extern char **environ;
char uName[20];

char *allowed[N] = {"cp", "touch", "mkdir", "ls", "pwd", "cat", "grep", "chmod", "diff", "cd", "exit", "help", "sendmsg"};

struct message
{
	char source[50];
	char target[50];
	char msg[200];
};

void terminate(int sig)
{
	printf("Exiting....\n");
	fflush(stdout);
	exit(0);
}

void sendmsg(char *user, char *target, char *msg)
{
	int server_fifo_fd;
	struct message req;

	strcpy(req.source, user);
	strcpy(req.target, target);
	strcpy(req.msg, msg);

	server_fifo_fd = open("serverFIFO", O_WRONLY);
	if (server_fifo_fd == -1)
	{
		perror("open serverFIFO in sendmsg");
		return;
	}

	write(server_fifo_fd, &req, sizeof(struct message));
	close(server_fifo_fd);
}

void *messageListener(void *arg)
{
	int user_fifo_fd;
	struct message incoming_msg;
	char fifo_name[50];

	strcpy(fifo_name, uName);

	// Open the user's FIFO in read-only mode
	user_fifo_fd = open(fifo_name, O_RDONLY);
	if (user_fifo_fd == -1)
	{
		perror("open user FIFO in messageListener");
		pthread_exit((void *)1);
	}

	// This dummyfd is needed to prevent open from blocking indefinitely
	// if there are no writers to the FIFO
	int dummyfd = open(fifo_name, O_WRONLY);
	if (dummyfd == -1)
	{
		perror("open user FIFO (dummy) in messageListener");
		close(user_fifo_fd);
		pthread_exit((void *)1);
	}

	while (1)
	{
		// Read incoming messages
		ssize_t bytes_read = read(user_fifo_fd, &incoming_msg, sizeof(struct message));
		if (bytes_read == -1)
		{
			perror("read from user FIFO in messageListener");
			break;
		}
		else if (bytes_read == 0)
		{
			// FIFO was closed by the other end, re-open it to continue listening
			close(user_fifo_fd);
			close(dummyfd);
			user_fifo_fd = open(fifo_name, O_RDONLY);
			dummyfd = open(fifo_name, O_WRONLY);
			continue;
		}

		// Print the incoming message
		printf("\nIncoming message from %s: %s\n", incoming_msg.source, incoming_msg.msg);
		fflush(stdout);
	}

	close(user_fifo_fd);
	close(dummyfd);
	pthread_exit((void *)0);
}

int isAllowed(const char *cmd)
{
	int i;
	for (i = 0; i < N; i++)
	{
		if (strcmp(cmd, allowed[i]) == 0)
		{
			return 1;
		}
	}
	return 0;
}

int main(int argc, char **argv)
{
	pid_t pid;
	char **cargv;
	char *path;
	char line[256];
	int status;
	posix_spawnattr_t attr;

	if (argc != 2)
	{
		printf("Usage: ./rsh <username>\n");
		exit(1);
	}
	signal(SIGINT, terminate);

	strcpy(uName, argv[1]);

	// Create user's FIFO
	if (mkfifo(uName, 0666) == -1)
	{
		perror("mkfifo user FIFO");
		// If FIFO already exists, it's fine, otherwise, it's an error
		// We can proceed if it's EEXIST
	}

	// create the message listener thread
	pthread_t listener_thread;
	if (pthread_create(&listener_thread, NULL, messageListener, NULL) != 0)
	{
		perror("pthread_create messageListener");
		exit(1);
	}

	while (1)
	{

		fprintf(stderr, "rsh>");

		if (fgets(line, 256, stdin) == NULL)
			continue;

		if (strcmp(line, "\n") == 0)
			continue;

		line[strlen(line) - 1] = '\0';

		char cmd[256];
		char line2[256];
		strcpy(line2, line);
		strcpy(cmd, strtok(line, " "));

		if (!isAllowed(cmd))
		{
			printf("NOT ALLOWED!\n");
			continue;
		}

		if (strcmp(cmd, "sendmsg") == 0)
		{
			char *targetUser = strtok(NULL, " ");
			if (targetUser == NULL)
			{
				printf("sendmsg: you have to specify target user\n");
				continue;
			}

			char *messageStart = strtok(NULL, ""); // Get the rest of the line as the message
			if (messageStart == NULL)
			{
				printf("sendmsg: you have to enter a message\n");
				continue;
			}

			// Trim leading space from message, if any
			if (*messageStart == ' ')
			{
				messageStart++;
			}
			sendmsg(uName, targetUser, messageStart);
			continue;
		}

		if (strcmp(cmd, "exit") == 0)
			break;

		if (strcmp(cmd, "cd") == 0)
		{
			char *targetDir = strtok(NULL, " ");
			if (strtok(NULL, " ") != NULL)
			{
				printf("-rsh: cd: too many arguments\n");
			}
			else
			{
				chdir(targetDir);
			}
			continue;
		}

		if (strcmp(cmd, "help") == 0)
		{
			printf("The allowed commands are:\n");
			for (int i = 0; i < N; i++)
			{
				printf("%d: %s\n", i + 1, allowed[i]);
			}
			continue;
		}

		cargv = (char **)malloc(sizeof(char *));
		cargv[0] = (char *)malloc(strlen(cmd) + 1);
		path = (char *)malloc(9 + strlen(cmd) + 1);
		strcpy(path, cmd);
		strcpy(cargv[0], cmd);

		char *attrToken = strtok(line2, " "); /* skip cargv[0] which is completed already */
		attrToken = strtok(NULL, " ");
		int n = 1;
		while (attrToken != NULL)
		{
			n++;
			cargv = (char **)realloc(cargv, sizeof(char *) * n);
			cargv[n - 1] = (char *)malloc(strlen(attrToken) + 1);
			strcpy(cargv[n - 1], attrToken);
			attrToken = strtok(NULL, " ");
		}
		cargv = (char **)realloc(cargv, sizeof(char *) * (n + 1));
		cargv[n] = NULL;

		// Initialize spawn attributes
		posix_spawnattr_init(&attr);

		// Spawn a new process
		if (posix_spawnp(&pid, path, NULL, &attr, cargv, environ) != 0)
		{
			perror("spawn failed");
			exit(EXIT_FAILURE);
		}

		// Wait for the spawned process to terminate
		if (waitpid(pid, &status, 0) == -1)
		{
			perror("waitpid failed");
			exit(EXIT_FAILURE);
		}

		// Destroy spawn attributes
		posix_spawnattr_destroy(&attr);
	}
	return 0;
}
