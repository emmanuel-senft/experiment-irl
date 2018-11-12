#include "display.h"
#include <iostream>
#include <QGraphicsColorizeEffect>
#include <interface.h>

using namespace std;

Display::Display(QObject *parent)
    :QGraphicsScene(parent)
{ // Constructor of Scene
}
void Display::init(QGraphicsView *view, std::vector<int> state, Interface* interface)
{
    QString rootName;
    if(ONPCPERSO)
        rootName = "D:/Dropbox/PhD/code/sophieReplication/images/";
    else{
        //rootName = "C:/Users/esenft/Dropbox/PhD/code/sophieReplication/images/";
        rootName = "C:/Users/PlymHRI/Dropbox/PhD/code/divers/old/sophieSparc/images/";
    }

    _lastState=vector<int>(state.size(),0);
    _imagesNames={"background","robotBody","robotHead","flour","eggs","spoon","bowl","bowlFlour","bowlEggs","bowlBoth","bowlMixed","tray","trayBatter"};
    for(unsigned i=0;i<_imagesNames.size();i++) _imagesInv[_imagesNames[i]]=i;

    for(unsigned i=0;i<_imagesNames.size();i++){
        QString fileName= rootName+QString::fromStdString(_imagesNames[i]);
        if(!fileName.isEmpty()){
            QImage image(fileName);
            if(image.isNull())
            {
                cout<<"Error Displaying image"<<endl;
                return;
            }
            QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(image));
            _images.push_back(item);
        }
        else
            cout<<"error"<<endl;
    }

    for(int i=0;i<3;i++)
        addItem(_images[i]);
    for(int i=0;i<4;i++){
        _activeObjects.push_back(_images[i+3]);
    }
    _activeObjects.push_back(_images[11]);
    for(int i=0;i<5;i++)
        addItem(_activeObjects[i]);

    //_activeObjects.push_back();
    _locationsObjects={{{320,7},{75,220},{220,110},{150,140},{140,110}},
                       {{345,110},{140,220},{220,110},{150,140},{140,110}},
                       {{320,160},{160,220},{220,110},{150,140},{140,110}},
                       {{260,205},{195,230},{220,110},{150,140},{140,110}},
                       {{340,70},{105,230},{220,110},{150,140},{140,110}}};
    _locationsRobot={{200,90},{200,120},{190,160}};
    _rotationRobot={0,90,180};
    _offsetHead={{27,4},{-4,27},{-27,-4}};
    _sizes={{76,76},{68,68},{40,30},{51,17},{37,42},{46,44},{44,18}};
    _rotationCentre={15,34};

    for(unsigned i=0;i<_locationsObjects.size();i++){
        QTransform trans;
        trans.translate(_locationsObjects[i][0].x, _locationsObjects[i][0].y);
        _activeObjects[i]->setTransform(trans);
    }
    view->setScene(this);
    _cadre=new QGraphicsRectItem(150,100,15,100);
    _filing=new QGraphicsRectItem(151,150,13,49);
    _rectangle=new QGraphicsRectItem(50,50,50,50);
    _cadre->setPen(QPen(Qt::white,1));
    _leftPressed=0;
    _rightPressed=0;
    _currentHover=-1;
    update(state);
    _interface=interface;
    _disabled=1;
}

void Display::update(std::vector<int> state){
    QTransform trans;
    trans.translate(_locationsRobot[state[0]].x, _locationsRobot[state[0]].y);
    trans.rotate(_rotationRobot[state[0]]);
    QTransform trans2;
    trans2.translate(_locationsRobot[state[0]].x+_offsetHead[state[0]].x, _locationsRobot[state[0]].y+_offsetHead[state[0]].y);
    trans2.rotate(_rotationRobot[state[0]]);
    _images[1]->setTransform(trans);
    _images[2]->setTransform(trans2);

    if(state[6]!=_lastState[6]){
        removeItem(_activeObjects[3]);
        _activeObjects[3]=_images[6+state[6]];
        addItem(_activeObjects[3]);
    }
    if(state[7]!=_lastState[7]){
        removeItem(_activeObjects[4]);
        _activeObjects[4]=_images[11+state[7]];
        addItem(_activeObjects[4]);
    }

    for(unsigned i=0;i<_locationsObjects.size();i++){
        QTransform trans;
        trans.translate(_locationsObjects[i][state[i+1]!=2?state[i+1]:state[i+1]+state[0]].x, _locationsObjects[i][state[i+1]!=2?state[i+1]:state[i+1]+state[0]].y);
        _activeObjects[i]->setTransform(trans);
    }
    highlightImage(_images[1],_currentHover==5?1:0);
    highlightImage(_images[2],_currentHover==5?1:0);
    for(unsigned i=0;i<_activeObjects.size();i++)
        highlightImage(_activeObjects[i],_currentHover==i?1:0);

    _lastState=state;
}


void Display::leftClick(int x, int y){
    _cadre->setRect(x,y-50,15,100);
    _filing->setRect(x+1,y,13,0);

    _cadreX=x;
    _cadreY=y;
    addItem(_cadre);
    addItem(_filing);
    _leftPressed=1;
}

