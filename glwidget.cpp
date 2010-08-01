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

#include "glwidget.h"
#include "model.h"
#include "entity.h"
#include "tank.h"
#include "ui/ui.h"
#include <QPainter>
#include <QPaintEngine>
//#include <Phonon/MediaObject>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
//#include "ui/window.h"
//#include "ui/cbutton.h"
#include "node.h"
#include "soundthread.h"

// constants
const qreal UnitSpeed = 40.0; // m/s
const qreal UnitAcceleration = 10.0; // m/s^2
const qreal UnitFrictionSide = 6.0;
const qreal UnitFrictionAll = 2.0;
const qreal EnemySpawnDistance = 20; // m
const qreal RotateSpeed = 60; // degrees/s
const qreal TowerRotateSpeed = 120; // degrees/s
const qreal BulletSpeed = 40; // m/s
const qreal NumberOfEnemies = 1;

// map and nodes
const int GLWidget::MapSize; //Vj: moved to header as i need them in another class
const qreal GLWidget::NodeSize;
//const int MapSize = 30; // 2n x 2n nodes
//const qreal NodeSize = 8; // each node is 8x8 m (the length of a tank)
const qreal NodeSizeSquared = GLWidget::NodeSize * GLWidget::NodeSize; // just for convenience
const qreal NodeSizeDiagonal = GLWidget::NodeSize * GLWidget::NodeSize + GLWidget::NodeSize * GLWidget::NodeSize; // just for convenience
const qreal UnitCollideDistance = NodeSizeSquared / 2.5; // divided by two because we shouldn't be too nazi ;) now units may move diagonally without interrupting neighbor nodes.

// gui and interactions
const qreal DragDropTreshold = 20;
const qreal ClickRadius = GLWidget::NodeSize / 2.0;

// weapon constants
const qreal ExplosionRadiusSquared = 3*3; // squared
const qreal ExplosionDamage = 30;
const qreal ExplosionForce = 20;
const qreal FireDistanceSquared = NodeSizeSquared * 6.0 * 6.0;
const qreal BulletSpawnTime = 2;
QVector3D Gravity(0, 0, -10); // m/s^2
const qreal GLWidget::MaxHealth;

// physics
const qreal SpringConstant = 5;
GLWidget::~GLWidget()
{
}

GLWidget::GLWidget(QWidget *parent) : QGLWidget(parent)
{
    //    soundbank = new KGrSoundBank(2);
    //    expsound = soundbank->loadSound("data/sounds/bomb-02.wav");
    //    expsound2 = soundbank->loadSound("data/sounds/bomb-02.ogg");
    //    qDebug() << QSound::isAvailable();
    //    explosion = Phonon::createPlayer(Phonon::GameCategory, Phonon::MediaSource("data/sounds/explosion-02.ogg"));
    //    explosion2 = Phonon::createPlayer(Phonon::GameCategory, Phonon::MediaSource("data/sounds/bomb-02.ogg"));
    //    explosion3 = Phonon::createPlayer(Phonon::GameCategory, Phonon::MediaSource("data/sounds/bomb-03.ogg"));
    useSound = true;
    drawHud = true;

    qsrand(time(NULL));
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setAutoBufferSwap(false);
    mdlMonkey = new Model("data/objects/monkey1.obj");
    mdlBox = new Model("data/objects/box.obj");
    mdlCannon = new Model("data/objects/cannon.obj");
    mdlHumanTankBody = new Model("data/objects/tank-body.obj");
    mdlHumanTankBody->scale *= 0.5;
    mdlHumanTankTower = new Model("data/objects/tank-head.obj");
    mdlHumanTankTower->scale *= 0.5;
    mdlEnemyTankBody = new Model("data/objects/tank-body.obj");
    mdlEnemyTankBody->scale *= 0.5;
    mdlEnemyTankTower = new Model("data/objects/tank-head.obj");
    mdlEnemyTankTower->scale *= 0.5;
    mdlBullet = new Model("data/objects/bullet.obj");
    mdlBullet->scale *= 0.6;
    mdlNode = new Model("data/objects/box.obj");
    mdlCoin = new Model("data/objects/coin.obj");
    mdlBall = new Model("data/objects/ball.obj");
    // initial values
    camera = QVector3D(0.0, 0.0, 40);
    lightPos = new QVector3D(0.0,-0.3,10.0);
    sndExplosion = "data/sounds/bomb.wav";
    QStringList audioSamples;
    audioSamples << sndExplosion;
    soundThread = new SoundThread(this, audioSamples);
    soundThread->start();
    //    sndExplosion = soundThread->loadSample("data/sounds/bomb.wav");
    qDebug() << "main thread is" << QThread::currentThreadId();
    grabKeyboard();
    grabMouse();
    // Sensors
    accelerometer = new QAccelerometer(this);
    accelerometer->connectToBackend();
    if (!accelerometer->start()) {
        qWarning("Could not start sensor:");
    } else {
        qWarning("Sensor started successfully");
    }
    if (accelerometer->isBusy())
        qWarning("sensor is busy");
    if (!accelerometer->isConnectedToBackend())
        qWarning("sensor is not connected to backend");
    if (!accelerometer->isActive())
        qFatal("Sensor not active!");
    // timer, should be set last, just in case
    timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(updateGL()));
    timer->setInterval(1);
    timer->start();
}

