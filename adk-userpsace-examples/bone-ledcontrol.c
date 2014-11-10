#include<unistd.h>
#include<signal.h>
#include<fcntl.h>
#include<time.h>

#include<stdio.h>
#include<stdlib.h>

void fillrtcbuf(char*,int);
void* eread(void*);
void* ewrite(void*);
void update_leds(int,int);
void update_trigger(int,int);

int led_state[4];
int led_fds[4],trigger_fds[4];
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
	led_fds[0]=open("/sys/class/leds/beaglebone::usr0/brightness",O_WRONLY);
	led_fds[1]=open("/sys/class/leds/beaglebone::usr1/brightness",O_WRONLY);
	led_fds[2]=open("/sys/class/leds/beaglebone::usr2/brightness",O_WRONLY);
	led_fds[3]=open("/sys/class/leds/beaglebone::usr3/brightness",O_WRONLY);

	trigger_fds[0]=open("/sys/class/leds/beaglebone::usr0/trigger",O_WRONLY);
	trigger_fds[1]=open("/sys/class/leds/beaglebone::usr1/trigger",O_WRONLY);
	trigger_fds[2]=open("/sys/class/leds/beaglebone::usr2/trigger",O_WRONLY);
	trigger_fds[3]=open("/sys/class/leds/beaglebone::usr3/trigger",O_WRONLY);

	pthread_t ptread,ptwrite;
	pthread_create(&ptread,NULL,eread,&fd);
	pthread_create(&ptwrite,NULL,ewrite,&fd);

	pthread_join(ptread);
	pthread_join(ptwrite);

	for(i=0;i<4;i++)
	{
		close(led_fds[i]);
		close(trigger_fds[i]);
	}
	return 0;
}
void* eread(void* pv)
{
	int k,i,fd;
	fd=*((int*)pv);
	char buf[64];
	char* ps[2]={"off","on"};
	while(1)
	{
		k=read(fd,buf,64);
		printf("k=%d\n",k);
		if(k<0)
		{
			perror("read");
			exit(2);
		}
		if(k==0) { printf("no data from aoa\n"); }
		if(buf[0]==0x41) update_leds(buf[1]-1,buf[2]);
		if(buf[0]==0x42) update_trigger(buf[1]-1,buf[2]);
		//if(buf[0]==0x43) update_seek(buf[1]);
	}
}
void update_leds(int id,int value)
{
	int i,k;
	if(value)
		k=write(led_fds[id],"1",1);
	else
		k=write(led_fds[id],"0",1);
	led_state[id]=value;
	for(i=0;i<4;i++)
		if(value)
			printf("led:%d is on\t",i+1);
		else
			printf("led:%d is off\t",i+1);
	printf("\n");
}
void update_trigger(int id,int value)
{
	int k;
	if(value){
		k=write(trigger_fds[id],"heartbeat",9);
		printf("LED%d--->trigger=HEARTBEAT\n",id);
	}
	else {
		k=write(trigger_fds[id],"none",4);
		printf("LED%d--->trigger=NONE\n",id);
	}
}

void* ewrite(void* pv)
{
	int fd=*((int*)pv);
	char rtcbuf[5];
	while(1) {
		fillrtcbuf(rtcbuf,5);
		write(fd,rtcbuf,sizeof(rtcbuf));
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
	//printf("%d:%d:%d\n",pbuf[1],pbuf[2],pbuf[3]);
}
