#include <stdio.h>
#include "../source/labengine.h"

void Run(void)
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

int main(void)
{
	if (LabInit())
	{
		Run();
		LabTerm();
	}
}
