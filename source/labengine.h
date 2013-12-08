#ifndef LABENGINE_H_INCLUDED
#define LABENGINE_H_INCLUDED

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
 * @brief Названия цветов цветовой палитры.
 *
 * Используются для назначения цвета линий и других графических объектов.
 * Передаются в качестве параметра в функцию <code>LabSetColor()</code>.
 *
 * @see LabSetColor
 */
typedef enum labcolor_t 
{ 
  LABCOLOR_RED = 0x000C,                                          ///< Красный
  LABCOLOR_GREEN = 0x000A,                                        ///< Зелёный
  LABCOLOR_BLUE = 0x0009,                                         ///< Синий
  LABCOLOR_BLACK = LABCOLOR_RED & LABCOLOR_GREEN & LABCOLOR_BLUE, ///< Чёрный
  LABCOLOR_WHITE = LABCOLOR_RED | LABCOLOR_GREEN | LABCOLOR_BLUE, ///< Белый
  LABCOLOR_YELLOW = LABCOLOR_RED | LABCOLOR_GREEN,                ///< Жёлтый
  LABCOLOR_CYAN = LABCOLOR_GREEN | LABCOLOR_BLUE,                 ///< Голубой
  LABCOLOR_MAGENTA = LABCOLOR_BLUE | LABCOLOR_RED,                ///< Пурпурный
    
} labcolor_t;

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
 * @brief Рисование линии.
 *
 * Рисование линии, соединяющей точки с координатами (x1, y1) и (x2, y2).
 *
 * @param x1 горизонтальная координата первой точки (0 слева)
 * @param y1 вертикальная координата первой точки (0 сверху)
 * @param x2 горизонтальная координата второй точки
 * @param y2 вертикальная координата второй точки.
 */
void LabDrawLine(int x1, int y1, int x2, int y2);

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
int LabInputKey(void);

/** 
 * Проверка на наличие необработанных нажатий клавиш.
 *
 * @return LAB_TRUE если есть необработанные нажатия клавиш, LAB_FALSE, если таких нет.
 */
labbool_t LabInputKeyReady(void);

/** 
 * Установка цвета для последующего рисования графических объектов.
 *
 * По умолчанию (до первого вызова этой функции) используется чёрный цвет.
 *
 * @param color новый цвет из перечисления <code>labcolor_t</code>.
 * @see labcolor_t
 */
void LabSetColor(labcolor_t color);


#ifdef __cplusplus
}
#endif

#endif // LABENGINE_H_INCLUDED