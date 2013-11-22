#include <stdio.h>
#include "../source/labengine.h"

int main(void)
{
  int key;
  int x = 100, y = 100;

  // initialize
  if (!LabInit())
  {
    printf("ERROR: Can not initialize the lab engine!\n");
    return -1;
  }

  printf("Press UP, DOWN, LEFT or RIGHT key to move the point\nOr other key to get information about it\nOr ESCAPE to quit...\n"); // window sizes
  while (1)
  {
    key = LabInputKey();
    if (key == LABKEY_ESC)
      break;
    switch (key)
    {
      case LABKEY_UP:
        y -= 3;
        break;
      case LABKEY_LEFT:
        x -= 3;
        break;
      case LABKEY_RIGHT:
        x += 3;
        break;
      case LABKEY_DOWN:
        y += 3;
        break;
      default:
        printf("'%c' was pressed!\n", key); //for character (= button face) keys
        break;
    }
    LabDrawLine(x, y, x + 10, y); 
  }

  /*printf("Width is %i, height is %i\n", LabGetWidth(), LabGetHeight()); // window sizes
  LabDrawLine(100, 200, 500, 200);
  LabDrawLine(0, 0, LabGetWidth(), LabGetHeight());*/

  // do the job
  printf("......Press ENTER to quit......\n");
  getchar();
  // terminate
  LabTerm();
  return 0;
}
