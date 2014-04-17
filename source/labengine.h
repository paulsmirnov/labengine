#ifndef LABENGINE_H_INCLUDED
#define LABENGINE_H_INCLUDED

#pragma comment(lib, "kernel32")
#pragma comment(lib, "user32")
#pragma comment(lib, "gdi32")

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Логический тип.
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
 * \defgroup lifecycle_group Lifecycle Methods
 *
 * Функции инициализации графического режима и окончания работы в нём.
 * Подробнее о группе: \ref lifecycle_group
 * @{
 */
/**
 * @brief Инициализация графического режима.
 * 
 * Необходим вызов перед работой с другими функциями библиотеки, то есть в графическом
 * режиме. Создаётся окно для графики.
 * По окончании работы в графическом режиме необходим вызов <code>LabTerm()</code>.
 * 
 * @return LAB_TRUE если инициализация прошла успешно, иначе - LAB_FALSE.
 * @see LabTerm
 */
labbool_t LabInit(void);

/**
 * @brief Завершение работы в графическом режиме.
 * 
 * Необходим вызов для окончания работы в графическом режиме и закрытия 
 * окна графического режима.
 * Перед началом работы в графическом режиме необходим вызов <code>LabInit()</code>.
 * @see LabInit
 */
void LabTerm(void);

/**
 * @brief Приостанавливает исполнение программы на time мс
 * 
 * @param time количество миллисекунд
 */
void LabDelay(int time);
/**@}*/


/**
 * \defgroup graphics_group Graphics Methods
 *
 * Функции графического вывода и получения информации об окне.
 * Подробнее о группе: \ref graphics_group
 * @{
 */

/**
 * @brief Названия цветов цветовой палитры.
 *
 * Используются для назначения цвета линий и других графических объектов.
 * Передаются в качестве параметра в функцию <code>LabSetColor()</code>.
 *
 * @see LabSetColor
 */
typedef enum labcolor_t 
{ 
  LABCOLOR_BLACK,         ///< Чёрный
  LABCOLOR_DARK_BLUE,     ///< Тёмно-синий
  LABCOLOR_DARK_GREEN,    ///< Тёмно-зелёный
  LABCOLOR_DARK_CYAN,     ///< Тёмно-голубой
  LABCOLOR_DARK_RED,      ///< Тёмно-красный
  LABCOLOR_DARK_MAGENTA,  ///< Тёмно-пурпурный
  LABCOLOR_BROWN,         ///< Коричневый
  LABCOLOR_LIGHT_GREY,    ///< Светло-серый
  LABCOLOR_DARK_GREY,     ///< Тёмно-серый
  LABCOLOR_BLUE,          ///< Синий
  LABCOLOR_GREEN,         ///< Зелёный
  LABCOLOR_CYAN,          ///< Голубой
  LABCOLOR_RED,           ///< Красный
  LABCOLOR_MAGENTA,       ///< Пурпурный
  LABCOLOR_YELLOW,        ///< Жёлтый
  LABCOLOR_WHITE,         ///< Белый

  LABCOLOR_COUNT          ///< Количество цветов в палитре
} labcolor_t;

/**
 * @brief Получение ширины окна.
 * 
 * @return целое число - текущая ширина окна.
 * @see LabGetHeight
 */
int LabGetWidth(void);

/**
 * @brief Получение высоты окна.
 * 
 * @return целое число - текущая высота окна.
 * @see LabGetWidth
 */
int LabGetHeight(void);

/**
 * @brief Очистка экрана черным цветом.
 * 
 * @see LabClearWith
 */
void LabClear();

/**
 * @brief Очистка экрана заданным цветом.
 * 
 * @param color цвет, которым проливается экран
 * @see LabClear
 */
void LabClearWith(labcolor_t color);

/** 
 * @brief Рисование линии.
 *
 * Рисование линии, соединяющей точки с координатами (x1, y1) и (x2, y2).
 * Последняя точка не рисуется.
 *
 * @param x1 горизонтальная координата первой точки (0 слева)
 * @param y1 вертикальная координата первой точки (0 сверху)
 * @param x2 горизонтальная координата второй точки
 * @param y2 вертикальная координата второй точки.
 */
void LabDrawLine(int x1, int y1, int x2, int y2);

/** 
 * @brief Рисование точки.
 *
 * Рисование точки с координатами (x, y).
 *
 * @param x горизонтальная координата точки
 * @param y вертикальная координата точки.
 */
void LabDrawPoint(int x, int y);

/** 
 * @brief Рисование прямоугольника.
 *
 * Рисование прямоугольника с левым верхним и правым нижним углами 
 * с координатами (x1, y1) и (x2, y2) соответственно.
 *
 * @param x1 горизонтальная координата левого верхнего угла
 * @param y1 вертикальная координата левого верхнего угла
 * @param x2 горизонтальная координата правого нижнего угла
 * @param y2 вертикальная координата правого нижнего угла.
 */
void LabDrawRectangle(int x1, int y1, int x2, int y2);

/** 
 * @brief Рисование окружности.
 *
 * Рисование окружности с координатами центра (x, y) и радиусом radius.
 *
 * @param x горизонтальная координата центра окружности
 * @param y вертикальная координата центра
 * @param radius радиус окружности.
 */
void LabDrawCircle(int x, int y, int radius);

/** 
 * @brief Рисование эллипса.
 *
 * Рисование эллипса с координатами центра (x, y) и полуосями a и b.
 *
 * @param x горизонтальная координата центра эллипса
 * @param y вертикальная координата центра
 * @param a большая полуось
 * @param b малая полуось.
 */
void LabDrawEllipse(int x, int y, int a, int b);

/** 
 * Установка цвета для последующего рисования графических объектов.
 *
 * По умолчанию (до первого вызова этой функции) используется чёрный цвет.
 *
 * @param color новый цвет из перечисления <code>labcolor_t</code>.
 * @see labcolor_t
 */
void LabSetColor(labcolor_t color);

/** 
 * Номер текущего цвета кисти.
 *
 * @return целое число - номер текущего цвета.
 */
labcolor_t LabGetColor(void);
/**@}*/


/**
 * \defgroup input_group Input Methods
 *
 * Группа функций, обеспечивающая возможность работы с клавиатурой.
 * Подробнее о группе: \ref input_group
 * @{
 */

/**
 * @brief Коды несимвольных клавиш.
 *
 * Специальные коды для клавиш Enter, Escape, стрелок...
 * Наряду с ASCII-кодами символьных клавиш, являются возвращаемым значением
 * из функции <code>LabInputKey()</code>.
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
  LABKEY_LEFT = 0x2500,       ///< Стрелка влево
  LABKEY_UP = 0x2600,         ///< Стрелка вверх
  LABKEY_RIGHT = 0x2700,      ///< Стрелка вправо
  LABKEY_DOWN = 0x2800,       ///< Стрелка вниз
 
} labkey_t;

/** 
 * @brief Ожидание нажатия клавиши, затем, возврат её кода.
 * 
 * Ожидает нажатия клавиши, а затем, когда клавиша нажата, возвращает её код -
 * ASCII-код для символьных клавиш или код из перечисления <code>labkey_t</code>
 * для несимвольных клавиш (такий как Enter, Escape, стрелки ... ).
 *
 * @return целое число - код нажатой клавиши.
 * @see labkey_t
 */
labkey_t LabInputKey(void);

/** 
 * Проверка на наличие необработанных нажатий клавиш.
 *
 * @return LAB_TRUE если есть необработанные нажатия клавиш, LAB_FALSE, если таких нет.
 */
labbool_t LabInputKeyReady(void);

/** @}*/


#ifdef __cplusplus
}
#endif

#endif // LABENGINE_H_INCLUDED