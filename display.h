#ifndef DISPLAY_H
#define DISPLAY_H

#include <QGraphicsScene>
#include <vector>
#include <map>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>

#define WINDOWW 400
#define WINDOWH 300

struct Point{
    int x;
    int y;
};

class Interface;

class Display : public QGraphicsScene
{
public:
    Display(QObject *parent);
    void init(QGraphicsView* view, std::vector<int> state, Interface *interface);
    void leftClick(int x, int y);
    void rightClick(int x, int y);
    void move(int x, int y);
    void leftRelease(int x, int y);
    void rightRelease();
    void update(std::vector<int> state);
    bool isMouseHoverRobot(int x, int y);
    int isMouseHoverObject(int x, int y);
    void exchangeImage(int id);
    void highlightImage(QGraphicsPixmapItem* image, bool highlight);
    void rotateHead(float angle);
    void enableGUI(bool enable);
    void cleanGUI();

private:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

    Interface* _interface;

    std::vector<QGraphicsPixmapItem*> _images;
    std::vector<QGraphicsPixmapItem*> _activeObjects;
    std::vector<std::string> _imagesNames; //{"background","robot","flour","eggs","spoon","bowl","bowlFlour","bowlEggs","bowlBoth","bowlMixed","tray","trayBatter"}
    std::map<std::string, int> _imagesInv;
    std::vector<std::vector<Point>> _locationsObjects; //
    std::vector<Point> _locationsRobot;
    std::vector<Point> _offsetHead;
    std::vector<std::vector<int>> _sizes;
    std::vector<int> _rotationRobot;
    std::vector<int> _lastState;    //{Lr, Lf, Le, Ls, Lb, Lt, Sb, St}
    QGraphicsRectItem* _cadre;
    QGraphicsRectItem* _filing;
    QGraphicsRectItem* _rectangle;
    Point _rotationCentre;

    int _cadreX;
    int _cadreY;
    int _currentHover;
    bool _leftPressed;
    bool _rightPressed;
    bool _disabled;


};

#endif // DISPLAY_H
