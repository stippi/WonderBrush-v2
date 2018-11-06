// Polski
// author: Marcin Konicki <ahwayakchih@neoni.net>

// addon stuff
extern "C" {
_EXPORT void init_strings(const char* strings[]);
}

// NOTES: Try to be as crisp as possible. If possible, try to make super menu labels
// and sub-items read as one command, ie consider the sub-item label as within the
// context of the super item label.
// Fine tune the alignment of tool tips by manually putting the linebreak character
// '\n' at the best position. You can change the Translation while WonderBrush is running!
// Try to sound professional, prefer passive over active voice. In the English translation,
// I don't follow this myself everywhere, but for example in German it sounds more professional
// to say "Doubleclick opens panel." than to say "Doubleclick to open panel!".
// "opens panel" is also better than to say "will open panel". Keep "professional" in mind.
// If you can still manage to add a certain level of coolness to the formulation, even better! :-)

// IMPORTANT: Translations are compiled as addons during runtime!
// If for some reason WonderBrush doesn't show your translation, it is most likely
// a syntaxt error in the file. Launch WonderBrush from the command line and see what it has to say.
// If you see an error message from GCC, it might be helpful, but be aware that the line number where it failed
// does not reference the correct line number in the original translation, since WonderBrush prepends
// some lines before compiling the addon.

