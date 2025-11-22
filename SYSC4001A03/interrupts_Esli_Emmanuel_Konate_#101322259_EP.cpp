/**
 * @file interrupts.cpp
 * @author Sasisekhar Govind, esli Emmanuel Konate 101322259
 * @brief template main.cpp file for Assignment 3 Part 1 of SYSC4001
 * 
 */

#include<interrupts_Esli_Emmanuel_Konate_#101322259.hpp>

// this will be the priority scheduler (so the lower the PID, the higher the priority)
void EP_Scheduler(std::vector<PCB> &ready_queue){
    std::sort(
        ready_queue.begin(),ready_queue.end(), [](const PCB &first, const PCB &second){
            // we reverse the sort so that the highest priority is at back
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

    std::string execution_status;
    execution_status = print_exec_header();

    //this is main loop
    while (!all_process_terminated(job_list) || job_list.empty()){
        //we check for the new arrivals
        for(auto &process : list_processes){
            if (process.arrival_time == current_time && process.state == NOT_ASSIGNED){
                if (assign_memory(process)){
                    process.state = READY;
                    ready_queue.push_back(process);
                    job_list.push_back(process);
                    execution_status += print_exec_status(current_time, process.PID, NEW, READY);
                }
            }
        }
        //check for he I/O completions
        for (auto it = wait_queue.begin(); it != wait_queue.end();){
            if (current_time >= it -> io_end_time){
                //find the process in the job_list and move it to ready
                for(auto &proc : job_list){
                    if(proc.PID == it->PID){
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

        //now we check if the running process needs I/O
    if(running.state == RUNNING){
        if (running.time_since_last_io >= running.io_freq){
            //process needs I/O
            running.state = WAITING;
            running.time_since_last_io = 0;
            wait_queue.push_back(WaitingProcess(running.PID, current_time, running.io_duration));
            execution_status += print_exec_status(current_time, running.PID, RUNNING, WAITING);
            sync_queue(job_list, running);
            idle_CPU(running);
        }
    }
    //now we schedule the next process with no preemption in EP
    if (running.state != RUNNING && !ready_queue.empty()){
        EP_Scheduler(ready_queue);
        run_process(running, job_list, ready_queue, current_time);
        execution_status += print_exec_status(current_time, running.PID, READY, RUNNING);
    }

    //we can execute the current process
    if(running.state == RUNNING){
        running.remaining_time--;
        running.time_since_last_io++;
        running.time_in_cpu++;
        sync_queue(job_list, running);

        //check if the process ended
        if (running.remaining_time == 0){
            terminate_process(running, job_list);
            execution_status += print_exec_status(current_time, running.PID, RUNNING, TERMINATED);
            idle_CPU(running);
        }
    }
    current_time++;
    //verifications
    if(current_time > 100000){
        std::cout << "This is a simulation timeout\n";
        break;
    }
        
    }
    execution_status += print_exec_footer();
    return std::make_tuple(execution_status);
}

int main(int argc, char** argv){
    if (argc != 2){
        std::cout << "ERROR\n We expected 1 argument and received " <<argc - 1 << std::endl;
        std::cout << "Usage: ./interrupts_EP <input_file.txt>" << std::endl;
        return -1;
    }
    auto file_name = argv[1];
    std::ifstream input_file;
    input_file.open(file_name);

    if(!input_file.is_open()){
        std::cerr << "Error: I am unable to open the file: " <<file_name << std::endl;
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
    std::cout << "Running the external priority scheduler with no preemption..\n";
    std::cout << "Loaded " << list_process.size() << " processes\n";

    auto [exec] = run_simulation(list_process);
    write_output(exec, "execution_EP.txt");

    std::cout << "Simulation Has now finished! The output was written inside execution_EP.txt\n";
    return 0;
}