#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#define MAXSTRINGSZ  4096

static pthread_key_t key;
static pthread_once_t init_done = PTHREAD_ONCE_INIT;
static pthread_mutex_t env_mutex = PTHREAD_MUTEX_INITIALIZER;

extern char **environ;

static void thread_init(void)
{
	pthread_key_create(&key, free);
}

void free(void *ptr)
{
	free(ptr);
}

char *getenv(const char *name)
{
	int i, len;
	char *envbuf;

	pthread_once(&init_done, thread_init);
	pthread_mutex_lock(&env_mutex);
	envbuf = (char *)pthread_getspecific(key);
	if (envbuf == NULL)
	{
		envbuf = malloc(MAXSTRINGSZ);
		if (envbuf = NULL)
		{
			pthread_mutex_unlock(&env_mutex);
			return(NULL);
		}
		pthread_setspecific(key, envbuf);
	}

	len = strlen(name);
	for (i = 0; environ[i] != NULL; i++)
	{
		if ( (strncmp(name, environ[i], len) == 0) && (environ[i][len] == '=') )
		{
			strncpy(envbuf, &environ[i][len+1], MAXSTRINGSZ-1);
			pthread_mutex_unlock(&env_mutex);
			return(envbuf);
		}
	}

	pthread_mutex_unlock(&env_mutex);

	return(NULL);
}

int main(int argc, char *argv[])
{
	//unsigned int i = 0;
	//char myEnv[10241];
	//memset(myEnv, 0, sizeof(myEnv));

	//pthread_t iret[10];
	//for (i = 0; i < 10; i++)
	//{
	//	pthread_create(iret+i, NULL, getenv, NULL);
	//}
	//strcpy(myEnv, getenv("HOME"));

	return 0;
}
