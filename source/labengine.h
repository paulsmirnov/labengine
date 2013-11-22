#ifndef LABENGINE_H_INCLUDED
#define LABENGINE_H_INCLUDED

/**
 * boolean type
 */
typedef enum boolean_t
{
  LAB_FALSE  /*!0*/,
  LAB_TRUE   /*!1*/
} boolean_t;

/**
 * virtual codes
 */
typedef enum labkey_t 
{ 
  LABKEY_ENTER = 0x0D00, 
  LABKEY_ESC = 0x1B00, 
  LABKEY_BACK = 0x0800,
  LABKEY_TAB = 0x0900, 
  LABKEY_PAGE_UP = 0x2100,
  LABKEY_PAGE_DOWN = 0x2200,
  LABKEY_LEFT = 0x2500, 
  LABKEY_UP = 0x2600,
  LABKEY_RIGHT = 0x2700, 
  LABKEY_DOWN = 0x2800, 

} labkey_t;

boolean_t LabInit(void);
void LabTerm(void);
int LabGetWidth(void);
int LabGetHeight(void);
void LabDrawLine(int x1, int y1, int x2, int y2);
int LabInputKey(void);
boolean_t LabInputKeyReady(void);

#endif // LABENGINE_H_INCLUDED
