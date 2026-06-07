#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_PROCESS 20
#define MAX_TIMEQUANTUM 10 
#define MAX_GANTT 1000


typedef struct {
	int pid;
	int arrival_time;
	int cpu_burst_time;
	int remaining_time;
	int priority;
	int completion_time;
	
	int io_start_time;
	int io_burst_time;
	int io_remaining_time;
} Process;

typedef struct Node {
	int index;
	struct Node *next;
} Node;

typedef struct {
	Node *front;
	Node *rear;
} Queue;

typedef struct {
	double avg_waiting_time;
	double avg_turnaround_time;
	
	int gantt[MAX_GANTT];
	int gantt_size;
} Result;


Process original[MAX_PROCESS];
Process process_list[MAX_PROCESS];
int process_count;

Queue ready_queue;
Queue waiting_queue;

int time_quantum = 3;


void config();

void initQueue(Queue *q);
int isEmpty(Queue *q);
void enqueue(Queue *q, int index);
int dequeue(Queue *q);
void clearQueue(Queue *q);
void updateWaitingQueue();

void createProcess();
void regenerateProcess();
void copyProcess();
void printProcess();

void scheduler();
void showMenu();

Result FCFS();
Result SJF();
Result PreemptiveSJF();
Result Priority();
Result PreemptivePriority();
Result RoundRobin();
void setTimeQuantum();

void calculateAverageTime(Result *result);

void printGanttChart(Result result);
void printAverageTime(Result result);
void printResult(char *name, Result result);
void printCenter(char *name);

void evaluation();


int main() {
	srand(time(NULL));
	
	printf("\n");
	printf("===============================================================\n");
	printCenter("CPU Scheduler Simulator");
	printf("===============================================================\n");
	printf(" Supported Algorithms\n");
	printf("  - FCFS\n");
	printf("  - SJF\n");
	printf("  - Preemptive SJF\n");
	printf("  - Priority\n");
	printf("  - Preemptive Priority\n");
	printf("  - Round Robin\n");
	printf("===============================================================\n");
	printf("\n");
	
	config();
	
	scheduler();
	
	return 0;
}


void config() {
	initQueue(&ready_queue);
	initQueue(&waiting_queue);
	
	while(1) {
		printf("Number of Processes (1 ~ %d) : ", MAX_PROCESS);
		scanf("%d", &process_count);
		
		if(process_count >= 1 && process_count <= MAX_PROCESS) break;
		
		printf("Invalid Input\n");
	}
	
	createProcess();
	
	printProcess();
}

void initQueue(Queue *q) {
	q->front = NULL;
	q->rear = NULL;
}

int isEmpty(Queue *q) {
	return q->front == NULL;
}

void enqueue(Queue *q, int index) {
	Node *new_node = (Node *)malloc(sizeof(Node));
	
	new_node->index = index;
	new_node->next = NULL;
	
	if(isEmpty(q)) {
		q->front = new_node;
		q->rear = new_node;
	}
	else {
		q->rear->next = new_node;
		q->rear = new_node;
	}
}

int dequeue(Queue *q) {
	if(isEmpty(q)) return -1;
	
	Node *temp = q->front;
	int index = temp->index;
	q->front = q->front->next;
	
	if(q->front == NULL) q->rear = NULL;
	
	free(temp);
	
	return index;
}

void clearQueue(Queue *q) {
	while(!isEmpty(q)) dequeue(q);
}

void updateWaitingQueue() {
	Node *prev = NULL;
	Node *cur = waiting_queue.front;
	
	while(cur != NULL) {
		int index = cur->index;
		
		process_list[index].io_remaining_time--;
		
		if(process_list[index].io_remaining_time <= 0) {
			enqueue(&ready_queue, index);
			
			if(prev == NULL) waiting_queue.front = cur->next;
			else prev->next = cur->next;
			
			if(cur == waiting_queue.rear) waiting_queue.rear = prev;
			
			Node *temp = cur;
			cur = cur->next;
			
			free(temp);
		}
		else {
			prev = cur;
			cur = cur->next;
		}
	}
}

