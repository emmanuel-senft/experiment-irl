#include "interface.h"
#include "utilities.h"
#include "ui_interface.h"
#include "qlearning.h"
#include "logger.h"
#include "display.h"
#include <QFile>
#include <QDir>

using namespace std;

Interface::Interface(QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::Interface)
{
    _participantID=0;
    _ui->setupUi(this);
    _ui->gridLayoutWidget->hide();

    //centralWidget()->setAttribute(Qt::WA_TransparentForMouseEvents);
    setMouseTracking(true);

    connect(_ui->pushButtonStart,SIGNAL(clicked()), this, SLOT(start()));
    connect(_ui->pushButtonReady,SIGNAL(clicked()), this, SLOT(sophieReady()));
    connect(_ui->pushButtonID,SIGNAL(clicked()), this, SLOT(getID()));
    //connect(_ui->pushButtonReset,SIGNAL(clicked()), this, SLOT(newExp()));

    _utils = new Utilities();

    _locationsRobot = {"shelf","table","oven"};
    for(unsigned i=0;i<_locationsRobot.size();i++) _locationsRobotInv[_locationsRobot[i]]=i;

    _locationsObjects = {"shelf","table","robot"};
    for(unsigned i=0;i<_locationsObjects.size();i++) _locationsObjectsInv[_locationsObjects[i]]=i;

    _objects = {"flour","eggs","spoon","bowl","tray"};
    for(unsigned i=0;i<_objects.size();i++) _objectsInv[_objects[i]]=i;

    _bowlState = {"empty","flour","eggs","both", "mixed"};
    for(unsigned i=0;i<_bowlState.size();i++) _bowlStateInv[_bowlState[i]]=i;

    _trayState = {"empty","batter","baked"};
    for(unsigned i=0;i<_trayState.size();i++) _trayStateInv[_trayState[i]]=i;

    _stateIndex = {"robotLoc","flourLoc","eggsLoc","spoonLoc","bowlLoc","trayLoc","bowlState","trayState"};
    for(unsigned i=0;i<_stateIndex.size();i++) _stateIndexInv[_stateIndex[i]]=i;
    _stateSize={3,3,3,3,3,3,5,3};
    _multStateSize={1,3,9,27,81,243,729,3645};

    _actions = {"TURN:LEFT","TURN:RIGHT","PUT-DOWN:","Pick-UP:Flour","Pick-UP:Eggs","Pick-UP:Spoon","Pick-UP:Bowl","Pick-UP:Tray","Use-On:Flour","Use-On:Eggs","Use-On:Spoon","Use-On:Bowl","Use-On:Tray"};
    for(unsigned i=0;i<_actions.size();i++) _actionsIndexInv[_actions[i]]=i;


    int nstates = 3*pow(4,5)*5*3;
    int nactions = _actions.size();
    _qlearning = new QLearning(nstates, nactions);
    _participantID=0;
    _interactionID=0;

    if(ONPCPERSO)
        _dataFolder="D:/Dropbox/PhD/experiments/Sophie replication/data";
    else{
        //_dataFolder="C:/Users/esenft/Dropbox/PhD/experiments/Sophie replication/data";
        _dataFolder="C:/Users/PlymHRI/Dropbox/PhD/experiments/Sophie replication/data";
        freopen("out.txt","w",stdout);
    }

    _lookAtTable={-50,0,25,50,-25};

    _cooldownGaze=0;
    _cooldownInit=0;
    _cooldownSelection=0;
    _cooldownLastGaze=0;

    _maxCooldownGaze=1;
    _maxCooldownSelection=2;
    _maxCooldownInit=0.5;

    _selectedAction=-1;
    _steps=0;
    _timeSteps=0;
    _start=0;
    _auto=AUTO;
    _optActions={6,1,2,0,3,1,11,0,2,4,1,11,0,2,5,1,11,0,2,7,1,2,6,12,2,7,1,2};
    _paused=0;
    _waitingToRestart=0;
    _origin=-1;
    _test=0;
    _ended=0;
    _pausedButton=0;
    _guided=0;
    _lastGuided=0;
    _toExecute=0;
    _timeGuided=0;
    _executionStep=0;
    _lastAction=-1;
    _reward=-0.04;

    initState();

    QString sheet="font: 87 15pt \"Arial Black\";";
    _ui->label->setStyleSheet(sheet);
    if(DISPLAY){
       _display= new Display(this);
       _display->init(_ui->graphicsView,_state,this);
    }
    else
       _ui->graphicsView->setHidden(1);

    if(PARTICIPANT){
        _ui->pushButtonReady->setDisabled(1);
    }
    _ui->labelAction->setHidden(1);
    _ui->labelSuccess->setHidden(1);
}

