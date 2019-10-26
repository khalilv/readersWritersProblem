# readersWritersProblem
Solved the readers writers problem using semaphores in C. 

Multiple readers can read the shared resource (a global integer) at a time while a writer must have exclusive access to the shared resource to change it (will increment it by 10). 

Ran by creating 500 reader threads and 10 writer threads. Each reader will try to read the resource argv[2] times while each writer will try to update the resource argv[1] times. 

Recorded the max, average and minimum waiting times of the writer threads and reader threads. Saw that a writers starvation problem had arisen with my orignal implementation. 

Fixed this issue by implementing a third semaphore to enforce first come first serve access for all threads. 

Compile with -lpthread and run with 2 integer arguments. 