void GLWidget::resetGame() {
//    regenerateNodes();
    // init all to zero (also avoids memory failures)
    lastFrameTime = 0.0;
    gameOver = false;
    dragging = false;
    inUi = false;
    frames = 0;
    score = 0;
    gameOverTime = 0.0;
    offset *= 0;
    recruitqueue = 0;
    // end init all to zero
    gametime.start();
    bullets.clear();
    enemies.clear();
    ball = new Entity(mdlBall);
    ball->position = QVector3D(0,-5,0);
    ball->mass = 0.5; // kg
    Tank* cannon = new Tank(mdlHumanTankBody, mdlHumanTankTower);
    cannon->position = QVector3D(10,5,1);
    cannon->positionNode = closestNode(cannon->position);
    cannon->team = TeamHumans;
    selectedUnit = cannon;
    units.append(cannon);
    Tank* cannon2 = new Tank(mdlHumanTankBody, mdlHumanTankTower);
    cannon2->position = QVector3D(1,1,1);
    cannon2->positionNode = closestNode(cannon2->position);
    cannon2->team = TeamHumans;
    units.append(cannon2);
    Entity* building = new Entity(mdlBox, Entity::TypeBuilding);
    building->position = QVector3D(-4,4,0);
    building->positionNode = closestNode(building->position);
    building->health = 1000;

    //building->addMenuPoitner(baseMenu);
    buildings.append(building);
    initEnemies();
    createCoin();
    testUnit = new Entity(mdlBox);
    testUnit->scale *= 0.1;
}

void GLWidget::initEnemies() {
    for(int i = 0; i < NumberOfEnemies; i++) {
        createEnemy();
    }
}

void GLWidget::resetEnemy(Entity* enemy) {
    qreal randomAngle = qrand() * 360; // set random position
    enemy->position = QVector3D(cos(randomAngle * M_PI / 180) * EnemySpawnDistance, sin(randomAngle * M_PI / 180) * EnemySpawnDistance, 0.0); // set random position
    enemy->positionNode = closestNode(enemy->position);
    qDebug() << "Position enemy:" << enemy->position;
    enemy->health = MaxHealth; // reset health
    if(buildings.count() > 0)
        enemy->currentTarget = buildings.first(); // attack any buildling
    else if(units.count() > 0)
        enemy->currentTarget = units.first(); // or attack any unit
    enemy->orders = Entity::OrderAttack;
}

void GLWidget::createEnemy() {
    qDebug() << "Creating enemy";
    Entity *enemy = new Entity(mdlHumanTankBody);
    enemies.append(enemy);
    resetEnemy(enemy);
}

