/**
 * @file interrupts.cpp
 * @author Sasisekhar Govind, Esli Emmanuel Konate 101322259
 * @brief template main.cpp file for Assignment 3 Part 1 of SYSC4001
 * 
 */

#include<interrupts_Esli_Emmanuel_Konate_#101322259.hpp>

const int QUANTUM = 100; // 100 ms quantum

// now we do the priority scheduler with preemption
void EP_RR_Scheduler(std::vector<PCB> &ready_queue){
    std::sort(
        ready_queue.begin(), ready_queue.end(), [](const PCB &first, const PCB &second){
            return (first.priority > second.priority);
        }
    );
}
std::tuple<std::string> run_simulation(std::vector<PCB> list_processes){
    std::vector<PCB> ready_queue;
    std::vector<WaitingProcess> wait_queue;
    std::vector<PCB> job_list;

    unsigned int current_time = 0;
    PCB running;
    idle_CPU(running);
    int quantum_remaining = 0;

    std::string execution_status;
    execution_status = print_exec_header();

    while(!all_process_terminated(job_list) && !job_list.empty()){
        // we check fro arrivals
        for (auto &process : list_processes){
            if (process.arrival_time == current_time && process.state == NOT_ASSIGNED){
                if(assign_memory(process)){
                    process.state = READY;
                    ready_queue.push_back(process);
                    job_list.push_back(process);
                    execution_status += print_exec_status(current_time, process.PID, NEW, READY);
                }
            }
        }
        // check for I/O completions
        for (auto it = wait_queue.begin(); it != wait_queue.end();){
            if (current_time >= it-> io_end_time){
                for (auto &proc : job_list){
                    if (proc.PID == it -> PID){
                        proc.state = READY;
                        proc.time_since_last_io = 0;
                        ready_queue.push_back(proc);
                        execution_status += print_exec_status(current_time, proc.PID, WAITING, READY);
                        break;
                    }
                }
                it = wait_queue.erase(it);
            } else {
                ++it;
            }
        }
        //check if the running process needs an I/O
        if(running.state == RUNNING){
            if (running.time_since_last_io >= running.io_freq){
                running.state = WAITING;
                running.time_since_last_io = 0;
                wait_queue.push_back(WaitingProcess(running.PID, current_time, running.io_duration));
                execution_status += print_exec_status(current_time, running.PID, RUNNING, WAITING);
                sync_queue(job_list, running);
                idle_CPU(running);
                quantum_remaining = 0;
            }
        }

        //check for preemption quantum or for higher priority
        if(running.state == RUNNING){
            bool should_preempt = false;

            //check quantum has expired 
            if (quantum_remaining <= 0){
                should_preempt = true;
            }
            //check for higher priority process in the ready queue
            if (!ready_queue.empty()){
                EP_RR_Scheduler(ready_queue);
                PCB highest_priority = ready_queue.back();
                if (highest_priority.priority < running.priority){
                    should_preempt = true;
                }
            }
            if (should_preempt){
                    running.state = READY;
                    ready_queue.push_back(running);
                    execution_status += print_exec_status(current_time, running.PID, RUNNING, READY);
                    sync_queue(job_list, running);
                    idle_CPU(running);
                    quantum_remaining = 0;
                }
            }

            // schedule the next process
            if(running.state != RUNNING && !ready_queue.empty()){
                EP_RR_Scheduler(ready_queue);
                run_process(running, job_list, ready_queue, current_time);
                execution_status += print_exec_status(current_time, running.PID, READY, RUNNING);
                quantum_remaining = QUANTUM;
            }
            // execute the current process
            if(running.state == RUNNING){
                running.remaining_time--;
                running.time_since_last_io++;
                running.time_in_cpu++;
                quantum_remaining--;
                sync_queue(job_list, running);
                if (running.remaining_time == 0){
                    terminate_process(running, job_list);
                    execution_status += print_exec_status(current_time, running.PID, RUNNING, TERMINATED);
                    idle_CPU(running);
                    quantum_remaining = 0;
                }
            }
            current_time++;
            if (current_time > 100000){
                std::cout << "The simulation timout now\n";
                break;
            }
        }
        
        execution_status+= print_exec_footer();
        return std::make_tuple(execution_status);
    }

int main(int argc, char** argv){
    if(argc != 2){
        std::cout << "ERROR!\n We expected 1 argument but instead received " <<argc - 1 << std::endl;
        std::cout << "Usage: ./interrupts_EP_RR <input_file.txt>" << std::endl;
        return -1;
    }
    auto file_name = argv[1];
    std::ifstream input_file;
    input_file.open(file_name);

    if (!input_file.is_open()){
        std::cerr <<"Error: We are unable to open the file: " << file_name <<std::endl;
        return -1;
    }

    std::string line;
    std::vector<PCB> list_process;
    while(std::getline(input_file, line)){
        if(line.empty() || line[0] == '#') continue;
        auto input_tokens = split_delim(line, ", ");
        auto new_process = add_process(input_tokens);
        list_process.push_back(new_process);
    }
    input_file.close();
    std::cout << "Running Priority + Round Robin with Preemption...\n";
    std::cout << "Loaded " << list_process.size() << " processes\n";
    auto [exec] = run_simulation(list_process);
    write_output(exec, "execution_EP_RR.txt");
    std::cout << "Simulation is now completed. The output is written to execution_EP_RR.txt\n";
    return 0;
}