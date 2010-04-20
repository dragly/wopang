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

#include "model.h"
#include <QtOpenGL>
#include <QPainter>

Entity::Entity(Model *model) {
    scale = QVector3D(1,1,1);
    position = QVector3D(0,0,0);
    velocity = QVector3D(0,0,0);
    rotation = QVector3D(0,0,0);
    this->model = model;
}

void Entity::setModel(Model *model) {
    this->model = model;
}

void Entity::draw(QMatrix4x4 modelview) {
    modelview.translate(position);
    modelview.rotate(rotation.x(), 1, 0, 0);
    modelview.rotate(rotation.y(), 0, 1, 0);
    modelview.rotate(rotation.z(), 0, 0, 1);
    modelview.scale(scale);
    model->draw(modelview);
}

Model::Model(QString filename) {
    texture = 0;
    vertexAttr = 0;
    normalAttr = 0;
    matrixUniform = 0;
    texCoordAttr = 0;
    textureUniform = 0;
    load(filename);
}

// Model functions
void Model::load(QString filename) {
    model = glmReadOBJ(filename.toLatin1().data());
    if(model->numtexcoords < 1) {
        qWarning() << "Missing UV map.";
    }
    GLMgroup* group;
    group = model->groups;
    groups.clear();
    while (group) {
        ModelGroup modelGroup;
        modelGroup.triangles.clear();
        for(int i = 0; i < group->numtriangles; i++) {
            ModelTriangle triangle;
            QVector<QVector3D> verts;
            for(int j = 0; j < 3; j++) {
                QVector3D vector(model->vertices[3 * model->triangles[group->triangles[i]].vindices[j] + 0],
                                 model->vertices[3 * model->triangles[group->triangles[i]].vindices[j] + 1],
                                 model->vertices[3 * model->triangles[group->triangles[i]].vindices[j] + 2]);
                verts.append(vector);
            }
            QVector<QVector3D> norms;
            for(int j = 0; j < 3; j++) {
                QVector3D vector(model->normals[3 * model->triangles[group->triangles[i]].nindices[j] + 0],
                                 model->normals[3 * model->triangles[group->triangles[i]].nindices[j] + 1],
                                 model->normals[3 * model->triangles[group->triangles[i]].nindices[j] + 2]);
                norms.append(vector);
            }
            if(model->numtexcoords > 0) {
                QVector<QVector3D> texs;
                for(int j = 0; j < 3; j++) {
                    QVector3D vector(model->texcoords[2 * model->triangles[group->triangles[i]].tindices[j] + 0],
                                     model->texcoords[2 * model->triangles[group->triangles[i]].tindices[j] + 1],
                                     model->texcoords[2 * model->triangles[group->triangles[i]].tindices[j] + 2]);
                    texs.append(vector);
                }
                triangle.texcoords = texs;
            }
            triangle.vertices = verts;
            triangle.normals = norms;
            modelGroup.triangles.append(triangle);
        }
        groups.append(modelGroup);
        group = group->next;
    }
    qDebug() << "loading file";
}
bool Model::setShaderFiles(QString fragmentShader, QString vertexShader) {
    if(!setFragmentShaderFile(fragmentShader)) {
        qDebug() << "Model::setShaderFiles(): Failed to set fragment shader" << fragmentShader;
        return false;
    }
    if(!setVertexShaderFile(vertexShader)) {
        qDebug() << "Model::setShaderFiles(): Failed to set vertex shader" << vertexShader;
        return false;
    }
    if(!linkShaderProgram()) {
        qDebug() << "Model::setShaderFiles(): Failed to link shader program";
        return false;
    }
    if(!initShaderProgram()) {
        qDebug() << "Model::setShaderFiles(): Failed to init shader program";
        return false;
    }
    return  true;
}
bool Model::setFragmentShaderFile(QString filename) {
    if(!program.addShaderFromSourceFile(QGLShader::Fragment, filename)) {
        qWarning() << "Could not load shader file " + filename + ": " << program.log();
        return false;
    }
    return true;
}
bool Model::setVertexShaderFile(QString filename) {
    if(!program.addShaderFromSourceFile(QGLShader::Vertex, filename)) {
        qWarning() << "Could not load shader file " + filename + ": " << program.log();
        return false;
    }
    return true;
}
bool Model::linkShaderProgram() {
    if(!program.link()) {
        qWarning() << "Failed to link program:" << program.log();
        return false;
    }
    return true;
}

bool Model::initShaderProgram() {
    vertexAttr = program.attributeLocation("vertex");
    normalAttr = program.attributeLocation("normal");
    texCoordAttr = program.attributeLocation("texCoord");
    matrixUniform = program.uniformLocation("matrix");
    textureUniform = program.uniformLocation("tex");
    if(!texCoordAttr) {
        qDebug() << "Failed to find texCoordAttr";
        return false;
    }
    if(!normalAttr) {
        qDebug() << "Failed to find normalAttr";
        return false;
    }
    return true;
}

void Model::draw(QMatrix4x4 modelview) {
    program.bind();
    program.setUniformValue(matrixUniform, modelview);
    glBindTexture(GL_TEXTURE_2D, texture);
    foreach(ModelGroup grp, groups) {
        foreach(ModelTriangle triangle, grp.triangles) {
            program.setAttributeArray(vertexAttr, triangle.vertices.constData());
            program.setAttributeArray(texCoordAttr, triangle.texcoords.constData());
            program.setAttributeArray(normalAttr, triangle.normals.constData());
            program.setUniformValue(textureUniform, 0);    // use texture unit 0
            program.enableAttributeArray(vertexAttr);
            program.enableAttributeArray(normalAttr);
            program.enableAttributeArray(texCoordAttr);
            glDrawArrays(GL_TRIANGLES, 0, triangle.vertices.size());
            program.disableAttributeArray(vertexAttr);
            program.disableAttributeArray(normalAttr);
            program.disableAttributeArray(texCoordAttr);
        }
    }
    program.release();
}
void Model::setTexture(GLuint texture) {
    this->texture = texture;
}