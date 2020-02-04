//
//  MNIST.h
//  CSCI-244 Project 2
//
//  Created by Andrew Valenzuela on 11/30/19.
//  Copyright © 2019 Andrew Valenzuela. All rights reserved.
//

#ifndef MNIST_h
#define MNIST_h
#pragma once
#include <iostream>
#include <stdint.h>
#include <vector>
#include <map>
#include <unordered_set>
#include <stdlib.h>
#include <stdio.h>
#include "imgFeat.h"


void die(const char *mess, std::string s = ""){
    printf("%s %s\n",mess, s.c_str());
    exit(1);
}
void printProg(const char *c, const double &percent){
      const int width = 40;
      int lpad = (percent * width);
      int rpad = width - lpad;
      printf("\r%s [%s%s]",c, std::string(lpad,'#').c_str(), std::string(rpad,'.').c_str());
      if (percent == 1)
          printf("\n");
      fflush (stdout);
}

class mnist{
    std::vector<imgFeat *> *train;
    std::vector<imgFeat *> *test;
    std::vector<imgFeat *> *DTrain;
    std::vector<imgFeat *> *DTest;
    
    size_t size;
    //  ratios of the training DB to use
    double PTrain;//  get training data on the entire DB
    int PTest;// using the training DB to test as well so I dont have to work with 2 files.
    
public:
    mnist(){
        PTrain = 1.0;
        PTest = 1.0;
        train = new std::vector<imgFeat *>;
        test = new std::vector<imgFeat *>;
        DTrain = new std::vector<imgFeat *>;
        DTest = new std::vector<imgFeat *>;
    }
    void setTestSize(const int x){ PTest = x;}
    //  loads the train data from the two files
    void readTrainData(const char *file1, const char *file2){
        const int N = 4;
        uint32_t imgHeader[N];
        unsigned char c[N];
        //  opens the files in read only
        FILE *insImg = fopen(file1, "r");
        FILE *insLab = fopen(file2, "r");
        //  checks if files opened without errors
        if (!insImg) die("Error loading file:",  file1);
        if (!insLab) die("Error loading file:",  file2);
        printf("Loading training pixel data\n");
        //  loop to read the header information: |MAGIC NUMBER|NUMBER OF IMAGES|ROWS|COLS|
        for (int i = 0; i < N; i++)
            if (fread(c, sizeof(c), 1, insImg))
                imgHeader[i] = littleE(c);
        printf("\nTraining image header loaded\n");
        //  image size l * w
        int imgSize = imgHeader[2] * imgHeader[3];
        //  loop to read in all the image pixel data
        for (int i = 0; i < imgHeader[1]; i++){
            printProg("Loading training image data", (double)(i+1)/imgHeader[1]);
            imgFeat *iF = new imgFeat();
            uint8_t e[1];//  will only work with an array becuse of fread()
            for (int j = 0; j < imgSize; j++){
                if (fread(e, sizeof(e), 1, insImg))
                    iF -> append(e[0]);
                else
                    die("Error reading training pixel data");
            }
            train -> push_back(iF);
        }
        printf("Training image data loaded\n");
        printf("Loading training label data\n");
        uint32_t labHeader[N];
        for (int i = 0; i < 2; i++)
            if(fread(c, sizeof(c), 1, insLab))
                labHeader[i] = littleE(c);
        printf("Training label header loaded\n");

        for (int i = 0; i < labHeader[1]; i++){
            printProg("Loading training labels", (double)(i+1)/labHeader[1]);
            uint8_t e[1];// array needed by fread()
            if (fread(e, sizeof(e), 1, insLab))
                train -> at(i) -> setLabel(e[0]);
            else
                die("Error reading training label data");
        }
        printf("Training label data loaded\n");
    }
    void readTestData(const char *file1, const char *file2){
        const int N = 4;
        uint32_t imgHeader[N];
        unsigned char c[N];
        //  opens the files in read only
        FILE *insImg = fopen(file1, "r");
        FILE *insLab = fopen(file2, "r");
        //  checks if files opened without errors
        if (!insImg) die("Error loading file:",  file1);
        if (!insLab) die("Error loading file:",  file2);
        printf("Loading test pixel data\n");
        //  loop to read the header information: |MAGIC NUMBER|NUMBER OF IMAGES|ROWS|COLS|
        for (int i = 0; i < N; i++)
            if (fread(c, sizeof(c), 1, insImg))
                imgHeader[i] = littleE(c);
        printf("\nTest image header loaded\n");
        //  image size l * w
        int imgSize = imgHeader[2] * imgHeader[3];
        //  loop to read in all the image pixel data
        for (int i = 0; i < imgHeader[1]; i++){
            printProg("Loading test image data", (double)(i+1)/imgHeader[1]);
            imgFeat *iF = new imgFeat();
            uint8_t e[1];//  will only work with an array becuse of fread()
            for (int j = 0; j < imgSize; j++){
                if (fread(e, sizeof(e), 1, insImg))
                    iF -> append(e[0]);
                else
                    die("Error reading test pixel data");
            }
            test -> push_back(iF);
        }
        printf("Test image data loaded\n");
        printf("Loading test label data\n");
        uint32_t labHeader[N];
        for (int i = 0; i < 2; i++)
            if(fread(c, sizeof(c), 1, insLab))
                labHeader[i] = littleE(c);
        printf("Test label header loaded\n");

        for (int i = 0; i < labHeader[1]; i++){
            printProg("Loading test labels", (double)(i+1)/labHeader[1]);
            uint8_t e[1];// array needed by fread()
            if (fread(e, sizeof(e), 1, insLab))
                test -> at(i) -> setLabel(e[0]);
            else
                die("Error reading test label data");
        }
        printf("Test label data loaded\n\n");
    }
    //  loads the Training DB into the DTrain and DTest
    void split(){
        std::unordered_set<int> seen;
        int sTrain = train -> size() * PTrain;
        int sTest = PTest;
        int count = 0;
        //  will load a ratio of the training data to use to train KNN
        //  currently set to 100%
        //  could just push the entire training data into the vector, but i want to keep the ability to use a smaller portion
        while (count < sTrain){
            int rIndex = rand() % train -> size();
            if (seen.find(rIndex) == seen.end()){
                DTrain -> push_back(train -> at(rIndex));
                seen.insert(rIndex);
                count++;
            }
        }
        //  resets the set
        seen.clear();
        count = 0;
        //  loads a ratio of training data to use for testing
        while (count < sTest){
            int rIndex = rand() % test -> size();
            if (seen.find(rIndex) == seen.end()){
                DTest -> push_back(test -> at(rIndex));
                seen.insert(rIndex);
                count++;
            }
        }
    }
    //  converts Big endian to little endian
    uint32_t littleE(const unsigned char* c){
        return ((c[0] << 24) | (c[1] << 16) | (c[2] << 8) | c[3]);
    }
    
    std::vector<imgFeat *> *getTrain(){ return DTrain;}
    std::vector<imgFeat *> *getTest(){ return DTest;}
};

#endif /* MNIST_h */
