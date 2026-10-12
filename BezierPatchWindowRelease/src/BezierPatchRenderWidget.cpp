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

// Math library includes
#include "Sort.h"

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

    // TEMP: test quick sort works
    std::vector<float> nums = { 1, 5, 4, 3, -1, 10 };
    std::vector<int> ids = { 0, 1, 2, 3, 4, 5 };
    QuickSort::Sort(nums, [] (float value) { return value; }, ids);

    std::cout << "Sorted numbers: " << std::endl;
    for (int i = 0; i < nums.size(); i++) {
        std::cout << "i: " << i << ", num: " << nums[i] << ", id: " << ids[i] << std::endl;
    }
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
        if (renderParameters->orthoProjection) {
            CalculateProjectionOrtho(
                    -aspect * 10/renderParameters->zTranslate,
                    aspect * 10/renderParameters->zTranslate,
                    -10/renderParameters->zTranslate,
                    10/renderParameters->zTranslate,
                    0.01,
                    200.0);
        } else {
            CalculateProjectionPerspective(
                -aspect * 0.01,
                aspect * 0.01,
                -0.01,
                0.01,
                0.01,
                200.0);
        }
    } else {
        if (renderParameters->orthoProjection) {
            CalculateProjectionOrtho(
                -10/renderParameters->zTranslate,
                10/renderParameters->zTranslate,
                -aspect * 10/renderParameters->zTranslate,
                aspect * 10/renderParameters->zTranslate,
                0.01,
                200.0);
        } else {
            CalculateProjectionPerspective( -0.01, 0.01,
                      -aspect * 0.01, aspect * 0.01,
                      0.01, 200.0);
        }
    }
}


