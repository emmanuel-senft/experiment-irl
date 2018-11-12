#ifndef QLEARNING_H
#define QLEARNING_H

#include <vector>
#include <map>

#define DEBUG 0
#define GREEDY 1

class Utilities;

class QLearning
{
public:
    QLearning(int nStates, int nActions);
    ~QLearning();
    void addPoint(int state, int action, float reward, std::vector<bool> possibleActions, int newState);
    std::map<int,float> getActions(int state, std::vector<bool> possibleActions);
    int getAction(int state, std::vector<bool> possibleActions);
    std::vector<float> getQValue(int state, std::vector<bool> possibleActions);

private:

    float getMaxQValue(int state, std::vector<bool> possibleActions);

    Utilities *utils;
    std::vector<std::vector<float>> _value;
    float _alpha;
    float _gamma;
    float _temperature;
    float _boundary;

};

#endif // QLEARNING_H
