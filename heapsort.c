#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "person.h"

#define NUM PAGE_SIZE/RECORD_SIZE
#define MAX 512

void readPage(FILE *fp,char *pagebuf,int pagenum);
void writePage(FILE *fp,const char *pagebuf,int pagenum);
void buildHeap(FILE *inputfp,char **heaparray);
void makeSortedFile(FILE *outputfp,char **heaparray);

int recordnum;
int pagenum;
char headerBuf[PAGE_SIZE];
int heapsize=0;
int cnt=1;//heap의개수
//필요한 경우 헤더 파일과 함수를 추가할 수 있음

// 과제 설명서대로 구현하는 방식은 각자 다를 수 있지만 약간의 제약을 둡니다.
// 레코드 파일이 페이지 단위로 저장 관리되기 때문에 사용자 프로그램에서 레코드 파일로부터 데이터를 읽고 쓸 때도
// 페이지 단위를 사용합니다. 따라서 아래의 두 함수가 필요합니다.
// 1. readPage(): 주어진 페이지 번호의 페이지 데이터를 프로그램 상으로 읽어와서 pagebuf에 저장한다
// 2. writePage(): 프로그램 상의 pagebuf의 데이터를 주어진 페이지 번호에 저장한다
// 레코드 파일에서 기존의 레코드를 읽거나 새로운 레코드를 쓸 때나
// 모든 I/O는 위의 두 함수를 먼저 호출해야 합니다. 즉, 페이지 단위로 읽거나 써야 합니다.

//
// 페이지 번호에 해당하는 페이지를 주어진 페이지 버퍼에 읽어서 저장한다. 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//
void readPage(FILE *fp, char *pagebuf, int pagenum)
{
	fseek(fp,PAGE_SIZE*pagenum,SEEK_SET);
	fread((void*)pagebuf,PAGE_SIZE,1,fp);
	return;
}

//
// 페이지 버퍼의 데이터를 주어진 페이지 번호에 해당하는 위치에 저장한다. 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//
void writePage(FILE *fp, const char *pagebuf, int pagenum)
{
	fseek(fp,PAGE_SIZE*pagenum,SEEK_SET);
	fwrite((void*)pagebuf,PAGE_SIZE,1,fp);
	return;
}

//
// 주어진 레코드 파일에서 레코드를 읽어 heap을 만들어 나간다. Heap은 배열을 이용하여 저장되며, 
// heap의 생성은 Chap9에서 제시한 알고리즘을 따른다. 레코드를 읽을 때 페이지 단위를 사용한다는 것에 주의해야 한다.
//
void buildHeap(FILE *inputfp, char **heaparray)
{
	int i,j;
	long long new_sn=0;
	char pageBuf[PAGE_SIZE];
	char recordBuf[RECORD_SIZE];
	memset(pageBuf,0xFF,PAGE_SIZE);
	memset(recordBuf,0xFF,RECORD_SIZE);

	for(i=1;i<=pagenum;i++)
	{
		readPage(inputfp,pageBuf,i);
		for(j=0;j<NUM;j++)
		{
			if(cnt>recordnum)
				return;

			strcpy(recordBuf,pageBuf+RECORD_SIZE*j);
			if((new_sn=atoll(recordBuf))==0)
			{//record가 비어있을경우
				break;
			}

			cnt = ++heapsize;
			heaparray[cnt] = (char*)malloc(RECORD_SIZE * sizeof(char));

			while((cnt!=1)&&(new_sn < atoll(heaparray[cnt/2])))
			{
				strcpy(heaparray[cnt],heaparray[cnt/2]);
				cnt /= 2;
			}

			memcpy(heaparray[cnt],recordBuf,RECORD_SIZE);

		}
	}
	return;

}

//
// 완성한 heap을 이용하여 주민번호를 기준으로 오름차순으로 레코드를 정렬하여 새로운 레코드 파일에 저장한다.
// Heap을 이용한 정렬은 Chap9에서 제시한 알고리즘을 이용한다.
// 레코드를 순서대로 저장할 때도 페이지 단위를 사용한다.
//
void makeSortedFile(FILE *outputfp, char **heaparray)
{
	int i=0;
	int j=0;
	int parent,child;
	int recordcnt=1;
	char pageBuf[PAGE_SIZE];
	char tempBuf[PAGE_SIZE];
	memset(pageBuf,0xff,PAGE_SIZE);
	memset(tempBuf,0xff,PAGE_SIZE);

	for(i=1;i<=pagenum;i++)
	{
		memset(pageBuf,0xff,PAGE_SIZE);
		for(j=0;j<NUM;j++)
		{
			if(recordcnt>recordnum)
			{
				break;
			}

			strcpy(pageBuf+j*RECORD_SIZE,heaparray[1]);

			strcpy(tempBuf,heaparray[heapsize]);

			heapsize--;
			parent=1;
			child=2;
			while(child <= heapsize)
			{
				if((child < heapsize) && (atoll(heaparray[child]) > atoll(heaparray[child+1])))
					child++;
				if(atoll(tempBuf) <= atoll(heaparray[child]))
					break;

				strcpy(heaparray[parent],heaparray[child]);
				parent=child;
				child*=2;
			}
			
			strcpy(heaparray[parent],tempBuf);

			recordcnt++;

		}
		writePage(outputfp,pageBuf,i);
	}

}

int main(int argc, char *argv[])
{
	FILE *inputfp;	// 입력 레코드 파일의 파일 포인터
	FILE *outputfp;	// 정렬된 레코드 파일의 파일 포인터
	char *heaparray[MAX];
	memset(headerBuf,0xFF,PAGE_SIZE);

	if(argc<4){
		fprintf(stderr,"a.out <option><orgin filename><sorted filename>\n");
		exit(1);
	}
	if(strcmp(argv[1],"s")){//잘못된 옵션일경우
		fprintf(stderr,"input -s option\n");
		exit(1);
	}
	if(access(argv[2],F_OK)<0)
	{
		fprintf(stderr,"%s doesn't exist.\n",argv[2]);
		exit(1);
	}
	else
	{
		if((inputfp=fopen(argv[2],"r+"))<0)
		{
			fprintf(stderr,"file open error for %s\n",argv[2]);
			exit(1);
		}
		readPage(inputfp,headerBuf,0);
		pagenum=headerBuf[0]-1;
		recordnum=headerBuf[4];
		buildHeap(inputfp,heaparray);
	}

	if((outputfp=fopen(argv[3],"w+"))<0)
	{
		fprintf(stderr,"file open error for %s\n",argv[3]);
		exit(1);
	}
	
	else
	{
		writePage(outputfp,headerBuf,0);
		makeSortedFile(outputfp,heaparray);
	}
	return 1;
}
