//////////////////////////////////////////////////////////////////////
//
//	University of Leeds
//	COMP 5812M Foundations of Modelling & Rendering
//	User Interface for Coursework
//
//	October, 2024
//
//  -----------------------------
//  Bezier Patch Render Widget
//  -----------------------------
//
//	Provides a widget that displays a fixed image
//	Assumes that the image will be edited (somehow) when Render() is called
//  
////////////////////////////////////////////////////////////////////////

// include guard
#ifndef BEZIER_PATCH_RENDER_WIDGET_H
#define BEZIER_PATCH_RENDER_WIDGET_H

#include <vector>

// include the relevant QT headers
#include <QOpenGLWidget>
#include <QMouseEvent>

// and include all of our own headers that we need
#include "ControlPoints.h"
#include "Homogeneous4.h"
#include "Point3.h"
#include "RGBAValue.h"
#include "RenderParameters.h"
#include "RGBAImage.h"


// TODO: Move to own file
// Structure of arrays implementation of vertex data.
// For this project, we are only interested in position and colour
struct VertexQueue {
	VertexQueue() {
		// Reserve a large space for pts
		Positions.reserve(2048);
		Colors.reserve(2048);
	}

	std::vector<Homogeneous4> Positions;
	std::vector<RGBAValue> Colors;

	void Clear() { Positions.clear(); Colors.clear(); }
};

// Queue structure for lines.
struct LineQueue {
	LineQueue() {
		// Reserve initial space.
		StartPositions.reserve(1024);
		StartColors.reserve(1024);
		EndPositions.reserve(1024);
		EndColors.reserve(1024);
	}

	std::vector<Homogeneous4> StartPositions;
	std::vector<RGBAValue>    StartColors;
	std::vector<Homogeneous4> EndPositions;
	std::vector<RGBAValue>    EndColors;

	void Clear() {
		StartPositions.clear();
		StartColors.clear();
		EndPositions.clear();
		EndColors.clear();
	}
};


// class for a render widget with arcball linked to an external arcball widget
class BezierPatchRenderWidget : public QOpenGLWidget {
	Q_OBJECT
private:
    // the Bezier patch control points to be rendered
    ControlPoints *patchControlPoints;

	// the render parameters to use
	RenderParameters *renderParameters;

    // An image to use as a framebuffer ...
    // ... that we will set individual pixels to
	RGBAImage frameBuffer;

	public:
	// constructor
    BezierPatchRenderWidget
			(
            // the Bezier patch control points to show
            ControlPoints 		*newPatchControlPoints,
			// the render parameters to use
			RenderParameters 	*newRenderParameters,
			// parent widget in visual hierarchy
			QWidget 			*parent
			);
	
	// destructor
    ~BezierPatchRenderWidget();
			
	protected:
	// called when OpenGL context is set up
	void initializeGL();
	// called every time the widget is resized
	void resizeGL(int w, int h);
	// called every time the widget needs painting
	void paintGL();

	// mouse-handling
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
private:
    void forceRepaint();

	// Sets the projection matrix to ortho.
	// Arguments behave same as glOrtho()
    void CalculateProjectionOrtho(float left, float right, float bottom, float top, float near, float far);

	// Sets projection matrix to perspective
	// Arguments behave same as glFrustum()
	void CalculateProjectionPerspective(float left, float right, float bottom, float top, float near, float far);

	// Allocates buffers for points lines etc
	void AllocateBuffers();

	// ======== BUFFERS ========

	// Holds projected points
	std::vector<Homogeneous4> m_ProjectedPts;

	// Grid points projected
	std::vector<Homogeneous4> m_GridPointsX;
	std::vector<Homogeneous4> m_GridPointsY;
	std::vector<Homogeneous4> m_GridPointsZ;

	// Holds grid line points
	std::vector<Homogeneous4> m_GridLinesX;
	std::vector<Homogeneous4> m_GridLinesY;
	std::vector<Homogeneous4> m_GridLinesZ;

	// Holds lines connecting control points
	std::vector<Homogeneous4> m_PatchLines;

    // ======== STATE ========
	// Or our "uniforms"

    // Projection matrix
    Matrix4 m_Projection;

	// Model view projection
	Matrix4 m_MVP;

	unsigned int m_PointSize = 1;

	// Keep track of how many frames we have executed
	unsigned long long m_NumFrames = 0;

	// ======== QUEUES ========
	// We use a queue system to batch draw all points

	// Clears the queues for recording
	void ClearQueues();

	// Queues a point to be drawn
	inline void QueuePoint(Homogeneous4 pos, RGBAValue color) { m_PointQueue.Positions.push_back(pos); m_PointQueue.Colors.push_back(color); };

	// Queues a line to be drawn
	inline void QueueLine(Homogeneous4 p1, RGBAValue c1, Homogeneous4 p2, RGBAValue c2) {
		m_LineQueue.StartPositions.push_back(p1);
		m_LineQueue.StartColors.push_back(c1);
		m_LineQueue.EndPositions.push_back(p2);
		m_LineQueue.EndColors.push_back(c2);
	}

	// The queue that holds our points
	VertexQueue m_PointQueue;

	// The queue that holds our lines
	LineQueue m_LineQueue;

	// ======== DRAWING PRIMITIVES ========

	// Draws points from a buffer
	void DrawPoints();

	// Draws lines from a buffer. lines arranged as [l0p0, l0p1, l1p0, l1p1, ...]
	void DrawLines();

	// Generates points to draw the bezier curve. Max allowed points is 4
	void DrawBezier(std::array<Homogeneous4, 4>& points, unsigned int resolution);

	// Evaluates the point at t. max points: 4
	Homogeneous4 EvaluateBezier(std::array<Homogeneous4, 4> points, unsigned int numpts, float t);

	// ======== UTIL FUNCTIONS ========

	// Projects a grid of points on the axis.
	// Buffer needs to be large enough to hold (numpts+1) ** 2 points
	// Puts points row column major
	void ProjectGridPoints(Homogeneous4* buff, unsigned int numpts, float size, unsigned int axis);

	// Clip a line from the line queue agains an axial plane.
	// returns true if the line is fully outside the plane
	// overwrites the points if clipped
	// inside is which side of the plane. value of 1 is inside, -1 is outside
	bool ClipAxial(unsigned int lineindex, int axis, float d, int inside);

	// Vertical and horisontal bresenham
	void DrawBresenhamVert(int x0, int y0, int x1, int y1, RGBAValue color);
	void DrawBresenhamHoriz(int x0, int y0, int x1, int y1, RGBAValue color);
signals:
	// these are general purpose signals, which scale the drag to 
	// the notional unit sphere and pass it to the controller for handling
	void BeginScaledDrag(int whichButton, float x, float y);
	// note that Continue & End assume the button has already been set
	void ContinueScaledDrag(float x, float y);
	void EndScaledDrag(float x, float y);
};

#endif
