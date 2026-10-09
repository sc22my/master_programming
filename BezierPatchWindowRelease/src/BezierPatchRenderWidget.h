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

	// Scratchpad of points.
    std::vector<Homogeneous4> m_Scratchpad;

    // ======== STATE ========
	// Or our "uniforms"

    // Projection matrix
    Matrix4 m_Projection;

	// Model view projection
	Matrix4 m_MVP;

	unsigned int m_PointSize = 3;

	// Scratchpad size. Should fit nicely into 80kb per core of l1 cache on lab i7 12700
	const unsigned int m_ScratchpadSize = 2048;

	// Sets the projection matrix to ortho.
	// Arguments behave same as glOrtho()
    void CalculateProjectionOrtho(float left, float right, float bottom, float top, float near, float far);

	// ======== DRAWING PRIMITIVES ========
	// Doesnt use an index buffer

	// Draws points from a buffer
	void DrawPoints(Point3* points, unsigned int numpts);

	// Draws lines from a buffer. lines arranged as [l0p0, l0p1, l1p0, l1p1, ...]
	void DrawLines(Point3* points, unsigned int numlines);

	// ======== UTIL FUNCTIONS ========

	// Clip a line agains an axial plane.
	// returns if the line is outside the plane
	// overwrites the points if clipped
	bool ClipAxial(Homogeneous4* points, int axis, float d);

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