Interface::~Interface()
{
    delete _ui;
    delete _utils;
    delete _qlearning;
    delete _timer;
    delete _logStateAct;
    _logEvent->log(QString::number(_timeSteps)+" "+QString::number(_steps)+" End");
    delete _logEvent;
}

void Interface::init(){
    QString toLog="Nb State used: "+QString::number(_stateIndex.size())+" nb action used: "+QString::number(_actions.size());
    _logStateAct->log(toLog);
    toLog="TotalTime ";
    for(unsigned i=0;i<_stateIndex.size();i++)
        toLog=toLog+QString::fromStdString(_stateIndex[i])+" ";
    for(unsigned i=0;i<_actions.size();i++)
        toLog=toLog+QString::fromStdString(_actions[i])+" ";
    toLog=toLog+"Suggested Selected remainingCooldown Guided Reward Canceled";
    _logStateAct->log(toLog);
    toLog="timeStep totalSteps Event";
    _logEvent->log(toLog);
    toLog="timeStep type side pix piy hover disabled";
    _logClick->log(toLog);
}

void Interface::initState(){
    if(DEBUGINT)
        cout<<"initState"<<endl;
    if(_steps!=0&&AUTO)
        _state = {_utils->getRandomInt(3),_utils->getRandomInt(3),_utils->getRandomInt(3),_utils->getRandomInt(3),_utils->getRandomInt(3),_utils->getRandomInt(3),_utils->getRandomInt(5),_utils->getRandomInt(3)};
    else
        _state = {_locationsRobotInv["shelf"],_locationsObjectsInv["shelf"],_locationsObjectsInv["shelf"],_locationsObjectsInv["shelf"],_locationsObjectsInv["shelf"],_locationsObjectsInv["shelf"],_bowlStateInv["empty"],_trayStateInv["empty"]};
    _lastState=_state;
    _lastAction=0;
    _experimentSteps=0;
    _ended=0;
    _guided=0;
}

void Interface::newExp(){
    if(DEBUGINT)
        cout<<"newExp"<<endl;
    _ui->labelSuccess->setHidden(1);
   _ui->labelAction->setText("");
   _ui->pushButtonStart->setHidden(1);
   _waitingToRestart=0;
   _paused=0;
   _reward=-0.04;
   _logEvent->log(QString::number(_timeSteps)+" "+QString::number(_steps)+ " NewExperiment "+QString::number(_test));
   initState();
   _ui->label->setHidden(0);
   initStep();
   if(!_test)
       _display->enableGUI(1);
   else
       _display->enableGUI(0);
}

void Interface::initStep(){
    if(DEBUGINT)
        cout<<"initStep"<<endl;

    if(!_guided){
        getSuggestedActions();
        initGaze();
    }
    update();
}

void Interface::start(){
    if(DEBUGINT)
        cout<<"Start"<<endl;
    _timer = new QTimer(this);
    connect(_timer, SIGNAL(timeout()), this, SLOT(step()));
    _timer->start(1000*STEPSIZE);

    _ui->label->setHidden(0);
    _start=1;

    update();
    _ui->pushButtonStart->setHidden(1);
    _ui->pushButtonReady->setDisabled(0);

    disconnect(_ui->pushButtonStart,SIGNAL(clicked()), this, SLOT(start()));
    connect(_ui->pushButtonStart,SIGNAL(clicked()), this, SLOT(newExp()));
    initStep();
    _display->enableGUI(1);
}