void createProcess() {
	int i;
	
	memset(original, 0, sizeof(original));
	
	for(i = 0; i < process_count; ++i) {
		original[i].pid = i + 1;
		original[i].arrival_time = rand() % 10;
		original[i].cpu_burst_time = rand() % 10 + 1;
		original[i].remaining_time = original[i].cpu_burst_time;
		original[i].priority = rand() % 5 + 1;
		original[i].completion_time = 0;
		
		if(rand() % 2 == 0 && original[i].cpu_burst_time > 1) {
			original[i].io_start_time = rand() % (original[i].cpu_burst_time - 1) + 1;
			original[i].io_burst_time = rand() % 5 + 1;
			original[i].io_remaining_time = original[i].io_burst_time;
		}
		else {
			original[i].io_start_time = -1;
			original[i].io_burst_time = 0;
			original[i].io_remaining_time = 0;
		}
	}
}

void regenerateProcess() {
	int new_count;
	
	while(1) {
		printf("New process Count (1 ~ %d) : ", MAX_PROCESS);
		scanf("%d", &new_count);
		
		if(new_count >= 1 && new_count <= MAX_PROCESS) {
			process_count = new_count;
			break;
		}
		
		printf("Invalid Input\n");
	}
	clearQueue(&ready_queue);
	clearQueue(&waiting_queue);
	
	createProcess();
	
	printProcess();
}

void copyProcess() {
	memset(process_list, 0, sizeof(process_list));
	
	for(int i = 0; i <process_count; ++i) process_list[i] = original[i];
}

void printProcess() {
	printf("\n");
	printf("===============================================================\n");
	printCenter("Process List");
	printf("===============================================================\n");
	printf("%-10s %-10s %-10s %-10s %-10s %-8s\n", "Process", "Arrival", "CPU Burst", "IO Start", "IO Burst", "Priority");
	printf("===============================================================\n");
	
	for(int i = 0; i < process_count; ++i) {
		printf("P%-9d %-10d %-10d ", original[i].pid, original[i].arrival_time, original[i].cpu_burst_time);
		
		if(original[i].io_start_time == -1) printf("%-10s %-10s ", "-", "-");
		else printf("%-10d %-10d ", original[i].io_start_time, original[i].io_burst_time);
		
		printf("%-8d\n", original[i].priority);
	}
	printf("===============================================================\n");
}

void scheduler() {
	int choice;
	
	while(1) {
		showMenu();
		
		scanf("%d", &choice);
		
		Result result;
		
		switch(choice) {
			case 1:
				result = FCFS();
				printResult("FCFS Scheduling Result", result);
				break;
				
			case 2:
				result = SJF();
				printResult("SJF Scheduling Result", result);
				break;
				
			case 3:
				result = PreemptiveSJF();
				printResult("Preemptive SJF Scheduling Result", result);
				break;
				
			case 4:
				result = Priority();
				printResult("Priority Scheduling Result", result);
				break;
				
			case 5:
				result = PreemptivePriority();
				printResult("Preemptive Priority Scheduling Result", result);
				break;
				
			case 6:
				result = RoundRobin();
				printResult("Round Robin Scheduling Result", result);
				break;
			case 7:
				setTimeQuantum();
				break;
				
			case 8:
				regenerateProcess();
				break;
				
			case 9:
				printProcess();
				break;
				
			case 10:
				evaluation();
				break;
				
			case 0:
				return;
				
			default:
				printf("invalid Input\n");
		}
	}
}

