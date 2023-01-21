# Parallel-computation-MPI-OpenMP


The project reads the file from a given input (input.txt included) .
Then sends the second half of the pictures (if the number is off then sends the smaller half ) to the second
computer with the help of MPI .
The usage of the MPI was choosen because if i can share the work between computers and cut the running time almost by two 
then why not ?
Each computer can use four cores for parallel usage witch i use with the help of openMP.
Each core takes a picture and all the objects and simultanusely looks for an object in a picture.
openMP was chosen because we need to take each picture and check on it all the objects till we find a match
this is ideal for the computer cores , when each one takes a job not having impact on the other proccesses.
NO CUDA WAS USED (I tried it for a bit but got lost with the memmory allocations , wanted to do shared memmory).

There is an array called finalAnswerArr witch will have 5 places for each picture:
picture id , object if , row , col, found.
We will need this information int the end to print info of each pictures ( if it was found and where).

The program goes through all the structs and counts all the numbers that each struct have for an
allocation for the buffer (needed to transfer half the pictures to the other computer) and then multiply it by 4
because each int is 4 bytes and the buffer is a char type witch is inly 1 byte.