void Interface::step(){
    if(DEBUGINT)
        cout<<"cdI "<<_cooldownInit<<" cdG "<<_cooldownGaze<<" cdA "<<_cooldownSelection<<" cdLG "<<_cooldownLastGaze<<endl;
    _timeSteps++;
    cout<<_timeSteps<<endl;
    if(!_test&&_timeSteps*STEPSIZE>60*MAXTIME)
        outOfTime();
    if(_waitingToRestart)
        return;
    cout<<"test"<<endl;
    if(_cooldownInit>0){
        _cooldownInit-=STEPSIZE;
        if(_cooldownInit<=0.01){
            _cooldownInit=0;
            initStep();
        }
    }
    else{
        if(_cooldownGaze>0){
            _cooldownGaze-=STEPSIZE;
            if(_cooldownGaze<=0.01){
                _cooldownGaze=0;
                switchGaze();
            }
        }
        if(_cooldownSelection>0){
            _cooldownSelection-=STEPSIZE;
            if(_cooldownSelection<=0.01){
                _cooldownSelection=0;
                _cooldownGaze=0;
                selectAction();
            }
        }
        else if(_cooldownLastGaze>0){
            _cooldownLastGaze-=STEPSIZE;
            if(_cooldownLastGaze<=0.01){
                _cooldownLastGaze=0;
                act();
            }
        }
    }
}

void Interface::action(int actionNumber){
    if(DEBUGINT)
        cout<<"actionNumber "<<actionNumber<<" name "<<_actions[actionNumber]<<endl;

    _logEvent->log(QString::number(_timeSteps)+" "+QString::number(_steps)+" "+QString::fromStdString(_actions[actionNumber]));

    _steps++;
    _experimentSteps++;
    if(_steps%1000==0)
        cout<<_steps<<endl;
    _selectedAction=actionNumber;
    _lastState=_state;
    switch (actionNumber){
    case 0:
        moveLeft();
        break;
    case 1:
        moveRight();
        break;
    case 2:
        drop();
        break;
    }
    if(actionNumber>2&&actionNumber<8)
        pick(actionNumber-3);

    if(actionNumber>7)
        useOn(actionNumber-8);

    if(!_auto)
        update();
}

void Interface::moveLeft(){
    int pos = _state[_stateIndexInv["robotLoc"]];
    pos--;
    if(pos==-1) pos=2;
    _state[_stateIndexInv["robotLoc"]]=pos;
}

void Interface::moveRight(){
    int pos = _state[_stateIndexInv["robotLoc"]];
    pos++;
    if(pos==3) pos=0;
    _state[_stateIndexInv["robotLoc"]]=pos;
}

void Interface::pick(int objectId){
    int index = getCarriedId(_state);
    if(index==-1&&_state[1+objectId]==_state[_stateIndexInv["robotLoc"]])
        _state[1+objectId]=_locationsObjectsInv["robot"];
}

void Interface::drop(){
    if(DEBUGINT)
        cout<<"enter drop"<<endl;
    int index = getCarriedId(_state);
    if(DEBUGINT)
        cout<<"index "<<index<<" loc "<<_state[_stateIndexInv["robotLoc"]]<<endl;
    if(index==-1){
        return;
    }
    if(_state[_stateIndexInv["robotLoc"]]==_locationsRobotInv["oven"]){
        if(index==_objectsInv["tray"]&&_state[_stateIndexInv["trayState"]]==_trayStateInv["batter"]){
            success();
            return;
        }
        else{
            failure();
            return;
        }
    }
    _state[index+1]=_state[_stateIndexInv["robotLoc"]];
}