// called every time the widget needs painting
void BezierPatchRenderWidget::paintGL() {
    // clear the (non-OpenGL) buffer where we will set pixels to:
    frameBuffer.clear(renderParameters->theClearColor);

    // now clear the OpenGL buffer:
    glClearColor(0.8, 0.8, 0.6, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // Set model view matrix. Translate the camera back by reverse of 10 units
    Matrix4 cameratranslation;
    cameratranslation.SetTranslation(Vector3(0, 0, -10));
    renderParameters->modelviewMatrix = cameratranslation * renderParameters->rotationMatrix;

    // Calculate MVP matrix
    m_MVP = m_Projection * renderParameters->modelviewMatrix;

    // Clear the render queue
    ClearQueues();

    // Project control points
    for (int i = 0; i < patchControlPoints->vertices.size(); i++) {
        Homogeneous4 pt = patchControlPoints->vertices[i];
        pt.w = 1;
        m_ProjectedPts[i] = m_MVP * pt;
    }

    // Render points
    if(renderParameters->verticesEnabled) {
        // Draw inactive colour first
        for (unsigned int i = 0; i < m_ProjectedPts.size(); i++) {
            if (i == renderParameters->activeVertex)
                QueuePoint(m_ProjectedPts[i], RGBAValue(255.f, 0.f, 0.f, 255.f));
            else
                QueuePoint(m_ProjectedPts[i], RGBAValue(191.f, 191.f, 191.f, 255.f));
        }
    }

    if(renderParameters->planesEnabled) {
        // Project grid points
        ProjectGridPoints(m_GridPointsX.data(), 5 , 10.f, 0);
        ProjectGridPoints(m_GridPointsY.data(), 5 , 10.f, 1);
        ProjectGridPoints(m_GridPointsZ.data(), 10, 10.f, 2);

        // Submit lines
        // X grid
        for (unsigned int i = 0; i <= 5; i++) {
            for (int j = 0; j < 5; j++) {
                QueueLine(
                    m_GridPointsX[i * 6 + j    ],
                    RGBAValue(0.0f, 64.f, 0.f, 255.f),
                    m_GridPointsX[i * 6 + j + 1],
                    RGBAValue(0.0f, 64.f, 0.f, 255.f)
                );

                QueueLine(
                    m_GridPointsX[i + j * 6],
                    RGBAValue(0.0f, 64.f, 0.f, 255.f),
                    m_GridPointsX[i + j * 6 + 6],
                    RGBAValue(0.0f, 64.f, 0.f, 255.f)
                );
            }
        }

        // Y grid
        for (unsigned int i = 0; i <= 5; i++) {
            for (int j = 0; j < 5; j++) {
                QueueLine(
                    m_GridPointsY[i * 6 + j    ],
                    RGBAValue(64.f, 0.f, 64.f, 255.f),
                    m_GridPointsY[i * 6 + j + 1],
                    RGBAValue(64.f, 0.f, 64.f, 255.f)
                );

                QueueLine(
                    m_GridPointsY[i + j * 6],
                    RGBAValue(64.f, 0.f, 64.f, 255.f),
                    m_GridPointsY[i + j * 6 + 6],
                    RGBAValue(64.f, 0.f, 64.f, 255.f)
                );
            }
        }

        // Z grid, different size
        for (unsigned int i = 0; i <= 10; i++) {
            for (int j = 0; j < 10; j++) {
                QueueLine(
                    m_GridPointsZ[i * 11 + j    ],
                    RGBAValue(64.f, 64.f, 0.f, 255.f),
                    m_GridPointsZ[i * 11 + j + 1],
                    RGBAValue(64.f, 64.f, 0.f, 255.f)
                );

                QueueLine(
                    m_GridPointsZ[i + j * 11],
                    RGBAValue(64.f, 64.f, 64.f, 255.f),
                    m_GridPointsZ[i + j * 11 + 11],
                    RGBAValue(64.f, 64.f, 64.f, 255.f)
                );
            }
        }
    }

    if(renderParameters->netEnabled) {
        // Create lines for patch
        for (int y = 0; y < 4; y++) {
            QueueLine(
                m_ProjectedPts[(y*4) + 0], RGBAValue(0.f, 255.f, 0.f, 255.f),
                m_ProjectedPts[(y*4) + 1], RGBAValue(0.f, 255.f, 0.f, 255.f)
            );
            QueueLine(
                m_ProjectedPts[(y*4) + 1], RGBAValue(0.f, 255.f, 0.f, 255.f),
                m_ProjectedPts[(y*4) + 2], RGBAValue(0.f, 255.f, 0.f, 255.f)
            );
            QueueLine(
                m_ProjectedPts[(y*4) + 2], RGBAValue(0.f, 255.f, 0.f, 255.f),
                m_ProjectedPts[(y*4) + 3], RGBAValue(0.f, 255.f, 0.f, 255.f)
            );
        }

        for (int x = 0; x < 4; x++) {
            QueueLine(
                m_ProjectedPts[0 + x], RGBAValue(0.f, 255.f, 0.f, 255.f),
                m_ProjectedPts[4 + x], RGBAValue(0.f, 255.f, 0.f, 255.f)
            );
            QueueLine(
                m_ProjectedPts[4 + x], RGBAValue(0.f, 255.f, 0.f, 255.f),
                m_ProjectedPts[8 + x], RGBAValue(255.f, 255.f, 255.f, 255.f)
            );
            QueueLine(
                m_ProjectedPts[8 + x], RGBAValue(0.f, 255.f, 0.f, 255.f),
                m_ProjectedPts[12 + x], RGBAValue(0.f, 255.f, 0.f, 255.f)
            );
        }
    }


    if(renderParameters->bezierEnabled) {
        for (float s = 0.0; s <= 1.0; s += 0.01) {
            for (float t = 0.0; t <= 1.0; t += 0.01) {

            }
        }
    }

    // Render the queues. Draw lines first because it looks better
    DrawLines();
    DrawPoints();

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
    float rangez = far - near;

    // Inverses
    float rangexinv = 1.f / rangex;
    float rangeyinv = 1.f / rangey;
    float rangezinv = 1.f / rangez;

    // X component
    m_Projection[0][0] = 2 * rangexinv;
    m_Projection[0][3] = -(right + left) * rangexinv;

    // Y component
    m_Projection[1][1] = 2 * rangeyinv;
    m_Projection[1][3] = -(top + bottom) * rangeyinv;

    // Z component, negative because camera faces -z
    m_Projection[2][2] = -2 * rangezinv;
    m_Projection[2][3] = -(near + far) * rangezinv;

    // Set w to 1
    m_Projection[3][3] = 1;
}

void BezierPatchRenderWidget::CalculateProjectionPerspective(float left, float right, float bottom, float top, float near, float far) {
    m_Projection.SetIdentity();
    
    float rangex = right - left;
    float rangey = top - bottom;
    float rangez = far - near;

    // Inverses
    float rangexinv = 1.f / rangex;
    float rangeyinv = 1.f / rangey;
    float rangezinv = 1.f / rangez;

    float near2 = 2 * near;

    m_Projection[0][0] = near2 * rangexinv;
    m_Projection[0][2] = (right + left) * rangexinv;

    m_Projection[1][2] = near2 * rangey;
    m_Projection[1][2] = (top + bottom) * rangeyinv;
    
    m_Projection[2][2] = -(far + near) * rangezinv;
    m_Projection[2][3] = -2 * far * near * rangezinv;

    m_Projection[3][2] = -1;
    m_Projection[3][3] = 0;
}

void BezierPatchRenderWidget::AllocateBuffers() {
    // Allocate space for control points
    m_ProjectedPts = std::vector<Homogeneous4>(patchControlPoints->vertices.size());

    // Allocate grid points
    m_GridPointsX = std::vector<Homogeneous4>(36);  // 6x6 grid
    m_GridPointsY = std::vector<Homogeneous4>(36);  // 6x6 grid
    m_GridPointsZ = std::vector<Homogeneous4>(121); // 11x11 grid

    // Allocate grid lines
    m_GridLinesX = std::vector<Homogeneous4>(2048); // TODO: set correct size
    m_GridLinesY = std::vector<Homogeneous4>(2048); // TODO: set correct size
    m_GridLinesZ = std::vector<Homogeneous4>(2048); // TODO: set correct size

    // lines connecting 4x4 points on 2 axes
    m_PatchLines = std::vector<Homogeneous4>(24 * 2);
}

void BezierPatchRenderWidget::ClearQueues() {
    m_PointQueue.Clear();
    m_LineQueue.Clear();
}

void BezierPatchRenderWidget::DrawPoints() {
    int ptsizehalf = m_PointSize << 1;

    // Apply homogenious part
    // TODO: Parallel
    for (auto& pt : m_PointQueue.Positions) {
        if (std::abs(pt.w) > std::numeric_limits<float>::epsilon()) pt = pt / pt.w;
    }

    // Sort by depth
    QuickSort::Sort(m_PointQueue.Positions, [] (Homogeneous4& value) { return -value.z; }, m_PointQueue.Colors);

    // TODO: parallelise
    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < m_PointQueue.Positions.size(); i++) {
        Homogeneous4 pt = m_PointQueue.Positions[i];
    
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
                    frameBuffer[y][x] = m_PointQueue.Colors[i];
            }
        }
    }
}

