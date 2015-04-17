#include <stdio.h>
#include <math.h>
#include "../source/labengine.h"

void RunTV(void)
{
	int width, height;
	int color;
	int i;
	int x;

	width = LabGetWidth() / LABCOLOR_COUNT;
	height = LabGetHeight();

	color = 0;
	for (color = 0; color < LABCOLOR_COUNT; color++)
	{
		LabSetColor(color);
		for (i = 0; i < width; i++)
		{
			x = color * width + i;
			LabDrawLine(x, 0, x, height);
		}
	}

	LabInputKey();
}

void DrawCircle(double angle, int radius, int color)
{
	int x, y;
	int co, si;

	x = LabGetWidth() / 2;
	y = LabGetHeight() / 2;
	co = (int)(cos(angle) * radius);
	si = (int)(sin(angle) * radius);

	LabSetColor(color);
	LabDrawLine(x + co, y + si, x - si, y + co);
	LabDrawLine(x - si, y + co, x - co, y - si);
	LabDrawLine(x - co, y - si, x + si, y - co);
	LabDrawLine(x + si, y - co, x + co, y + si);
}

void RunPoly(void)
{
	double angle = 0.0;
	int radius = LabGetHeight() / 4;
	while (!LabInputKeyReady())
	{
		LabClear();
		angle += 0.0001;
		DrawCircle(angle, radius, LABCOLOR_GREEN);
//		LabDelay(20);
		LabDrawFlush();
	}

	LabInputKey();
}

void RunTruecolor(void)
{
	int x, y, w, h, b, r, f;

	LabClear();

	for (y = 0, h = LabGetHeight(); y < h; y++) {
		for (x = 0, w = LabGetWidth(); x < w; x++) {
			b = x < 256 ? x : x < 512 ? 255 : 0;
			r = x < 256 ? 0 : x < 512 ? x - 256 : 0;
			f = x < 512 ? 0 : x - 512;
			LabSetColorRGB(r, r, b);
			LabDrawPoint(x, y);
		}
	}

	LabDrawFlush();
	LabInputKey();
}

int main(void)
{
	if (LabInit())
	{
		RunPoly();
		// RunTruecolor();
		LabTerm();
	}
}
