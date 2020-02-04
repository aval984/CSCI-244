//
//  main.cpp
//  CSCI-244 Project 2
//
//  Created by Andrew Valenzuela on 11/30/19.
//  Copyright © 2019 Andrew Valenzuela. All rights reserved.
//
#include "KNN.h"
#include "MNIST.h"

const int MAXTHREADS = 8;

int getRand(){ return rand();}
void start(knn *k, int kk, int passes){
    for (int i = 0; i < passes; i++){
        printf("Run %d of %d\n",i + 1, passes);
        for (int j = 0; j < MAXTHREADS; j++)
            k->runKNN(j+1,kk);
        std::ofstream outs(FILENAME, std::ios::app);
        outs << "\n";
        outs.close();
    }
}
int main(int argc, const char * argv[]) {
    if (argc < 3) die("usage: a.out number_of_test_cases(1-10,000) size_of_k\n");
    system("clear");
    system(MESSAGE);
    srand(static_cast<int>(time(NULL)));
    if (argc < 3) die("usage: a.out number_of_test_cases(1-10,000) size_of_k\n");
    int testSize = std::stoi(argv[1]);
    if (testSize < 1 or testSize > 10'000) die("Error: enter a test size between 1 and 10,000");
    int kSize = std::stoi(argv[2]);
    int passes = 1;
#ifdef TESTING
    passes = 10;
#endif
    mnist *m = new mnist();

    m->setTestSize(testSize);
    m->readTrainData("/Users/andrewvalenzuela/Desktop/CSCI-244/Project 2/CSCI-244 Project 2/CSCI-244 Project 2/train-images-idx3-ubyte", "/Users/andrewvalenzuela/Desktop/CSCI-244/Project 2/CSCI-244 Project 2/CSCI-244 Project 2/train-labels-idx1-ubyte");
    m->readTestData("/Users/andrewvalenzuela/Desktop/CSCI-244/Project 2/CSCI-244 Project 2/CSCI-244 Project 2/t10k-image-idx3-ubyte", "/Users/andrewvalenzuela/Desktop/CSCI-244/Project 2/CSCI-244 Project 2/CSCI-244 Project 2/t10k-labels-idx1-ubyte");
    m->split();
    
    knn *k = new knn();
    k -> setDTrain(m -> getTrain());
    k -> setDTest(m -> getTest());
    int kk = kSize;
    start(k,kk,passes);

}
