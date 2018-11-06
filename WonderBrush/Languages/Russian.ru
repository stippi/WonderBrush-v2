// Russian
// author: Va Milushnikov

// addon stuff
extern "C" {
_EXPORT void init_strings(const char* strings[]);
}

// init_strings
void
init_strings(const char* strings[])
{
	strings[FILE_MENU]			= "Файл";
	strings[NEW]				= "Новый";
	strings[NEW_CANVAS]			= "New Canvas";			// tool tip
	strings[OPEN]				= "Открыть";
	strings[OPEN_CANVAS]		= "Open Canvas";
	strings[SAVE_CANVAS]		= "Сохранить проект";
	strings[CLOSE]				= "Закрыть";
	strings[CLOSE_CANVAS]		= "Close Canvas";		// tool tip
	strings[QUIT]				= "Выход";
	strings[EDIT]				= "Редактировать";
	strings[UNDO]				= "Отменить";
	strings[REDO]				= "Восстановить";
//	strings[SETTINGS]			= "Окно";
	strings[FULLSCREEN]			= "На полный экран";
	strings[LANGUAGE]			= "Язык";
	strings[CANVAS]				= "Canvas";				// ie project, document
	strings[IMAGE]				= "Рисунок";				// ie bitmap
	strings[CLEAR]				= "Очистить";				// empties a layer or the entire image (not delete!)
	strings[FORMAT]				= "ФОрмат";				// color space of the image (RGB, Lab etc)
	strings[LAYER]				= "Слой";
	strings[MERGE_DOWN]			= "Смешать с нижним";			// combining of of current layer and the one underneath it
	strings[MERGE_VISIBLE]		= "Смешать видимые";		// combining of all visible layers
	strings[DELETE]				= "Удалить";
	// layer blending modes
	strings[MODE]				= "Режим";
	strings[NORMAL]				= "Нормальный";
	strings[MULTIPLY]			= "Multiply";
	strings[INVERSE_MULTIPLY]	= "Inverse Multiply";

	strings[HISTORY]			= "История";			// as in "modifier stack"
	strings[FREEZE]				= "Очистить историю";				// renders the result of the modifier stack and removes the modifiers (ie, you cannot edit it anymore)
	strings[CUT]				= "Вырезать";
	strings[COPY]				= "Скопировать";
	strings[PASTE]				= "Вставить";

	strings[SWATCHES]			= "Цвета";			// color palette

	strings[ZOOM]				= "Масштаь";
	strings[ALPHA]				= "Alpha";				// keep this as short as possible
	// brush options
	strings[OPACITY]			= "Прозрачность";
	strings[RADIUS]				= "Радиус";
	strings[HARDNESS]			= "Форма кисти";			// controls brush shape (soft...hard)
	strings[SPACING]			= "Отступ от кисти";			// distance in % of brush diameter at wich brush shapes will be put into the bitmap along the line of a stroke
	strings[SUBPIXELS]			= "Subpixels";			// allow subpixel precise positioning of tools
	strings[UNNAMED]			= "Безымянный";
	strings[DEFAULT]			= "По умолчанию";
	strings[TOOL]				= "Tool";				// for tool tips
	strings[ANY_MODIFIER]		= "Любой модификатор";		// as in modifier key on the keyboard (shift, option, alt etc)
	// tools
	strings[BRUSH]				= "Кисть";
	strings[CLONE]				= "Клонирование";				// clone bursh (transfers parts of an image to another place)
	strings[ERASER]				= "Стереть";
	strings[PEN]				= "Карандаш";
	strings[RESTORE]			= "Восстановить";			// opposite of eraser (restores alpha channel)
	strings[DROPPER]			= "Проба цвета";			// color picker
	strings[TEXT]				= "Текст";
	// zoom
	strings[ZOOM_IN]			= "Увеличить";			// tool tips
	strings[ZOOM_OUT]			= "Уменьшить";
	// actions
	strings[ADD_LAYER]			= "Добавить слой";			// the following strings are for display in the undo/redo menu
	strings[MERGE_LAYERS]		= "Смешать слой";		// a "modifier" is a step of changing the bitmap, ie a brush stroke or something else.
	strings[UNKOWN_MODIFIER]	= "Неизвестный модификатор";
	strings[ADD_MODIFIER]		= "Добавить модификатор";
	strings[ADD_MODIFIERS]		= "Добавить модификаторы";
	strings[CHANGE_COLOR]		= "Изменить цвет";
	strings[CHANGE_COLORS]		= "Изменить цвета";
	strings[REMOVE_MODIFIER]	= "Удалить модификатор";
	strings[REMOVE_MODIFIERS]	= "Удалить модификаторы";
	strings[MOVE_MODIFIER]		= "Переместить модификатор";
	strings[MOVE_MODIFIERS]		= "Переместить модификаторы";

	strings[OK]					= "Ok";
	strings[CANCEL]				= "Отменить";
	strings[NAME]				= "Имя";				// name of project
	strings[PICK_COLOR]			= "Выбрать цвет";			//  window title of color picker
	strings[WIDTH]				= "Ширина";				// pixel count
	strings[HEIGHT]				= "Высота";
}
