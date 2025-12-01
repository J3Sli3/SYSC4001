/**
 * @file interrupts.cpp
 * @author Sasisekhar Govind, Esli Emmanuel Konate 101322259
 * @brief template main.cpp file for Assignment 3 Part 1 of SYSC4001
 * 
 */

#include<interrupts_Esli_Emmanuel_Konate_#101322259.hpp>

// 100ms quantum
const int QUANTUM = 100;

//FCFS for ROUND ROBIN and it is taken from the front of the queue
void RR_Scheduler(std::vector<PCB> &ready_queue){
    //Round Robin makes used of FIFO so no sorting is needed
    // we will take from the back 
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
        //first we check for new arrivals
        for(auto &process : list_processes){
            if (process.arrival_time == current_time && process.state == NOT_ASSIGNED){
                if (assign_memory(process)){
                    process.state = READY;
                    ready_queue.insert(ready_queue.begin(), process);
                    job_list.push_back(process);
                    execution_status += print_exec_status(current_time, process.PID, NEW, READY);
                }
            }
        }
        // now we check for I/O
        for(auto it = wait_queue.begin(); it != wait_queue.end();){
            if (current_time >= it -> io_end_time){
                for (auto &proc : job_list){
                    if(proc.PID == it -> PID){
                        proc.state = READY;
                        proc.time_since_last_io = 0;
                        //add it to the front
                        ready_queue.insert(ready_queue.begin(), proc);
                        execution_status += print_exec_status(current_time, proc.PID, WAITING, READY);
                        break;
                    }
                }
                it = wait_queue.erase(it);
            } else {
                ++it;
            }
        }
        // now we check if the process currently running needs I/O
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

        //now we check for when the quantum expires with preemption
        if (running.state == RUNNING && quantum_remaining <= 0){
            running.state = READY;
            //add it to the front
            ready_queue.insert(ready_queue.begin(), running);
            execution_status += print_exec_status(current_time, running.PID, RUNNING, READY);
            sync_queue(job_list, running);
            idle_CPU(running);
        }

        //now we schedule for the next process
        if (running.state != RUNNING && !ready_queue.empty()){
            run_process(running, job_list, ready_queue, current_time);
            execution_status += print_exec_status(current_time, running.PID, READY, RUNNING);
            //we reset the quantum
            quantum_remaining = QUANTUM;
        }

        //then we can execute the current proces
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
            std::cout << "Simulation timeout\n";
            break;
        }
    }
    execution_status += print_exec_footer();
    return std::make_tuple(execution_status);
}

int main(int argc, char** argv){
    if (argc != 2){
        std::cout << "ERROR!\n We expected 1 argument but only received " <<argc - 1 <<std::endl;
        std::cout << "Usage: ./interrupts_RR <input_file.txt>" <<std::endl;
        return -1;
    }

    auto file_name = argv[1];
    std::ifstream input_file;
    input_file.open(file_name);

    if (!input_file.is_open()){
        std::cerr << "Error: We are unable to open file: " << file_name <<std::endl;
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

    std::cout << "We are now running the Round Robin Scheduler with quantum of 100ms...\n";
    std::cout << "Loaded " << list_process.size() << " process\n";
    auto [exec] = run_simulation(list_process);
    write_output(exec, "execution_RR.txt");
    std::cout<<"Simulationhas now finished! The output is written in execution_RR.txt\n";
    return 0;
}