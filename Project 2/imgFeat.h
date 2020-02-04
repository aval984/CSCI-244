//  imgFeat.h
//  CSCI-244 Project 2
//
//  Created by Andrew Valenzuela on 11/30/19.
//  Copyright © 2019 Andrew Valenzuela. All rights reserved.

#pragma once

#ifndef imgFeat_h
#define imgFeat_h
#include <vector>
#include <cstdint>

class imgFeat{
    std::vector<uint8_t> *features; //  holds the pixel data 28 x 28
    uint8_t label;
    double dist;
    
public:
    imgFeat(){features = new std::vector<uint8_t>;}
    
    ~imgFeat(){}
    
    void setFeat(std::vector<uint8_t> *vec){ features = vec;}
    
    void append(uint8_t val){ features->push_back(val);}
    
    void setLabel(uint8_t val){ label = val;}
    
    void setDist(double d){ dist = d;}
    
    double getDist(){ return dist;}
    
    size_t getSize(){ return features->size();}
    
    uint8_t getLabel(){ return label;}
    
    std::vector<uint8_t> *getFeatures(){ return features;}
    
    bool operator<(const imgFeat *rhs){ return label < rhs->label;}
};
#endif /* imgFeat_h */
