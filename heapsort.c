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
int cnt=1;//heap�ǰ���
//�ʿ��� ��� ��� ���ϰ� �Լ��� �߰��� �� ����

// ���� ������� �����ϴ� ����� ���� �ٸ� �� ������ �ణ�� ������ �Ӵϴ�.
// ���ڵ� ������ ������ ������ ���� �����Ǳ� ������ ����� ���α׷����� ���ڵ� ���Ϸκ��� �����͸� �а� �� ����
// ������ ������ ����մϴ�. ���� �Ʒ��� �� �Լ��� �ʿ��մϴ�.
// 1. readPage(): �־��� ������ ��ȣ�� ������ �����͸� ���α׷� ������ �о�ͼ� pagebuf�� �����Ѵ�
// 2. writePage(): ���α׷� ���� pagebuf�� �����͸� �־��� ������ ��ȣ�� �����Ѵ�
// ���ڵ� ���Ͽ��� ������ ���ڵ带 �аų� ���ο� ���ڵ带 �� ����
// ��� I/O�� ���� �� �Լ��� ���� ȣ���ؾ� �մϴ�. ��, ������ ������ �аų� ��� �մϴ�.

//
// ������ ��ȣ�� �ش��ϴ� �������� �־��� ������ ���ۿ� �о �����Ѵ�. ������ ���۴� �ݵ�� ������ ũ��� ��ġ�ؾ� �Ѵ�.
//
void readPage(FILE *fp, char *pagebuf, int pagenum)
{
	fseek(fp,PAGE_SIZE*pagenum,SEEK_SET);
	fread((void*)pagebuf,PAGE_SIZE,1,fp);
	return;
}

//
// ������ ������ �����͸� �־��� ������ ��ȣ�� �ش��ϴ� ��ġ�� �����Ѵ�. ������ ���۴� �ݵ�� ������ ũ��� ��ġ�ؾ� �Ѵ�.
//
void writePage(FILE *fp, const char *pagebuf, int pagenum)
{
	fseek(fp,PAGE_SIZE*pagenum,SEEK_SET);
	fwrite((void*)pagebuf,PAGE_SIZE,1,fp);
	return;
}

//
// �־��� ���ڵ� ���Ͽ��� ���ڵ带 �о� heap�� ����� ������. Heap�� �迭�� �̿��Ͽ� ����Ǹ�, 
// heap�� ������ Chap9���� ������ �˰����� ������. ���ڵ带 ���� �� ������ ������ ����Ѵٴ� �Ϳ� �����ؾ� �Ѵ�.
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
			{//record�� ����������
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
// �ϼ��� heap�� �̿��Ͽ� �ֹι�ȣ�� �������� ������������ ���ڵ带 �����Ͽ� ���ο� ���ڵ� ���Ͽ� �����Ѵ�.
// Heap�� �̿��� ������ Chap9���� ������ �˰����� �̿��Ѵ�.
// ���ڵ带 ������� ������ ���� ������ ������ ����Ѵ�.
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
	FILE *inputfp;	// �Է� ���ڵ� ������ ���� ������
	FILE *outputfp;	// ���ĵ� ���ڵ� ������ ���� ������
	char *heaparray[MAX];
	memset(headerBuf,0xFF,PAGE_SIZE);

	if(argc<4){
		fprintf(stderr,"a.out <option><orgin filename><sorted filename>\n");
		exit(1);
	}
	if(strcmp(argv[1],"s")){//�߸��� �ɼ��ϰ��
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
