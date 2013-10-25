#ifndef LABENGINE_H_INCLUDED
#define LABENGINE_H_INCLUDED

typedef enum boolean_t
{
  LAB_FALSE,
  LAB_TRUE
} boolean_t;

boolean_t LabInit(void);
void LabTerm(void);
int LabGetWidth(void);
int LabGetHeight(void);
void LabDrawLine(int x1, int y1, int x2, int y2);

#endif // LABENGINE_H_INCLUDED
