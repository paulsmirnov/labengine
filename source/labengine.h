#ifndef LABENGINE_H_INCLUDED
#define LABENGINE_H_INCLUDED

#pragma comment(lib, "kernel32")
#pragma comment(lib, "user32")
#pragma comment(lib, "gdi32")

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
 * \defgroup lifecycle_group Lifecycle Methods
 *
 * ������� ������������� ������������ ������ � ��������� ������ � ���.
 * ��������� � ������: \ref lifecycle_group
 * @{
 */
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
 * @brief ���������������� ���������� ��������� �� time ��
 * 
 * @param time ���������� �����������
 */
void LabDelay(int time);
/**@}*/


/**
 * \defgroup graphics_group Graphics Methods
 *
 * ������� ������������ ������ � ��������� ���������� �� ����.
 * ��������� � ������: \ref graphics_group
 * @{
 */

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
  LABCOLOR_BLACK,         ///< ׸����
  LABCOLOR_DARK_BLUE,     ///< Ҹ���-�����
  LABCOLOR_DARK_GREEN,    ///< Ҹ���-������
  LABCOLOR_DARK_CYAN,     ///< Ҹ���-�������
  LABCOLOR_DARK_RED,      ///< Ҹ���-�������
  LABCOLOR_DARK_MAGENTA,  ///< Ҹ���-���������
  LABCOLOR_BROWN,         ///< ����������
  LABCOLOR_LIGHT_GREY,    ///< ������-�����
  LABCOLOR_DARK_GREY,     ///< Ҹ���-�����
  LABCOLOR_BLUE,          ///< �����
  LABCOLOR_GREEN,         ///< ������
  LABCOLOR_CYAN,          ///< �������
  LABCOLOR_RED,           ///< �������
  LABCOLOR_MAGENTA,       ///< ���������
  LABCOLOR_YELLOW,        ///< Ƹ����
  LABCOLOR_WHITE,         ///< �����

  LABCOLOR_COUNT          ///< ���������� ������ � �������
} labcolor_t;

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
 * @brief ������� ������ ������ ������.
 * 
 * @see LabClearWith
 */
void LabClear();

/**
 * @brief ������� ������ �������� ������.
 * 
 * @param color ����, ������� ����������� �����
 * @see LabClear
 */
void LabClearWith(labcolor_t color);

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
 * @brief ��������� �����.
 *
 * ��������� ����� � ������������ (x, y).
 *
 * @param x �������������� ���������� �����
 * @param y ������������ ���������� �����.
 */
void LabDrawPoint(int x, int y);

/** 
 * @brief ��������� ��������������.
 *
 * ��������� �������������� � ����� ������� � ������ ������ ������ 
 * � ������������ (x1, y1) � (x2, y2) ��������������.
 *
 * @param x1 �������������� ���������� ������ �������� ����
 * @param y1 ������������ ���������� ������ �������� ����
 * @param x2 �������������� ���������� ������� ������� ����
 * @param y2 ������������ ���������� ������� ������� ����.
 */
void LabDrawRectangle(int x1, int y1, int x2, int y2);

/** 
 * @brief ��������� ����������.
 *
 * ��������� ���������� � ������������ ������ (x, y) � �������� radius.
 *
 * @param x �������������� ���������� ������ ����������
 * @param y ������������ ���������� ������
 * @param radius ������ ����������.
 */
void LabDrawCircle(int x, int y, int radius);

/** 
 * @brief ��������� �������.
 *
 * ��������� ������� � ������������ ������ (x, y) � ��������� a � b.
 *
 * @param x �������������� ���������� ������ �������
 * @param y ������������ ���������� ������
 * @param a ������� �������
 * @param b ����� �������.
 */
void LabDrawEllipse(int x, int y, int a, int b);

/** 
 * ��������� ����� ��� ������������ ��������� ����������� ��������.
 *
 * �� ��������� (�� ������� ������ ���� �������) ������������ ������ ����.
 *
 * @param color ����� ���� �� ������������ <code>labcolor_t</code>.
 * @see labcolor_t
 */
void LabSetColor(labcolor_t color);

/** 
 * ����� �������� ����� �����.
 *
 * @return ����� ����� - ����� �������� �����.
 */
labcolor_t LabGetColor(void);
/**@}*/


/**
 * \defgroup input_group Input Methods
 *
 * ������ �������, �������������� ����������� ������ � �����������.
 * ��������� � ������: \ref input_group
 * @{
 */

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
 * @brief �������� ������� �������, �����, ������� � ����.
 * 
 * ������� ������� �������, � �����, ����� ������� ������, ���������� � ��� -
 * ASCII-��� ��� ���������� ������ ��� ��� �� ������������ <code>labkey_t</code>
 * ��� ������������ ������ (����� ��� Enter, Escape, ������� ... ).
 *
 * @return ����� ����� - ��� ������� �������.
 * @see labkey_t
 */
labkey_t LabInputKey(void);

/** 
 * �������� �� ������� �������������� ������� ������.
 *
 * @return LAB_TRUE ���� ���� �������������� ������� ������, LAB_FALSE, ���� ����� ���.
 */
labbool_t LabInputKeyReady(void);

/** @}*/


#ifdef __cplusplus
}
#endif

#endif // LABENGINE_H_INCLUDED