void Interface::useOn(int objectId){
    bool success=0;
    
    int index = getCarriedId(_state);
    if(objectId==_objectsInv["bowl"]){
        if(index==_objectsInv["eggs"]){
            if(_state[_stateIndexInv["bowlState"]]==_bowlStateInv["empty"]){
                _state[_stateIndexInv["bowlState"]]=_bowlStateInv["eggs"];
                success=1;
            }
            if(_state[_stateIndexInv["bowlState"]]==_bowlStateInv["flour"]){
                _state[_stateIndexInv["bowlState"]]=_bowlStateInv["both"];
                success=1;
            }
        }
        if(index==_objectsInv["flour"]){
            if(_state[_stateIndexInv["bowlState"]]==_bowlStateInv["empty"]){
                _state[_stateIndexInv["bowlState"]]=_bowlStateInv["flour"];
                success=1;
            }
            if(_state[_stateIndexInv["bowlState"]]==_bowlStateInv["eggs"]){
                _state[_stateIndexInv["bowlState"]]=_bowlStateInv["both"];
                success=1;
            }
        }
        if(index==_objectsInv["spoon"]){
            if(_state[_stateIndexInv["bowlState"]]==_bowlStateInv["both"]){
                _state[_stateIndexInv["bowlState"]]=_bowlStateInv["mixed"];
                success=1;
            }
        }
    }
    if(objectId==_objectsInv["tray"]){
       if(index==_objectsInv["bowl"]){
            if(_state[_stateIndexInv["bowlState"]]==_bowlStateInv["mixed"]){
                _state[_stateIndexInv["trayState"]]=_trayStateInv["batter"];
                _state[_stateIndexInv["bowlState"]]=_bowlStateInv["empty"];
                success=1;
            }
        }
    }
    if(!success)
        failure();
}



void Interface::update(){
    if(_waitingToRestart)
        return;
    if(DISPLAY)
        _display->update(_state);

    if(_test&&_experimentSteps > MAXSTEPTEST)
        failure();
}

void Interface::success(){
    cout<<_steps<<"  GOAL!!"<<endl;
    //_start=1;
    _auto=0;
    _reward=1;
    _qlearning->addPoint(getState(_state),_selectedAction,_reward,getPossibleActions(_state),getState(_state));
    _lastAction=_selectedAction;
    _executionStep=_timeSteps;
    log();

    _ui->labelSuccess->setHidden(0);
    QString sheet="background-color: rgba(150, 150, 0, 150);border-width:1px;border-style:solid;border-color:grey;border-radius:10px;font:  12pt \"MS Shell Dlg 2\";color: rgba(255, 255, 255,200);";
    _ui->labelSuccess->setStyleSheet(sheet);
    _ui->labelSuccess->setText("SUCCESS!");
    _logEvent->log(QString::number(_timeSteps)+" "+QString::number(_steps)+" Success with expSteps: "+QString::number(_experimentSteps)+" test: "+QString::number(_test));
    endExperiment();
    if(_test){
        _ui->label->setText("Well Done! You successfully taught Sophie\nhow to bake a cake!");
        _ui->pushButtonReady->setDisabled(1);
        _ui->pushButtonStart->setDisabled(1);
    }
    else{
        ;//_ui->label->setText("Well Done! Sophie managed to bake the cake.");
        _ui->pushButtonStart->setDisabled(0);
    }
    _display->enableGUI(0);
}

void Interface::failure(){
    cout<<_steps<<"  FAIL!"<<endl;
    _reward=-1;
    _qlearning->addPoint(getState(_state),_selectedAction,_reward,getPossibleActions(_state),getState(_state));
    _lastAction=_selectedAction;
    _executionStep=_timeSteps;
    log();

    _ui->labelSuccess->setHidden(0);
    QString sheet="background-color: rgba(150, 0, 0, 150);border-width:1px;border-style:solid;border-color:grey;border-radius:10px;font:  12pt \"MS Shell Dlg 2\";color: rgba(255, 255, 255,200);";
    _ui->labelSuccess->setStyleSheet(sheet);
    QString label="DISASTER!\n"+QString::fromStdString(_actions[_selectedAction]);
    if(_actions[_selectedAction]=="PUT-DOWN:"){
        label=label+QString::fromStdString(_objects[getCarriedId(_state)]);
    }
    _ui->labelSuccess->setText(label);

    _logEvent->log(QString::number(_timeSteps)+" "+QString::number(_steps)+" Failure with expSteps: "+QString::number(_experimentSteps)+" test: "+QString::number(_test));
    endExperiment();
    if(_test){
        if(_experimentSteps>MAXSTEPTEST)
            _ui->label->setText("Sophie took too long to bake the cake.\nThe learning is a failure.");
        else
            _ui->label->setText("Sophie did not manage to bake the cake.\nThe learning is a failure.");
        _ui->pushButtonReady->setDisabled(1);
        _ui->pushButtonStart->setDisabled(1);
    }
    else
        ;//_ui->label->setText("Sadly, Sophie failed to bake the cake.");

    _display->enableGUI(0);
}