void GLWidget::createCoin() {
    qDebug() << "Creating coin";
    Entity* coin = new Entity(mdlCoin);
    qreal randomAngle = qrand() * 360; // set random position
    coin->position = QVector3D(cos(randomAngle * M_PI / 180) * EnemySpawnDistance, sin(randomAngle * M_PI / 180) * EnemySpawnDistance, 0.0);
    coin->velocity = -coin->position.normalized() * 7; // go towards the center
    coins.append(coin);
}

void GLWidget::initializeGL ()
{
    glClearColor(0.8f, 0.7f, 0.8f, 1.0f);
    // create and set shaders
    QGLShaderProgram *program = new QGLShaderProgram(this);
    program->addShaderFromSourceFile(QGLShader::Fragment, "data/shaders/fshader.glsl");
    program->addShaderFromSourceFile(QGLShader::Vertex, "data/shaders/vshader.glsl");
    program->link();
    mdlMonkey->setShaderProgram(program);
    mdlCannon->setShaderProgram(program);
    mdlBullet->setShaderProgram(program);
    mdlBox->setShaderProgram(program);
    mdlNode->setShaderProgram(program);
    mdlHumanTankBody->setShaderProgram(program);
    mdlHumanTankTower->setShaderProgram(program);
    mdlEnemyTankBody->setShaderProgram(program);
    mdlEnemyTankTower->setShaderProgram(program);
    mdlCoin->setShaderProgram(program);
    mdlBall->setShaderProgram(program);
    //    if(!monkeyModel->setShaderFiles("fshader.glsl","vshader.glsl")) {
    //        qDebug() << "Failed to set shader files.";
    //    }
    //    if(!cannonModel->setShaderFiles("fshader.glsl","vshader.glsl")) {
    //        qDebug() << "Failed to set shader files.";
    //    }
    //    if(!bulletModel->setShaderFiles("fshader.glsl","vshader.glsl")) {
    //        qDebug() << "Failed to set shader files.";
    //    }
    //    if(!boxModel->setShaderFiles("fshader.glsl","vshader.glsl")) {
    //        qDebug() << "Failed to set shader files.";
    //    }
    // end shaders
    // create and set textures
    GLuint texFur;
    glGenTextures(1, &texFur);
    texFur = bindTexture(QImage("data/textures/fur.resized.jpg"));
    GLuint texMetal;
    glGenTextures(1, &texMetal);
    texMetal = bindTexture(QImage("data/textures/metal.small.jpg"));
    GLuint texGolden;
    glGenTextures(1, &texGolden);
    texGolden = bindTexture(QImage("data/textures/golden.jpg"));
    GLuint texArmy;
    glGenTextures(1, &texArmy);
    texArmy = bindTexture(QImage("data/textures/army-texture.png"));
    GLuint yellowArmyTexture;
    glGenTextures(1, &yellowArmyTexture);
    yellowArmyTexture = bindTexture(QImage("data/textures/army-texture-yellow.png"));
    GLuint texBlue;
    glGenTextures(1, &texBlue);
    texBlue = bindTexture(QImage("data/textures/bluetex.png"));
    mdlBox->setTexture(texFur);
    mdlMonkey->setTexture(texFur);
    mdlCannon->setTexture(texMetal);
    mdlBullet->setTexture(texMetal);
    mdlHumanTankBody->setTexture(texArmy);
    mdlHumanTankTower->setTexture(texArmy);
    mdlEnemyTankBody->setTexture(yellowArmyTexture);
    mdlEnemyTankTower->setTexture(yellowArmyTexture);
    mdlCoin->setTexture(texGolden);
    mdlBall->setTexture(texBlue);
    // end textures

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    this->ui = new Ui(this);
    //    Window* menu = new Window(ui,0,0,0.2,0.3,Window::Center,true,"Menu");
    //    new Cbutton(menu,QPointF(0.015,0.05),"New game");
    resetGame();

}

