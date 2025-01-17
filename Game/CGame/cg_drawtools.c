/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// cg_drawtools.c -- helper functions called by cg_draw, cg_scoreboard, cg_info, etc
#include "cg_local.h"

/*
================
CG_AdjustFrom640

Adjusted for resolution and screen aspect ratio
================
*/
void CG_AdjustFrom640(float *x, float *y, float *w, float *h,qboolean stretch){
		float offset = 1.33333333333333f + ((cgs.screenXScale/cgs.screenYScale) -1.f);
		
		stretch = qtrue;
		*x *= cgs.screenXScale;
		*y *= cgs.screenYScale;
		*w *= stretch ? cgs.screenXScale : offset;
		*h *= stretch ? cgs.screenYScale : offset;
}

/*
================
CG_FillRect

Coordinates are 640*480 virtual values
=================
*/
void CG_FillRect(float x, float y, float width, float height, const float *color){
	trap_R_SetColor(color);

	CG_AdjustFrom640(&x, &y, &width, &height,qtrue);
	trap_R_DrawStretchPic(x, y, width, height, 0, 0, 0, 0, cgs.media.whiteShader);
	trap_R_SetColor(NULL);
}

/*
================
CG_DrawSides

Coords are virtual 640x480
================
*/
void CG_DrawSides(float x, float y, float w, float h, float size){
	CG_AdjustFrom640(&x, &y, &w, &h, qtrue);
	size *= cgs.screenXScale;
	trap_R_DrawStretchPic(x, y, size, h, 0, 0, 0, 0, cgs.media.whiteShader);
	trap_R_DrawStretchPic(x +w -size, y, size, h, 0, 0, 0, 0, cgs.media.whiteShader);
}

void CG_DrawTopBottom(float x, float y, float w, float h, float size){
	CG_AdjustFrom640(&x, &y, &w, &h,qtrue);
	size *= cgs.screenYScale;
	trap_R_DrawStretchPic(x, y, w, size, 0, 0, 0, 0, cgs.media.whiteShader);
	trap_R_DrawStretchPic(x, y +h -size, w, size, 0, 0, 0, 0, cgs.media.whiteShader);
}

/*
================
UI_DrawRect

Coordinates are 640*480 virtual values
=================
*/
void CG_DrawRect(float x, float y, float width, float height, float size, const float *color){
	trap_R_SetColor(color);
	CG_DrawTopBottom(x, y, width, height, size);
	CG_DrawSides(x, y, width, height, size);
	trap_R_SetColor(NULL);
}

/*
================
CG_DrawPic

Coordinates are 640*480 virtual values
=================
*/
void CG_DrawPic(qboolean stretch, float x, float y, float width, float height, qhandle_t hShader){
	CG_AdjustFrom640(&x, &y, &width, &height,stretch);
	trap_R_DrawStretchPic(x, y, width, height, 0, 0, 1, 1, hShader);
}

/*
===============
CG_DrawChar

Coordinates and size in 640*480 virtual screen size
===============
*/
void CG_DrawChar(int x, int y, int width, int height, int ch){
	int		row, col;
	float	frow, fcol,
			ax, ay, aw, ah,
			size;

	ch &= 255;
	if(ch == ' ') return;
	ax = x;
	ay = y;
	aw = width;
	ah = height;
	CG_AdjustFrom640(&ax, &ay, &aw, &ah, qtrue);
	row = ch>>4;
	col = ch&15;
	frow = row *.0625f;
	fcol = col *.0625f;
	size = .0625f;
	trap_R_DrawStretchPic(ax, ay, aw, ah, fcol, frow, fcol + size, frow + size, cgs.media.charsetShader);
}