void Interface::endExperiment(){
    _ui->labelSuccess->setHidden(0);
    _ended=1;

    if(PARTICIPANT){


        _ui->pushButtonStart->setText("New Training Session");
        _ui->pushButtonStart->setHidden(0);
    }
    _paused=1;
    _waitingToRestart=1;
    _ui->label->setText("");
}

int Interface::getState(std::vector<int> states){
    int state=0;
    for(unsigned i=0;i<states.size();i++){
        state+=states[i]*_multStateSize[i];
    }
    return state;
}

vector<int> Interface::getInvState(int state){
    vector<int> vec(_state.size(),0);
    if(state==0){
        vec[0]=1;
        return vec;
    }
    for(int i=_state.size()-1;i>=0;i--){
        vec[i]=state/_multStateSize[i];
        state=state%_multStateSize[i];
    }

    return vec;
}


vector<bool> Interface::getPossibleActions(std::vector<int> state){
    if(DEBUGINT)
        cout<<"getPossibleActions"<<endl;
    vector<bool> possActions(13,0);
    int index = getCarriedId(_state);

    possActions[0]=1;
    possActions[1]=1;

    if(index!=-1)
        possActions[2]=1;
    else
        possActions[2]=0;

    for(unsigned i=0;i<5;i++){
        if(state[i+1]==state[0]){
            possActions[3+i]=index==-1;     //index==-1 -> no obect possessed by robot;
            if(state[0]==_locationsRobotInv["table"])
                possActions[8+i]=index!=-1;
        }
    }
    return possActions;
}

int Interface::getActionNumberFromVector(std::vector<bool> vec){
    int val=0;
    for(unsigned i=0;i<vec.size();i++)
        val+=vec[i]*pow(2,i);
    return val;
}

std::vector<bool> Interface::getActionVectorFromNumber(int value){
    std::vector<bool> vec=vector<bool>(_actions.size(),0);
    if(value==0){
        vec[0]=1;
        return vec;
    }
    for(int i=vec.size()-1;i>=0;i--){
        int pow2=pow(2,i);
        //cout<<"value "<<value<<" pow2 "<<pow2<<" / "<<value/pow2<<" % "<<value%pow2<<endl;
        vec[i]=value/pow2;
        value=value%pow2;
    }
    return vec;
}

void Interface::getSuggestedActions(){
    if(DEBUGINT)
        cout<<"getSuggestedActions"<<endl;
    _lastSuggestedActions=_suggestedActions;
    _suggestedActions=_qlearning->getActions(getState(_state),getPossibleActions(_state));
    if(DEBUGINT){
        cout<<"get from qlearning : ";
        for(auto iter:_suggestedActions){
            cout<<iter.first<<"-"<<iter.second<<" | ";
        }
        cout<<endl;
    }
}

int Interface::getSuggestedAction(){
   return _qlearning->getAction(getState(_state),getPossibleActions(_state));
}

void Interface::log(){
    QString toLog=QString::number(_executionStep)+" ";
    for(unsigned i=0;i<_stateIndex.size();i++)
        toLog=toLog+QString::number(_lastState[i])+" ";
    vector<float>QVal=_qlearning->getQValue(getState(_lastState),getPossibleActions(_lastState));
    for(unsigned i=0;i<_actions.size();i++)
        toLog=toLog+QString::number(QVal[i])+" ";
    cout<<"size sugg "<<_lastSuggestedActions.size()<<endl;
    for(auto iter:_lastSuggestedActions){
        toLog=toLog+QString::number(iter.first)+"-";
    }
    toLog=toLog.left(toLog.size()-1);
    toLog=toLog+" "+QString::number(_lastAction)+" "+QString::number(_timeGuided)+" "+QString::number(_lastGuided)+" "+QString::number(_reward)+" "+QString::number(_cancel);
    _timeGuided=0;
    _lastGuided=0;
    cout<<"to log"<<toLog.toStdString()<<endl;
    _logStateAct->log(toLog);
}

