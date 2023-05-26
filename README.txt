#Round Robin Dispatcher
This program gives a list of randomly generated jobs (joblist.txt) that need to be scheduled for execution based on their arrival time and allowed execution time. Each job has a specific time quantum, and once that time is reached, the job is preempted and moved back to the queue if it hasn't finished yet and there are other waiting jobs.

Initially, all the jobs are loaded into the Job Dispatch queue. When a job "arrives," it is moved from the Job Dispatch queue to the Round Robin queue. Whenever there is no currently running process, the job at the front of the Round Robin queue is either launched as a new process (if it hasn't started running) or resumed (if it was suspended).

The Round Robin queue operates with a specified time quantum, which is determined by the user. When a process is launched or resumed, it continues running until one of two conditions is met: 
- either the remaining execution time of the process reaches zero
- or the process has run for at least the time quantum without interruption and there are waiting jobs in the Round Robin queue.

In the first case, the process is terminated, and in the second case, the process is preempted (suspended) and moved to the end of the Round Robin queue. The dispatcher immediately schedules another job from the Round Robin queue for execution if there are other jobs that have arrived or are suspended, once the currently running process is terminated or suspended.

## joblist.txt file
Each line of this file contains a job, giving the <arrival time> and <cpu time>



##To run the program:

- "make process"
- "make"
- "./RRD joblist.txt"