/*
==================
CG_DrawStringExt

Draws a multi-colored string with a drop shadow, optionally forcing
to a fixed color.

Coordinates are at 640 by 480 virtual resolution
==================
*/
void CG_DrawStringExt(int spacing, int x, int y, const char *string, const float *setColor, qboolean forceColor,
						qboolean shadow, int charWidth, int charHeight, int maxChars){
	vec4_t		color;
	const char	*s;
	int			xx;
	int			cnt;

	if(maxChars <= 0) maxChars = 32767; // do them all!
	spacing = spacing == -1 ? charWidth : spacing;
	// draw the drop shadow
	if(shadow){
		color[0] = color[1] = color[2] = 0;
		color[3] = .5f;
		trap_R_SetColor(color);
		s = string;
		xx = x;
		cnt = 0;
		while(*s && cnt < maxChars){
			if(Q_IsColorString(s)){
				s += 2;
				continue;
			}
			CG_DrawChar(xx+1, y+1, charWidth, charHeight, *s);
			cnt++;
			xx += spacing;
			s++;
		}
	}
	// draw the colored text
	s = string;
	xx = x;
	cnt = 0;
	trap_R_SetColor(setColor);
	while(*s && cnt < maxChars){
		if(Q_IsColorString(s)){
			if(!forceColor){
				memcpy(color, g_color_table[ColorIndex(*(s+1))], sizeof(color));
				color[3] = setColor[3];
				trap_R_SetColor(color);
			}
			s += 2;
			continue;
		}
		CG_DrawChar(xx, y, charWidth, charHeight, *s);
		xx += spacing;
		cnt++;
		s++;
	}
	trap_R_SetColor(NULL);
}

void CG_DrawBigString( int x, int y, const char *s, float alpha){
	float color[4];

	color[0] = color[1] = color[2] = 1.f;
	color[3] = alpha;
	CG_DrawStringExt(-1, x, y, s, color, qfalse, qtrue, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, 0);
}

void CG_DrawBigStringColor(int x, int y, const char *s, vec4_t color){
	CG_DrawStringExt(-1, x, y, s, color, qtrue, qtrue, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, 0);
}

void CG_DrawSmallString(int x, int y, const char *s, float alpha){
	float color[4];

	color[0] = color[1] = color[2] = 1.f;
	color[3] = alpha;
	CG_DrawStringExt(-1, x, y, s, color, qfalse, qtrue, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0);
}

void CG_DrawSmallStringCustom(int x, int y, int w, int h, const char *s, float alpha,int spacing){
	float color[4];

	color[0] = color[1] = color[2] = 1.f;
	color[3] = alpha;
	CG_DrawStringExt(spacing, x, y, s, color, qfalse, qtrue, w, h, 0);
}

void CG_DrawSmallStringHalfHeight(int x, int y, const char *s, float alpha){
	float color[4];

	color[0] = color[1] = color[2] = 1.f;
	color[3] = alpha;
	CG_DrawStringExt(-1, x, y, s, color, qfalse, qtrue, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT / 2, 0);
}

void CG_DrawSmallStringColor(int x, int y, const char *s, vec4_t color){
	CG_DrawStringExt(-1, x, y, s, color, qtrue, qtrue, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0);
}

void CG_DrawMediumStringColor(int x, int y, const char *s, vec4_t color){
	CG_DrawStringExt(-1, x, y, s, color, qtrue, qtrue, MEDIUMCHAR_WIDTH, MEDIUMCHAR_HEIGHT, 0);
}

/*
=================
CG_DrawStrlen

Returns character count, skiping color escape codes
=================
*/
int CG_DrawStrlen(const char *str){
	const char *s = str;
	int			count = 0;

	while(*s){
		if(Q_IsColorString(s)) s += 2;
		else{
			count++;
			s++;
		}
	}
	return count;
}

/*
=============
CG_TileClearBox

This repeats a 64*64 tile graphic to fill the screen around a sized down
refresh window.
=============
*/
static void CG_TileClearBox(int x, int y, int w, int h, qhandle_t hShader){
	float	s1, t1, s2, t2;

	s1 = x/64.f;
	t1 = y/64.f;
	s2 = (x+w)/64.f;
	t2 = (y+h)/64.f;
	trap_R_DrawStretchPic(x, y, w, h, s1, t1, s2, t2, hShader);
}

/*
==============
CG_TileClear

Clear around a sized down screen
==============
*/
void CG_TileClear(void){
	int		top, bottom, left, right,
			w, h;

	w = cgs.glconfig.vidWidth;
	h = cgs.glconfig.vidHeight;
	// full screen rendering
	if(!cg.refdef.x && !cg.refdef.y && cg.refdef.width == w && cg.refdef.height == h) return;
	top = cg.refdef.y;
	bottom = top + cg.refdef.height-1;
	left = cg.refdef.x;
	right = left + cg.refdef.width-1;
	// clear above view screen
	CG_TileClearBox(0, 0, w, top, cgs.media.backTileShader);
	// clear below view screen
	CG_TileClearBox(0, bottom, w, h -bottom, cgs.media.backTileShader);
	// clear left of view screen
	CG_TileClearBox(0, top, left, bottom -top +1, cgs.media.backTileShader);

	// clear right of view screen
	CG_TileClearBox(right, top, w -right, bottom -top +1, cgs.media.backTileShader);
}

