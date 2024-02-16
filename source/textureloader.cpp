#include "textureloader.h"

TextureLoader::TextureLoader()
{
}

GLuint TextureLoader::LoadTexture(const QString & file)
{

    int w, h;
    return LoadTexture(file, w, h);

}

GLuint TextureLoader::LoadTexture(const QString & file, int & width, int & height)
{

    QImage buf;

    if (buf.load(file)) {
        qDebug() << "\t Succesfully loaded image" << file;
    }
    else {
        qDebug() << "!!! Failed to load image" << file;
    }

    QImage tex1 = QGLWidget::convertToGLFormat( buf );

    GLuint texInd;

    glGenTextures(1, &texInd);
    glBindTexture(GL_TEXTURE_2D, texInd);

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, 4, tex1.width(), tex1.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, tex1.bits());

    glBindTexture(GL_TEXTURE_2D, 0);

    width = tex1.width();
    height = tex1.height();

    return texInd;

}
