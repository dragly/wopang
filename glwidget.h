//    Copyright (C) 2010 Svenn-Arne Dragly <s@dragly.com>
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//    Some parts of the code might still be from Nokia's Qt examples
//    and are of course Copyright (C) Nokia and/or its subsidiary(-ies).


// a
#ifndef GLWIDGET_H
#define GLWIDGET_H


#include <QAccelerometer>
#include <QtOpenGL>
#include <QtGui/qvector3d.h>
#include <QtGui/qmatrix4x4.h>
#include <QtOpenGL/qglshaderprogram.h>
#include <QTime>
#include <QVector>
//#include <Phonon/MediaObject>
#include "model.h"

QTM_USE_NAMESPACE

class Ui;
class Bubble;
class Cbutton;
class Node;
class Radar;
class Entity;
class Tank;
class SoundThread;

class GLWidget : public QGLWidget {

    Q_OBJECT

    friend class Ui;
    friend class Radar;
public:
    GLWidget(QWidget *parent = 0);
    ~GLWidget();

    enum Team {TeamHumans, TeamEnemies};
    QVector3D unProject(int x, int y);
    QVector3D unProject(int x, int y, QVector3D oldOffset);
    QPoint project(QVector3D position);

    static const qreal MaxHealth = 100.0;
    static const int MapSize = 30; // 2n x 2n nodes
    static const qreal NodeSize = 8; // each node is 8x8 m (the length of a tank)
    QList<Node*> findPath(Node* startPosition, Node* endPosition, QList<Node*> avoid = QList<Node*>());
    Node* closestNode(QVector3D position);
    // Sounds
    SoundThread *soundThread;
    QString sndExplosion;
    // Sensors
    QAccelerometer *accelerometer;
signals:
    void playSound(QString sample);
protected:
    void paintGL ();
    void initializeGL ();
    void resizeGL(int width, int height);
private:
    QVector3D rotation;
    QVector3D momentum;
    void createEnemy();
    void createCoin();
    void initEnemies();
    void resetGame();
    void resetEnemy(Entity* enemy);
    void fireBullet();
    void regenerateNodes();
    QTimer *timer;

    int score;

    int frames;
    QTime gametime;
    QTime frametime;
    QTime explosionSoundTime; // time since last explosion sound
    qreal aspectRatio;
    QVector3D camera;
    // Mouse
    QVector3D pressCursor;
    QVector3D dragCursor;
    QVector3D offset;
    QVector3D pressOffset;
    QVector3D lastDragOffset;
    // Painting
    QMatrix4x4 mainModelView;
    // Physics
    QVector3D gravity;
    // Game data
    QList<Entity*> enemies;
    QList<Entity*> units;
    QList<Entity*> buildings;
    QList<Entity*> bullets;
    QList<Entity*> coins;

    QList<Node*> nodes; // should probably have their own class - using Entity for convenience
    QHash<Node*, QList<Node*> > nodeNeighbors;

    QHash<Entity*, Entity*> bulletOwner;

    Entity* testUnit;
//    QHash<Entity*, QVector3D> bulletTargets;
    Entity* selectedUnit;
    Entity* ball;
    Model *mdlMonkey;
    Model *mdlBox;
    Model *mdlCannon;
    Model *mdlHumanTankBody;
    Model *mdlHumanTankTower;
    Model *mdlEnemyTankBody;
    Model *mdlEnemyTankTower;
    Model *mdlBullet;
    Model *mdlNode;
    Model *mdlCoin;
    Model *mdlBall;

    QVector3D *lightPos;
    bool gameOver;
    qreal gameOverTime;
    qreal lastFrameTime;
    QPoint dragLastPosition;
    QPoint dragStartPosition;
    bool dragging;
    QTime holdtime;
    bool inUi;
    QTime recruittime; //vj: temp for test.    //bool recruiting;
    int recruitqueue;

    bool useSound;
    bool drawHud;

    Ui* ui;

    // mouse events
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent *event);

    // keyboard events
    void keyPressEvent(QKeyEvent *event);
public slots:
    void recruitUnit();
};
#endif
