//Need to use a Job Dispatch Q and a RR Q
//Job Dispatch Q is where all the jobs are first loaded
// A job is sent do the RR Q when it has "arrived"
// job at from of RR Q is launched as NEW or RESUMED (if suspended before)

// Include files
#include "RRD.h"

//NOTE: git push origin master

int main(int argc, char **argv) {

  // Declarations
  FILE * input_list_stream = NULL;
  PcbPtr job_dispatch_queue = NULL;
  PcbPtr round_robin_queue = NULL;
  PcbPtr current_process = NULL;
  PcbPtr process = NULL;
  PcbPtr process_init_rr = NULL;
  PcbPtr transfer_process = NULL;
  int timer = 0;

  int num_processes = 0;
  int turnaround_time;
  int waiting_time;
  int sum_tt = 0;
  int sum_wt = 0;
  int time_quantum;
  //int counter = 0;


  // Error checking (from Exercise 3)
  if(argc <= 0) {

    fprintf(stderr, "FATAL: Bad arguments array\n");
    exit(EXIT_FAILURE);

  } else if(argc != 2) {

    fprintf(stderr, "Usage: %s <TESTFILE>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  if(!(input_list_stream = fopen(argv[1], "r"))) {
    fprintf(stderr, "ERROR: Could not open \%s\"\n", argv[1]);
    exit(EXIT_FAILURE);
  }


  // Initialise and populate Job Dispatch Queue
  while(!feof(input_list_stream)) {
    process = createnullPcb();
    if(fscanf(input_list_stream, "%d, %d",
        &(process->arrival_time),
        &(process->service_time)) != 2) {
          free(process);
          continue;
    }
    //remaining cpu time is set to the processes service time (from file)
    process->remaining_cpu_time = process->service_time; //fix this line? necessary?
    process->status = PCB_INITIALIZED;
    job_dispatch_queue = enqPcb(job_dispatch_queue, process);
  }


  // Initialise Round Robin Queue
  process_init_rr = createnullPcb();


  // Ask user for time quantum
  printf("Please enter a time quantum as an integer: ");
  if(scanf("%d", &time_quantum) != 1) {
    fprintf(stderr, "ERROR: Did not give integer for time quantum\n");
    exit(EXIT_FAILURE);
  }
  if(time_quantum < 1) {
    fprintf(stderr, "ERROR: Need to give time quantum of at least 1\n");
    exit(EXIT_FAILURE);
  }


  //4. While there is a currently running process or either queue is not empty
  while(current_process || (job_dispatch_queue || round_robin_queue)) {
    //printf("4\n");
    // (i) Unload any arrived pending processes from job q to rr q
    if(job_dispatch_queue && job_dispatch_queue->arrival_time == timer) {
      // Dequeue from Job Dispatch and Enqueue to Round Robin
      //printf("dq from jdq at time %d\n", timer);
      process_init_rr = deqPcb(&job_dispatch_queue);
      process_init_rr->remaining_cpu_time = process_init_rr->service_time;
      round_robin_queue = enqPcb(round_robin_queue, process_init_rr);
    }


    // (ii) if a process is currently running,
    if(current_process) {
      //printf("ii\n");
      // (a) decrease remaining cpu time
      current_process->tq++;
      current_process->remaining_cpu_time--;


      // (b) if times up
      if(current_process->remaining_cpu_time <= 0) {
        //printf("b\n");
        //send SIGINT to process to terminate
        //free up process structure memory (?????) ***
        //printf("TERMINATING\n");
        terminatePcb(current_process);
        //Calculate turnaround time and waiting time
        turnaround_time = timer - current_process->arrival_time;
        sum_tt += turnaround_time;
        waiting_time = turnaround_time - current_process->service_time;
        sum_wt += waiting_time;
        num_processes++;

        //set current process to null
        current_process = NULL;
        //free(current_process);



      // (c) else if other processes are waiting in rr q
      //      think i need to handle time quantum here
      } else if(round_robin_queue) {
        //printf("c\n");
        if(current_process->tq >= time_quantum) {
          //printf("c..\n");
          //send SIGTSTP to suspend current process
          transfer_process = suspendPcb(current_process);
          //enq it to back of rr q
          round_robin_queue = enqPcb(round_robin_queue, transfer_process);
          current_process = deqPcb(&round_robin_queue);
          startPcb(current_process);
          current_process->tq = 0;
          //counter = 0;
        }



      }
    }

    // (iii) if no process is currently running and rr q is not empty
    if(!current_process && round_robin_queue) {
      //printf("a\n");
      // (a) deq process from head of rr q
      transfer_process = deqPcb(&round_robin_queue);
      // startPcb(current_process);

      // (b) if process is a suspended process
      if(transfer_process->status == PCB_SUSPENDED) {
        //send SIGCONT to resume it
        startPcb(transfer_process); //***
      } else {
        // (c) start it (fork and exec)
        startPcb(transfer_process); //****

      }
      // (d) set process as currently running process
      //printf("starting process!\n");
      //startPcb(current_process);
      current_process = transfer_process;
    }

    // (iv) sleep for quantum (may/may not be same as time_quantum)
    sleep(1);

    // (v) increase timer by quantum
    timer++;

    //increase counter
    //counter++;

    // (vi) go back to 4

  }

  // Print average turnaround time and average waiting time
  // Terminate RR dispatcher
  float avg_wt = (float)sum_wt / num_processes;
  float avg_tt = (float)sum_tt / num_processes;
  printf("num processes: %d\n", num_processes);
  printf("Average turnaround time: %.1f\n", avg_tt);
  printf("Average waiting time: %.1f\n", avg_wt);
  exit(EXIT_SUCCESS);

}
