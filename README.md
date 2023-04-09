# durandalc

## design
In main thread there is a durandalc class run in stack and durandalc own a vector to store the file need to update

also durandal has a thread pool, durandal use one thread to get the file need to upload from the vector(this vector 

is in main thread's stack and locked by a mutex) and upload it to remote domain (by a thread local http server).

Also, a thread is used to get the file need to upload and then update the filename vector ( referred previous ).