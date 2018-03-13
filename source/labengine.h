#ifndef LABENGINE_H_INCLUDED
#define LABENGINE_H_INCLUDED
#pragma once

/**
 * @mainpage Introduction
 
LabEngine ������������ ����� ���������� ����������� ���������� ��� ����������
������������ ����� � ������ ���������� ���������� ��� Windows. ��� ���������
�� ������ �����, �� �������� � ����������� ������� ���������� � ������� ������
� WinAPI, ������ ��������� ������������ � ����������� �������.

����������� ��������� � �������������� ���������� �������� ��������� �������:

@code{.c}
#include "labengine.h"

int main(void)
{
  if (LabInit())    // �������������
  {
    // ...          // ����� ����� ���-�� �������

    LabTerm();      // ���������� ������
  }
  return 0;
}
@endcode

��� ��������� �������������� ���������� �, � ������ ������, ����� �� ���������
������ ���������. ����� ������ ��������, ��� ����������� ���� ��� ���������, ��
��� �� �����������.

����� ����������� ������ ��������� �������� ���:

@code
#include "labengine.h"

int main(void)
{
  // ���������������� ����������
  if (LabInit())
  {
    // ������ ������� ����
    int width = LabGetWidth();
    int height = LabGetHeight();
  
    // ���������� ������� �������������
    LabSetColor(LABCOLOR_RED);
    LabDrawRectangle(0, 0, width, height);
    
    // ���������� ������ �����
    LabSetColor(LABCOLOR_GREEN);
    LabDrawLine(1, 1, width - 1, height - 1);
    LabDrawLine(1, height - 2, width - 1, 0);

    // ���������� �������� � ����
    LabDrawFlush();

    // ��������� ������� ������� � ��������� ������
    LabInputKey();
    LabTerm();
  }
  return 0;
}
@endcode

�������� �������� �� ���������� (���� --- � ����� ������� ����). �� ���������
� Windows ���������� �������� ����� ����� � ��������������� �� �������� (�����
�� ������� �� ��������� ���������).

��������� ���������� ���������:

@image html basics.png

*/

#pragma comment(lib, "kernel32")
#pragma comment(lib, "user32")
#pragma comment(lib, "gdi32")

#if defined(_DEBUG) || !defined(NDEBUG)
#define LABENGINE_LIB_SUFFIX "-dbg"
#else
#define LABENGINE_LIB_SUFFIX
#endif

#ifdef _MSC_VER
#if   _MSC_VER >= 1900 // Visual Studio 2015
#pragma comment(lib, "labengine-vs15" LABENGINE_LIB_SUFFIX)
#elif _MSC_VER >= 1800 // Visual Studio 2013
#pragma comment(lib, "labengine-vs13" LABENGINE_LIB_SUFFIX)
#elif _MSC_VER >= 1700 // Visual Studio 2012
#pragma comment(lib, "labengine-vs12" LABENGINE_LIB_SUFFIX)
#elif _MSC_VER >= 1600 // Visual Studio 2010
#pragma comment(lib, "labengine-vs10" LABENGINE_LIB_SUFFIX)
#elif _MSC_VER >= 1500 // Visual Studio 2008
#pragma comment(lib, "labengine-vs08" LABENGINE_LIB_SUFFIX)
#else
#error This Visual Studio version is not supported.
#endif
#else
#error This compiler is not supported.
#endif