void Interface::getInteractionID(){
    bool opened=0;
    int interactionID=0;
    QFile file;
    do{
        QString filename=QString::fromStdString(_dataFolder)+"/ID"+QString::number(_participantID)+"/RL/interaction"+QString::number(interactionID)+"/stateactions.txt";
        file.setFileName(filename);
        opened = file.open(QIODevice::ReadOnly | QIODevice::Text);
        cout<<_participantID<<" "<<interactionID<<" "<<opened<<endl;
        if(opened){
            file.close();
            interactionID++;
        }
    }while(opened);

    _interactionID = interactionID;
}

void Interface::getID(){
    cout<<"enter id"<<endl;
    bool ok;
    int id = _ui->lineEdit->text().toInt(&ok,10);
    if(ok){
        cout<<"ok"<<endl;
        _participantID=id;
        cout<<"in"<<endl;
        if(RECORD)
            getInteractionID();
        cout<<"out"<<endl;

        QDir subDir(QString::fromStdString(_dataFolder)+"/ID"+QString::number(_participantID)+"/RL/interaction"+QString::number(_interactionID));
        if (!subDir.exists()) {
            subDir.mkpath(".");
        }

        cout<<"Participant "<<_participantID<<" interaction "<<_interactionID<<(RECORD ? " recording":" NON RECORDING")<<endl;

        _logStateAct = new Logger(QString::fromStdString(_dataFolder)+"/ID"+QString::number(_participantID)+"/RL/interaction"+QString::number(_interactionID)+"/stateactions.txt");
        _logEvent = new Logger(QString::fromStdString(_dataFolder)+"/ID"+QString::number(_participantID)+"/RL/interaction"+QString::number(_interactionID)+"/event.txt");
        _logClick = new Logger(QString::fromStdString(_dataFolder)+"/ID"+QString::number(_participantID)+"/RL/interaction"+QString::number(_interactionID)+"/click.txt");

        init();

        update();

        _ui->gridLayoutWidget->show();
        _ui->gridLayoutWidget_2->hide();
    }
}
void Interface::sophieReady(){
    _test=1;
    _ui->pushButtonReady->setDisabled(1);
    _logEvent->log(QString::number(_timeSteps)+" "+QString::number(_steps)+" Ready");
    newExp();
    //_maxCooldownInit=0.05;
    _maxCooldownSelection=1;
    _ui->label->setText("Test in Progress.");
    //_maxCooldownGaze=0.05;
}

void Interface::setReward(float reward, int id){
    if(!_steps)
        return;

    _reward=reward;
    _logEvent->log(QString::number(_timeSteps)+" "+QString::number(_steps)+ " Reward "+QString::number(reward));
    if(reward<0)
        cancelAction();
}

void Interface::setGuidance(int id){
	if(DEBUGINT)
        cout<<"guidance"<<endl;
    if(_cooldownLastGaze>0)
       return;   
    _guided = 0;
    int index = getCarriedId(_state);
    if(_state[0]==0){
        if(index==-1&&_state[1+id]==0){
            _toExecute=3+id;
            _guided=1;
        }
    }
    if(_state[0]==1){
        if(index==-1){
            if(_state[1+id]==1){
                _toExecute=3+id;
                _guided=1;
            }
        }
        else if(_state[1+id]==1){
            _toExecute=8+id;
            _guided=1;
        }
    }
    if(_guided){
        _timeGuided=_cooldownSelection;
		_cooldownSelection = 0.5;
        if(_cooldownInit>0){
            _cooldownInit=0;
            _timeGuided=_maxCooldownSelection;
        }
        _logEvent->log(QString::number(_timeSteps)+" "+QString::number(_steps)+ " Guidance "+QString::fromStdString(_objects[id])+" working");
    }
    else
        _logEvent->log(QString::number(_timeSteps)+" "+QString::number(_steps)+ " Guidance "+QString::fromStdString(_objects[id])+" nonworking");
    _lastGuided=_guided;
}