void BezierPatchRenderWidget::DrawLines() {
    // Sort by start position vertex.
    QuickSort::Sort(
        m_LineQueue.StartPositions,
        [] (Homogeneous4& hom) { return -hom.z; },
        m_LineQueue.EndPositions,
        m_LineQueue.StartColors,
        m_LineQueue.EndColors
    );

    // Apply homogenious coords
    for (unsigned int i = 0; i < m_LineQueue.StartPositions.size(); i++) {
        if (std::abs(m_LineQueue.StartPositions[i].w) > std::numeric_limits<float>::epsilon())
            m_LineQueue.StartPositions[i] = m_LineQueue.StartPositions[i] / m_LineQueue.StartPositions[i].w;
        if (std::abs(m_LineQueue.EndPositions[i].w) > std::numeric_limits<float>::epsilon())
            m_LineQueue.EndPositions[i] = m_LineQueue.EndPositions[i] / m_LineQueue.EndPositions[i].w;
    }

    // Clip lines agains ndc bounds
    for (int i = 0; i < m_LineQueue.StartPositions.size(); i ++) {
        // Clip against axes. TODO: maybe unroll
        for (int axis = 0; axis < 3; axis++) {
            ClipAxial(i, axis, 1, 1);
            ClipAxial(i, axis, -1, -1);
        }

        // Convert into pixel space
        int p0fbx = (m_LineQueue.StartPositions[i].x + 1) * 0.5 * frameBuffer.width;
        int p0fby = (m_LineQueue.StartPositions[i].y + 1) * 0.5 * frameBuffer.height;
        int p1fbx = (m_LineQueue.EndPositions[i].x   + 1) * 0.5 * frameBuffer.width;
        int p1fby = (m_LineQueue.EndPositions[i].y   + 1) * 0.5 * frameBuffer.height;

        // Use bresenham's algorithm to draw the line
        // TODO: lerp the colours as well
        if (std::abs(p1fby - p0fby) < std::abs(p1fbx - p0fbx)) {
            if (p0fbx < p1fbx)
                DrawBresenhamHoriz(p0fbx, p0fby, p1fbx, p1fby, m_LineQueue.StartColors[i]);
            else
                DrawBresenhamHoriz(p1fbx, p1fby, p0fbx, p0fby, m_LineQueue.StartColors[i]);
        } else {
            if (p0fby < p1fby)
                DrawBresenhamVert(p0fbx, p0fby, p1fbx, p1fby, m_LineQueue.StartColors[i]);
            else
                DrawBresenhamVert(p1fbx, p1fby, p0fbx, p0fby, m_LineQueue.StartColors[i]);
        }
    }
}

