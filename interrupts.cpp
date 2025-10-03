/**
 *
 * @file interrupts.cpp
 * @author Justin Sy
 * @author Alex Rusu
 */

#include<interrupts.hpp>

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
        else if (activity == "SYSCALL") 
        {
            int device_num = duration_intr;

            auto [interrupt_exec_log, new_time] = intr_boilerplate
                                            (currentTime, device_num, context_save_time, vectors);
            
            execution += interrupt_exec_log;
            currentTime = new_time;

            //isr body
            execution += std::to_string(currentTime) + ", " + std::to_string(isr_activity_time) + ", execution ISR (syscall to device " + std::to_string(device_num)
            + ")\n";
            currentTime += isr_activity_time;

            execution += std::to_string(currentTime) + ", " + std::to_string(iret_ms) + ", IRET\n";
            currentTime += iret_ms;

        }
        else if (activity == "END_IO") {
            return 0;
        }
        else{
        }


        /************************************************************************/

    }

    input_file.close();

    write_output(execution);

    return 0;
}