void Interface::learn(){
    if(!_experimentSteps)
        return;
    if(DEBUGINT)
        cout<<"learn"<<endl;
    log();
    _qlearning->addPoint(getState(_lastState),_lastAction,_reward,getPossibleActions(_state),getState(_state));
    _reward=-0.04;
}

void Interface::act(){
    learn();
    _executionStep=_timeSteps;
    if(DEBUGINT)
        cout<<"act"<<endl;
    lookAt(1);
    QString label;
    QString actionStr=QString::fromStdString(_actions[_selectedAction]);

    int index = getCarriedId(_state);
    if(_selectedAction==2){
        QString suffix=QString::fromStdString(_objects[index]);
        suffix.replace(0,1,suffix[0].toUpper());
        actionStr=actionStr+suffix;
    }
    if(_selectedAction>7){
        QString suffix=QString::fromStdString(_objects[index]);
        suffix.replace(0,1,suffix[0].toUpper());
        suffix=":"+suffix+">";
        actionStr=actionStr.split(":")[0]+suffix+actionStr.split(":")[1];
    }

    label="  "+QString::number(_experimentSteps)+": "+actionStr;
    _ui->labelAction->setHidden(0);
    _ui->labelAction->setText(label);
    _cancel=0;
    action(_selectedAction);
    _lastAction=_selectedAction;
    if(!_waitingToRestart){
        _cooldownInit=_maxCooldownInit;
    }

    //choose action
    //reset variables
    //update label
    //execute action
}

void Interface::switchGaze(){
    if(DEBUGINT)
        cout<<"switch"<<endl;
    int gazeTo=0;
    int index=_utils->getRandomInt(_gazingProba.size());
    float total=0;
    int i=0;
    for(auto iter : _gazingProba){
        if(i==index)
            gazeTo=iter.first;
        i++;
        total+=iter.second;
    }
    _cooldownGaze=2*_maxCooldownGaze*_gazingProba.size()*_gazingProba[gazeTo]/(2*total);

    lookAt(gazeTo);
}

void Interface::cancelAction(){
    if(_cancel)
        return;
    if(_lastAction<2){
        _toExecute=!_lastAction;
        _cancel=1;
    }
    else if(_lastAction==2){
        int index = getCarriedId(_lastState);
        displayVector(_lastState);
        cout<<"index "<<index<<endl;
        if(index!=-1){
            _toExecute=3+index;
            _cancel=1;
        }
    }
    else if(_lastAction<7){
        _toExecute=2;
        _cancel=1;
    }

    if(_cancel){
        cout<<"action canceled "<<_toExecute<<endl;
        _guided=1;
        _lastGuided=1;
        _cooldownInit=0;
        _cooldownGaze=0;
        _cooldownLastGaze=0;
        _cooldownSelection=0.5;
    }
}

