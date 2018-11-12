#include "qlearning.h"
#include "utilities.h"

using namespace std;

QLearning::QLearning(int nStates, int nActions){
    _value = vector<vector<float>>(nStates, vector<float>(nActions,0.));
    //_transition = vector<vector<int>>(nStates, vector<int>(nActions,-1));
    _alpha =  0.3;
    _gamma = 0.75;
    _temperature = 0.07;
    _boundary=0.2;
    utils = new Utilities();
}

QLearning::~QLearning(){

}

void QLearning::addPoint(int state, int action, float reward, std::vector<bool> possibleActions, int newState){
    //_transition[state][action]=newState;
    _value[state][action]=_value[state][action]+_alpha*(reward+_gamma*getMaxQValue(newState, possibleActions)-_value[state][action]);
     //cout<<"reward "<<reward<<" state "<<state<<" action "<<action<<" value "<<_value[state][action]<<endl;
}

float QLearning::getMaxQValue(int state, vector<bool> possibleActions){
    if(possibleActions.size()==0) return -0.5;
    if(DEBUG) cout<<"getMaxQ"<<endl;
    float val = _value[state][possibleActions[0]];
    float max=val;
    if(DEBUG){
        cout<<"values "<<endl;
        for(unsigned i=0;i<possibleActions.size();i++)
            if(possibleActions[i]!=0)
                cout<<i<<" ";
        cout<<endl;
    }
    for(unsigned i=0;i<possibleActions.size();i++){
        if(possibleActions[i]==0) continue;
        val = _value[state][i];
        if(DEBUG) cout<<val<<" ";
        if(val>max) max=val;
    }
    if(DEBUG) cout<<endl;
    return max;
}


std::vector<float> QLearning::getQValue(int state, vector<bool> possibleActions){
    vector<float> QValues(possibleActions.size(),0);
    for(unsigned i=0;i<possibleActions.size();i++)
        QValues[i] = _value[state][i];
    return QValues;
}

std::map<int,float> QLearning::getActions(int state, vector<bool> possibleActions){
    if(DEBUG) cout<<"getAction"<<endl;

    float val = _value[state][possibleActions[0]];
    float max=val;
    vector<int> indexes;
    std::map<int,float> values;
    if(DEBUG){
        cout<<"values "<<endl;
        for(unsigned i=0;i<possibleActions.size();i++)
            if(possibleActions[i]!=0)
                cout<<i<<" ";
        cout<<endl;
    }
    for(unsigned i=0;i<possibleActions.size();i++){
        if(possibleActions[i]==0) continue;
        val = _value[state][i];
        if(val>max)
            max=val;
    }
    for(unsigned i=0;i<possibleActions.size();i++){
        if(possibleActions[i]==0) continue;
        val = _value[state][i];
        if(DEBUG) cout<<val<<" ";
        if(val>=max-_boundary){
            indexes.push_back(i);
        }
    }
    for(unsigned i=0;i<indexes.size();i++)
        values[indexes[i]]=_value[state][indexes[i]];
    if(DEBUG) cout<<endl;
    return values;
}

int QLearning::getAction(int state, vector<bool> possibleActions){
    if(DEBUG) cout<<"getAction"<<endl;

    float val = _value[state][possibleActions[0]];
    float max=val;
    vector<int> indexes;
    if(DEBUG){
        cout<<"values "<<endl;
        for(unsigned i=0;i<possibleActions.size();i++)
            if(possibleActions[i]!=0)
                cout<<i<<" ";
        cout<<endl;
    }
    for(unsigned i=0;i<possibleActions.size();i++){
        if(possibleActions[i]==0) continue;
        val = _value[state][i];
        if(DEBUG) cout<<val<<" ";
        if(val==max){
            indexes.push_back(i);
        }
        else if(val>max){
            max=val;
            indexes.clear();
            indexes.push_back(i);
        }
    }
    if(DEBUG) cout<<endl;
    return indexes[utils->getRandomInt(indexes.size())];;
}
/*        actionSelected=indexes[utils->getRandomInt(indexes.size())];
    }
    else{       //SoftMax Boltzman
        vector<float> valExp;
        float acc=0;
        if(DEBUG) cout<<"Exp "<<endl;
        for(unsigned i=0;i<possibleActions.size();i++){
            valExp.push_back(possibleActions[i]*exp(_value[state][i]/_temperature));
            acc+=valExp[i];
            if(DEBUG) cout<<" "<<valExp[i];
        }
        float random=utils->getRandom()*acc;
        if(DEBUG) cout<<" Acc "<<acc<<" Rand "<<random<<endl;
        acc=0;
        for(unsigned i=0;i<possibleActions.size();i++){
            if(possibleActions[i]==0) continue;
            acc+=valExp[i];
            if(acc>random){
                actionSelected=i;
                break;
            }
        }
    }
    if(DEBUG) cout<<"Chosen "<<actionSelected<<endl;*/


