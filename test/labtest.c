#include <stdio.h>
#include "../source/labengine.h"

int main(void)
{
  // initialize
  if (!LabInit())
  {
    printf("ERROR: Can not initialize the lab engine!\n");
    return -1;
  }

  printf("Width is %i, height is %i\n", LabGetWidth(), LabGetHeight()); //window sizes

  // do the job
  printf("... here I do something...\nOK, press ENTER\n");
  getchar();
  // terminate
  LabTerm();
  return 0;
}
