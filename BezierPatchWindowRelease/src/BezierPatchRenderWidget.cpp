//////////////////////////////////////////////////////////////////////
//
//  University of Leeds
//  COMP 5812M Foundations of Modelling & Rendering
//  User Interface for Coursework
////////////////////////////////////////////////////////////////////////


#include <math.h>
#include <thread>
#include <random>
#include <QTimer>
// include open mp for the advanced tasks
#include <omp.h>
#include <algorithm>

// include the header file
#include "BezierPatchRenderWidget.h"

#include <QElapsedTimer>
#include <GL/glut.h>
#include <GL/gl.h>

#include <chrono>
#include <thread>

//	Ken Shoemake's ArcBall
#include "ArcBall.h"

#define N_THREADS 16

// constructor
BezierPatchRenderWidget::BezierPatchRenderWidget (
        // the Bezier patch control points to show
        ControlPoints       *newPatchControlPoints,
        // the render parameters to use
        RenderParameters    *newRenderParameters,
        // parent widget in visual hierarchy
        QWidget             *parent
        )
    // the : indicates a member initialiser list ...
    // ... it is good practice to use it where possible
    : 
    // start by calling inherited constructor with parent widget's pointer
    QOpenGLWidget(parent),
    // then store the pointers that were passed in
    patchControlPoints(newPatchControlPoints),
    renderParameters(newRenderParameters)
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &BezierPatchRenderWidget::forceRepaint);
    timer->start(30);

    // Initialise projection matrix with default value
    CalculateProjectionOrtho(1, 1, 1, 1, 1, 1);

    // Allocate space for point cache
    m_Scratchpad = std::vector<Homogeneous4>(m_ScratchpadSize);
}

void BezierPatchRenderWidget::forceRepaint(){
    update();
}

// destructor
BezierPatchRenderWidget::~BezierPatchRenderWidget() {
    // empty (for now)
    // all of our pointers are to data owned by another class
    // so we have no responsibility for destruction
    // and OpenGL cleanup is taken care of by Qt
}

// called when OpenGL context is set up
void BezierPatchRenderWidget::initializeGL()
    { // BezierPatchRenderWidget::initializeGL()
    } // BezierPatchRenderWidget::initializeGL()

// called every time the widget is resized
void BezierPatchRenderWidget::resizeGL(int w, int h) {
    // resize the render image
    frameBuffer.Resize(w, h);

    // Updare render parameters
    renderParameters->windowWidth = w;
    renderParameters->windowHeight = h;

    // Recalculate projection matrix
    float aspect = (float)w / (float)h;
    
    if (aspect > 1) {
        CalculateProjectionOrtho(
                -aspect * 10/renderParameters->zTranslate,
                aspect * 10/renderParameters->zTranslate,
                -10/renderParameters->zTranslate,
                10/renderParameters->zTranslate,
                0.01,
                200.0);
    } else {
        CalculateProjectionOrtho(
            -10/renderParameters->zTranslate,
            10/renderParameters->zTranslate,
            -aspect * 10/renderParameters->zTranslate,
            aspect * 10/renderParameters->zTranslate,
            0.01,
            200.0);
    }
}


// called every time the widget needs painting
void BezierPatchRenderWidget::paintGL() {
    // clear the (non-OpenGL) buffer where we will set pixels to:
    frameBuffer.clear(renderParameters->theClearColor);

    // now clear the OpenGL buffer:
    glClearColor(0.8, 0.8, 0.6, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);


    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            frameBuffer[i][j] = RGBAValue(255.f, 255.f, 255.f, 255.f);
        }
    }

    // Set model view matrix. TEMP: ortho only
    renderParameters->modelviewMatrix = renderParameters->rotationMatrix;

    // Calculate MVP matrix
    m_MVP = m_Projection * renderParameters->modelviewMatrix;

    // Render points
    if(renderParameters->verticesEnabled) {
        DrawPoints(patchControlPoints->vertices.data(), patchControlPoints->vertices.size());
    }

    if(renderParameters->planesEnabled) {

        // Planes are axis aligned grids made up of lines

        // Draw the vertical x axis plane (in purple)

        // Draw the vertical y axis plane (in blue)

        // Draw the flat plane (in brown)

        // Refer to RenderWidget.cpp for the precise colours.

    }

    if(renderParameters->netEnabled)
    {// UI control for showing the Bezier control net
     // (control points connected with lines)
    }// UI control for showing the Bezier control net


    if(renderParameters->bezierEnabled) {
        for (float s = 0.0; s <= 1.0; s += 0.01) {
            for (float t = 0.0; t <= 1.0; t += 0.01) {

            }
        }
    }

    // Put the custom framebufer on the screen to display the image
    glDrawPixels(frameBuffer.width, frameBuffer.height, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer.block);
}