#undef LABENGINE_LIB_SUFFIX

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
 * @defgroup lifecycle_group Lifecycle Methods
 *
 * ������� ������������� ������������ ������ � ��������� ������ � ��.
 *
 * @{
 */

/**
 * ��������� ������������� ����������.
 *
 * ������������ ��� ������ ������� LabInitWith(). ��� ������ LabInit()
 * ��������� �� �����������, ������������ ��������� �������� �� ���������.
 */
typedef struct labparams_t
{
  unsigned width;  ///< ������ ������ ��� ���������
  unsigned height; ///< ������ ������ ��� ���������
  unsigned scale;  ///< ����������� ��������������� ������ ��� ������ �� �����
} labparams_t;

/**
 * @brief ���������������� ����������.
 * 
 * ����� ������� � ������� ��������� ���������� ���������� � ����������������.
 * ��� ���� ����������� ���������� ��������� ������, �������� �����������
 * ����. ��� ������������� ������ ���������, �������� �� �������� �� ���������,
 * ������� ������������ �������� LabInitWith().
 * �� ��������� ������ � ����������� ��������� ����� LabTerm().
 * 
 * @return @ref LAB_TRUE ���� ������������� ������ �������, ����� - @ref LAB_FALSE.
 * @see LabInitWith, LabTerm
 */
labbool_t LabInit(void);

/**
 * @brief ���������������� ���������� � �����������.
 *
 * ����� ������� � ������� ��������� ���������� ���������� � ����������������.
 * ��� ���� ����������� ���������� ��������� ������, �������� �����������
 * ����. ���� ��� ������������� �������� ���������, �������� �� �������� �� ���������,
 * ������� ������������ �������� LabInit().
 * �� ��������� ������ � ����������� ��������� ����� LabTerm().
 *
 * @param params ��������� ������������� ����������
 *
 * @return @ref LAB_TRUE ���� ������������� ������ �������, ����� - @ref LAB_FALSE.
 * @see LabInit, LabTerm
 */
labbool_t LabInitWith(labparams_t const* params);

/**
 * @brief ��������� ������ c �����������.
 * 
 * ��������� ����� ��� ��������� ������ � ����������� ������ � �������� 
 * ���� ������������ ������.
 * ����� ������� ������ � ����������� ������ ��������� ����� LabInit().
 *
 * @see LabInit, LabInitWith
 */
void LabTerm(void);

/**
 * @brief ������������� ���������� ���������.
 * 
 * @param time ���������� �����������
 */
void LabDelay(int time);

/**@}*/


/**
 * @defgroup graphics_group Graphics Methods
 *
 * ������� ������������ ������ � ��������� ���������� �� ����.
 * ��������� � ������: @ref graphics_group
 * @{
 */

/**
 * @brief �������� ������ �������� �������.
 *
 * ������������ ��� ���������� ����� ����� � ������ ����������� ��������.
 * ���������� � �������� ��������� � ������� LabSetColor().
 *
 * @see LabSetColor
 */
typedef enum labcolor_t 
{ 
  LABCOLOR_NA = -1,       ///< ���� ����������

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

  LABCOLOR_COUNT,         ///< ���������� ������ � �������
} labcolor_t;

/**
 * @brief ������ ������ ����.
 * 
 * @return ������ ������������ ����.
 * @see LabGetHeight
 */
int LabGetWidth(void);

/**
 * @brief ������ ������ ����.
 * 
 * @return ������ ������������ ����.
 * @see LabGetWidth
 */
int LabGetHeight(void);

/**
 * @brief �������� ����.
 *
 * ��� ������� ������������ ������ ����. ��� ������� ����� ������
 * ������� ���� ������ ������, ���������������� �������� LabClearWith().
 *
 * @see LabClearWith
 */
void LabClear();

/**
 * @brief �������� ���� �������� ������.
 * 
 * @param color ����, ������� ����������� �����
 * @see LabClear
 */
void LabClearWith(labcolor_t color);

/**
 * @brief ���������� ������� ����.
 *
 * ��������� ������ ���� �� ������ ����������� ������� � �������� ��������,
 * ������������� ��� ������������ ��������� ����������� ��������
 * ��������� <code>LabDraw...()</code>.
 * �� ��������� (�� ������� ������ ���� �������) ������������ ����� ����
 * (@ref LABCOLOR_WHITE).
 *
 * @param color ����� ���� �� ������������ <code>labcolor_t</code>.
 * @see labcolor_t, LabSetColorRGB()
 */
void LabSetColor(labcolor_t color);

/** 
 * @brief ���������� ������� ���� �� �����������.
 *
 * ��������� ������� ���� �� 16,777,216 ������ � �������� ��������,
 * ������������� ��� ������������ ��������� ����������� ��������
 * ��������� <code>LabDraw...()</code>. ����������� ������� ��� ����
 * ������������, ���� ������������ ��� ���������� �������� ���������
 * ��������� (�������) - �������, ������ � �����, � �������� ��
 * 0 (���������� ���������) �� 255 (������������ ������� ����������).
 *
 * ��������� ��������� ���� �� ����������� ����������� �����������
 * �������, ������� LabGetColor() ����� ���������� @ref LABCOLOR_NA.
 *
 * @param r ������� ������� ����������, �� 0 �� 255.
 * @param g ������� ������ ����������, �� 0 �� 255.
 * @param b ������� ����� ����������, �� 0 �� 255.
 *
 * @see LabSetColor
 */
void LabSetColorRGB(int r, int g, int b);

/** 
 * @brief ������ ������� ����.
 *
 * @return ������� ���� �� ����������� �������, ������������ ��� ���������
 * ��������� <code>LabDraw...()</code>.
 */
labcolor_t LabGetColor(void);


/** 
 * @brief ���������� ������ �����.
 *
 * ��������� �����, ����������� ����� � ������������ (x1, y1) � (x2, y2).
 * ��������� ����� �� ��������.
 *
 * @param x1 �������������� ���������� ������ ����� (0 �����)
 * @param y1 ������������ ���������� ������ ����� (0 ������)
 * @param x2 �������������� ���������� ������ �����
 * @param y2 ������������ ���������� ������ �����.
 */
void LabDrawLine(int x1, int y1, int x2, int y2);

/** 
 * @brief ���������� �����.
 *
 * ��������� ����� � ������������ (x, y).
 *
 * @param x �������������� ���������� �����
 * @param y ������������ ���������� �����.
 */
void LabDrawPoint(int x, int y);

/** 
 * @brief ���������� �������������.
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
 * @brief ���������� ����������.
 *
 * ��������� ���������� � ������������ ������ (x, y) � �������� radius.
 *
 * @param x �������������� ���������� ������ ����������
 * @param y ������������ ���������� ������
 * @param radius ������ ����������.
 */
void LabDrawCircle(int x, int y, int radius);

/** 
 * @brief ���������� ������.
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
 * @brief �������� ���������� ����� ��������� �� �����.
 *
 * ����� ����������� ������� <code>LabDraw...()</code>, ��� �����������
 * ������ ������������������ ��� ������ �� �������� �� ������, � ��
 * ��������������� ������ � ������. �� ����� ���������� ����� ������
 * �������� ������ �����, ����� ����� ������� ������������ �������,
 * ���� ��� ����� ������ ���� �������.
 *
 * �������� ������������� ���� ������� ����������� � ��������������� ����������
 * ������� ����������� � ������ � ����������� ����������� ������������
 * ��� �� �����. ��� ������� �������� � ��������� ���������� �������� ������
 * ������� ����� ���������� ������� �����.
 *
 */
void LabDrawFlush(void);

/**@}*/


/**
 * @defgroup input_group Input Methods
 *
 * ������ �������, �������������� ����������� ������ � �����������.
 * ��������� � ������: @ref input_group
 * @{
 */

/**
 * @brief ���� ������������ ������.
 *
 * ����������� ���� ��� ������ Enter, Escape, �������...
 * ������ � ASCII-������ ���������� ������, �������� ������������ ���������
 * �� ������� LabInputKey().
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
 * @brief ������� ������� ������� � ������ � ���.
 * 
 * ������� ������� �������, � �����, ����� ������� ������, ���������� � ��� -
 * ASCII-��� ��� ���������� ������ ��� ��� �� ������������ <code>labkey_t</code>
 * ��� ������������ ������ (����� ��� Enter, Escape, ������� ... ).
 *
 * @return ��� ������� �������.
 * @see labkey_t
 */
labkey_t LabInputKey(void);

/**
 * ��������� ������� �������������� ������� ������.
 *
 * @return @ref LAB_TRUE ���� ���� �������������� ������� ������, @ref LAB_FALSE, ���� ����� ���.
 */
labbool_t LabInputKeyReady(void);

/** @}*/


#ifdef __cplusplus
}
#endif

#endif // LABENGINE_H_INCLUDED
