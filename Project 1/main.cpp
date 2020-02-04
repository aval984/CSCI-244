//  CSCI-244_Project1
//
//  Created by Andrew Valenzuela on 10/20/19.
//  Copyright © 2019 Andrew Valenzuela. All rights reserved.
#include <iostream>
#include <stdio.h>
#include <vector>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <fstream>
#include <numeric>
#include <cmath>
#include <sys/resource.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>

#include <mach/mach.h>
#include <mach/task.h>
#include <mach/task_info.h>
#include <mach/mach_init.h>

using namespace std;
//shared mem to hold values
double *pidStdDev = (double*)mmap(NULL, 5 * sizeof(double), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

void pid1func(int = 500);
void memleak2_0();
size_t memcheck();
int fib(int);
double stdDev(vector<size_t>, double&);

ofstream outs("output.txt",ios::app);
//ofstream outs2("output2.txt",ios::app);

int main(int argc, char **argv){
    
    cerr << "Parent process pid: " << getpid() << " is using: " << memcheck()/1024 << " KB of Memory\n";
    cerr << "Creating child process pid\n";
    pid_t pid = fork();//make a new child
    if(!pid){
        pid1func();//call the first function
        kill(getpid(), SIGKILL);//no grandchildren
    }
    wait(NULL);//wait for first child to finish before continueing
    wait(NULL);//using one wait gave me problems two waits works the way i want
    
    cerr << "Creating child process pid2\n";
    pid_t pid2 = fork();//make a new child
    if (!pid2){
        cout << "Child process pid2 created successfully\n\n";
        memleak2_0();//call the second function
        kill(getpid(), SIGKILL);//no grandchildren
    }
    wait(NULL);
    wait(NULL);//using one wait gave me problems two waits works the way i want
    cout << "Creating a new Child\n";
    pid_t pidGuess = fork();//new child
    if (!pidGuess){
        //runs the same function as pid, didnt want to rewrite the function to not overwrite the stdDev and means
        srand((unsigned int)time(NULL));
        //if(true){//always run child 1
        //if(false){//always run child 2
        if((rand() % 2)){//coin flip
            cerr << "Child process created successfully\n\n";
            cout << "Running child 1 function with pid of: " << getpid() << endl;
            vector<size_t> pidGuessVec;
            for (int i = 0; i < 250; i++){
                fib(30);
                pidGuessVec.push_back(memcheck()/1024);
            }
            //calc mean
            *(pidStdDev + 4) = accumulate(pidGuessVec.begin(),pidGuessVec.end(),0.0)/pidGuessVec.size();
            kill(getpid(), SIGKILL);
        }else{
            cout << "Child process created successfully\n\n";
            cout << "Running child 2 function with pid of: " << getpid() << endl;
            //runs the same function as pid2, didnt want to rewrite the function to not overwrite the stdDev and means
            vector<size_t> vec3;
            for (int i = 0; i < 250; i++){
                vector<uint64_t*> vec2;
                for (int j = 0; j < 10'000; j++)
                    vec2.push_back(new uint64_t);
                vec3.push_back(memcheck()/1024);
                while (!vec2.empty()){
                    delete vec2.back();
                    vec2.pop_back();
                }
                vec2.clear();
            }
            *(pidStdDev + 4) = accumulate(vec3.begin(),vec3.end(),0.0)/vec3.size();
            kill(getpid(), SIGKILL);
        }
    }
    wait(NULL);
    wait(NULL);
    //hold values, easier to read
    double p1std = *pidStdDev, p1mean = *(pidStdDev + 1), p2std = *(pidStdDev + 2),
           p2mean = *(pidStdDev + 3), pidGuessmean = *(pidStdDev + 4);
    cout << "Average memory used by the guesspid: " << pidGuessmean << endl;
    cout << "Average memory used by the child 1 function: " << p1mean << " with a standard deviation of: " << p1std << endl;
    cout << "Average memory used by the child 2 function: " << p2mean << " with s standard deviation of: " << p2std << endl;
    if (pidGuessmean < p1mean + (3*p1std) and pidGuessmean > p1mean - (3*p1std)){
        cout << "this is probably a child 1 function\n";
    }else if (pidGuessmean < p2mean + (3*p2std) and pidGuessmean > p2mean - (3*p2std)){
        cout << "this is probably a child 2 function\n";
    }else{
        cout << "Could not determine which function this is running\n";
    }
    cout << "ending the program\n";
}
void pid1func(int runs){
    double mean = 0.0;
    vector<size_t> pidVec;
    cerr << "Child process pid created successfully\n\n";
    //will run fib of 30 runs number of times default is 500 and push the mem use into a vector
    for (int i = 0; i < runs; i++){
        fib(30);
        pidVec.push_back(memcheck()/1024);//div to get KB
    }
    for (auto v : pidVec)
        outs << v << endl;
    *pidStdDev = stdDev(pidVec, mean);//sets some shared mem to the value of the std dev of the mem use
    *(pidStdDev + 1) = mean;//sets the shared mem to the mean
}
void memleak2_0(){
    double mean = 0.0;
    vector<size_t> vec3;
    //this will allocate some memory 500 times and push the mem use into a vector
    for (int i = 0; i < 500; i++){
        vector<uint64_t*> vec2;
        //will dynamically allocate 'size' number of new 64bit ints and push them into a vector
        for (int j = 0; j < 10'000; j++)
            vec2.push_back(new uint64_t);
        vec3.push_back(memcheck()/1024);//push the mem use into vecto
        //delete all the allocated mem
        while (!vec2.empty()){
            delete vec2.back();
            vec2.pop_back();
        }
        vec2.clear();//clear anything left in the vector probaly not needed
    }
    for (auto v : vec3)
        outs << v << endl;
    *(pidStdDev + 2) = stdDev(vec3, mean);//set shared mem to std dev
    *(pidStdDev + 3) = mean;//set shared mem to mean
}
int fib(int x){
    //classic recursive fib
    if (x < 3) return 1;
    return fib(x - 1) + fib(x - 2);
}
//returns the maxrss of a process
size_t memcheck(){
    struct task_basic_info info;
    task_t task = 0;
    if (task_for_pid(current_task(), getpid(), &task) != KERN_SUCCESS)
        abort();
    mach_msg_type_number_t infoCount = TASK_BASIC_INFO_COUNT;
    task_info(task,TASK_BASIC_INFO, (task_info_t)&info, &infoCount);
    return info.resident_size;
}
//calc the std dev of the mem use
double stdDev(vector<size_t> vec, double &mean){
    double sqSum = 0;
    mean = accumulate(vec.begin(), vec.end(), 0.0)/vec.size();
    for (auto v : vec)
        sqSum += pow(v - mean, 2);
    return sqrt(sqSum/vec.size());
}

/*
 another way to get the mem use. smaller than what im using but idk will some times give different values
 size_t memcheck(){
    struct rusage usage;
    getrusage(RUSAGE_SELF,&usage);
    return usage.ru_maxrss;
 }
 */