// mouse-handling
void BezierPatchRenderWidget::mousePressEvent(QMouseEvent *event)
    { // BezierPatchRenderWidget::mousePressEvent()
    // store the button for future reference
    int whichButton = int(event->button());
    // scale the event to the nominal unit sphere in the widget:
    // find the minimum of height & width   
    float size = (width() > height()) ? height() : width();
    // scale both coordinates from that
    float x = (2.0f * event->x() - size) / size;
    float y = (size - 2.0f * event->y() ) / size;

    
    // and we want to force mouse buttons to allow shift-click to be the same as right-click
    unsigned int modifiers = event->modifiers();
    
    // shift-click (any) counts as right click
    if (modifiers & Qt::ShiftModifier)
        whichButton = Qt::RightButton;
    
    // send signal to the controller for detailed processing
    emit BeginScaledDrag(whichButton, x,y);
    } // BezierPatchRenderWidget::mousePressEvent()
    
void BezierPatchRenderWidget::mouseMoveEvent(QMouseEvent *event)
    { // BezierPatchRenderWidget::mouseMoveEvent()
    // scale the event to the nominal unit sphere in the widget:
    // find the minimum of height & width   
    float size = (width() > height()) ? height() : width();
    // scale both coordinates from that
    float x = (2.0f * event->x() - size) / size;
    float y = (size - 2.0f * event->y() ) / size;
    
    // send signal to the controller for detailed processing
    emit ContinueScaledDrag(x,y);
    } // BezierPatchRenderWidget::mouseMoveEvent()
    
void BezierPatchRenderWidget::mouseReleaseEvent(QMouseEvent *event)
    { // BezierPatchRenderWidget::mouseReleaseEvent()
    // scale the event to the nominal unit sphere in the widget:
    // find the minimum of height & width   
    float size = (width() > height()) ? height() : width();
    // scale both coordinates from that
    float x = (2.0f * event->x() - size) / size;
    float y = (size - 2.0f * event->y() ) / size;
    
    // send signal to the controller for detailed processing
    emit EndScaledDrag(x,y);
}

void BezierPatchRenderWidget::CalculateProjectionOrtho(float left, float right, float bottom, float top, float near, float far) {
    m_Projection.SetIdentity();

    float rangex = right - left;
    float rangey = top - bottom;
    float rangez = near - far;

    // Inverses
    float rangexinv = 1.f / rangex;
    float rangeyinv = 1.f / rangey;
    float rangezinv = 1.f / rangez;

    // X component
    m_Projection[0][0] = 2 * rangexinv;
    m_Projection[3][0] = -(right + left) * rangexinv;

    // Y component
    m_Projection[1][1] = 2 * rangeyinv;
    m_Projection[3][1] = -(top + bottom) * rangeyinv;

    // Z component
    m_Projection[2][2] = 2 * rangezinv;
    m_Projection[3][2] = -(near + far) * rangezinv;

    // Set w to 1
    m_Projection[3][3] = 1;
}

void BezierPatchRenderWidget::DrawPoints(Point3* points, unsigned int numpts) {
    int ptsizehalf = m_PointSize << 1;

    // TODO: parallelise
    for (int i = 0; i < numpts; i++) {
        Homogeneous4 pt(points[i]);
        pt.w = 1;

        // Project
        pt = m_MVP * pt;

        // Clip against ndc bounds
        if (pt.x < -1 || pt.x > 1) continue;
        if (pt.y < -1 || pt.y > 1) continue;
        if (pt.z < -1 || pt.z > 1) continue;

        // Get pixel space coords
        long fbx = (pt.x + 1) * 0.5 * frameBuffer.width;
        long fby = (pt.y + 1) * 0.5 * frameBuffer.height;

        // Fill in point
        long startx = std::max(fbx - ptsizehalf, 0l);
        long starty = std::max(fby - ptsizehalf, 0l);
        int endx = std::min(fbx + ptsizehalf, frameBuffer.width);
        int endy = std::min(fby + ptsizehalf, frameBuffer.height);

        int r2 = ptsizehalf * ptsizehalf;
        for (long y = starty; y < endy; y++) {
            for (long x = startx; x < endx; x++) {
                // Check circle radius
                int dy = y - fby;
                int dx = x - fbx;

                int d = (dy * dy) + (dx * dx);
                if (d <= r2)
                    frameBuffer[y][x] = RGBAValue(255.f, 255.f, 255.f, 255.f);
            }
        }
    }
}