void showMenu() {
	printf("\n");
	printf("===============================================================\n");
	printCenter("Main Menu");
	printf("===============================================================\n");
	
	printf(" 1.  FCFS\n");
	printf(" 2.  SJF\n");
	printf(" 3.  Preemptive SJF\n");
	printf(" 4.  Priority\n");
	printf(" 5.  Preemptive Priority\n");
	printf(" 6.  Round Robin\n");
	printf(" 7.  Set Time Quantum (Current = %d)\n", time_quantum);
	
	printf(" 8.  Regenerate Processes\n");
	printf(" 9.  Show Process List\n");
	
	printf(" 10. Evaluation\n");
	
	printf(" 0.  Exit\n");
	
	printf("===============================================================\n");
	printf("\n");
	printf("Select : ");
}

Result FCFS(){
	Result result = {0};
	int completed = 0;
	int current_time = 0;
	int current = -1;
	
	copyProcess();
	
	clearQueue(&ready_queue);
	clearQueue(&waiting_queue);

	while(completed < process_count) {		
		for(int i = 0; i < process_count; ++i) {
			if(process_list[i].arrival_time == current_time) enqueue(&ready_queue, i);
		}
		
		if(current == -1 && !isEmpty(&ready_queue)) current = dequeue(&ready_queue);
		
		updateWaitingQueue();
		
		current_time++;
		
		if(current != -1) {
			process_list[current].remaining_time--;
			
			result.gantt[result.gantt_size++] = process_list[current].pid;
		}
		else result.gantt[result.gantt_size++] = -1;
		
		if(current != -1 && process_list[current].remaining_time == 0) {
			process_list[current].completion_time = current_time;
			
			completed++;
			
			current = -1;
		}
		
		if(current != -1) {
			int executed_time = process_list[current].cpu_burst_time - process_list[current].remaining_time;
			
			if(process_list[current].io_start_time != -1 && executed_time == process_list[current].io_start_time) {
				enqueue(&waiting_queue, current);
				process_list[current].io_start_time = -1;
				current = -1;
			}
		}
	}
	calculateAverageTime(&result);

	return result;
}

Result SJF(){
	Result result = {0};
	int completed = 0;
	int current_time = 0;
	int current = -1;
	
	copyProcess();
	
	clearQueue(&ready_queue);
	clearQueue(&waiting_queue);

	while(completed < process_count) {		
		for(int i = 0; i < process_count; ++i) {
			if(process_list[i].arrival_time == current_time) enqueue(&ready_queue, i);
		}
		
		if(current == -1 && !isEmpty(&ready_queue)) {
			Node *prev = NULL;
			Node *cur = ready_queue.front;
			
			Node *min_prev = NULL;
			Node *min_node = cur;
			
			int min_remaining = process_list[cur->index].remaining_time;
			
			while(cur != NULL) {
				if(process_list[cur->index].remaining_time < min_remaining) {
					min_remaining = process_list[cur->index].remaining_time;
					min_node = cur;
					min_prev = prev;
				}
				prev = cur;
				cur = cur->next;
			}
			current = min_node->index;
			
			if(min_prev == NULL) ready_queue.front = min_node->next;
			else min_prev->next = min_node->next;
			
			if(min_node == ready_queue.rear) ready_queue.rear = min_prev;
			
			free(min_node);
		}
		
		updateWaitingQueue();
		
		current_time++;
		
		if(current != -1) {
			process_list[current].remaining_time--;
			
			result.gantt[result.gantt_size++] = process_list[current].pid;
		}
		else result.gantt[result.gantt_size++] = -1;
		
		if(current != -1 && process_list[current].remaining_time == 0) {
			process_list[current].completion_time = current_time;
			
			completed++;
			
			current = -1;
		}
		
		if(current != -1) {
			int executed_time = process_list[current].cpu_burst_time - process_list[current].remaining_time;
			
			if(process_list[current].io_start_time != -1 && executed_time == process_list[current].io_start_time) {
				enqueue(&waiting_queue, current);
				process_list[current].io_start_time = -1;
				current = -1;
			}
		}
	}
	calculateAverageTime(&result);

	return result;
}