void Interface::initGaze(){
    if(DEBUGINT)
        cout<<"init"<<endl;
    _gazingObject.clear();
    _gazingProba.clear();
    for(auto iter : _suggestedActions){
        int key = iter.first;
        float value = iter.second;
        if(key==2){
            if(_state[0]!=2)
                _gazingObject[getCarriedId(_state)]=value;
        }
        if(key>2&&key<8)
            _gazingObject[key-3]=value;
        if(key>7)
            _gazingObject[key-8]=value;
    }

    if(_gazingObject.size()>0){
        bool init=0;
        float maxi=0;
        float mini=0;
        for(auto iter : _gazingObject){
            int key = iter.first;
            float value = iter.second;
            if(!init){
                maxi=value;
                mini=value;
                init=1;
            }
            if(DEBUGINT)
                cout<<"key "<<key<<" value "<<value<<endl;
            if(value>maxi)
                maxi=value;
            if(value<mini)
                mini=value;
        }
        if(DEBUGINT)
            cout<<"mini "<<mini<<" maxi "<<maxi<<endl;
        if(maxi>0){
            float total=0;
            for(auto iter : _gazingObject)
                if(iter.second>0)
                    total+=iter.second;
            for(auto iter : _gazingObject)
                if(iter.second>0)
                    _gazingProba[iter.first]=iter.second/total;
        }
        if(maxi==0){
            int n=0;
            for(auto iter : _gazingObject)
                if(iter.second==0)
                    n++;
            for(auto iter : _gazingObject)
                if(iter.second==0)
                    _gazingProba[iter.first]=1./(float)n;
        }
        if(maxi<0){
            float total=0;
            for(auto iter : _gazingObject)
                total+=-1./iter.second;
            for(auto iter : _gazingObject)
                _gazingProba[iter.first]=-total/iter.second;
        }
        if(_test){
            _cooldownSelection=0;
            _cooldownGaze=0;
            selectAction();
        }
        else{
            _cooldownSelection=2*_maxCooldownGaze*_gazingProba.size();
            switchGaze();
        }

        if(DEBUGINT)
            cout<<"size "<<_gazingProba.size()<<endl;
    }
    if(_suggestedActions.size()==1){
        _cooldownSelection=0;
        _cooldownGaze=0;
        selectAction();
    }
    else if(!_gazingObject.size()||_gazingProba.size()==1)
        _cooldownSelection=_maxCooldownSelection/2;
}

void Interface::selectAction(){
    cout<<"guided "<<_guided<<" toex "<<_toExecute<<endl;
    if(!_guided)
        _selectedAction = getSuggestedAction();
    else
        _selectedAction=_toExecute;

    _guided=0;
    if(_selectedAction==2){
		if(_state[0]==2)
			_display->rotateHead(0);
		else
			lookAt(getCarriedId(_state));
        _cooldownLastGaze=_maxCooldownGaze;
    }
    if(_selectedAction>2&&_selectedAction<8){
        lookAt(_selectedAction-3);
        _cooldownLastGaze=_maxCooldownGaze;
    }
    if(_selectedAction>=8){
        lookAt(_selectedAction-8);
        _cooldownLastGaze=_maxCooldownGaze;
    }
    if(_selectedAction<2){
        _display->rotateHead(0);
        _cooldownLastGaze=_maxCooldownGaze;
    }
}


void Interface::lookAt(int objectNumber){
     _display->rotateHead(_state[0]==1?-_lookAtTable[objectNumber]:_lookAtTable[objectNumber]);
}

int Interface::getCarriedId(std::vector<int> state){
    vector<int> objectsLoc(state.begin()+1,state.begin()+6);
    return getIndexInVector(objectsLoc, _locationsObjectsInv["robot"]);
}

void Interface::click(bool side, int pix, int piy, int hover, bool disabled){
    QString toLog=QString::number(_timeSteps)+" click "+(side?"right ":"left ")+QString::number(pix)+" "+QString::number(piy)+" "+QString::number(hover)+" "+QString::number(disabled);
    _logClick->log(toLog);
}

void Interface::release(bool side, int pix, int piy, int hover, bool disabled){
    QString toLog=QString::number(_timeSteps)+" release "+(side?"right ":"left ")+QString::number(pix)+" "+QString::number(piy)+" "+QString::number(hover)+" "+QString::number(disabled);
    _logClick->log(toLog);
}

template<typename Type, typename A> int Interface::getIndexInVector(std::vector<Type,A> vec, Type val){
    return (std::find(vec.begin(), vec.end(), val) != vec.end()) ? (std::find(vec.begin(), vec.end(), val) - vec.begin()) : -1 ;
}

template<typename Type, typename A> void Interface::displayVector(std::vector<Type,A> vec){
    for(unsigned i=0;i<vec.size();i++)
        cout<<vec[i]<<" ";
    cout<<endl;
    return;
}

void Interface::outOfTime(){
    _ui->label->setText("You ran out of time for this learning session\n You can press Sophie is Ready to test what you have taught.");
    _ui->labelSuccess->setHidden(1);
    _ui->labelAction->setText("");
    _ended=1;
    _paused=1;
    _waitingToRestart=1;
    _ui->pushButtonStart->setEnabled(0);
    _logEvent->log("OutOfTime");
}