void GLWidget::paintGL()
{
    qreal currentFrameTime = gametime.elapsed() / 1000.0;
    qreal dt = currentFrameTime - lastFrameTime;
    // general physics
    QAccelerometerReading *reading = accelerometer->reading();
    gravity = QVector3D(-reading->x(), -reading->y(), 0);
    QVector3D ballAcceleration;
    if(!gameOver) { // do logic
        // ball forces
        qreal stretch = (ball->position.length() - 6.0);
        QVector3D ballSpringForce;
        if(stretch > 0) {
            ballSpringForce = - SpringConstant * stretch * ball->position.normalized();
        }
        QVector3D damping = -ball->velocity * 0.3;
        QVector3D ballForce = gravity + ballSpringForce + damping;
        ballAcceleration = ballForce / ball->mass;

        ball->velocity = ball->velocity + ballAcceleration * dt;

        // Calculate positions
        foreach(Entity* coin, coins) {
            coin->move(dt);
            if((ball->position - coin->position).lengthSquared() < 5) {
                qDebug() << "Removing coin caught";
                coins.removeAll(coin);
                createCoin();
                score += 100;
            }
            if(coin->position.lengthSquared() > 500) {
                qDebug() << "Removing coin too far out" << coin->position.lengthSquared();
                coins.removeAll(coin);
                createCoin();
            }
        }


        ball->move(dt);

    } // endif gameover
    //    }


    QPainter painter;
    painter.begin(this);
    painter.beginNativePainting();
    painter.setRenderHint(QPainter::HighQualityAntialiasing, true);

    glClearColor(0.88f, 0.88f, 0.9f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    glFrontFace(GL_CW);
    glCullFace(GL_FRONT);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    mainModelView = QMatrix4x4(); // reset
    // set up the main view (affects all objects)
    mainModelView.perspective(40.0, aspectRatio, 1.0, 150.0);
    mainModelView.lookAt(camera + offset,QVector3D(0,0,0) + offset,QVector3D(0.0,1.0,0.0));

    foreach(Entity* coin, coins) {
        coin->draw(mainModelView, lightPos);
    }

    ball->draw(mainModelView, lightPos);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    painter.endNativePainting();

    painter.setPen(Qt::blue);
    QString framesPerSecond;
    framesPerSecond.setNum(frames /(frametime.elapsed() / 1000.0), 'f', 2);
    painter.drawText(20, 40, framesPerSecond + " fps");
    painter.drawText(width() - 200, 60, "score: " + QString::number(score));
    painter.drawLine(project(QVector3D(0,0,0)), project(ball->position));

    if(gameOver) {
        QFont font;
        font.setPixelSize(height() / 4);
        painter.setFont(font);
        painter.drawText(QRectF(width() / 4, height() / 4, width() / 2, height() / 2),Qt::AlignCenter,tr("Game\nOver!"));
    }
    //    painter.drawText(20,80,"Verts: " + QString::number(cannon->model->vertices[20]));
    //    painter.drawText(20, 80, "Verts: " + QString::number(cannon->vertices.first().x()));


    painter.end();


    swapBuffers();

    if (!(frames % 100)) {
        frametime.start();
        frames = 0;
    }
    frames ++;
    lastFrameTime = currentFrameTime;
}

void GLWidget::regenerateNodes() {
    nodes.clear();
    nodeNeighbors.clear();
    for(int i = -MapSize; i < MapSize; i++) {
        for(int j = -MapSize; j < MapSize; j++) {
            bool tooClose = false;
            QVector3D position((qreal)i * NodeSize, (qreal)j * NodeSize, 0);
            foreach(Entity* building, buildings){
                if((building->position - position).lengthSquared() < NodeSizeSquared) {
                    tooClose = true;
                }
            }
            if(!tooClose) {
                Node* node = new Node();
                node->position = position;
                nodes.append(node);
            }
        }
    }
    foreach(Node* node, nodes) {
        QList<Node*> neighbors;
        foreach(Node* possibleNeighbor, nodes) {
            if((node->position - possibleNeighbor->position).lengthSquared() <= NodeSizeDiagonal && possibleNeighbor != node) { // if the distance between nodes are 1x1, a diagonal nodeneighbor will be x^2 = 1^2 + 1^2 away (Pythagoras)
                neighbors.append(possibleNeighbor);
            }
        }
        nodeNeighbors.insert(node, neighbors);
    }
}

QList<Node*> GLWidget::findPath(Node* startNode, Node* goalNode, QList<Node*> avoid) {
    // bug fix - if one of the nodes for some strange reason does not exist (has been seen happening) - use a completely different node
    if(!nodes.contains(startNode)) {
        startNode = nodes.first();
    }
    if(!nodes.contains(goalNode)) {
        goalNode = nodes.first();
    }
    // end bug fix
    qDebug() << "Finding path from" << startNode->position << "to" << goalNode->position;
    // che
    QList<Node*> closedSet;
    QList<Node*> openSet;
    QHash<Node*, qreal> gscore;
    QHash<Node*, qreal> hscore;
    QHash<Node*, qreal> fscore;
    QHash<Node*, Node*> cameFrom; // 1st came from 2nd parameter
    openSet.append(startNode);
    gscore.insert(startNode, 0);
    hscore.insert(startNode, (goalNode->position - startNode->position).lengthSquared());
    fscore.insert(startNode, (goalNode->position - startNode->position).lengthSquared());
    while (openSet.count() > 0) {
        Node* x;
        qreal lowestFScore = 0;
        bool firstFScore = true;
        foreach(Node* node, openSet) {
            qreal curFscore = fscore.value(node);
            if(curFscore < lowestFScore || firstFScore) {
                x = node;
                lowestFScore = curFscore;
                firstFScore = false;
            }
        }
        if(x == goalNode) {
            // reconstruct path
            QList<Node*> path;
            Node* currentNode = goalNode; // start at the goal
            //            path.prepend(goalPosition); // this is no longer legal - we can't be where there are no nodes
            while(currentNode != startNode) { // if we're not there yet
                path.prepend(currentNode); // add the current node's position to the beginning of the list
                currentNode = cameFrom.value(currentNode); // find out where this node came from
            }
            //            path.prepend(startNode); // always add the startnode to begin with

            //            path.prepend(startPosition); // this is completely unecessary! We're already there!
            return path; // return our path
        }
        openSet.removeAll(x);
        closedSet.append(x);
        foreach(Node* y, nodeNeighbors.value(x)) {
            if(closedSet.contains(y) || avoid.contains(y)) { // if in closed set or we should avoid this node
                continue;
            }
            qreal tentativeGScore = 0;
            bool tentativeIsBetter = false;
            tentativeGScore = gscore[x] + (x->position - y->position).lengthSquared();
            if(!openSet.contains(y) && !closedSet.contains(y)) {
                openSet.append(y);
                tentativeIsBetter = true;
            } else if(tentativeGScore < gscore.value(y)) {
                tentativeIsBetter = true;
            }
            if(tentativeIsBetter) {
                cameFrom.insert(y, x);
                gscore.insert(y, tentativeGScore);
                hscore.insert(y, (goalNode->position - y->position).lengthSquared());
                fscore.insert(y, gscore[y] + hscore[y]);
            }
        }
    }
    QList<Node*> nonfunctional;
    nonfunctional.append(startNode);
    qDebug() << "Path not found!";
    return nonfunctional; // we failed to find a path, just return the point we're at
}
QPoint GLWidget::project(QVector3D position) {
    position =  mainModelView * position;
    qreal winX = width() * (position.x() + 1) / 2;
    qreal winY = height() - height() * (position.y() + 1) / 2;
    return QPoint((int)winX, (int)winY);
}

QVector3D GLWidget::unProject(int x, int y) {

    // project click down to plane
    // about the mathematics: http://www.opengl.org/sdk/docs/man/xhtml/gluUnProject.xml
    // mainModelView should be our modelview projection matrix
    QMatrix4x4 inv = mainModelView.inverted();
    qreal coordx = 2*(qreal) x / (qreal) width() - 1;
    qreal coordy = 2*(qreal) (height() - y) / (qreal) height() - 1;
    QVector4D nearPoint4 = inv * QVector4D(coordx, coordy, -1, 1); // winZ = 2 * 0 - 1 = -1
    QVector4D farPoint4 = inv * QVector4D(coordx, coordy, 1, 1); // win> = 2 * 1 - 1 = 1
    if(nearPoint4.w() == 0.0) {
        return QVector3D();
    }
    qreal w = 1.0/nearPoint4.w();
    QVector3D nearPoint = QVector3D(nearPoint4);
    nearPoint *= w;
    w = 1.0/farPoint4.w();
    QVector3D farPoint = QVector3D(farPoint4);
    farPoint *= w;
    QVector3D dir = farPoint - nearPoint;
    if (dir.z()==0.0) // if we are looking in a flat direction we won't hit the ground
        return QVector3D(0,0,0);

    qreal t = - nearPoint.z() / dir.z(); // how long it is to the ground
    QVector3D cursor = nearPoint + dir * t;
    return cursor;
}

Node* GLWidget::closestNode(QVector3D position) {
    // first we find the closest node to us
    qreal lowestDistance = 0;
    Node* foundNode;
    bool firstStartDistance = true;
    foreach(Node* node, nodes) {
        qreal startDistance = (node->position - position).lengthSquared();
        if(startDistance < lowestDistance || firstStartDistance) {
            lowestDistance = startDistance;
            foundNode = node;
            firstStartDistance = false;
        }
    }
    return foundNode;
}

// Suggested mouse/finger interactions:
//        select several units: Press and hold for one second, then drag
//   imp  select unit: tap (less than one second)
//   imp  move or attack: tap (after selecting own unit(s)) on ground or enemy
//   imp  move map: drag and drop anywhere, just don't hold finger down first
//        deselect: press and hold for one second
void GLWidget::mousePressEvent(QMouseEvent *event)
{
    ui->convertMousePos(event->x(),event->y());
    if (event->button() == Qt::LeftButton) {
        if (ui->mouseClick()) {
            inUi=true;
            return;
        }
        inUi=false;

        if(gameOver) { // make sure we have had the game over text shown for 1.5 seconds
            qDebug() << lastFrameTime - gameOverTime;
            if(lastFrameTime - gameOverTime > 1.5) {
                resetGame();
            }
            return;
        }
        holdtime.restart();

        pressCursor = unProject(event->x(), event->y());
        dragCursor = pressCursor;
        dragStartPosition = event->pos();
        pressOffset = offset;
        testUnit->position = unProject(event->x(), event->y());
        lastDragOffset = offset;
    }
}
// Dragging events
void GLWidget::mouseMoveEvent(QMouseEvent* event) {
    ui->convertMousePos(event->x(),event->y());

    if (event->buttons() & Qt::LeftButton) {
        if (inUi) {
            ui->move();
            return;
        }
        // this should be improved. This method is not accurate.
        QVector3D currentCursor = unProject(event->x(), event->y());
        QVector3D currentDragOffset = offset;
        if(dragging) {
            offset -= (currentCursor - dragCursor) - (currentDragOffset - lastDragOffset); // offset is negative to get the "drag and drop"-feeling
            lastDragOffset = currentDragOffset;
            dragCursor = currentCursor;
        } else {
            if(holdtime.elapsed() > 1000) { // TODO: selection mode

            } else if((QVector3D(dragStartPosition) - QVector3D(event->pos())).length() > DragDropTreshold) { // if we have been dragging for more than ten pixels
                dragging = true;
            }
        }
    }
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event) {
    ui->convertMousePos(event->x(),event->y());
    if (event->button() == Qt::LeftButton) {
        if (inUi) {
            ui->mouseRelease();
        }
        dragging = false;
    }
}

void GLWidget::keyPressEvent(QKeyEvent *event) {
    qDebug() << "Key event" << event->key() << Qt::Key_S;
    switch(event->key()) {
    case Qt::Key_S:
        useSound = !useSound;
        break;
    case Qt::Key_H:
        drawHud = !drawHud;
        break;
    }
}

void GLWidget::recruitUnit(/*location ? */) {
    /*if (enoughCash)*/  //one of your buildings. For now, just build cannons.
    if (recruitqueue == 0) {
        recruittime.restart();
    }

    recruitqueue++;
    //cash-= price;
}

void GLWidget::resizeGL(int width, int height) {
    aspectRatio = (qreal) width / (qreal) height;
    ui->resize();
}