/*
================
CG_FadeColor
================
*/
float *CG_FadeColor(int startMsec, int totalMsec, int fadeTime){
	static vec4_t	color;
	int				t;

	if(!startMsec) return NULL;
	t = cg.time - startMsec;
	if(t >= totalMsec) return NULL;
	// fade out
	if(totalMsec -t < fadeTime) color[3] = (totalMsec-t) *1.f / fadeTime;
	else						color[3] = 1.f;
	color[0] = color[1] = color[2] = 1.f;
	return color;
}

/*
=================
CG_GetColorForHealth
=================
*/
void CG_GetColorForHealth(int powerLevel, int armor, vec4_t hcolor){
	int		count, max;

	// calculate the total points of damage that can
	// be sustained at the current powerLevel / armor level
	if(powerLevel <= 0){
		// black
		VectorClear(hcolor);
		hcolor[3] = 1.f;
		return;
	}
	count = armor;
	max = powerLevel *ARMOR_PROTECTION / (1.f-ARMOR_PROTECTION);
	if(max < count) count = max;
	powerLevel += count;
	// set the color based on powerLevel
	hcolor[0] = hcolor[3] = 1.f;
		 if(powerLevel >= 100)	hcolor[2] = 1.f;
	else if(powerLevel < 66)	hcolor[2] = 0;
	else						hcolor[2] = (powerLevel-66) / 33.f;
		 if(powerLevel > 60)	hcolor[1] = 1.f;
	else if(powerLevel < 30)	hcolor[1] = 0;
	else						hcolor[1] = (powerLevel-30) / 30.f;
}

/*
=================
CG_ColorForHealth
=================
*/
void CG_ColorForHealth(vec4_t hcolor){ CG_GetColorForHealth(cg.snap->ps.powerLevel[plCurrent], 0, hcolor);}

