//  KNN.h
//  CSCI-244 Project 2
//
//  Created by Andrew Valenzuela on 11/30/19.
//  Copyright © 2019 Andrew Valenzuela. All rights reserved.

#pragma once

#ifndef KNN_h
#define KNN_h

#include <stdint.h>
#include <fstream>
#include <climits>
#include <time.h>
#include <chrono>
#include <vector>
#include <memory>
#include <cmath>
#include <map>
#include "imgFeat.h"
#include "MNIST.h"

#ifdef THREADS
#include <thread>
#define FILENAME "/Users/andrewvalenzuela/Desktop/CSCI-244/Project 2/CSCI-244 Project 2/CSCI-244 Project 2/threads-output.csv"
#define METHOD "threads"
#define MESSAGE "figlet ANDREW VALENZUELA KNN THREADS | lolcat"
#elif defined PTHREAD
#include <pthread.h>
#define FILENAME "/Users/andrewvalenzuela/Desktop/CSCI-244/Project 2/CSCI-244 Project 2/CSCI-244 Project 2/pthread-output.csv"
#define METHOD "pthread"
#define MESSAGE "figlet ANDREW VALENZUELA KNN PTHREAD | lolcat"
#elif defined ASYNC
#include <future>
#define FILENAME "/Users/andrewvalenzuela/Desktop/CSCI-244/Project 2/CSCI-244 Project 2/CSCI-244 Project 2/async-output.csv"
#define METHOD "async"
#define MESSAGE "figlet ANDREW VALENZUELA KNN ASYNC | lolcat"
#else
#error Implementation not defined. Re-compile and define a multithreading method
#endif
//  gloabal vector to hold the kNN of each of the threads
//  each thread has its own vector to avoid using mutexes
std::vector<std::vector<uint8_t>> globalVec;
//  timer to track speed of knn
struct myclock{
    std::chrono::high_resolution_clock::time_point start;
    double duration;
    std::ofstream outs;
    myclock(const char *c){
        outs.open(c, std::ios::app);
        if (!outs) die("Error opening file:", c);
        start = std::chrono::high_resolution_clock::now();
    }
    //  destructor writes time to screen and file, is invoked when runKNN returns to main
    ~myclock(){
        duration = (std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start)).count();
        outs << duration << ",";
        printf("Time to run KNN: %.2fs\n", duration);
    }
};
//  struck to hold args for the pthread implementation
struct threadData{
    std::vector<imgFeat *> *v;
    imgFeat *d;
    int numThreads, kval;
};
//  simple function to print a progress bar
void pBar (const double &percent){
    const int width = 100;
    int val = (percent * 100);
    int lpad = (percent * width);
    int rpad = width - lpad;
    printf("\r%3d%% [%s%s]",val, std::string(lpad,'#').c_str(), std::string(rpad,'.').c_str());
    if (percent == 1)
        printf("\n");
    fflush (stdout);
}
//  euclidian distance from point to point
double eDist(imgFeat *d, imgFeat *x){
    double dist = 0.0;
    if (d -> getSize() != x -> getSize())
        die("Error Vector sizes do not match\n");
    for (size_t i = 0; i < d -> getSize(); i++)
        dist += (pow(d -> getFeatures() -> at(i) - x -> getFeatures() -> at(i), 2));
    return dist;
}
//  KNN for use with pthread i had to much trouble trying to get it to work with the class member function
void* kN(void* tD){
    struct threadData *td = (struct threadData*)tD;
    int kval = td->kval;
    int threadNum = td->numThreads;
    imgFeat *d = td->d;
    std::vector<imgFeat *> vec = *td->v;
    double max = std::numeric_limits<double>::max();
    double min = max;
    double prevMin = min;
    int index = 0;
    for (int i = 0; i < kval; i++){
        for (int j = 0; j < vec.size(); j++){
            double dist = eDist(d, vec.at(j));
            vec.at(j) -> setDist(dist);
            if (dist < min){
                min = dist;
                index = j;
            }
        }
        globalVec.at(threadNum).push_back(vec.at(index)->getLabel());
        prevMin = min;
        min = max;
    }
    return NULL;
}
class knn{
    std::vector<imgFeat *> *DTrain;
    std::vector<imgFeat *> *DTest;
    
public:
    knn(){}
    //  this splits my training into separate vectors and runs the KNN algorithm based on the thread implementation
    void kHelper(imgFeat *d, int numThreads, int kval){
        globalVec.resize(numThreads);
        int split = static_cast<int>(DTrain->size())/numThreads;
        for (auto &gv : globalVec)
            gv.reserve(split);
        std::vector<std::vector<imgFeat *>> *vec = new std::vector<std::vector<imgFeat *>>;
        //  splits the training DB into numThreads different vectors
        for (int i = 0; i < numThreads; i++)
            vec->push_back(std::vector<imgFeat *>(DTrain->begin() + (i * split), DTrain->begin() + (i * split) + split));
#ifdef THREADS
        //  vector of numThreads threads
        std::vector<std::thread> threads;
        //  needed to pass into thread creation
        knn n;
        //  make numThreads threads and run kNearest passing in portion of the training DB and the thread number
        for (int i = 0; i < numThreads; i++)
            threads.emplace_back(std::thread(&knn::kNearest,&n,d,vec->at(i),i,kval));
        //  joins the threads to the main thread
        for (auto &th : threads)
            th.join();
#elif defined PTHREAD
        //  array of numThreads pthreads
        pthread_t threads[numThreads];
        for (int i = 0; i < numThreads; i++){
            //  new struct to hold args
            struct threadData *tD = new struct threadData;
            (*tD).v = &vec->at(i);
            (*tD).d = d;
            (*tD).numThreads = i;
            (*tD).kval = kval;
            //  creates a new thread and starts the algroithm passing the struct
            pthread_create(&threads[i], NULL, &kN, (void*)tD);
        }
        // joins the threads
        for (int i = 0; i < numThreads; i++)
            pthread_join(threads[i], NULL);
#elif defined ASYNC
        //  a vector of numthreads futures
        std::vector<std::future<void>> myFutures;
        //  instance of knn to pass to async
        knn n;
        //  saves the return val in to a vector to enable concurrency
        //  makes numthreads async threads
        for (int i = 0; i < numThreads; i++){
            myFutures.push_back(std::async(std::launch::async,&knn::kNearest,&n,d,vec->at(i),i,kval));
        }
#else
        //  something must have really gone wrong to get here
        printf("Error: Something went wrong somewhere\n");
        exit(1);
#endif
        delete vec;
    }
    //  KNN algorithm, for every data point passed the function will add the k smallest distances for every train point in the db
    void kNearest(imgFeat *d,std::vector<imgFeat *> vec, int threadNum, int kval){
        // variables to keep track of the nearest neighbor
        double max = std::numeric_limits<double>::max();
        double min = max;
        double prevMin = min;
        int index = 0;//  index of nearest neighbor
        //  loops over all data points in the partition of the training DB passed in
        for (int i = 0; i < kval; i++){
            for (int j = 0; j < vec.size(); j++){
                double dist = eDist(d, vec.at(j));
                vec.at(j) -> setDist(dist);//  sets the distance i dont think i need this
                if (dist < min){
                    min = dist;
                    index = j;
                }
            }
            globalVec.at(threadNum).push_back(vec.at(index)->getLabel());
            prevMin = min;
            min = max;
        }
    }
    void setDTrain(std::vector<imgFeat *> *vec){ DTrain = vec;}
    void setDTest(std::vector<imgFeat *> *vec){ DTest = vec;}
    //  predicts what the image is
    int predict(){
        int c = 0, cc = 0;
        std::vector<uint8_t> vec;
        uint8_t max = 0;
        //  combines the split vectors into one
        for (auto gv : globalVec)
            for (auto v : gv)
                vec.push_back(v);
        //  finds the mode of the labels
        for (int i = 0; i < vec.size(); i++){
            c = static_cast<uint8_t>(count(vec.begin() + i,vec.end(),vec.at(i)));
            if (c > cc){
                cc = c;
                max = vec.at(i);
            }
            i += c - 1;
        }
        //  clears the split vectors
        for (auto &v : globalVec)
            v.clear();
        //  returns the label with the most "votes"
        return max;
    }
    //  entry point from main
    void runKNN(int numThreads = 1, int kval = 1){
        //  starts a timer and prints/saves the times
        myclock myClock(FILENAME);
        printf("Running KNN using <%s> with %lu training images and %lu test images. Running %d threads and K = %d\n", METHOD,(DTrain->size()/numThreads)*numThreads,DTest->size(),numThreads, kval);
        int right = 0, index = 0;

        //  starts KNN for every test image in Dtest
        for (auto *d : *DTest){
            kHelper(d, numThreads, kval);
            int pred = predict();
            index++;
            //printf("Image: %d Predict: %d -> Actual: %d\n", index, pred, d -> getLabel());
            pBar((double)index/DTest->size());
            if (pred == d -> getLabel())
                right++;
        }
        printf("Correctly predicted %d out of %lu test cases %.2f%%\n", right, DTest->size(),((double)right * 100)/ (double)DTest -> size());
    }
};
#endif /* KNN_h */
    
