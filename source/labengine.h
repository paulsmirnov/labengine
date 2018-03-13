#ifndef LABENGINE_H_INCLUDED
#define LABENGINE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief ���������� ���.
 *
 * � ����� �� ����������� ���������� ��������� (����������) ���, � ������
 * ���� ������������ ��� int. ����� �������� ���� ������ ��������� ��������,
 * ���������� ������������� ������ ������������.
 */
typedef enum labbool_t
{
  LAB_FALSE, ///< ���� (0)
  LAB_TRUE   ///< ������ (1)
} labbool_t;

/**
 * @brief ���� ������������ ������.
 *
 * ����������� ���� ��� ������ Enter, Escape, �������...
 * ������ � ASCII-������ ���������� ������, �������� ������������ ���������
 * �� ������� <code>LabInputKey()</code>.
 *
 * @see LabInputKey
 */
typedef enum labkey_t
{ 
  LABKEY_ENTER = 0x0D00,      ///< Enter 
  LABKEY_ESC = 0x1B00,        ///< Escape 
  LABKEY_BACK = 0x0800,       ///< Backspace 
  LABKEY_TAB = 0x0900,        ///< Tab 
  LABKEY_PAGE_UP = 0x2100,    ///< PageUp 
  LABKEY_PAGE_DOWN = 0x2200,  ///< PageDown 
  LABKEY_LEFT = 0x2500,       ///< ������� �����
  LABKEY_UP = 0x2600,         ///< ������� �����
  LABKEY_RIGHT = 0x2700,      ///< ������� ������
  LABKEY_DOWN = 0x2800,       ///< ������� ����
 
} labkey_t;

/**
 * @brief �������� ������ �������� �������.
 *
 * ������������ ��� ���������� ����� ����� � ������ ����������� ��������.
 * ���������� � �������� ��������� � ������� <code>LabSetColor()</code>.
 *
 * @see LabSetColor
 */
typedef enum labcolor_t 
{ 
  LABCOLOR_RED = 0x000C,                                          ///< �������
  LABCOLOR_GREEN = 0x000A,                                        ///< ������
  LABCOLOR_BLUE = 0x0009,                                         ///< �����
  LABCOLOR_BLACK = LABCOLOR_RED & LABCOLOR_GREEN & LABCOLOR_BLUE, ///< ׸����
  LABCOLOR_WHITE = LABCOLOR_RED | LABCOLOR_GREEN | LABCOLOR_BLUE, ///< �����
  LABCOLOR_YELLOW = LABCOLOR_RED | LABCOLOR_GREEN,                ///< Ƹ����
  LABCOLOR_CYAN = LABCOLOR_GREEN | LABCOLOR_BLUE,                 ///< �������
  LABCOLOR_MAGENTA = LABCOLOR_BLUE | LABCOLOR_RED,                ///< ���������
    
} labcolor_t;

/**
 * @brief ������������� ������������ ������.
 * 
 * ��������� ����� ����� ������� � ������� ��������� ����������, �� ���� � �����������
 * ������. �������� ���� ��� �������.
 * �� ��������� ������ � ����������� ������ ��������� ����� <code>LabTerm()</code>.
 * 
 * @return LAB_TRUE ���� ������������� ������ �������, ����� - LAB_FALSE.
 * @see LabTerm
 */
labbool_t LabInit(void);

/**
 * @brief ���������� ������ � ����������� ������.
 * 
 * ��������� ����� ��� ��������� ������ � ����������� ������ � �������� 
 * ���� ������������ ������.
 * ����� ������� ������ � ����������� ������ ��������� ����� <code>LabInit()</code>.
 * @see LabInit
 */
void LabTerm(void);

/**
 * @brief ��������� ������ ����.
 * 
 * @return ����� ����� - ������� ������ ����.
 * @see LabGetHeight
 */
int LabGetWidth(void);

/**
 * @brief ��������� ������ ����.
 * 
 * @return ����� ����� - ������� ������ ����.
 * @see LabGetWidth
 */
int LabGetHeight(void);

/** 
 * @brief ��������� �����.
 *
 * ��������� �����, ����������� ����� � ������������ (x1, y1) � (x2, y2).
 *
 * @param x1 �������������� ���������� ������ ����� (0 �����)
 * @param y1 ������������ ���������� ������ ����� (0 ������)
 * @param x2 �������������� ���������� ������ �����
 * @param y2 ������������ ���������� ������ �����.
 */
void LabDrawLine(int x1, int y1, int x2, int y2);

/** 
 * @brief �������� ������� �������, �����, ������� � ����.
 * 
 * ������� ������� �������, � �����, ����� ������� ������, ���������� � ��� -
 * ASCII-��� ��� ���������� ������ ��� ��� �� ������������ <code>labkey_t</code>
 * ��� ������������ ������ (����� ��� Enter, Escape, ������� ... ).
 *
 * @return ����� ����� - ��� ������� �������.
 * @see labkey_t
 */
int LabInputKey(void);

/** 
 * �������� �� ������� �������������� ������� ������.
 *
 * @return LAB_TRUE ���� ���� �������������� ������� ������, LAB_FALSE, ���� ����� ���.
 */
labbool_t LabInputKeyReady(void);

/** 
 * ��������� ����� ��� ������������ ��������� ����������� ��������.
 *
 * �� ��������� (�� ������� ������ ���� �������) ������������ ������ ����.
 *
 * @param color ����� ���� �� ������������ <code>labcolor_t</code>.
 * @see labcolor_t
 */
void LabSetColor(labcolor_t color);


#ifdef __cplusplus
}
#endif

#endif // LABENGINE_H_INCLUDED