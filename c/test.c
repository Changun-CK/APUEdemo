#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

struct st_FileInfo
{
	int id;
	char filename[301];
	char mtime[301];
};

void *th_main(void *arg);

int main()
{
	int i = 0;
	pthread_t ret[10];
	time_t localTime;
	char strLocalTime[301];

	struct tm *stSystime;

	//time(&localTime);
	localTime = 1595302941;
	stSystime = localtime(&localTime);
	snprintf(strLocalTime, 300, "%04u-%02u-%02u %02u:%02u:%02u", stSystime->tm_year+1900, stSystime->tm_mon+1, stSystime->tm_mday, stSystime->tm_hour, stSystime->tm_min, stSystime->tm_sec);

	printf("localtime:%s\n", strLocalTime);
	return 0;

	struct st_FileInfo stFileInfo[10];
	for (i = 0; i < 10; i++)
	{
		stFileInfo[i].id = i;
		snprintf(stFileInfo[i].filename, 300, "file_%d", i);
		strncpy(stFileInfo[i].mtime, strLocalTime, 300);
	}

	i = 0;
	void *iret;
	while (i < 10)
	{
		pthread_create(&ret[i], NULL, th_main, (void*)&stFileInfo[i]);
		i++;
	}

	for (i = 0; i < 10; i++)
	{
		if (i == 5)
		{
			stFileInfo[i].id = i +1000;
			snprintf(stFileInfo[i].filename, 300, "file_%d", i);
			strncpy(stFileInfo[i].mtime, strLocalTime, 300);
		}
		pthread_join(ret[i], &iret);
	}

	for (i = 0; i < 10; i++)
	{
		printf("KKKKid:%d, filename:%s, mtime:%s\n", stFileInfo[i].id, stFileInfo[i].filename, stFileInfo[i].mtime);
	}


	return 0;
}

void *th_main(void *arg)
{
	struct st_FileInfo *fileinfo = (struct st_FileInfo*)arg;
	printf("filename:%s, mtime:%s\n", fileinfo->filename, fileinfo->mtime);

	snprintf(fileinfo->filename, 300, "file_%d", fileinfo->id + 30);
}
