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

#endif // LABENGINE_H_INCLUDED
