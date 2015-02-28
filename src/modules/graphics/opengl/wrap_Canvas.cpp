/**
 * Copyright (c) 2006-2015 LOVE Development Team
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

#include "wrap_Canvas.h"
#include "Graphics.h"

namespace love
{
namespace graphics
{
namespace opengl
{

Canvas *luax_checkcanvas(lua_State *L, int idx)
{
	return luax_checktype<Canvas>(L, idx, "Canvas", GRAPHICS_CANVAS_T);
}

int w_Canvas_renderTo(lua_State *L)
{
	Canvas *canvas = luax_checkcanvas(L, 1);
	luaL_checktype(L, 2, LUA_TFUNCTION);

	Graphics *graphics = Module::getInstance<Graphics>(Module::M_GRAPHICS);

	if (graphics)
	{
		// Save the current Canvas so we can restore it when we're done.
		std::vector<Canvas *> oldcanvases = graphics->getCanvas();

		for (Canvas *c : oldcanvases)
			c->retain();

		luax_catchexcept(L, [&](){ graphics->setCanvas(canvas); });

		lua_settop(L, 2); // make sure the function is on top of the stack
		lua_call(L, 0, 0);

		graphics->setCanvas(oldcanvases);

		for (Canvas *c : oldcanvases)
			c->release();
	}

	return 0;
}

int w_Canvas_newImageData(lua_State *L)
{
	Canvas *canvas = luax_checkcanvas(L, 1);
	love::image::Image *image = luax_getmodule<love::image::Image>(L, "image", MODULE_IMAGE_T);
	int x = luaL_optint(L, 2, 0);
	int y = luaL_optint(L, 3, 0);
	int w = luaL_optint(L, 4, canvas->getWidth());
	int h = luaL_optint(L, 5, canvas->getHeight());

	love::image::ImageData *img = nullptr;
	luax_catchexcept(L, [&](){ img = canvas->newImageData(image, x, y, w, h); });

	luax_pushtype(L, "ImageData", IMAGE_IMAGE_DATA_T, img);
	img->release();
	return 1;
}

int w_Canvas_getFormat(lua_State *L)
{
	Canvas *canvas = luax_checkcanvas(L, 1);
	Canvas::Format format = canvas->getTextureFormat();
	const char *str;
	if (!Canvas::getConstant(format, str))
		return luaL_error(L, "Unknown Canvas format.");

	lua_pushstring(L, str);
	return 1;
}

int w_Canvas_getMSAA(lua_State *L)
{
	Canvas *canvas = luax_checkcanvas(L, 1);
	lua_pushinteger(L, canvas->getMSAA());
	return 1;
}

static const luaL_Reg functions[] =
{
	// From wrap_Texture.
	{ "getWidth", w_Texture_getWidth },
	{ "getHeight", w_Texture_getHeight },
	{ "getDimensions", w_Texture_getDimensions },
	{ "setFilter", w_Texture_setFilter },
	{ "getFilter", w_Texture_getFilter },
	{ "setWrap", w_Texture_setWrap },
	{ "getWrap", w_Texture_getWrap },

	{ "renderTo", w_Canvas_renderTo },
	{ "newImageData", w_Canvas_newImageData },
	{ "getFormat", w_Canvas_getFormat },
	{ "getMSAA", w_Canvas_getMSAA },
	{ 0, 0 }
};

extern "C" int luaopen_canvas(lua_State *L)
{
	return luax_register_type(L, "Canvas", functions);
}

} // opengl
} // graphics
} // love
