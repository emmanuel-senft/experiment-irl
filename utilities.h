#ifndef UTILITIES_H
#define UTILITIES_H

#include <math.h>
#include <random>
#include <iostream>
#include <vector>

class Utilities
{
public:
    Utilities();

    float sigmoid(float x, float slope=0.25, float max = 1, float min =0, float offset=0);
    float gaussian(float x, float max=1, float mean=0, float stdev=1);
    float getNoise(float mean, float stdev);
    float getRandom();
    int getRandomInt(int max);
    template<typename Type, typename A> bool isInVector(std::vector<Type,A> vec, Type val);
    template<typename Type, typename A> int getIndexInVector(std::vector<Type,A> vec, Type val);
    template<typename Type, typename A> void removeInVector(std::vector<Type,A> & vec, Type val);
    template<typename Type, typename A> void displayVector(std::vector<Type,A> vec);
};

#endif // UTILITIES_H
