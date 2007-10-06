/* === S Y N F I G ========================================================= */
/*!	\file renderer_canvas.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
**	\endlegal
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "renderer_canvas.h"
#include "workarea.h"
#include <ETL/misc>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Renderer_Canvas::~Renderer_Canvas()
{
}
std::vector< std::pair<Glib::RefPtr<Gdk::Pixbuf>,int> >&
Renderer_Canvas::get_tile_book()
{
	return get_work_area()->get_tile_book();
}

bool
Renderer_Canvas::get_full_frame()const
{
	return get_work_area()->get_full_frame();
}

int Renderer_Canvas::get_refreshes()const
{
	return get_work_area()->get_refreshes();
}

bool
Renderer_Canvas::get_canceled()const
{
	return get_work_area()->get_canceled();
}

bool
Renderer_Canvas::get_queued()const
{
	return get_work_area()->get_queued();
}

bool
Renderer_Canvas::get_rendering()const
{
	return get_work_area()->get_rendering();
}

void
Renderer_Canvas::render_vfunc(
	const Glib::RefPtr<Gdk::Drawable>& drawable,
	const Gdk::Rectangle& expose_area
)
{
	assert(get_work_area());
	if(!get_work_area())
		return;

//	const synfig::RendDesc &rend_desc(get_work_area()->get_canvas()->rend_desc());

	const synfig::Vector focus_point(get_work_area()->get_focus_point());

	std::vector< std::pair<Glib::RefPtr<Gdk::Pixbuf>,int> >& tile_book(get_tile_book());

	int drawable_w,drawable_h;
	drawable->get_size(drawable_w,drawable_h);

	// Calculate the window coordinates of the top-left
	// corner of the canvas.
	const synfig::Vector::value_type
		x(focus_point[0]/get_pw()+drawable_w/2-get_w()/2),
		y(focus_point[1]/get_ph()+drawable_h/2-get_h()/2);

	/*const synfig::Vector::value_type window_startx(window_tl[0]);
	const synfig::Vector::value_type window_endx(window_br[0]);
	const synfig::Vector::value_type window_starty(window_tl[1]);
	const synfig::Vector::value_type window_endy(window_br[1]);
	*/
	const int
		tile_w(get_work_area()->get_tile_w()),
		tile_h(get_work_area()->get_tile_h());

	const int
		w(get_w()),
		h(get_h());

	Glib::RefPtr<Gdk::GC> gc(Gdk::GC::create(drawable));

	if(!tile_book.empty())
	if(get_full_frame())
	{
		if(tile_book[0].first)
		{
			drawable->draw_pixbuf(
				gc, //GC
				tile_book[0].first, //pixbuf
				0, 0,	// Source X and Y
				round_to_int(x),round_to_int(y),	// Dest X and Y
				-1,-1,	// Width and Height
				Gdk::RGB_DITHER_MAX,		// RgbDither
				2, 2 // Dither offset X and Y
			);
		}
		if(tile_book[0].second!=get_refreshes() && get_canceled()==false && get_rendering()==false && get_queued()==false)
			get_work_area()->async_update_preview();
	}
	else
	{
		const int width_in_tiles(w/tile_w+(w%tile_w?1:0));
		const int height_in_tiles(h/tile_h+(h%tile_h?1:0));

		int u(0),v(0),tx,ty;
		int u1(0),v1(0),u2(width_in_tiles), v2(height_in_tiles);

		bool needs_refresh(false);

		u1=int(-x/tile_w);
		v1=int(-y/tile_h);
		u2=int((-x+drawable_w)/tile_w+1);
		v2=int((-y+drawable_h)/tile_h+1);
		if(u2>width_in_tiles)u2=width_in_tiles;
		if(v2>height_in_tiles)v2=height_in_tiles;
		if(u1<0)u1=0;
		if(v1<0)v1=0;

		for(v=v1;v<v2;v++)
		{
			for(u=u1;u<u2;u++)
			{
				int index=v*width_in_tiles+u;
				if(int(tile_book.size())>index && tile_book[index].first)
				{
					tx=u*tile_w;
					ty=v*tile_w;

					drawable->draw_pixbuf(
						gc, //GC
						tile_book[index].first, //pixbuf
						0, 0,	// Source X and Y
						round_to_int(x)+tx,round_to_int(y)+ty,	// Dest X and Y
						-1,-1,	// Width and Height
						Gdk::RGB_DITHER_MAX,		// RgbDither
						2, 2 // Dither offset X and Y
					);
				}
				if(tile_book[index].second!=get_refreshes())
					needs_refresh=true;
			}
		}
		if(needs_refresh==true && get_canceled()==false && get_rendering()==false && get_queued()==false)
		{
			//queue_render_preview();
			get_work_area()->async_update_preview();
			//update_preview();
			//return true;
		}

	}

	// Draw the border around the rendered region
	{
		gc->set_rgb_fg_color(Gdk::Color("#000000"));
		gc->set_line_attributes(1,Gdk::LINE_SOLID,Gdk::CAP_BUTT,Gdk::JOIN_MITER);
		drawable->draw_rectangle(
			gc,
			false,	// Fill?
			round_to_int(x),round_to_int(y),	// x,y
			w,h	//w,h
		);
	}
}