void Display::move(int x, int y){
    if(_leftPressed){
        int diff = _cadreY-y;

        if(diff>0){
            _filing->setPen(QPen(Qt::green,1));
            _filing->setBrush(QBrush(Qt::green));
            if(diff<49)
                _filing->setRect(_cadreX+1,y,13,diff);
            else
                _filing->setRect(_cadreX+1,_cadreY-49,13,49);
        }
        else{

            _filing->setPen(QPen(Qt::red,1));
            _filing->setBrush(QBrush(Qt::red));
            if(diff>-49)
                _filing->setRect(_cadreX+1,_cadreY,13,-diff);
            else
                _filing->setRect(_cadreX+1,_cadreY,13,49);
        }
    }
    else{
        int objId=isMouseHoverObject(x,y);
        if(objId!=-1){
            _currentHover=objId;
        }
        else{
            if(isMouseHoverRobot(x,y)){
                _currentHover=5;
            }
            else{
                _currentHover=-1;
            }
        }
    }
    update(_lastState);
}

void Display::leftRelease(int x, int y){
    if(_leftPressed){
        removeItem(_cadre);
        removeItem(_filing);
        int diff = _cadreY-y;
        if(diff>50) diff=50;
        if(diff<-50) diff=-50;
        _interface->setReward((float)diff/50., _currentHover);
        _leftPressed=0;
    }
}

void Display::rightClick(int x, int y){
//    _cadre->setRect(x,y-50,15,100);
//    _filing->setRect(x+1,y,13,0);
    _rectangle->setRect(x-25,y-25,50,50);
    _rectangle->setPen(QPen(Qt::yellow,1));
    addItem(_rectangle);
    if(_currentHover!=-1&&_currentHover!=5){
        _interface->setGuidance(_currentHover);
    }
}

void Display::rightRelease(){
    //removeItem(_cadre);
    //removeItem(_filing);
    removeItem(_rectangle);
}

void Display::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(_disabled)
        return;
    int x=event->scenePos().x();
    int y=event->scenePos().y();
    if(x>0&&x<WINDOWW&&y>0&&y<WINDOWH)
        move(x,y);
}

void Display::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    int x=event->scenePos().x();
    int y=event->scenePos().y();
    _interface->click(event->button()==Qt::RightButton,x,y,isMouseHoverObject(x,y),_disabled);
    if(_disabled)
        return;
    if(event->button()==Qt::LeftButton&&x>8&&x<WINDOWW-8&&y>50&&y<WINDOWH-50)
        leftClick(x,y);
    if(event->button()==Qt::RightButton&&x>13&&x<WINDOWW-13&&y>13&&y<WINDOWH-13)
        rightClick(x,y);
}

void Display::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    int x=event->scenePos().x();
    int y=event->scenePos().y();
    _interface->release(event->button()==Qt::RightButton,x,y,isMouseHoverObject(x,y),_disabled);
    if(_disabled)
        return;

    if(event->button()==Qt::LeftButton)
        leftRelease(x,y);
    if(event->button()==Qt::RightButton)
        rightRelease();
}

bool Display::isMouseHoverRobot(int x, int y){
    int rx=_locationsRobot[_lastState[0]].x;
    int ry=_locationsRobot[_lastState[0]].y;
    int sx=_sizes[0][0]+15;
    int sy=_sizes[0][1]+5;

    switch(_lastState[0]){
    case 0:
        if(x>rx&&x<rx+sx&&y>ry&&y<ry+sy)
            return 1;
    break;
    case 1:
        if(x<rx&&x>rx-sy&&y>ry&&y<ry+sx)
            return 1;
    break;
    case 2:
        if(x<rx&&x>rx-sx&&y<ry&&y>ry-sy)
            return 1;
    break;
    }
    return 0;
}

int Display::isMouseHoverObject(int x, int y){
    for(int i=0;i<5;i++){
        int ox=_locationsObjects[i][_lastState[i+1]!=2?_lastState[i+1]:_lastState[i+1]+_lastState[0]].x;
        int oy=_locationsObjects[i][_lastState[i+1]!=2?_lastState[i+1]:_lastState[i+1]+_lastState[0]].y;
        if(x>ox&&x<ox+_sizes[i+2][0]&&y>oy&&y<oy+_sizes[i+2][1])
            return i;
    }
    return -1;
}

void Display::highlightImage(QGraphicsPixmapItem *image, bool highlight){
    QGraphicsColorizeEffect* colour = new QGraphicsColorizeEffect();
    colour->setColor(QColor(200, 200, 0));
    if(highlight&&!_disabled){
        colour->setStrength(0.8);
    }
    else
        colour->setStrength(0);
    image->setGraphicsEffect(colour);
}


void Display::rotateHead(float angle){
    QTransform trans;
    _images[2]->setTransformOriginPoint(_rotationCentre.x,_rotationCentre.y);
    _images[2]->setRotation(angle);
}

void Display::enableGUI(bool enable){
    _disabled=!enable;
    if(_disabled)
        cleanGUI();
}

void Display::cleanGUI(){
    removeItem(_cadre);
    removeItem(_filing);
    removeItem(_rectangle);


    highlightImage(_images[1],0);
    highlightImage(_images[2],0);
    for(unsigned i=0;i<_activeObjects.size();i++)
        highlightImage(_activeObjects[i],0);
}