/*
=================
UI_DrawProportionalString2
=================
*/
static int	propMap[128][3] = {
	{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
	{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
	{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
	{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
	{0, 0, PROP_SPACE_WIDTH},	// SPACE
	{ 11, 122,	7},				// !
	{154, 181, 14},				// "
	{ 55, 122, 17},				// #
	{ 79, 122, 18},				// $
	{101, 122, 23},				// %
	{153, 122, 18},				// &
	{  9,  93,	7},				// '
	{207, 122,	8},				// (
	{230, 122,	9},				// )
	{177, 122, 18},				// *
	{ 30, 152, 18},				// +
	{ 85, 181,	7},				// ,
	{ 34,  93, 11},				// -
	{110, 181,	6},				// .
	{130, 152, 14},				// /
	{ 22,  64, 17},				// 0
	{ 41,  64, 12},				// 1
	{ 58,  64, 17},				// 2
	{ 78,  64, 18},				// 3
	{ 98,  64, 19},				// 4
	{120,  64, 18},				// 5
	{141,  64, 18},				// 6
	{204,  64, 16},				// 7
	{162,  64, 17},				// 8
	{182,  64, 18},				// 9
	{ 59, 181,	7},				// :
	{ 35, 181,	7},				// ;
	{203, 152, 14},				// <
	{ 56,  93, 14},				// =
	{228, 152, 14},				// >
	{177, 181, 18},				// ?
	{ 28, 122, 22},				// @
	{  5,	4, 18},				// A
	{ 27,	4, 18},				// B
	{ 48,	4, 18},				// C
	{ 69,	4, 17},				// D
	{ 90,	4, 13},				// E
	{106,	4, 13},				// F
	{121,	4, 18},				// G
	{143,	4, 17},				// H
	{164,	4,	8},				// I
	{175,	4, 16},				// J
	{195,	4, 18},				// K
	{216,	4, 12},				// L
	{230,	4, 23},				// M
	{  6,  34, 18},				// N
	{ 27,  34, 18},				// O
	{ 48,  34, 18},				// P
	{ 68,  34, 18},				// Q
	{ 90,  34, 17},				// R
	{110,  34, 18},				// S
	{130,  34, 14},				// T
	{146,  34, 18},				// U
	{166,  34, 19},				// V
	{185,  34, 29},				// W
	{215,  34, 18},				// X
	{234,  34, 18},				// Y
	{  5,  64, 14},				// Z
	{ 60, 152,	7},				// [
	{106, 151, 13},				// '\'
	{83 , 152,	7},				// ]
	{128, 122, 17},				// ^
	{  4, 152, 21},				// _
	{134, 181,	5},				// '
	{  5,	4, 18},				// A
	{ 27,	4, 18},				// B
	{ 48,	4, 18},				// C
	{ 69,	4, 17},				// D
	{ 90,	4, 13},				// E
	{106,	4, 13},				// F
	{121,	4, 18},				// G
	{143,	4, 17},				// H
	{164,	4,	8},				// I
	{175,	4, 16},				// J
	{195,	4, 18},				// K
	{216,	4, 12},				// L
	{230,	4, 23},				// M
	{  6,  34, 18},				// N
	{ 27,  34, 18},				// O
	{ 48,  34, 18},				// P
	{ 68,  34, 18},				// Q
	{ 90,  34, 17},				// R
	{110,  34, 18},				// S
	{130,  34, 14},				// T
	{146,  34, 18},				// U
	{166,  34, 19},				// V
	{185,  34, 29},				// W
	{215,  34, 18},				// X
	{234,  34, 18},				// Y
	{  5,  64, 14},				// Z
	{153, 152, 13},				// {
	{ 11, 181,	5},				// |
	{180, 152, 13},				// }
	{ 79,  93, 17},				// ~
	{  0,	0, -1}				// DEL
};	

static int propMapB[26][3] = {
	{ 11,  12, 33},
	{ 49,  12, 31},
	{ 85,  12, 31},
	{120,  12, 30},
	{156,  12, 21},
	{183,  12, 21},
	{207,  12, 32},
	{ 13,  55, 30},
	{ 49,  55, 13},
	{ 66,  55, 29},
	{101,  55, 31},
	{135,  55, 21},
	{158,  55, 40},
	{204,  55, 32},
	{ 12,  97, 31},
	{ 48,  97, 31},
	{ 82,  97, 30},
	{118,  97, 30},
	{153,  97, 30},
	{185,  97, 25},
	{213,  97, 30},
	{ 11, 139, 32},
	{ 42, 139, 51},
	{ 93, 139, 32},
	{126, 139, 31},
	{158, 139, 25},
};

#define PROPB_GAP_WIDTH		4
#define PROPB_SPACE_WIDTH	12
#define PROPB_HEIGHT		36
/*
=================
UI_DrawBannerString
=================
*/
static void UI_DrawBannerString2(int x, int y, const char* str, vec4_t color){
	const char		*s;
	unsigned char	ch;
	float			ax, ay, aw, ah,
					frow, fcol, fwidth, fheight;

	// draw the colored text
	trap_R_SetColor(color);
	ax = x *cgs.screenXScale +cgs.screenXBias;
	ay = y *cgs.screenYScale;
	s = str;
	while(*s){
		ch = *s & 127;
		if(ch == ' ')
			ax += ((float)PROPB_SPACE_WIDTH + (float)PROPB_GAP_WIDTH)* cgs.screenXScale;
		else if(ch >= 'A' && ch <= 'Z'){
			ch -= 'A';
			fcol =		(float)propMapB[ch][0] / 256.f;
			frow =		(float)propMapB[ch][1] / 256.f;
			fwidth =	(float)propMapB[ch][2] / 256.f;
			fheight =	(float)PROPB_HEIGHT / 256.f;
			aw =		(float)propMapB[ch][2] *cgs.screenXScale;
			ah =		(float)PROPB_HEIGHT *cgs.screenYScale;
			trap_R_DrawStretchPic(ax, ay, aw, ah, fcol, frow, fcol +fwidth, frow +fheight, cgs.media.charsetPropB);
			ax += (aw +(float)PROPB_GAP_WIDTH *cgs.screenXScale);
		}
		s++;
	}
	trap_R_SetColor(NULL);
}

void UI_DrawBannerString(int x, int y, const char* str, int style, vec4_t color){
	const char *	s;
	int				ch, width;
	vec4_t			drawcolor;

	// find the width of the drawn text
	s = str;
	width = 0;
	while(*s){
		ch = *s;
		if(ch == ' ')					width += PROPB_SPACE_WIDTH;
		else if(ch >= 'A' && ch <= 'Z')	width += propMapB[ch - 'A'][2] + PROPB_GAP_WIDTH;
		s++;
	}
	width -= PROPB_GAP_WIDTH;
	switch(style & UI_FORMATMASK){
		case UI_CENTER:
			x -= width / 2;
			break;
		case UI_RIGHT:
			x -= width;
			break;
		case UI_LEFT:
		default:
			break;
	}
	if(style & UI_DROPSHADOW){
		drawcolor[0] = drawcolor[1] = drawcolor[2] = 0;
		drawcolor[3] = color[3];
		UI_DrawBannerString2(x+1, y+1, str, drawcolor);
	}
	UI_DrawBannerString2(x, y, str, color);
}

int UI_ProportionalStringWidth(const char* str){
	const char *	s;
	int				ch, charWidth, width;

	s = str;
	width = 0;
	while(*s){
		ch = *s & 127;
		charWidth = propMap[ch][2];
		if(charWidth != -1){
			width += charWidth;
			width += PROP_GAP_WIDTH;
		}
		s++;
	}
	width -= PROP_GAP_WIDTH;
	return width;
}

static void UI_DrawProportionalString2(int x, int y, const char* str, vec4_t color, float sizeScale, qhandle_t charset){
	const char		*s;
	unsigned char	ch;
	float	ax, ay, aw, ah,
			frow, fcol, fwidth, fheight;

	// draw the colored text
	trap_R_SetColor(color);
	ax = x *cgs.screenXScale +cgs.screenXBias;
	ay = y *cgs.screenYScale;
	s = str;
	while(*s){
		ch = *s & 127;
		if(ch == ' ') aw = (float)PROP_SPACE_WIDTH * cgs.screenXScale * sizeScale;
		else if(propMap[ch][2] != -1){
			fcol =		(float)propMap[ch][0] / 256.f;
			frow =		(float)propMap[ch][1] / 256.f;
			fwidth =	(float)propMap[ch][2] / 256.f;
			fheight =	(float)PROP_HEIGHT / 256.f;
			aw =		(float)propMap[ch][2] *cgs.screenXScale *sizeScale;
			ah =		(float)PROP_HEIGHT *cgs.screenYScale *sizeScale;
			trap_R_DrawStretchPic(ax, ay, aw, ah, fcol, frow, fcol +fwidth, frow +fheight, charset);
		}
		else aw = 0;
		ax += (aw +(float)PROP_GAP_WIDTH *cgs.screenXScale *sizeScale);
		s++;
	}
	trap_R_SetColor(NULL);
}

/*
=================
UI_ProportionalSizeScale
=================
*/
float UI_ProportionalSizeScale( int style ) {
	if(style & UI_SMALLFONT) return .75f;
	return 1.f;
}

/*
=================
UI_DrawProportionalString
=================
*/
void UI_DrawProportionalString(int x, int y, const char* str, int style, vec4_t color){
	vec4_t	drawcolor;
	int		width;
	float	sizeScale;

	sizeScale = UI_ProportionalSizeScale(style);
	switch(style & UI_FORMATMASK){
		case UI_CENTER:
			width = UI_ProportionalStringWidth(str) *sizeScale;
			x -= width / 2;
			break;
		case UI_RIGHT:
			width = UI_ProportionalStringWidth(str) *sizeScale;
			x -= width;
			break;
		case UI_LEFT:
		default:
			break;
	}
	if(style & UI_DROPSHADOW){
		drawcolor[0] = drawcolor[1] = drawcolor[2] = 0;
		drawcolor[3] = color[3];
		UI_DrawProportionalString2(x+1, y+1, str, drawcolor, sizeScale, cgs.media.charsetProp);
	}
	if(style & UI_INVERSE){
		drawcolor[0] = color[0] * .8f;
		drawcolor[1] = color[1] * .8f;
		drawcolor[2] = color[2] * .8f;
		drawcolor[3] = color[3];
		UI_DrawProportionalString2(x, y, str, drawcolor, sizeScale, cgs.media.charsetProp);
		return;
	}
	if(style & UI_PULSE){
		drawcolor[0] = color[0] *.8f;
		drawcolor[1] = color[1] *.8f;
		drawcolor[2] = color[2] *.8f;
		drawcolor[3] = color[3];
		UI_DrawProportionalString2(x, y, str, color, sizeScale, cgs.media.charsetProp);
		drawcolor[0] = color[0];
		drawcolor[1] = color[1];
		drawcolor[2] = color[2];
		drawcolor[3] = .5f + .5f *sin(cg.time / PULSE_DIVISOR);
		UI_DrawProportionalString2(x, y, str, drawcolor, sizeScale, cgs.media.charsetPropGlow);
		return;
	}
	UI_DrawProportionalString2(x, y, str, color, sizeScale, cgs.media.charsetProp);
}
