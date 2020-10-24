// Include files
#include "RRD.h"

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

  // 1. and 2.
  // Initialise and populate Job Dispatch Queue
  while(!feof(input_list_stream)) {
    process = createnullPcb();
    if(fscanf(input_list_stream, "%d, %d",
        &(process->arrival_time),
        &(process->service_time)) != 2) {
          free(process);
          continue;
    }
    // Remaining cpu time is set to the processes service time (from file)
    process->remaining_cpu_time = process->service_time;
    process->status = PCB_INITIALIZED;
    job_dispatch_queue = enqPcb(job_dispatch_queue, process);
  }


  // Initialise Round Robin Queue
  process_init_rr = createnullPcb();


  // 3. Ask user for time quantum
  printf("Please enter a time quantum as an integer: ");
  if(scanf("%d", &time_quantum) != 1) {
    fprintf(stderr, "ERROR: Did not give integer for time quantum\n");
    exit(EXIT_FAILURE);
  }
  if(time_quantum < 1) {
    fprintf(stderr, "ERROR: Need to give time quantum of at least 1\n");
    exit(EXIT_FAILURE);
  }


  // 4. While there is a currently running process or either queue is not empty
  while(current_process || (job_dispatch_queue || round_robin_queue)) {

    // (i) Unload any arrived pending processes from JD q to RR q
    while(job_dispatch_queue && job_dispatch_queue->arrival_time <= timer) {
      // Dequeue from Job Dispatch and Enqueue to Round Robin
      process_init_rr = deqPcb(&job_dispatch_queue);
      process_init_rr->remaining_cpu_time = process_init_rr->service_time;
      round_robin_queue = enqPcb(round_robin_queue, process_init_rr);
    }


    // (ii) if a process is currently running,
    if(current_process) {

      // (a) decrease remaining cpu time
      // increase tq by 1
      current_process->tq++;
      current_process->remaining_cpu_time--;


      // (b) if times up
      if(current_process->remaining_cpu_time <= 0) {

        // Terminate process
        terminatePcb(current_process);

        //Calculate turnaround time and waiting time
        turnaround_time = timer - current_process->arrival_time;
        sum_tt += turnaround_time;
        waiting_time = turnaround_time - current_process->service_time;
        sum_wt += waiting_time;
        num_processes++;

        // Free up process structure memory and set current process to null
        free(current_process);
        current_process = NULL;


      // (c) else if other processes are waiting in RR q
      } else if(round_robin_queue) {

        if(current_process->tq >= time_quantum) {

          // Suspend current process
          transfer_process = suspendPcb(current_process);
          // Enq it to back of RR q
          round_robin_queue = enqPcb(round_robin_queue, transfer_process);
          // Dequeue process from front and start it; reset tq to 0
          current_process = deqPcb(&round_robin_queue);
          startPcb(current_process);
          current_process->tq = 0;
        }

        // If no processes in RR q and time_quantum is reached
      } else if(!round_robin_queue && current_process->tq >= time_quantum) {
        // Reset tq to 0 to do another round
        current_process->tq = 0;
      }
    }

    // (iii) If no process is currently running and RR q is not empty
    if(!current_process && round_robin_queue) {

      // (a) Deq process from head of RR q
      transfer_process = deqPcb(&round_robin_queue);

      // (b) If process is a suspended process
      if(transfer_process->status == PCB_SUSPENDED) {
        //Send SIGCONT to resume it
        // startPcb resumes the process
        startPcb(transfer_process);
      } else {
        // (c) Else start it (fork and exec)
        // startPcb also does this
        startPcb(transfer_process);
      }
      // (d) Set process as currently running process
      // Reset tq to 0
      transfer_process->tq = 0;
      current_process = transfer_process;
    }

    // (iv) Sleep for quantum (may/may not be same as time_quantum)
    // My implementation sleeps for 1 and uses the value 'tq' in PcbPtr
    sleep(1);

    // (v) Increase timer by quantum
    // My implementation increases timer by 1 each time; uses value 'tq' in PcbPtr
    timer++;

    // (vi) Go back to 4
  }

  // Print average turnaround time and average waiting time
  float avg_wt = (float)sum_wt / num_processes;
  float avg_tt = (float)sum_tt / num_processes;
  printf("num processes: %d\n", num_processes);
  printf("Average turnaround time: %.1f\n", avg_tt);
  printf("Average waiting time: %.1f\n", avg_wt);

  // 5. Terminate RR dispatcher
  exit(EXIT_SUCCESS);
}
