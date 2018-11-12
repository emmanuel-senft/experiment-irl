#include "utilities.h"
#include <algorithm>
#include <windows.h>

using namespace std;

Utilities::Utilities()
{
    srand(GetTickCount());
}


float Utilities::sigmoid(float x, float slope, float max, float min, float offset){
    return min+(max-min)/(1+exp(-slope*(x-offset)));
}

float Utilities::gaussian(float x, float max, float mean, float stdev){
    return max*exp(-(x-mean)*(x-mean)/(2*stdev*stdev));
}

float Utilities::getRandom(){
    float random=rand()%1000/1000.;
    //cout<<random<<endl;
    return random;
}


int Utilities::getRandomInt(int max){
    return rand()%(max);
}

float Utilities::getNoise(float mean, float stdev){
    std::mt19937 mt(rand());
    std::normal_distribution<double> distribution(mean,stdev);
    return distribution(mt);
}

template<typename Type, typename A> bool Utilities::isInVector(std::vector<Type,A> vec, Type val){
    return std::find(vec.begin(), vec.end(), val) != vec.end();
}

template<typename Type, typename A> void Utilities::removeInVector(std::vector<Type,A> & vec, Type val){
    vec.erase(std::remove(vec.begin(), vec.end(), val), vec.end());
}


template<typename Type, typename A> int Utilities::getIndexInVector(std::vector<Type,A> vec, Type val){
    return (std::find(vec.begin(), vec.end(), val) != vec.end()) ? (std::find(vec.begin(), vec.end(), val) - vec.begin()) : -1 ;
}

template<typename Type, typename A> void Utilities::displayVector(std::vector<Type,A> vec){
    for(unsigned i=0;i<vec.size();i++)
        cout<<vec[i]<<" ";
    cout<<endl;
    return;
}