Result PreemptiveSJF(){
	Result result = {0};
	int completed = 0;
	int current_time = 0;
	int current = -1;
	
	copyProcess();
	
	clearQueue(&ready_queue);
	clearQueue(&waiting_queue);

	while(completed < process_count) {		
		for(int i = 0; i < process_count; ++i) {
			if(process_list[i].arrival_time == current_time) enqueue(&ready_queue, i);
		}
		
		if(current != -1) {
			enqueue(&ready_queue, current);
			current = -1;
		}
		
		if(!isEmpty(&ready_queue)) {
			Node *prev = NULL;
			Node *cur = ready_queue.front;
			
			Node *min_prev = NULL;
			Node *min_node = cur;
			
			int min_remaining = process_list[cur->index].remaining_time;
			
			while(cur != NULL) {
				if(process_list[cur->index].remaining_time <= min_remaining) {
					min_remaining = process_list[cur->index].remaining_time;
					min_node = cur;
					min_prev = prev;
				}
				prev = cur;
				cur = cur->next;
			}
			current = min_node->index;
			
			if(min_prev == NULL) ready_queue.front = min_node->next;
			else min_prev->next = min_node->next;
			
			if(min_node == ready_queue.rear) ready_queue.rear = min_prev;
			
			free(min_node);
		}
		
		updateWaitingQueue();
		
		current_time++;
		
		if(current != -1) {
			process_list[current].remaining_time--;
			
			result.gantt[result.gantt_size++] = process_list[current].pid;
		}
		else result.gantt[result.gantt_size++] = -1;
		
		if(current != -1 && process_list[current].remaining_time == 0) {
			process_list[current].completion_time = current_time;
			
			completed++;
			
			current = -1;
		}
		
		if(current != -1) {
			int executed_time = process_list[current].cpu_burst_time - process_list[current].remaining_time;
			
			if(process_list[current].io_start_time != -1 && executed_time == process_list[current].io_start_time) {
				enqueue(&waiting_queue, current);
				process_list[current].io_start_time = -1;
				current = -1;
			}
		}
	}
	calculateAverageTime(&result);

	return result;
}

Result Priority(){
	Result result = {0};
	int completed = 0;
	int current_time = 0;
	int current = -1;
	
	copyProcess();
	
	clearQueue(&ready_queue);
	clearQueue(&waiting_queue);

	while(completed < process_count) {		
		for(int i = 0; i < process_count; ++i) {
			if(process_list[i].arrival_time == current_time) enqueue(&ready_queue, i);
		}
		
		if(current == -1 && !isEmpty(&ready_queue)) {
			Node *prev = NULL;
			Node *cur = ready_queue.front;
			
			Node *min_prev = NULL;
			Node *min_node = cur;
			
			int min_priority = process_list[cur->index].priority;
			
			while(cur != NULL) {
				if(process_list[cur->index].priority < min_priority) {
					min_priority = process_list[cur->index].priority;
					min_node = cur;
					min_prev = prev;
				}
				prev = cur;
				cur = cur->next;
			}
			current = min_node->index;
			
			if(min_prev == NULL) ready_queue.front = min_node->next;
			else min_prev->next = min_node->next;
			
			if(min_node == ready_queue.rear) ready_queue.rear = min_prev;
			
			free(min_node);
		}
		
		updateWaitingQueue();
		
		current_time++;
		
		if(current != -1) {
			process_list[current].remaining_time--;
			
			result.gantt[result.gantt_size++] = process_list[current].pid;
		}
		else result.gantt[result.gantt_size++] = -1;
		
		if(current != -1 && process_list[current].remaining_time == 0) {
			process_list[current].completion_time = current_time;
			
			completed++;
			
			current = -1;
		}
		
		if(current != -1) {
			int executed_time = process_list[current].cpu_burst_time - process_list[current].remaining_time;
			
			if(process_list[current].io_start_time != -1 && executed_time == process_list[current].io_start_time) {
				enqueue(&waiting_queue, current);
				process_list[current].io_start_time = -1;
				current = -1;
			}
		}
	}
	calculateAverageTime(&result);

	return result;
}

