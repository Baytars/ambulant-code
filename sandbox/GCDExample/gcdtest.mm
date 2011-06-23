// Build with:
//	g++ gcdtest.mm -o gcdtest
//

#include <stdio.h>
#include <dispatch/dispatch.h>

int main(int argc, const char * argv[])
{
    dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
    dispatch_group_t group = dispatch_group_create();
    
    FILE *devNull = fopen("/dev/null", "a");

    const int NumConcurrentBlocks = 20;
    const int NumLoopsPerBlock = 100000;

    for (int i = 0; i < NumConcurrentBlocks; i++)
    {
        dispatch_group_async(group, queue, ^{
            fprintf(stderr,"Started block %d\n", i);
            for (int j = 0; j < NumLoopsPerBlock; j++)
            {
                fprintf(devNull, "Write to /dev/null\n");
            }
            fprintf(stderr,"Finished block %d\n", i);
        });
    }

    dispatch_group_wait(group, DISPATCH_TIME_FOREVER);
    dispatch_release(group);
    fclose(devNull);
    
    return 0;
}