/**
 *
 * @file interrupts.cpp
 * @author Sasisekhar Govind, Esli Emmanuel Konate 101322259, Nitish, Grover
 *
 * SYSC4001 Assignment 1, The Interrupt Simulator
 */

#include<interrupts.hpp>
#include <iostream>
#include<fstream>
#include<string>
#include<vector>
#include<sstream>
#include<cstdlib> //this will help me with the rand() method
#include <ctime> //this will help me with the time() method

using namespace std; //we won't have to use std::XXX throughout the code

int main(int argc, char** argv) {

    //vectors is a C++ std::vector of strings that contain the address of the ISR
    //delays  is a C++ std::vector of ints that contain the delays of each device
    //the index of these elemens is the device number, starting from 0
    //parse_args returns a tuple<vector<string>, vector<int>>
    auto [vectors, delays] = parse_args(argc, argv); //auto = compiler figures out the type
    //this will handle the files
    std::ifstream input_file(argv[1]);

    std::string trace;      //!< string to store single line of trace file
    std::string execution;  //!< string to accumulate the execution output

    /******************ADD YOUR VARIABLES HERE*************************/

    //this will be the time variable for the simulation
    int current_time = 0;

    /******************************************************************/

    //parse each line of the input trace file
    //the loop will read the line into the string 'trace' and
    //will continue until end of line.
    while(std::getline(input_file, trace)) {
        //parse the trace line using the function
        //it will return the type depending on CPU, SYSCALL, END_IO and a duration
        //for the CPU, the  duration is in ms and for SYSCALL/END_IO
        auto [activity, duration_intr] = parse_trace(trace);

        /******************ADD YOUR SIMULATION CODE HERE*************************/
        //this is a simple execution of the CPU BURST, so the program runs on the CPU
        //There are no interrupts so its just normal processing
        if (activity == "CPU"){
            execution += to_string(current_time) + ", " + to_string(duration_intr) +", CPU Burst\n";
            //now we advance the clock 
            current_time += duration_intr;
            //the system call will start I/O operation
        } else if (activity == "SYSCALL"){
            //now we extract ehich device we are calling
            int device_num = duration_intr;
            //So now we switch the mode, In kernel mode, the CPU can access ressources hidden in kernel
            //this will take around 1ms

            execution += to_string(current_time) + ", 1, switch to the kernel mode \n";
            current_time += 1;

            //now we save the state of the current process so we can later come back 
            //and it takes 10ms
            execution += to_string(current_time) + ", 10, context saved\n";
            current_time += 10;

            //so now we access the vector table stored in memory and each entry is VECTOR_SIZE bytes.
            //then we caculate the memory address
            // so it is base + (device_number x entry_size)
            char vector_address[10];
            sprintf(vector_address, "0x%04X", (ADDR_BASE + (device_num * VECTOR_SIZE)));
            execution += to_string(current_time) + ", 1, find vector " + to_string(device_num) + " in memory position " + string(vector_address) + "\n";
            current_time += 1;

            //now we read the ISR address from the vector and we load it into the PC
            execution += to_string(current_time) + ", 1, load address " + vectors.at(device_num) + " into the PC\n";
            current_time += 1;

            //Total ISR time is delays[device_num] - 13 as seen in assignment

            int isr_time = delays[device_num] - 13;

            //~40ms for the first activity we run the ISR
            execution += to_string(current_time) + ", 40, SYSCALL: run the ISR (device driver)\n";
            current_time += 40; // activity 1 : runs the ISR at 40ms
            
            //activity 2, transfers the data at 40ms
            execution += to_string(current_time) + ", 40, transfer data from device to memory\n";
            current_time += 40;

            //activity 3 will check for errors with the remaining time
            int error_check_time = isr_time - 80; // so the total isr time - the two 40ms activities
            execution += to_string(current_time) + ", " + to_string(error_check_time) + ", check for errors\n";
            current_time += error_check_time;

        } else if (activity == "END_IO"){

            //so END_IO is a hardware interrupt made by the device and it will solely happen
            //when the I/O device completes its operation
            //just like when a disk has finished reading the data.

            //first we extract which device has completed its I/O
            int device_num = duration_intr;

            //now we switch the mode again user -> kernel
            execution += to_string(current_time) + ", 1, switch to kernel mode\n";
            current_time += 1;


            //so we have to save the current process again just like the SYSCALL
            //this takes around 10ms per assignment
            execution += to_string(current_time) + ", 10, context saved\n";
            current_time += 10;

            //now just like for SYSCALL we have to look for the interrupt ine hte vector
            //then we have to calculate the memory address for the device vector
            char vector_address[10];
            sprintf(vector_address, "0x%04X", (ADDR_BASE + (device_num * VECTOR_SIZE)));
            execution += to_string(current_time) + ", 1, find vector " + to_string(device_num) + " in memory position " + string(vector_address) + "\n";
            current_time += 1;

            //then now we have to load the ISR address
            execution += to_string(current_time) + ", 1, load address " + vectors.at(device_num) + " into the PC\n";
            current_time += 1;

            //now we have to find the total ISR time which s delays[device_num] - 13
            int isr_time = delays[device_num] - 13;

            //now for END_IO , the first activity us 40ms and the remainder will go towards checking the 
            //status of the device
            execution += to_string(current_time) + ", 40, ENDIO: run the ISR (device driver)\n";
            current_time += 40;

            //now the remaining time is to check the status of the device
            int status_check_time = isr_time - 40;
            execution += to_string(current_time) + ", " + to_string(status_check_time) + ", check device status\n";
            current_time += status_check_time;
        }

        /************************************************************************/

    }

    input_file.close();

    write_output(execution);

    return 0;
}