void BezierPatchRenderWidget::ProjectGridPoints(Homogeneous4* buff, unsigned int numpts, float size, unsigned int axis) {
    float step = size / numpts;
    float start = -size/2;

    // which axes to write into
    unsigned int i1 = axis == 0 ? 1 : 0;
    unsigned int i2 = axis == 2 ? 1 : 2;

    // Loop through rows and columns inclusive
    int count = 0;
    for (int row = 0; row <= numpts; row++) {
        for (int column = 0; column <= numpts; column++) {
            float rowpos = start + (row * step);
            float colpos = start + (column * step);

            // Set correct points
            Homogeneous4 pos(0.f, 0.f, 0.f, 1.f);
            pos[i1] = rowpos;
            pos[i2] = colpos;

            buff[count] = m_MVP * pos;
            count++;
        }
    }
}

bool BezierPatchRenderWidget::ClipAxial(unsigned int lineindex, int axis, float d, int inside) {
    // distances of points to clip plane
    // and which side it is on
    // diff: distance between pts on axis
    float d1, d2, sameside, diff;

    Homogeneous4 p1 = m_LineQueue.StartPositions[lineindex];
    Homogeneous4 p2 = m_LineQueue.EndPositions[lineindex];

    d1 = (p1[axis] - d) * inside;
    d2 = (p2[axis] - d) * inside;
    sameside = d1 * d2;
    diff = d1 - d2;

    // Discard outside. plane normal faces outside ndc bounding box
    if (sameside > 0 && d1 > 0) return true;

    // Clip if necessarry by using lerp
    if (sameside < 0) {
        Homogeneous4 newpt = LERP(p2, p1, d1/diff);
        
        // TODO: lerp colours as well
        if (d1 > 0) {
            m_LineQueue.StartPositions[lineindex] = newpt;
        } else {
            m_LineQueue.EndPositions[lineindex] = newpt;
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