#include"painting.h"
#include"myProto.h"
#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <omp.h>

//reads all the info in the given file and allocates memory where needed
void readFromFile(const char *fileName,Paint** pictures,Paint** objects,double* match,int* pictNum,int*objNum)
{
	FILE* fp;
	int i,j;
	Paint* tmp1;
	Paint* tmp2;
	
	if((fp=fopen(fileName,"r"))==0)
	{
		printf("Cant open the file");
		exit(0);
	}

	fscanf(fp,"%lf",match);
	fscanf(fp,"%d",pictNum);
	tmp1=(Paint*)malloc(sizeof(Paint)*(*pictNum));
	if(!tmp1)
	{
		printf("Allocation failed.\n");
		exit(0);
	}
	for(i=0;i<*pictNum;i++)
	{
		fscanf(fp,"%d",&tmp1[i].id);
		fscanf(fp,"%d",&tmp1[i].size);
		tmp1[i].paint=(int*)malloc(sizeof(int)*tmp1[i].size*tmp1[i].size);
		if(!tmp1[i].paint)
		{
			printf("Allocation failed.\n");
			exit(0);
		}
		for(j=0;j<tmp1[i].size*tmp1[i].size;j++)
		{
			fscanf(fp,"%d",&tmp1[i].paint[j]);
		}
	}


	fscanf(fp,"%d",objNum);
	tmp2=(Paint*)malloc(sizeof(Paint)*(*objNum));
	if(!tmp2)
	{
		printf("Allocation failed.\n");
		exit(0);
	}
	for(i=0;i<*objNum;i++)
	{
		fscanf(fp,"%d",&tmp2[i].id);
		fscanf(fp,"%d",&tmp2[i].size);
		tmp2[i].paint=(int*)malloc(sizeof(int)*tmp2[i].size*tmp2[i].size);
		if(!tmp2[i].paint)
		{
			printf("Allocation failed.\n");
			exit(0);
		}
		for(j=0;j<tmp2[i].size*tmp2[i].size;j++)
		{
			fscanf(fp,"%d",&tmp2[i].paint[j]);
		}
	}
	*pictures=tmp1;
	*objects=tmp2;
	fclose(fp);
}

//checks the found , if it is 1 then prints all the given info on a file if found=0 then prints that the program found nothing
void writeToFile(const char* fileName,int picId,int objId,int row,int col,int found)
{
	FILE* fp;

	if((fp=fopen(fileName,"a"))==0)
	{
		printf("Cannot open file %s for writing\n",fileName);
		return;
	}

	if(found!=1)
		fprintf(fp,"Picture %d No Objects were found\n",picId);
	else
		fprintf(fp,"Picture %d found Object %d in Position(%d,%d)\n",picId,objId,row,col);

	fclose(fp);
}

//packs picture/object in a given buffer
void packPicture(Paint* picture,char** buffer,int* position,int bufferSize)
{
	MPI_Pack(&(picture->id),1,MPI_INT,*buffer,bufferSize,position,MPI_COMM_WORLD);
	MPI_Pack(&(picture->size),1,MPI_INT,*buffer,bufferSize,position,MPI_COMM_WORLD);
	MPI_Pack(picture->paint,picture->size*picture->size,MPI_INT,*buffer,bufferSize,position,MPI_COMM_WORLD);
}

//checks a single place in the matrix if it has a match with a given object
void checkBlock(int row,int col,int* picture,int* object,int pictSize,int objSize,double match,int* answerArr)
{
	int i,j,found;
	double diff,sum=0;
	int* currentPictStartI=picture+row*pictSize+col;
	int matNum,subMatNum;

	for(i=0;i<objSize;i++)
	for(j=0;j<objSize;j++)
	{
		matNum=*(currentPictStartI+i*pictSize+j);
		subMatNum=*(object+i*objSize+j);

		diff=abs((double)(matNum-subMatNum)/(double)matNum); 
		sum+=diff;

//==================================================================================
//### WITH THOSE TWO LINES OF CODE BELLOW I COUD HAVE SHORTENED THE RUN TIME TO LESS THEN ONE SECOND BUT THEN THE PARALLLEL COMPUTING WONT MATTER
//AND THAT IS THE HOLE POINT OF THE PROJECT SO I DIDNT ###
/*
		if(sum>match)
			return;
*/
//=================================================================================
	}
	if(sum<=match)
	{
		found=1;
		answerArr[2]=row;
		answerArr[3]=col;
		answerArr[4]=found;
	}
}

//goes throught the hole matrix and check if there is a match with a given object , returns if object found or not
int checkObjInPict(int* picture,int* object,int pictSize,int objSize,double match,int* answerArr)
{
	int i,j;
	int checksInAPict=pictSize-objSize+1; //number of checks that need to be done in each direction

	for(i=0;i<checksInAPict;i++)
		for(j=0;j<checksInAPict;j++)
		{
			checkBlock(i,j,picture,object,pictSize,objSize,match,answerArr);
			if(answerArr[4]==1)
				return answerArr[4];
		}
	return answerArr[4];
}


