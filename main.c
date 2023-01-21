#include <stdio.h>
#include <stdlib.h>
#include "myProto.h"
#include "mpi.h"
#include "painting.h"
#include <omp.h>

#define FILE_NAME_IN "input.txt"
#define FILE_NAME_OUT "output.txt"
#define THREAD_NUM 4


int main(int argc,char* argv[])
{

	int myRank,size,position; //ranks and size of proccesses, position for the MPI_Packing 
	MPI_Status status;
	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD,&size);
	MPI_Comm_rank(MPI_COMM_WORLD,&myRank);
	if(size!=2)
	{
		printf("Please run with 2 processes.\n");
		fflush(stdout);
		MPI_Abort(MPI_COMM_WORLD,1);
	}


	int totalNum,pictNum,objNum,i,j,bufferSize=0; //given amount of pictures,amount of objects , i,j abd the size of the buffer
	double match;    //the given match
	Paint* pictures; //array of pictures
	Paint* objects; //array of objects
	int* finalAnswersArr; //the array that will hold the found object id in picture id and in what row and column
	char* buffer; //buffer for the MPI_Pack

	//proccess zero reads the data from txt, allocates what it needs and sends information to proccess one
	if(myRank==0)
	{
		readFromFile(FILE_NAME_IN,&pictures,&objects,&match,&pictNum,&objNum);
		totalNum=pictNum;
		finalAnswersArr=(int*)malloc(sizeof(int)*5*pictNum); //there will be 5 places for each picture :pictID, objID, row, col, found
		if(!finalAnswersArr)
		{
			printf("Allocation failed.\n");
			MPI_Abort(MPI_COMM_WORLD,1);
		}
		
		//counts the amount of info that will be sent(pictures and objects) to proccess one for buffer size
		for(i=pictNum/2;i<pictNum;i++)
			bufferSize+=(pictures[i].size*pictures[i].size+2)*4; //size*size(square matrix) + 2(id+size)  and *4 (integer is 4 bytes)
		for(i=0;i<objNum;i++)
			bufferSize+=(objects[i].size*objects[i].size+2)*4; //size*size(square matrix) + 2(id+size)  and *4 (integer is 4 bytes)

		buffer=(char*)malloc(sizeof(char)*bufferSize); //allocate the buffer 
		if(!buffer)
		{
			printf("Allocation failed.\n");
			MPI_Abort(MPI_COMM_WORLD,1);
		}
		//send buffer size,amount of pictures,amount of objects and match
		MPI_Send(&bufferSize,1,MPI_INT,1,0,MPI_COMM_WORLD);
		MPI_Send(&pictNum,1,MPI_INT,1,0,MPI_COMM_WORLD);
		MPI_Send(&objNum,1,MPI_INT,1,0,MPI_COMM_WORLD);
		MPI_Send(&match,1,MPI_DOUBLE,1,0,MPI_COMM_WORLD);

		//packing half the pictures and all the objects and sending it to proccess one
		position=0;
		for(i=pictNum-pictNum/2;i<pictNum;i++)
			packPicture(&pictures[i],&buffer,&position,bufferSize);
		for(i=0;i<objNum;i++)
			packPicture(&objects[i],&buffer,&position,bufferSize);
		MPI_Send(buffer,position,MPI_PACKED,1,0,MPI_COMM_WORLD);
		pictNum-=pictNum/2;
	}
	//proccess one recieving all that was sent, allocating what needed and unpacking the MPI_Package
	else
	{
		MPI_Recv(&bufferSize,1,MPI_INT,0,0,MPI_COMM_WORLD,&status);
		MPI_Recv(&pictNum,1,MPI_INT,0,0,MPI_COMM_WORLD,&status);
		pictNum/=2; 	//the total number of pictures was sent but proccess one has only one
		MPI_Recv(&objNum,1,MPI_INT,0,0,MPI_COMM_WORLD,&status);
		MPI_Recv(&match,1,MPI_DOUBLE,0,0,MPI_COMM_WORLD,&status);
		buffer=(char*)malloc(sizeof(char)*bufferSize);
		if(!buffer)
		{
			printf("Allocation failed.\n");
			MPI_Abort(MPI_COMM_WORLD,1);
		}
		//recieving the buffer with all the info
		MPI_Recv(buffer,bufferSize,MPI_PACKED,0,0,MPI_COMM_WORLD,&status);
		position=0;
		
		//allocating the pictures array
		pictures=(Paint*)malloc(sizeof(Paint)*(pictNum));
		if(!pictures)
		{
			printf("Allocation failed.\n");
			MPI_Abort(MPI_COMM_WORLD,1);
		}
		//unpacking all the pictures
		for(i=0;i<pictNum;i++)
		{
			MPI_Unpack(buffer,bufferSize,&position,&pictures[i].id,1,MPI_INT,MPI_COMM_WORLD);
			MPI_Unpack(buffer,bufferSize,&position,&pictures[i].size,1,MPI_INT,MPI_COMM_WORLD);
			pictures[i].paint=(int*)malloc(sizeof(int)*pictures[i].size*pictures[i].size);
			if(!pictures[i].paint)
			{
				printf("Allocation failed.\n");
				MPI_Abort(MPI_COMM_WORLD,1);
			}
			MPI_Unpack(buffer,bufferSize,&position,pictures[i].paint,pictures[i].size*pictures[i].size,MPI_INT,MPI_COMM_WORLD);
		}
		
		//doing the same to objects
		objects=(Paint*)malloc(sizeof(Paint)*(objNum));
		if(!pictures)
			{
				printf("Allocation failed.\n");
				MPI_Abort(MPI_COMM_WORLD,1);
			}
		for(i=0;i<objNum;i++)
		{
			MPI_Unpack(buffer,bufferSize,&position,&objects[i].id,1,MPI_INT,MPI_COMM_WORLD);
			MPI_Unpack(buffer,bufferSize,&position,&objects[i].size,1,MPI_INT,MPI_COMM_WORLD);
			objects[i].paint=(int*)malloc(sizeof(int)*objects[i].size*objects[i].size);
			if(!objects[i].paint)
			{
				printf("Allocation failed.\n");
				MPI_Abort(MPI_COMM_WORLD,1);
			}
			MPI_Unpack(buffer,bufferSize,&position,objects[i].paint,objects[i].size*objects[i].size,MPI_INT,MPI_COMM_WORLD);
		}
		
		//allocating the array for the answers :row,col,etc...
		finalAnswersArr=(int*)malloc(sizeof(int)*5*pictNum); //*5 because for each picture we have 5 spots : pictID,objID,row,col,found
		if(!finalAnswersArr)
		{
			printf("Allocation failed.\n");
			MPI_Abort(MPI_COMM_WORLD,1);
		}
	}
	


