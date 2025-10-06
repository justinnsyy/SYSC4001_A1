/**
 *
 * @file interrupts.cpp
 * @author Justin Sy
 * @author Alex Rusu
 */

#include<interrupts.hpp>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <vector>

int main(int argc, char** argv) {

    //vectors is a C++ std::vector of strings that contain the address of the ISR
    //delays  is a C++ std::vector of ints that contain the delays of each device
    //the index of these elemens is the device number, starting from 0
    auto [vectors, delays] = parse_args(argc, argv);
    std::ifstream input_file(argv[1]);

    std::string trace;      //!< string to store single line of trace file
    std::string execution;  //!< string to accumulate the execution output

    /******************ADD YOUR VARIABLES HERE*************************/
    int vectorTable[VECTOR_SIZE];
    int currentTime = 0;
    int context_save_time = 10;
    int isr_activity_time = 40;
    const int iret_ms = 1;
    std::vector<int> io_ready_at(delays.size(), -1);
    
    /******************************************************************/

    //parse each line of the input trace file
    while(std::getline(input_file, trace)) {
        auto [activity, duration_intr] = parse_trace(trace);

        /******************ADD YOUR SIMULATION CODE HERE*************************/
        if (activity == "CPU") 
        {
            execution += std::to_string(currentTime) + ", " + std::to_string(duration_intr) + 
                         ", CPU burst\n";

            currentTime += duration_intr;
        }
        else if (std::strcmp(activity.c_str(), "SYSCALL") == 0 || std::strcmp(activity.c_str(), "END_IO")  == 0)
        {
            if (activity == "SYSCALL" || std::strcmp(activity.c_str(), "SYSCALL") == 0) {
                int device_num = duration_intr;

                int dly = (device_num >= 0 && device_num < (int)delays.size()) ? delays[device_num] : 0;
                io_ready_at[device_num] = currentTime + dly;

                execution += std::to_string(currentTime) + ", 0, start I/O on device "
                + std::to_string(device_num) + " (will be ready at "
                + std::to_string(io_ready_at[device_num]) + " ms)\n";
                
                //Kernel steps
                auto [interrupt_exec_log, new_time] = intr_boilerplate
                                                (currentTime, device_num, context_save_time, vectors);
                
                execution += interrupt_exec_log;
                currentTime = new_time;

                //ISR body and IRET and restore
                execution += std::to_string(currentTime) + ", " + std::to_string(isr_activity_time)
                        + ", execution ISR (syscall to device " + std::to_string(device_num) + ")\n";
                currentTime += isr_activity_time;

                execution += std::to_string(currentTime) + ", " + std::to_string(iret_ms) + ", IRET\n";
                currentTime += iret_ms;

                execution += std::to_string(currentTime) + ", " + std::to_string(context_save_time) + ", context restored\n";
                currentTime += context_save_time;
            }
            else if (activity == "END_IO") {
                int device_num = duration_intr;

                // If not yet finished, CPU idles until recorded completion time
                int ready_at = (device_num >= 0 && device_num < (int)io_ready_at.size()) ? io_ready_at[device_num] : currentTime;

                if (currentTime < ready_at) {
                    execution += std::to_string(currentTime) + ", " + std::to_string(ready_at - currentTime)
                            + ", CPU idle waiting for end of I/O " + std::to_string(device_num) + "\n";
                    currentTime = ready_at;
                }

                // Completion interrupt
                execution += std::to_string(currentTime) + ", 0, end of I/O "
                        + std::to_string(device_num) + ": interrupt\n";

                // Kernel Steps
                auto [interrupt_exec_log, new_time] =
                    intr_boilerplate(currentTime, device_num, context_save_time, vectors);
                execution += interrupt_exec_log;
                currentTime = new_time;

                // ISR body and IRET and restore
                execution += std::to_string(currentTime) + ", " + std::to_string(isr_activity_time)
                        + ", execution ISR (end-of-I/O " + std::to_string(device_num) + ")\n";
                currentTime += isr_activity_time;

                execution += std::to_string(currentTime) + ", " + std::to_string(iret_ms) + ", IRET\n";
                currentTime += iret_ms;

                execution += std::to_string(currentTime) + ", " + std::to_string(context_save_time) + ", context restored\n";
                currentTime += context_save_time;

                // Clear pending I/O
                if (device_num >= 0 && device_num < (int)io_ready_at.size()) io_ready_at[device_num] = -1;
            }
        } 


        /************************************************************************/

    }

    input_file.close();

    write_output(execution);

    return 0;
}