Result PreemptivePriority(){
	Result result = {0};
	int completed = 0;
	int current_time = 0;
	int current = -1;
	
	copyProcess();
	
	clearQueue(&ready_queue);
	clearQueue(&waiting_queue);

	while(completed < process_count) {		
		for(int i = 0; i < process_count; ++i) {
			if(process_list[i].arrival_time == current_time) enqueue(&ready_queue, i);
		}
		
		if(current != -1) {
			enqueue(&ready_queue, current);
			current = -1;
		}
		
		if(!isEmpty(&ready_queue)) {
			Node *prev = NULL;
			Node *cur = ready_queue.front;
			
			Node *min_prev = NULL;
			Node *min_node = cur;
			
			int min_priority = process_list[cur->index].priority;
			
			while(cur != NULL) {
				if(process_list[cur->index].priority <= min_priority) {
					min_priority = process_list[cur->index].priority;
					min_node = cur;
					min_prev = prev;
				}
				prev = cur;
				cur = cur->next;
			}
			current = min_node->index;
			
			if(min_prev == NULL) ready_queue.front = min_node->next;
			else min_prev->next = min_node->next;
			
			if(min_node == ready_queue.rear) ready_queue.rear = min_prev;
			
			free(min_node);
		}
		
		updateWaitingQueue();
		
		current_time++;
		
		if(current != -1) {
			process_list[current].remaining_time--;
			
			result.gantt[result.gantt_size++] = process_list[current].pid;
		}
		else result.gantt[result.gantt_size++] = -1;
		
		if(current != -1 && process_list[current].remaining_time == 0) {
			process_list[current].completion_time = current_time;
			
			completed++;
			
			current = -1;
		}
		
		if(current != -1) {
			int executed_time = process_list[current].cpu_burst_time - process_list[current].remaining_time;
			
			if(process_list[current].io_start_time != -1 && executed_time == process_list[current].io_start_time) {
				enqueue(&waiting_queue, current);
				process_list[current].io_start_time = -1;
				current = -1;
			}
		}
	}
	calculateAverageTime(&result);

	return result;
}

Result RoundRobin(){
	Result result = {0};
	int completed = 0;
	int current_time = 0;
	int current = -1;
	int time_slice_count = 0;
	
	copyProcess();
	
	clearQueue(&ready_queue);
	clearQueue(&waiting_queue);

	while(completed < process_count) {		
		for(int i = 0; i < process_count; ++i) {
			if(process_list[i].arrival_time == current_time) enqueue(&ready_queue, i);
		}
		
		if(current != -1 && time_slice_count >= time_quantum) {
			enqueue(&ready_queue, current);
			current = -1;
			time_slice_count = 0;
		}
		
		if(current == -1 && !isEmpty(&ready_queue)) {
			current = dequeue(&ready_queue);
			time_slice_count = 0;
		}
		
		updateWaitingQueue();
		
		current_time++;
		
		if(current != -1) {
			process_list[current].remaining_time--;
			
			time_slice_count++;
			
			result.gantt[result.gantt_size++] = process_list[current].pid;
		}
		else result.gantt[result.gantt_size++] = -1;
		
		if(current != -1 && process_list[current].remaining_time == 0) {
			process_list[current].completion_time = current_time;
			
			completed++;
			
			current = -1;
			time_slice_count = 0;
		}
		
		if(current != -1) {
			int executed_time = process_list[current].cpu_burst_time - process_list[current].remaining_time;
			
			if(process_list[current].io_start_time != -1 && executed_time == process_list[current].io_start_time) {
				enqueue(&waiting_queue, current);
				process_list[current].io_start_time = -1;
				current = -1;
				time_slice_count = 0;
			}
		}
	}
	calculateAverageTime(&result);

	return result;
}

