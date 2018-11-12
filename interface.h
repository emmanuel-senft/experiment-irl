#ifndef INTERFACE_H
#define INTERFACE_H

#include <QMainWindow>
#include <QLabel>
#include <QPushbutton>
#include <vector>
#include <map>
#include <QTimer>
#include <QSignalMapper>
#include <QMouseEvent>

#define STEPSIZE 0.1
#define AUTO 0
#define DISPLAY 1
#define PARTICIPANT 1
#define RECORD 1
#define MAXSTEPTEST 70
#define INTERACTIONNUMBER 3
#define WINDOW0X 266
#define WINDOW0Y 20
#define WINDOWW 400
#define WINDOWH 300
#define MAXCOOLDOWNGAZE 1
#define DEBUGINT 1
#define MAXTIME 25
#define ONPCPERSO 0



class Utilities;
class QLearning;
class Logger;
class Display;

namespace Ui {
class Interface;
}

class Interface : public QMainWindow
{
    Q_OBJECT

public:
    explicit Interface(QWidget *parent = 0);
    ~Interface();
    void setReward(float reward, int id);
    void setGuidance(int id);
    void click(bool side, int pix, int piy, int hover, bool disable);
    void release(bool side, int pix, int piy, int hover, bool disable);

private slots:
    void action(int actionNumber); //origin 0:human, 1 accept, 2 automatic
    void drop();
    void init();
    void initState();
    void moveLeft();
    void moveRight();
    void pick(int objectId);
    void useOn(int objectId);
    void update();
    void success();
    void failure();
    int getState(std::vector<int> states);
    std::vector<int> getInvState(int state);
    std::vector<bool> getPossibleActions(std::vector<int> state);       // move(2),drop(1),pick(5),use(5)
    int getActionNumberFromVector(std::vector<bool> vec);
    std::vector<bool> getActionVectorFromNumber(int value);
    void getSuggestedActions();
    int getSuggestedAction();
    void start();
    void step();
    void log();
    void newExp();
    void getID();
    void getInteractionID();
    void endExperiment();
    void sophieReady();
    void learn();
    void act();
    void initGaze();
    void switchGaze();
    void initStep();
    void cancelAction();
    void selectAction();
    void lookAt(int objectNumber);
    int getCarriedId(std::vector<int> state);
    void outOfTime();

public:
    template<typename Type, typename A> int getIndexInVector(std::vector<Type,A> vec, Type val);
    template<typename Type, typename A> void displayVector(std::vector<Type,A> vec);

private:

    Ui::Interface* _ui;
    Utilities* _utils;
    QLearning* _qlearning;
    QTimer* _timer;
    Logger* _logStateAct;
    Logger* _logEvent;
    Logger* _logClick;
    Display* _display;


    std::vector<int> _state;                //  {Lr, Lf, Le, Ls, Lb, Lt, Sb, St}
    std::vector<int> _lastState;                //  {Lr, Lf, Le, Ls, Lb, Lt, Sb, St}
    std::vector<std::string> _locationsRobot;    //  {"shelf","table","oven"};
    std::map<std::string, int> _locationsRobotInv;

    std::vector<std::string> _locationsObjects;    //  {"shelf","table","robot"};
    std::map<std::string, int> _locationsObjectsInv;

    std::vector<std::string> _objects;      //  {"flour","eggs","spoon","bowl","tray"}
    std::map<std::string, int> _objectsInv;

    std::vector<std::string> _bowlState;    //  {"empty","flour","eggs","both","mixed"};
    std::map<std::string, int> _bowlStateInv;

    std::vector<std::string> _trayState;    //  {"empty","batter","baked"}
    std::map<std::string, int> _trayStateInv;

    std::vector<std::string> _stateIndex;   //  {"robotLoc","flourLoc","eggsLoc","spoonLoc","bowlLoc","trayLoc","bowlState","trayState"}
    std::map<std::string, int> _stateIndexInv;
    std::vector<int> _stateSize;            //  {3,3,3,3,3,3,5,3}
    std::vector<int> _multStateSize;        //  {1,3,9,27,81,243,729,3645}


    std::vector<std::string> _actions; //{"TURN:LEFT","TURN:RIGHT","PUT-DOWN:","Pick-UP:Flour","Pick-UP:Eggs","Pick-UP:Spoon","Pick-UP:Bowl","Pick-UP:Tray","Use-On:Flour","Use-On:Eggs","Use-On:Spoon","Use-On:Bowl","Use-On:Tray"};
    std::map<std::string, int> _actionsIndexInv;

    std::map<int,float> _suggestedActions;
    std::map<int,float> _lastSuggestedActions;
    std::map<int,float> _gazingObject;
    std::map<int,float> _gazingProba;
    std::vector<int> _guidedAction;
    std::vector<float> _lookAtTable;

    int _selectedAction;
    int _lastAction;
    int _toExecute;
    float _cooldownGaze;
    float _cooldownSelection;
    float _cooldownInit;
    float _cooldownLastGaze;
    float _maxCooldownGaze;
    float _maxCooldownSelection;
    float _maxCooldownInit;
    int _steps;

    int _timeSteps;
    int _executionStep;
    bool _start;
    bool _auto;
    bool _reduc;
    float _maxCooldown;
    bool _paused;
    bool _pausedButton;
    bool _waitingToRestart;
    int _origin;
    int _participantID;
    int _interactionID;
    int _experimentSteps;
    bool _test;
    bool _ended;
    bool _pressed;
    bool _guided;
    bool _lastGuided;
    float _reward;
    bool _cancel;
    float _timeGuided;

    std::vector<int> _optActions;
	std::string _dataFolder;
};

#endif // INTERFACE_H
