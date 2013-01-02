/**
 * Copyright (c) 2006-2012 LOVE Development Team
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

#include "Image.h"

// STD
#include <cstring> // For memcpy

#include <iostream>
using namespace std;

namespace love
{
namespace graphics
{
namespace opengl
{

Image::Image(love::image::ImageData *data)
	: width((float)(data->getWidth()))
	, height((float)(data->getHeight()))
	, texture(0)
{
	data->retain();
	this->data = data;

	memset(vertices, 255, sizeof(vertex)*4);

	vertices[0].x = 0;
	vertices[0].y = 0;
	vertices[1].x = 0;
	vertices[1].y = height;
	vertices[2].x = width;
	vertices[2].y = height;
	vertices[3].x = width;
	vertices[3].y = 0;

	vertices[0].s = 0;
	vertices[0].t = 0;
	vertices[1].s = 0;
	vertices[1].t = 1;
	vertices[2].s = 1;
	vertices[2].t = 1;
	vertices[3].s = 1;
	vertices[3].t = 0;

	settings.filter = getDefaultFilter();
}

Image::~Image()
{
	if (data != 0)
		data->release();
	unload();
}

float Image::getWidth() const
{
	return width;
}

float Image::getHeight() const
{
	return height;
}

const vertex *Image::getVertices() const
{
	return vertices;
}

love::image::ImageData *Image::getData() const
{
	return data;
}

void Image::getRectangleVertices(int x, int y, int w, int h, vertex *vertices) const
{
	// Check upper.
	x = (x+w > (int)width) ? (int)width-w : x;
	y = (y+h > (int)height) ? (int)height-h : y;

	// Check lower.
	x = (x < 0) ? 0 : x;
	y = (y < 0) ? 0 : y;

	vertices[0].x = 0;
	vertices[0].y = 0;
	vertices[1].x = 0;
	vertices[1].y = (float)h;
	vertices[2].x = (float)w;
	vertices[2].y = (float)h;
	vertices[3].x = (float)w;
	vertices[3].y = 0;

	float tx = (float)x/width;
	float ty = (float)y/height;
	float tw = (float)w/width;
	float th = (float)h/height;

	vertices[0].s = tx;
	vertices[0].t = ty;
	vertices[1].s = tx;
	vertices[1].t = ty+th;
	vertices[2].s = tx+tw;
	vertices[2].t = ty+th;
	vertices[3].s = tx+tw;
	vertices[3].t = ty;
}

void Image::draw(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky) const
{
	static Matrix t;

	t.setTransformation(x, y, angle, sx, sy, ox, oy, kx, ky);
	drawv(t, vertices);
}

void Image::drawq(love::graphics::Quad *quad, float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky) const
{
	static Matrix t;
	const vertex *v = quad->getVertices();

	t.setTransformation(x, y, angle, sx, sy, ox, oy, kx, ky);
	drawv(t, v);
}

void Image::setFilter(const Image::Filter &f)
{
	bind();
	setTextureFilter(f);
}

Image::Filter Image::getFilter() const
{
	bind();
	return getTextureFilter();
}

void Image::setWrap(Image::Wrap &w)
{
	bind();
	setTextureWrap(w);
}

Image::Wrap Image::getWrap() const
{
	bind();
	return getTextureWrap();
}

void Image::bind() const
{
	if (texture == 0)
		return;

	bindTexture(texture);
}

bool Image::load()
{
	return loadVolatile();
}

void Image::unload()
{
	return unloadVolatile();
}

bool Image::loadVolatile()
{
	if (hasNpot())
		return loadVolatileNPOT();
	else
		return loadVolatilePOT();
}

bool Image::loadVolatilePOT()
{
	glGenTextures(1,(GLuint *)&texture);
	bindTexture(texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	float p2width = next_p2(width);
	float p2height = next_p2(height);
	float s = width/p2width;
	float t = height/p2height;

	vertices[1].t = t;
	vertices[2].t = t;
	vertices[2].s = s;
	vertices[3].s = s;

	glTexImage2D(GL_TEXTURE_2D,
				 0,
				 GL_RGBA8,
				 (GLsizei)p2width,
				 (GLsizei)p2height,
				 0,
				 GL_RGBA,
				 GL_UNSIGNED_BYTE,
				 0);

	glTexSubImage2D(GL_TEXTURE_2D,
					0,
					0,
					0,
					(GLsizei)width,
					(GLsizei)height,
					GL_RGBA,
					GL_UNSIGNED_BYTE,
					data->getData());

	setFilter(settings.filter);
	setWrap(settings.wrap);

	return true;
}

bool Image::loadVolatileNPOT()
{
	glGenTextures(1,(GLuint *)&texture);
	bindTexture(texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D,
				 0,
				 GL_RGBA8,
				 (GLsizei)width,
				 (GLsizei)height,
				 0,
				 GL_RGBA,
				 GL_UNSIGNED_BYTE,
				 data->getData());

	setFilter(settings.filter);
	setWrap(settings.wrap);

	return true;
}

void Image::unloadVolatile()
{
	settings.filter = getFilter();
	settings.wrap = getWrap();
	// Delete the hardware texture.
	if (texture != 0)
	{
		deleteTexture(texture);
		texture = 0;
	}
}

void Image::drawv(const Matrix &t, const vertex *v) const
{
	bind();

	glPushMatrix();

	glMultMatrixf((const GLfloat *)t.getElements());

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(2, GL_FLOAT, sizeof(vertex), (GLvoid *)&v[0].x);
	glTexCoordPointer(2, GL_FLOAT, sizeof(vertex), (GLvoid *)&v[0].s);
	glDrawArrays(GL_QUADS, 0, 4);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	glPopMatrix();
}

bool Image::hasNpot()
{
	return GLEE_ARB_texture_non_power_of_two != 0;
}

} // opengl
} // graphics
} // love
