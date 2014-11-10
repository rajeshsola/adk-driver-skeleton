#include<unistd.h>
#include<signal.h>
#include<fcntl.h>
#include<time.h>

#include<stdio.h>
#include<stdlib.h>

int stop_read=0;
int stop_write=0;
void handler(int signo)
{
	stop_read=1;
	stop_write=1;
	printf("stop adk communication\n");
}
void fillrtcbuf(char*,int);
void* eread(void*);
void* ewrite(void*);
int main(int argc,char* argv[])
{
	int fd,k,i;
	char *devfile;
	if(argc>1)
		devfile=argv[1];
	else
		devfile="/dev/aoa-skel0";
		fd=open(devfile,O_RDWR);
	if(fd<0)
	{
		perror("open");
		exit(0);
	}
	printf("opening %s\n",devfile);
	signal(SIGINT,handler);
	pthread_t ptread,ptwrite;
	pthread_create(&ptread,NULL,eread,&fd);
	pthread_create(&ptwrite,NULL,ewrite,&fd);

	pthread_join(ptread);
	pthread_join(ptwrite);
	return 0;
}
void* eread(void* pv)
{
	int k,i,fd,value;
	fd=*((int*)pv);
	char buf[64];
	int led_state[4]={},led_id;
	int trigger_state,trigger_id;
	double per;
	char* ps[2]={"off","on"};
	while(!stop_read)
	{
		k=read(fd,buf,64);
		//printf("k=%d\n",k);
		if(k<0)
		{
			perror("read");
			exit(2);
		}
		if(k==0) { printf("no data from aoa\n"); }
		if(buf[0]==0x41) {
			led_id=buf[1]&0xf;
			value=buf[2]&0xf;
			led_state[led_id-1]=value;
			for(i=0;i<4;i++)
				printf("led:%d is %s\t",i+1,ps[led_state[i]]);
			printf("\n");
		}
		if(buf[0]==0x42) {
			trigger_id=buf[1];
			trigger_state=buf[2];
			if(trigger_state)
				printf("LED%d::trigger state:heartbeat\n",trigger_id);
			else
				printf("LED%d::trigger state:none\n",trigger_id);
		}
		if(buf[0]==0x43)
		{
			per=buf[1]*100/100.0;
			printf("volume/brightness=%.2lf%\n",per);
		}
		switch(buf[0])
		{
			case 0x70:printf("acessory initialized\n");
				  break;
			case 0x73:printf("activity resumed:%d\n",buf[1]);
				  break;
			case 0x72:printf("activity paused\n");
				  break;
			case 0x74:printf("activity stopped\n");
				  break;
			case 0x71:printf("activity started\n");
				  break;
			case 0x75:printf("accessory closed\n");
				  break;
			case 0x76:printf("activity created+adk ctor\n");
				  break;
			case 0x77:printf("starting reading thread\n");
				  break;
	  	}			  
	}
}
void* ewrite(void* pv)
{
	int fd=*((int*)pv);
	int k;
	char rtcbuf[5];
	while(!stop_write) {
		fillrtcbuf(rtcbuf,5);
		//printf("going to write\n");
		k=write(fd,rtcbuf,sizeof(rtcbuf));
		//printf("write status=%d\n",k);
		sleep(1);
	}
}
void fillrtcbuf(char* pbuf,int len)
{
	pbuf[0]=0x46;
	time_t t1=time(0),temp;
	struct tm* pt=localtime(&t1);
	pbuf[1]=pt->tm_hour;
	pbuf[2]=pt->tm_min;
	pbuf[3]=pt->tm_sec;
	pbuf[4]='\n';
//	printf("%d:%d:%d\n",pbuf[1],pbuf[2],pbuf[3]);
}
