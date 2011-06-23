#include <stdio.h>
#include <sys/time.h>
#include <dispatch/dispatch.h>
//#define GROUP

int main(int argc, const char * argv[])
{
	struct timeval start, end;
	long mtime, seconds, useconds;
    //dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
//    #ifdef GROUP
    //dispatch_group_t group = dispatch_group_create();
//    #endif
    
    FILE *devNull = fopen("/dev/null", "a");

    const int NumConcurrentBlocks = 20;
    const int NumLoopsPerBlock = 100000;

	int i;
    for (i = 0; i < NumConcurrentBlocks; i++)
    {
 //    	#ifdef GRUOP
 /*
        dispatch_group_async(group, queue, ^{
 	       	gettimeofday(&start, NULL);
            printf("Started block %d at %ld.%ld second\n", i, start.tv_sec, start.tv_usec);
            int j;
            for (j = 0; j < NumLoopsPerBlock; j++)
            {
                fprintf(devNull, "Write to /dev/null\n");
            }
            gettimeofday(&end, NULL);
            printf("Finished block %dat %ld.%ld second\n", i, start.tv_sec, start.tv_usec);
        });
 //       #else
 */
 
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),  ^{
 	       	gettimeofday(&start, NULL);
            printf("Started block %d at %ld.%ld second\n", i, start.tv_sec, start.tv_usec);
            int j;
            for (j = 0; j < NumLoopsPerBlock; j++)
            {
                fprintf(devNull, "Write to /dev/null\n");
            }
            gettimeofday(&end, NULL);
            printf("Finished block %dat %ld.%ld second\n", i, start.tv_sec, start.tv_usec);
        });
        
 //       #endif
    }

//#ifdef GROUP
    //dispatch_group_wait(group, DISPATCH_TIME_FOREVER);
    //dispatch_release(group);
//#endif
    fclose(devNull);
    
    return 0;
}