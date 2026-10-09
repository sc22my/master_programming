//////////////////////////////////////////////////////////////////////
//
//  University of Leeds
//  COMP 5812M Foundations of Modelling & Rendering
//  User Interface for Coursework
////////////////////////////////////////////////////////////////////////


#include <limits>
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
#include "Homogeneous4.h"

#define N_THREADS 16

// shortcuts. TODO: move to mathlib
#define LERP(p1, p2, t) (t*p1) + ((1-t)*p2) 

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

    // Allocate buffers
    AllocateBuffers();
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

    // Set model view matrix. TEMP: ortho only
    renderParameters->modelviewMatrix = renderParameters->rotationMatrix;

    // Calculate MVP matrix
    m_MVP = m_Projection * renderParameters->modelviewMatrix;

    // Project control points
    for (int i = 0; i < patchControlPoints->vertices.size(); i++) {
        Homogeneous4 pt = patchControlPoints->vertices[i];
        pt.w = 1;
        m_ProjectedPts[i] = m_MVP * pt;
    }

    // Create lines for patch
    int count = 0;
    for (int y = 0; y < 4; y++) {
        m_PatchLines[count + 0] = m_ProjectedPts[(y*4) + 0];
        m_PatchLines[count + 1] = m_ProjectedPts[(y*4) + 1];
        m_PatchLines[count + 2] = m_ProjectedPts[(y*4) + 1];
        m_PatchLines[count + 3] = m_ProjectedPts[(y*4) + 2];
        m_PatchLines[count + 4] = m_ProjectedPts[(y*4) + 2];
        m_PatchLines[count + 5] = m_ProjectedPts[(y*4) + 3];
        count += 6;
    }

    for (int x = 0; x < 4; x++) {
        m_PatchLines[count + 0] = m_ProjectedPts[0  + x];
        m_PatchLines[count + 1] = m_ProjectedPts[4  + x];
        m_PatchLines[count + 2] = m_ProjectedPts[4  + x];
        m_PatchLines[count + 3] = m_ProjectedPts[8  + x];
        m_PatchLines[count + 4] = m_ProjectedPts[8  + x];
        m_PatchLines[count + 5] = m_ProjectedPts[12 + x];
        count += 6;
    }

    // Render points
    if(renderParameters->verticesEnabled) {
        // Draw inactive colour first
        DrawPoints(
            m_ProjectedPts.data(),
            m_ProjectedPts.size(),
            RGBAValue(191.f, 191.f, 191.f, 255.f));

        // Draw over the points. TODO: test for speedup without overdraw
        DrawPoints(
            m_ProjectedPts.data() + renderParameters->activeVertex,
            1,
            RGBAValue(255.f, 0.f, 0.f, 255.f));
    }

    if(renderParameters->planesEnabled) {
        // Planes are axis aligned grids made up of lines

        // Draw the vertical x axis plane (in purple)

        // Draw the vertical y axis plane (in blue)

        // Draw the flat plane (in brown)

        // Refer to RenderWidget.cpp for the precise colours.

    }

    if(renderParameters->netEnabled) {
        DrawLines(m_PatchLines.data(),
            m_PatchLines.size() / 2,
            RGBAValue(200.f, 200.f, 200.f, 255.f));
    }


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

void BezierPatchRenderWidget::AllocateBuffers() {
    // Allocate space for control points
    m_ProjectedPts = std::vector<Homogeneous4>(patchControlPoints->vertices.size());

    // Allocate enough for 3 grids
    m_GridLines = std::vector<Homogeneous4>(m_LinesPerGrid * 3);

    // lines connecting 4x4 points on 2 axes
    m_PatchLines = std::vector<Homogeneous4>(24 * 2);
}

void BezierPatchRenderWidget::DrawPoints(Homogeneous4* points, unsigned int numpts, RGBAValue color) {
    int ptsizehalf = m_PointSize << 1;

    // TODO: parallelise
    for (int i = 0; i < numpts; i++) {
        Homogeneous4 pt = points[i];
    
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
                    frameBuffer[y][x] = color;
            }
        }
    }
}

void BezierPatchRenderWidget::DrawLines(Homogeneous4* points, unsigned int numlines, RGBAValue color) {
    // Clip lines agains ndc bounds
    for (int i = 0; i < numlines << 1; i += 2) {
        // Clip against axes. TODO: maybe unroll
        for (int axis = 0; axis < 3; axis++) {
            ClipAxial(points + i, axis, 1, 1);
            ClipAxial(points + i, axis, -1, -1);
        }

        // Convert into pixel space
        int p0fbx = ((points + i + 0)->x + 1) * 0.5 * frameBuffer.width;
        int p0fby = ((points + i + 0)->y + 1) * 0.5 * frameBuffer.height;
        int p1fbx = ((points + i + 1)->x + 1) * 0.5 * frameBuffer.width;
        int p1fby = ((points + i + 1)->y + 1) * 0.5 * frameBuffer.height;

        // Use bresenham's algorithm to draw the line
        if (std::abs(p1fby - p0fby) < std::abs(p1fbx - p0fbx)) {
            if (p0fbx < p1fbx)
                DrawBresenhamHoriz(p0fbx, p0fby, p1fbx, p1fby, color);
            else
                DrawBresenhamHoriz(p1fbx, p1fby, p0fbx, p0fby, color);
        } else {
            if (p0fby < p1fby)
                DrawBresenhamVert(p0fbx, p0fby, p1fbx, p1fby, color);
            else
                DrawBresenhamVert(p1fbx, p1fby, p0fbx, p0fby, color);
        }
    }
}

bool BezierPatchRenderWidget::ClipAxial(Homogeneous4* points, int axis, float d, int inside) {
    // distances of points to clip plane
    // and which side it is on
    // diff: distance between pts on axis
    float d1, d2, side, diff;

    d1 = (points[0][axis] - d) * inside;
    d2 = (points[1][axis] - d) * inside;
    side = d1 * d2;
    diff = d1 - d2;

    // Discard outside
    if (side > 0 && d1 < 0) return true;

    // Clip if necessarry by using lerp
    if (side < 0) {
        Homogeneous4 newpt = LERP(points[0], points[1], d1/diff);

        if (d1 < 0) {
            points[0] = newpt;
        } else {
            points[1] = newpt;
        }

        return false;
    } else {
        // Line fully inside plane, no need to clip
        return false;
    }
}

void BezierPatchRenderWidget::DrawBresenhamHoriz(int x0, int y0, int x1, int y1, RGBAValue color) {
    int dx = x1 - x0;
    int dy = y1 - y0;

    int dir = 1;
    if (dy < 0) {
        dir = -1;
        dy = -dy;
    }

    int D = (2 * dy) - dx;
    int y = y0;
    for (int x = x0; x < x1; x++) {
        frameBuffer[y][x] = color;

        if (D > 0) {
            y = y + dir;
            D = D + (2 * (dy - dx));
        } else {
            D = D + 2*dy;
        }
    }
}

void BezierPatchRenderWidget::DrawBresenhamVert(int x0, int y0, int x1, int y1, RGBAValue color) {
    int dx = x1 - x0;
    int dy = y1 - y0;

    int dir = 1;
    if (dx < 0) {
        dir = -1;
        dx = -dx;
    }

    int D = (2 * dx) - dy;
    int x = x0;
    for (int y = y0; y < y1; y++) {
        frameBuffer[y][x] = color;

        if (D > 0) {
            x = x + dir;
            D = D + (2 * (dx - dy));
        } else {
            D = D + 2*dx;
        }
    }
}