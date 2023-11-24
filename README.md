# ARP PROJECT

##



## Implementation Advice
------------------------

### Include needed
'''
    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <sys/wait.h>
    #include <sys/types.h>
    #include <stdlib.h>
    #include <errno.h>
'''
With a FIFO I have also:
'''
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <fcntl.h> // open
'''


### Architecture side




### Ncurses side

* At the beginning ''' getmaxyx(window,row,col) '''  
This to obyain the value in which scale the motion and all the parameters that can change dimensions (Thaks to a initial proportion)
* 
