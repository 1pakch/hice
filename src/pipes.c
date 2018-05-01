// pipes.c - an example multithreaded program using pipes

#include <pthread.h>
#include <stdio.h>
#include <pipe.h>

#define BUFSIZE 5
#define NWORKERS 6
#define PUSHLEN 20

typedef struct {
	pipe_consumer_t* pipe;
	pthread_t thread;
	int id;
} worker_ctx_t;

void *worker(void *ctx_)
{
	worker_ctx_t* ctx = (worker_ctx_t*) ctx_;

	int buf[BUFSIZE];
	char str[BUFSIZE*4 + 80];
	char *pos;
	size_t n;

	while (n = pipe_pop(ctx->pipe, buf, BUFSIZE)) {
		pos = str;
		pos += sprintf(pos, "T%2d:%3d items: ", ctx->id, n) ;
		for (int i=0; i < n; ++i) {
			pos += sprintf(pos, "%4d", buf[i]);
		}
		*pos++ = '\n';
		*pos++ = 0;
		printf("%s", str);
	}
	printf("T%2d: pipe is empty and closed, terminating...\n", ctx->id);
}

int main()
{
	pipe_t* p = pipe_new(sizeof(int), 10);
	pipe_producer_t* pipe = pipe_producer_new(p);

	worker_ctx_t workers[NWORKERS];
	for (int i=0; i<NWORKERS; ++i) {
		workers[i].pipe = pipe_consumer_new(p);
		workers[i].id = i;
	}
	pipe_free(p);

	for (int i=0; i<NWORKERS; ++i) {
		pthread_create(&workers[i].thread, NULL, worker, &workers[i]);
	}

	for (int i=0; i<PUSHLEN; ++i) {
		int buf[PUSHLEN];
		for (int j=0; j<PUSHLEN; buf[j++]= i*PUSHLEN+j);
		pipe_push(pipe, buf, PUSHLEN); 
	}
	pipe_producer_free(pipe);

	for (int i=0; i<NWORKERS; ++i) {
		pthread_join(workers[i].thread, NULL);
	}
}