//==========================================================================================

	//devide each proccess into a given amount of  cores(I choose four)
	//each core takes ONE picture and ALL the objects
	//if a sertain picture found a certain object then it coppies pictures ID and goes to next available picture
	#pragma omp parallel for num_threads(THREAD_NUM) private(j)
	for(i=0;i<pictNum;i++)
	{
		for(j=0;j<objNum;j++)
		{
			checkObjInPict(pictures[i].paint,objects[j].paint,pictures[i].size,objects[j].size,match,(finalAnswersArr+i*5));
			//updates objects ID
			finalAnswersArr[i*5]=pictures[i].id;
			if(finalAnswersArr[i*5+4]==1)
			{
				finalAnswersArr[i*5+1]=objects[j].id;
				printf("found pict num %d and object num %d\n",pictures[i].id,objects[j].id); //this line is for visualization of what is happening
				break;
			}
                   printf("didnt found pict num %d with object num %d\n",pictures[i].id,objects[j].id);//this line is for visualization of what is happening
		}
	}
//========================================================================================

	//proccess one sends the answer array to proccess zero
	//proccess zero recieves the answer array , place it at the end and prints in in the "output" text
	if(myRank==0)
	{
		//5 is because each picture have 5 spots , and was given only half array so pictNum
		MPI_Recv(finalAnswersArr+5*pictNum,5*pictNum,MPI_INT,1,0,MPI_COMM_WORLD,&status);
		
		//prints all the answers
		for(int i=0;i<totalNum;i++)
			writeToFile(FILE_NAME_OUT,finalAnswersArr[i*5],finalAnswersArr[i*5+1],finalAnswersArr[i*5+2],finalAnswersArr[i*5+3],finalAnswersArr[i*5+4]);

	}
	else
	{
		MPI_Send(finalAnswersArr,5*pictNum,MPI_INT,0,0,MPI_COMM_WORLD);
	}

//==========================================================================================

	MPI_Finalize();

	return 0;
}





