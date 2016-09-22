// ModelView.c++ - a basic combined Model and View for OpenGL

#include <iostream>
#include <limits>

#include "ModelView.h"
#include "Controller.h"
#include "ShaderIF.h"

double ModelView::mcRegionOfInterest[6] = { -1.0, 1.0, -1.0, 1.0, -1.0, 1.0 };
bool ModelView::aspectRatioPreservationEnabled = false;
int ModelView::colorModeRotation = -1;

ModelView::ModelView(ShaderIF* sIF) : shaderIF(sIF)
{
	// useless constructor, I guess?
}

ModelView::ModelView(ShaderIF* sIF, vec2_f* pos, int pos_length, bool is_axis) : shaderIF(sIF), verticies(pos_length)
{
	if(is_axis)
		colorMode = -1;
	else
		colorMode = getColorMode();
	
	initializeGeometry(pos, pos_length);
}

ModelView::~ModelView()
{
	if (vao > 0) // hasn't already been deleted
	{
		glDeleteBuffers(1, &vbo);
		glDeleteVertexArrays(1, &vao);
		vao = vbo = 0;
	}
}

void ModelView::initializeGeometry(vec2_f* pos, int pos_length)
{
	
	xmin = ymin = std::numeric_limits<float>::max();
	xmax = ymax = std::numeric_limits<float>::min();
	
	for (int i=0 ; i < pos_length ; i++)
	{
		if (pos[i][0] < xmin)
			xmin = pos[i][0];
		else if (pos[i][0] > xmax)
			xmax = pos[i][0];
		if (pos[i][1] < ymin)
			ymin = pos[i][1];
		else if (pos[i][1] > ymax)
			ymax = pos[i][1];
	}
	
	printf("Limits: %f, %f, %f, %f\n", xmin, xmax, ymin, ymax);
	
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	
	// store position of points to vbo
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, pos_length * sizeof(vec2_f), pos, GL_STATIC_DRAW);
	
	glVertexAttribPointer(shaderIF->pvaLoc("position"), 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(shaderIF->pvaLoc("position"));
}

void ModelView::compute2DScaleTrans(float* scaleTransF) // CLASS METHOD
{
	// TODO: This code can be used as is, BUT be absolutely certain you
	//       understand everything about how it works.

	double xmin = mcRegionOfInterest[0];
	double xmax = mcRegionOfInterest[1];
	double ymin = mcRegionOfInterest[2];
	double ymax = mcRegionOfInterest[3];

	if (aspectRatioPreservationEnabled)
	{
		// preserve aspect ratio. Make "region of interest" wider or taller to
		// match the Controller's viewport aspect ratio.
		double vAR = Controller::getCurrentController()->getViewportAspectRatio();
		matchAspectRatio(xmin, xmax, ymin, ymax, vAR);
	}

    // We are only concerned with the xy extents for now, hence we will
    // ignore mcRegionOfInterest[4] and mcRegionOfInterest[5].
    // Map the overall limits to the -1..+1 range expected by the OpenGL engine:
	double scaleTrans[4];
	linearMap(xmin, xmax, -1.0, 1.0, scaleTrans[0], scaleTrans[1]);
	linearMap(ymin, ymax, -1.0, 1.0, scaleTrans[2], scaleTrans[3]);
	for (int i=0 ; i<4 ; i++)
		scaleTransF[i] = static_cast<float>(scaleTrans[i]);
}

// xyzLimits: {mcXmin, mcXmax, mcYmin, mcYmax, mcZmin, mcZmax}
void ModelView::getMCBoundingBox(double* xyzLimits) const
{
	// TODO:
	// Put this ModelView instance's min and max x, y, and z extents
	// into xyzLimits[0..5]. (-1 .. +1 is OK for z direction for 2D models)
	xyzLimits[0] = xmin;
	xyzLimits[1] = xmax;
	xyzLimits[2] = ymin;
	xyzLimits[3] = ymax;
	xyzLimits[4] = -1.0;
	xyzLimits[5] =  1.0; // (zmin, zmax) (really 0..0)
}

bool ModelView::handleCommand(unsigned char key, double ldsX, double ldsY)
{
	return true;
}

// linearMap determines the scale and translate parameters needed in
// order to map a value, f (fromMin <= f <= fromMax) to its corresponding
// value, t (toMin <= t <= toMax). Specifically: t = scale*f + trans.
void ModelView::linearMap(double fromMin, double fromMax, double toMin, double toMax,
					  double& scale, double& trans) // CLASS METHOD
{
	scale = (toMax - toMin) / (fromMax - fromMin);
	trans = toMin - scale*fromMin;
}

void ModelView::matchAspectRatio(double& xmin, double& xmax,
        double& ymin, double& ymax, double vAR)
{
	// TODO: This code can be used as is, BUT be absolutely certain you
	//       understand everything about how it works.

	double wHeight = ymax - ymin;
	double wWidth = xmax - xmin;
	double wAR = wHeight / wWidth;
	if (wAR > vAR)
	{
		// make window wider
		wWidth = wHeight / vAR;
		double xmid = 0.5 * (xmin + xmax);
		xmin = xmid - 0.5*wWidth;
		xmax = xmid + 0.5*wWidth;
	}
	else
	{
		// make window taller
		wHeight = wWidth * vAR;
		double ymid = 0.5 * (ymin + ymax);
		ymin = ymid - 0.5*wHeight;
		ymax = ymid + 0.5*wHeight;
	}
}

void ModelView::render() const
{
	// save the current GLSL program in use
	GLint pgm;
	glGetIntegerv(GL_CURRENT_PROGRAM, &pgm);

	// draw the triangles using our vertex and fragment shaders
	glUseProgram(shaderIF->getShaderPgmID());

	// TODO: set scaleTrans (and all other needed) uniform(s)
	float scaleTrans[4];
	compute2DScaleTrans(scaleTrans);
	glUniform4fv(shaderIF->ppuLoc("scaleTrans"), 1, scaleTrans);
	
	glUniform1i(shaderIF->ppuLoc("colorMode"), colorMode);

	// TODO: make require primitive call(s)
	
	glBindVertexArray(vao);
	glDrawArrays(GL_LINE_STRIP, 0, verticies);

	// restore the previous program
	glUseProgram(pgm);
}

void ModelView::setMCRegionOfInterest(double xyz[6])
{
	for (int i=0 ; i<6 ; i++)
		mcRegionOfInterest[i] = xyz[i];
}

int ModelView::getColorMode()
{
	colorModeRotation++;
	if(colorModeRotation > ___COLOR_MODE_LIMIT___)
		colorModeRotation = 0;
	
	return colorModeRotation;
}
