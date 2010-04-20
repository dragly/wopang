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

#ifndef MODEL_H
#define MODEL_H

#include <QObject>
#include <QtOpenGL>
#include <QPainter>
#include <QPaintEngine>

#include "glm.h"

class ModelTriangle
{
public:
    QVector<QVector3D> vertices;
    QVector<QVector3D> normals;
    QVector<QVector3D> texcoords;
signals:

public slots:

};

class ModelGroup
{
public:
    QVector<ModelTriangle> triangles;
signals:

public slots:

};
class Model
{
public:
    Model(QString filename);

    void load(QString filename);
    void draw(QMatrix4x4 modelview);
    void setTexture(GLuint texture);
    bool setShaderFiles(QString fragmentShader, QString vertexShader);
    bool setVertexShaderFile(QString filename);
    bool setFragmentShaderFile(QString filename);
    bool linkShaderProgram();
    bool initShaderProgram();
private:
    // variables
    GLMmodel *model;
    GLuint texture;
    QGLShaderProgram program;
    QVector<QVector3D> vertices;
    QVector<QVector3D> normals;
    QVector<ModelGroup> groups;
    int vertexAttr;
    int normalAttr;
    int matrixUniform;
    int texCoordAttr;
    int textureUniform;
};

class Entity
{
public:
    Entity(Model *model);

    // functions

    void draw(QMatrix4x4 modelview);
//    void setTexture(GLuint texture);
//    bool setShaderFiles(QString fragmentShader, QString vertexShader);
//    bool setVertexShaderFile(QString filename);
//    bool setFragmentShaderFile(QString filename);
//    bool linkShaderProgram();
//    bool initShaderProgram();
    void setModel(Model *model);

    // variables
    QVector3D position;
    QVector3D velocity;
    QVector3D rotation;
    QVector3D scale;
private:
    Model *model;
};

#endif // MODEL_H