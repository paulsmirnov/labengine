#ifndef LABENGINE_H_INCLUDED
#define LABENGINE_H_INCLUDED

/**
 * Логический тип.
 *
 * В языке Си отсутствует выделенный булевский (логический) тип, и вместо
 * него используется тип int. Чтобы избежать этой потери семантики значений,
 * библиотека предоставляет данное перечисление.
 */
typedef enum labbool_t
{
  LAB_FALSE, ///< Ложь (0)
  LAB_TRUE   ///< Истина (1)
} labbool_t;

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

labbool_t LabInit(void);
void LabTerm(void);
int LabGetWidth(void);
int LabGetHeight(void);
void LabDrawLine(int x1, int y1, int x2, int y2);
int LabInputKey(void);
labbool_t LabInputKeyReady(void);

#endif // LABENGINE_H_INCLUDED