// init_strings
void
init_strings(const char* strings[])
{
	strings[FILE_MENU]			= "Plik";
	strings[NEW]				= "Nowy";						// will show up in Canvas menu
	strings[NEW_CANVAS]			= "Nowy Projekt";				// tool tip
	strings[NEW_LAYER]			= "Nowa";						// these strings were added
	strings[NEW_BRUSH]			= "Nowy";						// by request to be able to
	strings[NEW_PALETTE]		= "Nowa";						// reflect different genders in "New"
	strings[OPEN]				= "Otwórz";
	strings[LOAD]				= "Wgraj";
	strings[OPEN_CANVAS]		= "Otwórz Projekt";
	strings[EXPORT_CANVAS]		= "Eksportuj Projekt";
	strings[EXPORT]				= "Eksportuj";
	strings[EXPORT_AS]			= "Eksportuj jako";
	strings[SAVE_CANVAS]		= "Zapisz Projekt";
	strings[SAVE]				= "Zapisz";
	strings[SAVE_AS]			= "Zapisz jako";
	strings[SETUP_PAGE]			= "Ustawienia wydruku";
	strings[PRINT]				= "Drukuj";
	strings[CLOSE]				= "Zamknij";
	strings[CLOSE_CANVAS]		= "Zamknij Projekt";			// tool tip
	strings[ABOUT]				= "O programie";
	strings[QUIT]				= "Zakończ";

	strings[LOADING_FILE]		= "Wczytywanie pliku:";

	strings[EDIT]				= "Edycja";
	strings[UNDO]				= "Cofnij";
	strings[REDO]				= "Powtórz";
	
	strings[SETTINGS]			= "Ustawienia";
	strings[FULLSCREEN]			= "Pełny ekran";
	strings[SHOW_GRID]			= "Pokaż siatkę pikseli";		// if zoomed in far enough,
																// display a grid arround each pixel
	strings[LANGUAGE]			= "Język";
	strings[CANVAS]				= "Projekt";					// ie project, document
	strings[IMAGE]				= "Obraz";						// ie bitmap
	strings[CLEAR]				= "Wyczyść";
	strings[RESIZE]				= "Skaluj";
	strings[ROTATE_90]			= "90°";
	strings[ROTATE_180]			= "180°";
	strings[ROTATE_270]			= "-90°";

	strings[FORMAT]				= "Format";						// color space of the canvas (RGB, Lab etc)

	strings[EXPORT_FORMAT]		= "Format";						// ie PNG, SVG etc
	strings[CURSOR_SOURCE]		= "Plik źródłowy C Kursor";
	strings[BITMAP_SOURCE]		= "Plik źródłowy C Bitmapę";
	strings[SVG_FORMAT]			= "SVG (Nie wszystkie typy obiektów)";
	strings[AI_FORMAT]			= "Adobe Illustrator (Nie wszystkie typy obiektów)";
	strings[RDEF_FORMAT]		= "Kod źródłowy RDef";

	strings[LAYER]				= "Warstwa";
	strings[DUPLICATE]			= "Duplikuj";
	strings[MERGE_DOWN]			= "Scal z poniższą warstwą";	// combining of current layer and the
																// one underneath it
	strings[MERGE_VISIBLE]		= "Scal wszystkie widoczne";	// combining of all visible layers
	strings[RENAME]				= "Zmień nazwę";
	strings[DELETE]				= "Usuń";
	strings[DELETE_ALL]			= "Usuń wszystkie";

	// layer blending modes
	strings[MODE]				= "Tryb";
	strings[NORMAL]				= "Normalny";
	strings[ERASE]				= "Gumka";
	strings[MULTIPLY]			= "Pomnażanie";
	strings[INVERSE_MULTIPLY]	= "Odwrócone pomnażanie";
	strings[LUMINANCE]			= "Limunescencja";
	strings[MULTIPLY_ALPHA]		= "Krycie";
	strings[MULTIPLY_INVERSE_ALPHA]	= "Odwrotne krycie";
	strings[REPLACE_RED]		= "Zamiana Czerwonego";
	strings[REPLACE_GREEN]		= "Zamiana Zielonego";
	strings[REPLACE_BLUE]		= "Zamiana Niebieskiego";
	strings[DARKEN]				= "Przyciemnienie";
	strings[LIGHTEN]			= "Rozjaśnienie";

	strings[HISTORY]			= "Historia";					// as in "modifier stack"
	strings[OBJECT]				= "Obiekt";						// new name for the "History"
	strings[FREEZE]				= "Scal w bitmapę";				// renders the result of the modifier stack
																// into one "bitmap modifier" and replaces
																// all previous modifiers (ie, you cannot edit
																// them anymore) by the single bitmap modifier
	strings[CUT]				= "Wytnij";
	strings[COPY]				= "Kopiuj";
	strings[PASTE]				= "Wklej";
	strings[MULTI_PASTE]		= "Wklej do wielu";				// paste one ore more properties to
																// *multiple* objects at once
	strings[REMOVE_ALL]			= "Usuń wszystkie";

	strings[OBJECT_SELECTION]	= "Wybór obiektów";				// change selection shortcuts
	strings[SELECT_ALL_OBJECTS]	= "Wszystkie";					// in Object and Property list
	strings[SELECT_NO_OBJECTS]	= "Żaden";
	strings[INVERT_SELECTION]	= "Odwrotnie";
	strings[ALL_OBJECTS]		= "Wszystkie";
	strings[PROPERTY_SELECTION]	= "Wybór właściwości";
	strings[SELECT_ALL_PROPERTIES] = "Wszystkie";
	strings[SELECT_NO_PROPERTIES] = "Żadna";
	strings[SPECIAL]			= "Specjalne";

	// filters
	strings[ADD_FILTER]			= "Dodaj filtr";				// the actual menu
	strings[DROP_SHADOW]		= "Cień";
	strings[GAUSSIAN_BLUR]		= "Rozmycie Gaussa";
	strings[INVERT]				= "Negatyw";
	strings[BRIGHTNESS]			= "Jasność";
	strings[CONTRAST]			= "Kontrast";
	strings[SATURATION]			= "Saturacja";
	strings[MOTION_BLUR]		= "Rozmycie ruchu";
	strings[HALFTONE]			= "Półton";
	strings[DOT]				= "Kropka";						// mode options of Halftone
	strings[LINE]				= "Linia";
	strings[DITHER]				= "Szum";
	strings[DITHER_FS]			= "Szum (FS)";
	strings[DITHER_NC]			= "Szum (NK)";
	strings[NOISE]				= "Mora";
	strings[HUE_SHIFT]			= "Przesunięcie barwy";			// filter not yet written
	strings[DISPERSION]			= "Rozproszenie";
	strings[STRENGTH]			= "Nasilenie";
	strings[LUMINANCE_ONLY]		= "Tylko luminescencja";
	strings[WARPSHARP]			= "Warpsharp";
	strings[LAMBDA]				= "Lambda";
	strings[MU]					= "µ";
	strings[NON_MAXIMAL_SUPPRESSION] = "NMS";
	strings[SOLID_AREA]			= "Obszar";

	strings[PROPERTY]			= "Właściwość";
	strings[UNKOWN_PROPERTY]	= "(nieznana)";

	strings[SWATCHES]			= "Paleta barw";				// color palette
	strings[ZOOM]				= "Przybliżenie";
	strings[ALPHA]				= "Krycie";						// keep this as short as possible
	strings[ALPHA_CHANNEL]		= "Kanał przezroczystości";		// used for properties
	strings[COLOR]				= "Barwa";						// used for properties
	strings[GRADIENT]			= "Gradient";					// used for properties (?)

	// pick objects tool
	strings[SELECTABLE]			= "Wybór obiektów";				// which object types are "selectable"

	// gradient options
	strings[GRADIENT_TYPE]					= "Typ";
	strings[GRADIENT_TYPE_LINEAR]			= "Linearny";
	strings[GRADIENT_TYPE_RADIAL]			= "Okręg";
	strings[GRADIENT_TYPE_DIAMONT]			= "Diament";
	strings[GRADIENT_TYPE_CONIC]			= "Stożek";
	strings[GRADIENT_TYPE_XY]				= "X-Y";
	strings[GRADIENT_TYPE_XY_SQRT]			= "Kwadrat(X-Y)";
	strings[GRADIENT_INTERPOLATION]			= "Interpolacja";
	strings[GRADIENT_INTERPOLATION_LINEAR]	= "Linearna";
	strings[GRADIENT_INTERPOLATION_SMOOTH]	= "Gładka";
	strings[GRADIENT_INHERITS_TRANSFORM]	= "Dziedzicz Transformacje";

	// brush options
	strings[OPACITY]			= "Krycie";
	strings[MIN_OPACITY]		= "Krycie min.";					// property label (keep short)
	strings[DYN_OPACITY]		= "Krycie dyn.";					// property label (keep short)
	strings[RADIUS]				= "Promień";
	strings[MIN_RADIUS]			= "Promień min.";				// property label (keep short)
	strings[DYN_RADIUS]			= "Promień dyn.";				// property label (keep short)
	strings[HARDNESS]			= "Twardość";					// controls brush shape (soft...hard)
	strings[MIN_HARDNESS]		= "Twardość min.";				// property label (keep short)
	strings[DYN_HARDNESS]		= "Twardość dyn.";				// property label (keep short)
	strings[SPACING]			= "Odstępy";					// distance in % of brush diameter at wich brush
																// shapes will be put into the bitmap along the
																// line of a stroke
																// ! also used for Text tool (spacing between chars)
	strings[MIN_SPACING]		= "Odstępy min.";				// property label (keep short), not yet used
	strings[DYN_SPACING]		= "Odstępy dyn.";				// property label (keep short), not yet used
	strings[SUBPIXELS]			= "Podpiksele";					// allow subpixel precise positioning of tools
	strings[SOLID]				= "Twardy";
	strings[TILT]				= "Przechylenie";				// tablet specific, tilt of pen
	strings[BLUR_RADIUS]		= "Prom. rozmycia";				// property label (keep short), also used for Dispersion filter
	// translate options
	strings[X_LABEL]			= "X";
	strings[Y_LABEL]			= "Y";
	strings[X_SCALE]			= "Skala X";
	strings[Y_SCALE]			= "Skala Y";
	strings[TRANSLATION_X]		= "X";
	strings[TRANSLATION_Y]		= "Y";
	strings[SCALE_X]			= "X";
	strings[SCALE_Y]			= "Y";
	strings[ANGLE]				= "Kąt";

	strings[X_OFFSET]			= "Offset X";
	strings[Y_OFFSET]			= "Offset Y";
	// crop & select options
	strings[LEFT]				= "Lewa";
	strings[TOP]				= "Góra";
	strings[ENTIRE_CANVAS]		= "Wszystko";
	// bucket fill options
	strings[TOLERANCE]			= "Tolerancja";
	strings[SOFTNESS]			= "Miękkość";
	strings[CONTIGUOUS_AREA]	= "Sąsiadujące piksele";
	// shape options
	strings[PATH]				= "Ścieżka";					// property label
	strings[OUTLINE]			= "Kontur";
	strings[TRANSFORM]			= "Transformacja";
	strings[CLOSED]				= "Zamknięty";					// ie first and last point of path
																// are connected
	strings[CAP_MODE]			= "Końcówka";
	strings[BUTT_CAP]			= "Kwadratowa";
	strings[SQUARE_CAP]			= "Kwadratowa zewnętrzna";
	strings[ROUND_CAP]			= "Zaokrąglona";
	strings[JOIN_MODE]			= "Łączenie";
	strings[MITER_JOIN]			= "Kąt prosty";
	strings[ROUND_JOIN]			= "Zaokrąglone";
	strings[BEVEL_JOIN]			= "Ścięte";

	strings[CIRCLE]				= "Okrąg";						// not yet used
	strings[RECT]				= "Kwadrat";					// not yet used
	strings[ROUND_CORNERS]		= "Zaokrąglone rogi";			// not yet used
	strings[TOGGLE_ROUND_CORNERS]		= "Przełącz zaokrąglone rogi";		// not yet used
	strings[ROUND_CORNER_RADIUS]		= "Promień";						// not yet used
	strings[CHANGE_ROUND_CORNER_RADIUS]	= "Zmień promień zaokrąglonych rogów";	// not yet used
	// text options
	strings[FONT]				= "Czcionka";
	strings[SIZE]				= "Rozmiar";
	strings[ROTATION]			= "Obrót";
	strings[TEXT_INPUT]			= "Tekst";						// label for text control
	strings[ADVANCE_SCALE]		= "Odstępy";					// additional spacing between chars
																// (not used anymore)
	strings[HINTING]			= "Hinting";					// not used, hinting means that the shapes
																// of chars are modified to more directly
																// hit pixels
	strings[KERNING]			= "Kerning";					// kerning means that the distance between two
																// glyphs is individual per their combination
	strings[TEXT_LINE_SPACING]	= "Odstępy wierszy";			// font height * line spacing = vertical line dist
	// color picker tool
	strings[TIP_SIZE]			= "Rozmiar końcówki";
	strings[PIXEL_1x1]			= "1x1 pikseli";
	strings[PIXEL_3x3]			= "3x3 pikseli";
	strings[PIXEL_5x5]			= "5x5 pikseli";
	strings[INCLUDE_ALL_LAYERS]	= "Uwzględniaj wszystkie warstwy";
	// tools
	strings[BRUSH]				= "Pędzel";
	strings[CLONE]				= "Kopia";						// clone bursh (transfers parts of an image to another place)

	strings[ERASER]				= "Gumka - pędzel";
	strings[PEN]				= "Ołówek";
	strings[PEN_ERASER]			= "Gumka - ołówek";
	strings[RESTORE]			= "Krycie";						// opposite of eraser (restores alpha channel)
	strings[DROPPER]			= "Pobór barwy";				// color picker
	strings[TEXT]				= "Tekst";
	strings[BLUR]				= "Rozmycie";
	strings[BITMAP]				= "Bitmapa";
	strings[SELECT]				= "Wybierz";
	strings[TRANSLATE]			= "Transformacja";
	strings[CROP]				= "Przycięcie";
	strings[BUCKET_FILL]		= "Wypełnienie";
	strings[SHAPE]				= "Kształt";
	strings[ELLIPSE]			= "Elipsa";
	strings[ROUND_RECT]			= "(Zaokrąglony) Prostokąt";	// the tool also creates non-round rectangles
	strings[EDIT_GRADIENT]		= "Edytor Gradientu";
	strings[GUIDES]				= "Wyznaczniki";
	strings[PICK_OBJECTS]		= "Wybór obiektów";
	strings[CLIPBOARD]			= "Schowek";

	// zoom
	strings[ZOOM_IN]			= "Przybliż";					// tool tips
	strings[ZOOM_OUT]			= "Oddal";

	// actions
	strings[ADD_LAYER]			= "Dodaj Warstwę";				// the following strings are for display in the
																// undo/redo menu
	strings[REMOVE_LAYER]		= "Usuń Warstwę";				// a "modifier" is a step of changing the bitmap,
																// ie a brush stroke or something else.
	strings[REMOVE_LAYERS]		= "Usuń Warstwy";
	strings[MOVE_LAYER]			= "Przenieś Warstwę";			// ie drag sorting to another pos in list
	strings[MOVE_LAYERS]		= "Przenieś Warstwy";
	strings[MERGE_LAYERS]		= "Scal Warstwy";
	strings[UNKOWN_MODIFIER]	= "Nieznany Obiekt";
	strings[ADD_MODIFIER]		= "Dodaj Obiekt";
	strings[ADD_MODIFIERS]		= "Dodaj Obiekty";
	strings[CHANGE_COLOR]		= "Zmień barwę";
	strings[CHANGE_COLORS]		= "Zmień barwy";
	strings[REMOVE_MODIFIER]	= "Usuń Obiekt";
	strings[REMOVE_MODIFIERS]	= "Usuń Obiekty";
	strings[MOVE_MODIFIER]		= "Przenieś Obiekt";
	strings[MOVE_MODIFIERS]		= "Przenieś Obiekty";
	strings[TRANSLATE_MODIFIER]	= "Przesuń Obiekt";
	strings[TRANSLATE_MODIFIERS]	= "Przesuń Obiekty";
	strings[CHANGE_LAYER_SETTINGS]	= "Zmień ustawienia Warstwy";
	strings[CROP_CANVAS]		= "Przytnij Projekt";
	strings[RESIZE_CANVAS]		= "Skaluj Projekt";
	strings[ROTATE_CANVAS]		= "Obróć Projekt";
	strings[INSERT_CANVAS]		= "Wstaw Projekt";
	strings[CHANGE_OPACITY]		= "Zmień krycie";
	strings[EDIT_MODIFIER]		= "Edytuj Obiekt";

	// pasting properties
	strings[PASTE_PROPERTY]		= "Wklej Właściwość";			// paste one property to one object
	strings[PASTE_PROPERTIES]	= "Wklej Właściwości";			// paste multiple properties to one object
	strings[MULTI_PASTE_PROPERTY] = "Wklej Właściwość masowo";	// paste one property to multiple objects
	strings[MUTLI_PASTE_PROPERTIES] = "Wklej Właściwości masowo";	// paste multiple property to multiple objects

	// shape tool history
	strings[ADD_CONTROL_POINT]	= "Dodaj punkt kontrolny";
	strings[TOGGLE_OUTLINE]		= "Przełącz kontur";
	strings[CHANGE_OUTLINE_WIDTH] = "Zmień szerokość konturu";
	strings[CHANGE_CAP_MODE]	= "Zmień typ końcówki";
	strings[CHANGE_JOIN_MODE]	= "Zmień typ łączenia";
	strings[MODIFY_CONTROL_POINT] = "Modyfikuj punkt kontrolny";
	strings[CLOSE_PATH_ACTION]	= "Zamknij ścieżkę";
	strings[INSERT_CONTROL_POINT] = "Wstaw punkt kontrolny";
	strings[REMOVE_CONTROL_POINT] = "Usuń punkt kontrolny";
	strings[REMOVE_CONTROL_POINTS] = "Usuń punkty kontrolne";
	strings[CHANGE_POINT_SELECTION] = "Zmień zaznaczenie";
	strings[NUDGE_CONTROL_POINT] = "Przesuń punkt kontrolny";
	strings[NUDGE_CONTROL_POINTS] = "Przesuń punkty kontrolne";
	strings[SELECT_PATH]		= "Wybierz Ścieżkę";
	strings[CREATE_NEW_PATH]	= "Nowa Ścieżka";
	strings[DELETE_PATH]		= "Usuń Ścieżkę";
	strings[TEXT_TO_SHAPE]		= "Tekst w Kształt";
	strings[TEXTS_TO_SHAPES]	= "Teksty w Kształty";
	strings[TRANSFORM_CONTROL_POINTS] = "Transformuj Punkty Kontrolne";
	strings[MERGE_SHAPE_OBJECTS] = "Scal Kształty";

	strings[FILLING_RULE]		= "Zasada wypełniania";
	strings[NON_ZERO]			= "Nie zero";
	strings[EVEN_ODD]			= "Parzyste-nieparzyste";
	// text tool history
	strings[CHANGE_FONT_SIZE]	= "Zmień rozmiar czcionki";
	strings[CHANGE_TEXT_ROTATION] = "Zmień obrót tekstu";
	strings[MOVE_TEXT]			= "Przesuń tekst";
	strings[CHANGE_FONT]		= "Zmień czcionkę";
	strings[CHANGE_TEXT]		= "Zmień tekst";
	strings[CHANGE_FONT_ADVANCE_SCALE] = "Zmień odstępy";
	strings[ALIGNMENT]			= "Wyrównanie";
	strings[CHANGE_ALIGNMENT]	= "Zmień wyrównanie";
	strings[ALIGNMENT_LEFT]		= "Do lewej";
	strings[ALIGNMENT_CENTER]	= "Wyśrodkowane";
	strings[ALIGNMENT_RIGHT]	= "Do prawej";
	strings[ALIGNMENT_JUSTIFY]	= "Justowane";
	strings[CHANGE_TEXT_WIDTH]	= "Zmień szerokość tekstu";
	strings[PARAGRAPH_INSET]	= "Wcięcie";
	strings[CHANGE_PARAGRAPH_INSET]	= "Zmień wcięcie";
	strings[PARAGRAPH_SPACING]	= "Odstępy p.";		// additional spacing between paragraphs
	// crop tool
	strings[MODIFY_CROP_RECT]	= "Modyfikuj region przycięcia";
	// select tool
	strings[MODIFY_SELECT_RECT]	= "Modyfikuj region wybierania";
	// transformations
	strings[TRANSFORMATION]		= "Transformacja";
	strings[BEGIN_TRANSFORMATION] = "Rozpoczęcie Transformacji";
	strings[FINISH_TRANSFORMATION]	= "Zakończenie Transformacji";
	strings[MOVE]				= "Przesuń";
	strings[ROTATE]				= "Obróć";
	strings[SCALE]				= "Skaluj";
	strings[MOVE_PIVOT]			= "Przesuń Pivot";
	strings[TRANSFORM_OBJECT]	= "Transformacja Obiektu";
	strings[TRANSFORM_OBJECTS]	= "Transformacja Obiektów";
	// guide tool
	strings[SHOW_GUIDES]		= "Pokaż wyznaczniki";
	strings[HORIZONTAL]			= "X";
	strings[VERTICAL]			= "Y";
	strings[UNIT]				= "Jednostka";
	strings[UNIT_PERCENT]		= "%";
	strings[UNIT_PIXEL]			= "px";
	strings[UNIT_CM]			= "cm";
	strings[UNIT_MM]			= "mm";
	strings[UNIT_INCH]			= "inch";
	strings[UNIT_DPI]			= "dpi";
	// general
	strings[OK]					= "OK";
	strings[CONFIRM]			= "Akceptuj";					// apply changes
	strings[CANCEL]				= "Anuluj";
	strings[REVERT]				= "Przywróć";					// revert changes made in panel
	strings[REMOVE]				= "Remove";
	strings[NAME]				= "Nazwa";						// name of project
	strings[UNNAMED]			= "(nazwa)";
	strings[DEFAULT]			= "Domyślny";
	strings[MERGED]				= "Scalony";
	strings[TOOL]				= "Narzędzie";					// for tool tips
	strings[ANY_MODIFIER]		= "Klawisz <Ctrl> lub <Alt>";	// as in modifier key on the keyboard (shift, option, alt etc)

	strings[RETURN]				= "Klawisz <Enter>";
	strings[ESCAPE]				= "Klawisz <Esc>";
	strings[SET]				= "Ustaw";
	strings[COUNT]				= "Liczba";
	strings[INTERPOLATION]		= "Interpolacja";
	strings[NEAREST_NEIGHBOR]	= "Powtórzenie";
	strings[BILINEAR]			= "Bilinearne";
	strings[RETRY]				= "Spróbuj ponownie";
	strings[OVERWRITE]			= "Nadpisz";
	// panels
	strings[PICK_COLOR]			= "Wybierz barwę";				// window title of color picker
	strings[WIDTH]				= "Szerokość";					// pixel count
	strings[HEIGHT]				= "Wysokość";
	strings[BRUSH_PANEL]		= "Wybierz pędzel";
	strings[BRUSH_NAME]			= "Nazwa pędzla";
	strings[PROGRAM_SETTINGS]	= "Ustawienia programu";
	// program settings
	strings[ON_STARTUP]			= "Na starcie";
	strings[DO_NOTHING]			= "Nie rób nic";
	strings[ASK_OPEN]			= "Pokaż okno Otwórz";
	strings[ASK_NEW]			= "Pokaż okno Nowy";
	strings[AUTO_SIZE]			= "Utwórz nowy projekt o rozmiarze";
	strings[LIVE_UPDATES]		= "Dynamiczne zmiany w nawigatorze i ikonach warstw";
	strings[USE_SYSTEM_LANGUAGE] = "Użyj język używany przez System";
	strings[STARTUP_ACTION]		= "Polecenie startowe";
	strings[INTERFACE]			= "Interfejs";
	// requestors
	strings[AKS_SAVE_CHANGES]	= "Zamknąć projekt bez zapisywania (nie zapisane modyfikacje zostaną usunięte)?";
	strings[DISCARD]			= "Usuń zmiany";
	strings[SAVING_FAILED]		= "Zapisywanie się nie powiodło!";
	strings[ERROR]				= "Błąd";						// alias reason for above
	strings[BLIP]				= "Blip!";						// As in "Mission Earth"

	strings[DEMO_MODE]			= "WonderBrush działa w trybie demonstracyjnym. "
								  "Zapisywanie projektów jest zablokowane.\n\n"
								  "Zarejestruj się poprzez sklep online Kagi "
								  "aby otrzymać klucz odblokowujący.";
	strings[NEVER_MIND]			= "Nie ważne";
	strings[OPEN_TRANSLATIONKIT_ERROR]	= "Otworzenie pliku '%s' przy pomocy "
								  "Translation Kit się nie powiodło.\n\n"
								  "Plik nie zawiera obrazu lub Translator "
								  "odpowiedzialny za format pliku "
								  "nie został zainstalowany.";
	strings[OPEN_NATIVE_ERROR]	= "Otworzenie pliku '%s' jako projektu "
								  "WonderBrush się nie powiodło.\n\n";	// yes, that's two \n
	strings[UNSUPPORTED_CURSOR_SIZE] = "Projekt musi mieć rozmiar conajmniej 16 x 16 "
								  "pikseli. Inne rozmiary kursora "
								  "nie są obsługiwane.";
	strings[SELECTED_AREA_EMPTY] = "Zaznaczony region był pusty.";
	strings[ENTRY_DOES_NOT_EXIST] = "Nie można wgrać '%s' ponieważ nie istnieje.";
	strings[ENTRY_IS_A_FOLDER]	= "Nie można wgrać '%s' ponieważ to folder.";
	strings[WARN_SVG_DATA_LOSS]	= "WonderBrush mógł nie odczytać "
								  "wszystkich danych z SVG. "
								  "Nadpisując oryginalny plik "
								  "te dane mogą zostać "
								  "utracone.";

	// tool tips										// please test the appearance of the tips and
														// try to align them manually by putting the
														// '\n' in the right spot!

	strings[TOOL_TIPS]			= "Pokaż podpowiedzi";
	strings[ALPHA_TIP]			= "Ustawia procent nieprzezroczystości dla aktualnie wybranej warstwy.";
	strings[VISIBILITY_TIP]		= "Przełącza widzialność aktualnie wybranej warstwy.";
	strings[PRESSURE_CONTROL_TIP]	= "Włącza kontrolę poprzez nacisk pióra.";
	strings[BRUSH_PREVIEW_TIP]	= "Podgląd pędzli. Kliknij aby otworzyć okno menadżera pędzli.";
	strings[CURRENT_COLOR_TIP]	= "Aktualna barwa. Kliknij aby otworzyć okno wyboru barw.";
	strings[NAVIGATOR_TIP]		= "Podgląd projektu. Kliknij aby przesunąc widoczną część.\n"
								  "Użyj kółka myszy aby przybliżyć/oddalić.";
	strings[LAYER_LIST_TIP]		= "Sortuj poprzez przeciąganie i upuszczanie. Kliknij oko\n"
								  "żeby przełączyć widzialność. Kliknij podwójnie aby zmienić nazwę.";
	strings[HISTORY_LIST_TIP]	= "Sortuj poprzez przeciąganie i upuszczanie. Upuść barwę by zmienić zabarwienie\n"
								  "aktualnie wybranej modyfikacji.";
	strings[PROPERTY_LIST_TIP]	= "Zmieniaj, Kopiuj i Wklejaj właściwości Obiektu.";
	strings[SUB_PIXEL_TIP]		= "Pobieraj pozycję myszy/pióra z precyzją na poziomie podpikseli.";
	strings[SOLID_TIP]			= "Twardość 100%.";
	strings[TILT_TIP]			= "Zniekształcaj kształt pędzla wraz z przechyleniem pióra.";
	strings[FULLSCREEN_TIP]		= "Przełącza pomiędzy trybem pełnego ekranu a trybem okna";
	strings[GRADIENT_CONTROL_TIP] = "Przeciągnij i upuść kolor aby dodać lub edytować punkt koloru.\n"
									"Kliknij podwójnie na punkt koloru aby pobrać jego kolor.";
	strings[GRADIENT_INHERITS_TRANSFORM_TIP] = "Dziedzicz transformacje od Obiektu";

	// quick help
	strings[READY]				= "Gotowy.";
	strings[NO_CANVAS_HELP]		= "Utwórz nowy projekt albo wgraj projekt lub obraz z pliku.";
	strings[NO_HELP]			= "Żadnych dodatkowych sztuczek.";
	strings[CROP_HELP]			= "Zaznacz region, wciśnij klawisz <Enter> aby potwierdzić.\nPrzytrzymując wciśnięty klawisz <Shift> zachowasz proporcje.";
	strings[TRANSLATE_HELP]		= "Wybierz obiekty do transformacji. Przytrzymując wciśnięty klawisz <Shift> zachowasz kierunek.";
	strings[EDIT_GRADIENT_HELP]	= "Edytuj punkty kolorów, typ i transformacje.";
	strings[STROKE_HELP]		= "Przytrzymując wciśnięty klawisz <Shift> przedłużysz poprzednie pociągnięcie i namalujesz prostą linię.";
	strings[CLONE_HELP]			= "Pierwsze kliknięcie definiuje źródło, drugie odległość.\nWciśnij klawisz <Ctrl> aby wyzerować odległość.";
	strings[BUCKET_FILL_HELP]	= "\"Miękkość\" kontroluje solidność barwy wypełnienia.";
	strings[TEXT_HELP]			= "Wciśnij klawisz <Enter> aby zaakceptować zmiany.";
	strings[SHAPE_HELP]			= "Przytrzymaj wciśnięty klawisz <Ctrl> lub <Alt> aby uzyskać dodatkową opcję.\nWciśnij <Enter> aby zakończyć edycję Kształtu.";
	strings[SELECT_HELP]		= "Zaznacz region który chcesz skopiować do schowka systemowego.";
	strings[ELLIPSE_HELP]		= "Kliknij i przeciągnij aby utworzyć elipsę.\n<Shift> utrzymuje proporcje (pomaga utworzyć koło).";
	strings[ROUND_RECT_HELP]	= "Kliknij i przeciągnij aby utworzyć prostokąt.\n<Shift> utrzymuje proporcje (pomaga utworzyć kwadrat).";
	strings[PICK_OBJECTS_HELP]	= "Zaznacz obiekty poprzez kliknięcie na nie lub rozciągnięcie pola zaznaczenia.\n<Shift> utrzymuje poprzednie zaznaczenie.";
	strings[GUIDES_HELP]		= "Dodaj, edytuj lub usuwaj wyznaczniki.";

	strings[ENTER_SERIAL_NUMBER] = "Wpisz Numer Seryjny który został dostarczony "
								   "wraz z Twoją kopią WonderBrush.";
	strings[WRONG_SERIAL_NUMBER] = "Wpisany Numer Seryjny jest nieprawidłowy.";
}