void setTimeQuantum() {
	int new_timequantum;
	
	printf("\n");
	printf("===============================================================\n");
	printCenter("Set Time Quantum");
	printf("===============================================================\n");
	printf("Current Time Quantum : %d\n", time_quantum);
	
	while(1) {
		printf("\n");
		printf("New process Count (1 ~ %d) : ", MAX_TIMEQUANTUM);
		scanf("%d", &new_timequantum);
		
		if(new_timequantum >= 1 && new_timequantum <= MAX_TIMEQUANTUM) {
			time_quantum = new_timequantum;
			break;
		}
		
		printf("Invalid Input\n");
	}
}

void calculateAverageTime(Result *result) {
	double total_wt = 0;
	double total_tat = 0;
	
	for(int i = 0; i < process_count; ++i) {
		int tat = process_list[i].completion_time - process_list[i].arrival_time;
		int wt = tat - process_list[i].cpu_burst_time - process_list[i].io_burst_time;
		
		total_tat += tat;
		total_wt += wt;
	}
	result->avg_waiting_time = total_wt / process_count;
	result->avg_turnaround_time = total_tat / process_count;
}

void printGanttChart(Result result) {
	int i = 0;
	
	printf("\n|");
	
	while(i < result.gantt_size) {
		int cur = result.gantt[i];
		int j = i;
		
		while(j < result.gantt_size && result.gantt[j] == cur) j++;
		
		if(cur == -1) printf(" IDLE |");
		else printf("  P%-2d |", cur);
		
		i = j;
	}
	
	printf("\n0");
	
	i = 0;
	
	while(i < result.gantt_size) {
		int cur = result.gantt[i];
		int j = i;
		
		while(j < result.gantt_size && result.gantt[j] == cur) j++;
		
		printf("%7d", j);
		
		i = j;
	}
	
	printf("\n");
}

void printAverageTime(Result result) {
	printf("Average Waiting Time    : %.2f\n", result.avg_waiting_time);
	printf("Average Turnaround Time : %.2f\n", result.avg_turnaround_time);
}

void printResult(char *name, Result result) {
	printf("\n");
	printf("===============================================================\n");
	printCenter(name);
	printf("===============================================================\n");

	printGanttChart(result);
	
	printf("\n");
	
	printAverageTime(result);
	
	printf("===============================================================\n");
}

void printCenter(char *name) {
	int left = (62 - strlen(name)) / 2;
	int right = (63-strlen(name)) / 2;

	for(int i = 0; i < left; ++i) printf(" ");
	printf("%s", name);
	for(int i = 0; i < right; ++i) printf(" ");
	printf("\n");
}

void evaluation() {
	printf("\n");
	printf("===============================================================\n");
	printCenter("Scheduling Algorithms Evaluation");
	printf("===============================================================\n");
	
	Result fcfs = FCFS();
	Result sjf 	= SJF();
	Result psjf = PreemptiveSJF();
	Result prio = Priority();
	Result pp 	= PreemptivePriority();
	Result rr 	= RoundRobin();
	
	printf("%-25s%-19s%-19s\n", "Algorithm", "Avg Waiting Time", "Avg Turnaround Time");
	printf("===============================================================\n");
	
	printf("%-25s%-19.2f%-19.2f\n", "FCFS", fcfs.avg_waiting_time, fcfs.avg_turnaround_time);
	printf("%-25s%-19.2f%-19.2f\n", "SJF", sjf.avg_waiting_time, sjf.avg_turnaround_time);
	printf("%-25s%-19.2f%-19.2f\n", "Preemptive SJF", psjf.avg_waiting_time, psjf.avg_turnaround_time);
	printf("%-25s%-19.2f%-19.2f\n", "Priority", prio.avg_waiting_time, prio.avg_turnaround_time);
	printf("%-25s%-19.2f%-19.2f\n", "Preemptive Priority", pp.avg_waiting_time, pp.avg_turnaround_time);
	printf("%-25s%-19.2f%-19.2f\n", "Round Robin", rr.avg_waiting_time, rr.avg_turnaround_time);
	
	printf("===============================================================\n");
}	//10