// Nederlands
// author: Tim de Jong <mikbaggins@yahoo.co.uk>

// addon stuff
extern "C" {
_EXPORT void init_strings(const char* strings[]);
}

// init_strings
void
init_strings(const char* strings[])
{
	strings[FILE_MENU]			= "Bestand";
	strings[NEW]				= "Nieuw";
	strings[NEW_CANVAS]			= "Nieuw Document";
	strings[NEW_LAYER]			= "Nieuw";
	strings[NEW_BRUSH]			= "Nieuw";
	strings[NEW_PALETTE]		= "Nieuw";
	strings[OPEN]				= "Openen";
	strings[LOAD]				= "Laden";
	strings[OPEN_CANVAS]		= "Document openen";
	strings[EXPORT_CANVAS]		= "Exporteren Document";
	strings[EXPORT]				= "Exporteren";
	strings[EXPORT_AS]			= "Exporteren als";
	strings[SAVE_CANVAS]		= "Document opslaan";
	strings[SAVE]				= "Opslaan";
	strings[SAVE_AS]			= "Opslaan als";
	strings[SETUP_PAGE]			= "Pagina instellen";
	strings[PRINT]				= "Afdrukken";
	strings[CLOSE]				= "Sluiten";
	strings[CLOSE_CANVAS]		= "Document sluiten";
	strings[ABOUT]				= "Over";
	strings[QUIT]				= "Afsluiten";
	
	strings[LOADING_FILE]		= "Bestand wordt geladen:";

	strings[EDIT]				= "Bewerken";
	strings[UNDO]				= "Ongedaan maken";
	strings[REDO]				= "Herstellen";
	
	strings[SETTINGS]			= "Instellingen";
	strings[FULLSCREEN]			= "Volledig scherm";
	strings[SHOW_GRID]			= "Pixelgrid tonen";
	strings[LANGUAGE]			= "Taal";
	
	strings[CANVAS]				= "Document";
	strings[IMAGE]				= "Afbeelding";
	strings[CLEAR]				= "Wissen";
	strings[RESIZE]				= "Grootte veranderen";
	strings[ROTATE_90]			= "90°";
	strings[ROTATE_180]			= "180°";
	strings[ROTATE_270]			= "-90°";

	strings[FORMAT]				= "Formaat";

	strings[EXPORT_FORMAT]		= "Formaat";
	strings[CURSOR_SOURCE]		= "Cursor Broncode";
	strings[BITMAP_SOURCE]		= "Bitmap Broncode";
	strings[SVG_FORMAT]			= "SVG (Niet alle objecttypen)";
	strings[AI_FORMAT]			= "Adobe Illustrator (Niet alle objecttypen)";
	strings[RDEF_FORMAT]		= "RDef Broncode";
	
	strings[LAYER]				= "Laag";
	strings[DUPLICATE]			= "Dupliceren";
	strings[MERGE_DOWN]			= "Met onderliggende samensmelten";
	strings[MERGE_VISIBLE]		= "Zichtbare samensmelten";
	strings[RENAME]				= "Hernoemen";
	strings[DELETE]				= "Verwijderen";
	strings[DELETE_ALL]			= "Alles verwijderen";
	// layer blending modes
	strings[MODE]				= "Modus";
	strings[NORMAL]				= "Normaal";
	strings[ERASE]				= "Wissen";
	strings[MULTIPLY]			= "Vermenigvuldigen";
	strings[INVERSE_MULTIPLY]	= "Inverse vermenigvuldigen";
	strings[LUMINANCE]			= "Helderheid";
	strings[MULTIPLY_ALPHA]		= "Alfakanaal";
	strings[MULTIPLY_INVERSE_ALPHA]	= "Geïnverteerd alfakanaal";
	strings[REPLACE_RED]		= "Rood vervangen";
	strings[REPLACE_GREEN]		= "Groen vervangen";
	strings[REPLACE_BLUE]		= "Blauw vervangen";
	strings[DARKEN]				= "Verdonkeren";
	strings[LIGHTEN]			= "Verhelderen";
	
	strings[HISTORY]			= "Protocol";
	strings[OBJECT]				= "Object";
	strings[FREEZE]				= "Bevriezen";
	strings[CUT]				= "Knippen";
	strings[COPY]				= "Kopiëren";
	strings[PASTE]				= "Invoegen";
	strings[MULTI_PASTE]		= "Meervoudig invoegen";
	strings[REMOVE_ALL]			= "Alles verwijderen";
	strings[OBJECT_SELECTION]	= "Selecteren";
	strings[SELECT_ALL_OBJECTS]	= "Alles";
	strings[SELECT_NO_OBJECTS]	= "Niets";
	strings[INVERT_SELECTION]	= "Selectie omdraaien";
	strings[ALL_OBJECTS]		= "Alles";
	strings[PROPERTY_SELECTION]	= "Selecteren";
	strings[SELECT_ALL_PROPERTIES] = "Alles";
	strings[SELECT_NO_PROPERTIES] = "Niets";
	strings[SPECIAL]			= "Speciaal";

	strings[ADD_FILTER]			= "Filter";
	strings[DROP_SHADOW]		= "Slagschaduw";
	strings[GAUSSIAN_BLUR]		= "Gaussische blur";
	strings[INVERT]				= "Inverteren";
	strings[BRIGHTNESS]			= "Helderheid";
	strings[CONTRAST]			= "Contrast";
	strings[SATURATION]			= "Verzadiging";
	strings[MOTION_BLUR]		= "Motion blur";
	strings[HALFTONE]			= "Halftoon";
	strings[DOT]				= "Punt";
	strings[LINE]				= "Lijn";
	strings[DITHER]				= "Verdeling";
	strings[DITHER_FS]			= "Verdeling (FS)";
	strings[DITHER_NC]			= "Verdeling (NK)";
	strings[NOISE]				= "Ruis";
	strings[HUE_SHIFT]			= "Kleurrotatie";
	strings[DISPERSION]			= "Verstrooiing";
	strings[STRENGTH]			= "Sterkte";
	strings[LUMINANCE_ONLY]		= "Alleen Helderheid";
	strings[WARPSHARP]			= "Warpsharp";
	strings[LAMBDA]				= "Lambda";
	strings[MU]					= "µ";
	strings[NON_MAXIMAL_SUPPRESSION] = "NMS";
	strings[SOLID_AREA]			= "Gebied";
	
	strings[PROPERTY]			= "Eigenschap";
	strings[UNKOWN_PROPERTY]	= "<onbekend>";

	strings[SWATCHES]			= "Kleuren";
	strings[ZOOM]				= "Zoom";
	strings[ALPHA]				= "Alfa";
	strings[ALPHA_CHANNEL]		= "Alfa kanaal";
	strings[COLOR]				= "Kleur";
	strings[GRADIENT]			= "Gradient";
	// pick objects tool
	strings[SELECTABLE]			= "Selecteerbaar";		// which object types are "selectable"

	// gradient options
	strings[GRADIENT_TYPE]					= "Type";
	strings[GRADIENT_TYPE_LINEAR]			= "Lineair";
	strings[GRADIENT_TYPE_RADIAL]			= "Radiaal";
	strings[GRADIENT_TYPE_DIAMONT]			= "Diamant";
	strings[GRADIENT_TYPE_CONIC]			= "Conisch";
	strings[GRADIENT_TYPE_XY]				= "X-Y";
	strings[GRADIENT_TYPE_XY_SQRT]			= "Wortel(X-Y)";
	strings[GRADIENT_INTERPOLATION]			= "Interpolatie";
	strings[GRADIENT_INTERPOLATION_LINEAR]	= "Lineair";
	strings[GRADIENT_INTERPOLATION_SMOOTH]	= "Vloeiend";
	strings[GRADIENT_INHERITS_TRANSFORM]	= "Transformatie overerven";
	
	// brush options
	strings[OPACITY]			= "Ondoorschijnendheid";
	strings[MIN_OPACITY]		= "min ondoorsch.";
	strings[DYN_OPACITY]		= "dyn ondoorsch.";
	strings[RADIUS]				= "Radius";
	strings[MIN_RADIUS]			= "min radius";
	strings[DYN_RADIUS]			= "dyn radius";
	strings[HARDNESS]			= "Scherpte";
	strings[MIN_HARDNESS]		= "min scherpte";
	strings[DYN_HARDNESS]		= "dyn scherpte";
	strings[SPACING]			= "Afstanden";
	strings[MIN_SPACING]		= "min afst.";
	strings[DYN_SPACING]		= "dyn afst.";
	strings[SUBPIXELS]			= "Subpixel";
	strings[SOLID]				= "Vast";
	strings[TILT]				= "Overhelling";
	strings[BLUR_RADIUS]		= "Blur radius";
	// translate options
	strings[X_LABEL]			= "X";
	strings[Y_LABEL]			= "Y";
	strings[X_SCALE]			= "X factor";
	strings[Y_SCALE]			= "Y factor";
	strings[TRANSLATION_X]		= "X";
	strings[TRANSLATION_Y]		= "Y";
	strings[SCALE_X]			= "X";
	strings[SCALE_Y]			= "Y";
	strings[ANGLE]				= "Hoek";

	strings[X_OFFSET]			= "X afstand";
	strings[Y_OFFSET]			= "Y afstand";
	// crop & select options
	strings[LEFT]				= "Links";
	strings[TOP]				= "Boven";
	strings[ENTIRE_CANVAS]		= "Alles";
	// bucket fill options
	strings[TOLERANCE]			= "Tolerantie";
	strings[SOFTNESS]			= "Zachtheid";
	strings[CONTIGUOUS_AREA]	= "Aangrenzend Bereik";
	// shape options
	strings[PATH]				= "Pad";	
	strings[OUTLINE]			= "Uitlijning";
	
	strings[CAP_MODE]			= "Einden";		// Uihuihui, hier müsste
	strings[BUTT_CAP]			= "Stomp";	// mal einer mit mehr
	strings[SQUARE_CAP]			= "Rechthoekig";		// Ahnung einspringen...
	strings[ROUND_CAP]			= "Rond";
	strings[JOIN_MODE]			= "Hoeken";
	strings[MITER_JOIN]			= "Voortzettend";
	strings[ROUND_JOIN]			= "Rond";
	strings[BEVEL_JOIN]			= "Afgevlakt";
	
	strings[TRANSFORM]			= "Verschuiven";
	strings[CLOSED]				= "Gesloten";
	strings[CIRCLE]				= "Cirkel";
	strings[RECT]				= "Rechthoek";
	strings[ROUND_CORNERS]		= "Ronde hoeken";
	strings[TOGGLE_ROUND_CORNERS]		= "Ronde Hoeken Aan/Uit";		// not yet used
	strings[ROUND_CORNER_RADIUS]		= "Radius";						// not yet used
	strings[CHANGE_ROUND_CORNER_RADIUS]	= "Verander Radius Ronde Hoeken";	// not yet used
	// text options
	strings[FONT]				= "Lettertype";
	strings[SIZE]				= "Grootte";
	strings[ROTATION]			= "Rotatie";
	strings[TEXT_INPUT]			= "Tekst";
	strings[ADVANCE_SCALE]		= "Karakterafstanden";
	strings[HINTING]			= "Hinting";
	strings[KERNING]			= "Kerning";
	strings[TEXT_LINE_SPACING]	= "Regelafstand";
	// color picker tool
	strings[TIP_SIZE]			= "Pengrootte";
	strings[PIXEL_1x1]			= "1x1 Pixel";
	strings[PIXEL_3x3]			= "3x3 Pixel";
	strings[PIXEL_5x5]			= "5x5 Pixel";
	strings[INCLUDE_ALL_LAYERS]	= "Alle lagen invoegen";
	// tools
	strings[BRUSH]				= "Penseel";
	strings[CLONE]				= "Stempel";
	strings[ERASER]				= "Gum";
	strings[PEN]				= "Pen";
	strings[PEN_ERASER]			= "Pen wisser";
	strings[RESTORE]			= "Herstellen";
	strings[DROPPER]			= "Pipet";
	strings[TEXT]				= "Tekst";
	strings[BLUR]				= "Blur";
	strings[BITMAP]				= "Afbeelding";
	strings[SELECT]				= "Selectie";
	strings[TRANSLATE]			= "Verschuiven";
	strings[CROP]				= "Uitsnijden";
	strings[BUCKET_FILL]		= "Opvullen";
	strings[SHAPE]				= "Vorm";
	strings[ELLIPSE]			= "Ellips";
	strings[ROUND_RECT]			= "(Afgeronde) Rechthoek";
	strings[EDIT_GRADIENT]		= "Gradient bewerken";
	strings[GUIDES]				= "Hulplijnen";
	strings[PICK_OBJECTS]		= "Kies Objecten";
	strings[CLIPBOARD]			= "Klembord";
	// zoom
	strings[ZOOM_IN]			= "Inzoomen";
	strings[ZOOM_OUT]			= "Uitzoomen";
	// actions
	strings[ADD_LAYER]			= "Laag toevoegen";
	strings[REMOVE_LAYER]		= "Laag verwijderen";
	strings[REMOVE_LAYERS]		= "Lagen verwijderen";
	strings[MOVE_LAYER]			= "Laag verplaatsen";
	strings[MOVE_LAYERS]		= "Lagen verplaatsen";
	strings[MERGE_LAYERS]		= "Lagen samensmelten";
	strings[UNKOWN_MODIFIER]	= "Onbekend object";
	strings[ADD_MODIFIER]		= "Object toevoegen";
	strings[ADD_MODIFIERS]		= "Objecten toevoegen";
	strings[CHANGE_COLOR]		= "Kleur veranderen";
	strings[CHANGE_COLORS]		= "Kleuren veranderen";
	strings[REMOVE_MODIFIER]	= "Object verwijderen";
	strings[REMOVE_MODIFIERS]	= "Objecten verwijderen";
	strings[MOVE_MODIFIER]		= "Object verplaatsen";
	strings[MOVE_MODIFIERS]		= "Objecten verplaatsen";
	strings[TRANSLATE_MODIFIER]	= "Object verschuiven";
	strings[TRANSLATE_MODIFIERS]	= "Objecten verschuiven";
	strings[CHANGE_LAYER_SETTINGS]	= "Laageigenschappen veranderen";
	strings[CROP_CANVAS]		= "Document uitsnijden";
	strings[RESIZE_CANVAS]		= "Documentgrootte veranderen";
	strings[ROTATE_CANVAS]		= "Document roteren";
	strings[INSERT_CANVAS]		= "Document invoegen";
	strings[CHANGE_OPACITY]		= "Ondoorschijnendheid veranderen";
	strings[EDIT_MODIFIER]		= "Object bewerken";
	// pasting properties
	strings[PASTE_PROPERTY]		= "Eigenschap invoegen";
	strings[PASTE_PROPERTIES]	= "Eigenschappen invoegen";
	strings[MULTI_PASTE_PROPERTY] = "Eigenschap meervoudig invoegen";
	strings[MUTLI_PASTE_PROPERTIES] = "Eigenschappen meervoudig invoegen";
	// shape tool history
	strings[ADD_CONTROL_POINT]	= "Controlepunt toevoegen";
	strings[TOGGLE_OUTLINE]		= "Omtrek aan-/uitzetten";
	strings[CHANGE_OUTLINE_WIDTH] = "Breedte omtrek veranderen";
	strings[CHANGE_CAP_MODE]	= "Einden veranderen";
	strings[CHANGE_JOIN_MODE]	= "Hoeken veranderen";
	strings[MODIFY_CONTROL_POINT] = "Controlepunt veranderen";
	strings[CLOSE_PATH_ACTION]	= "Pad sluiten";
	strings[INSERT_CONTROL_POINT] = "Controlepunt invoegen";
	strings[REMOVE_CONTROL_POINT] = "Controlepunt wissen";
	strings[REMOVE_CONTROL_POINTS] = "Controlepunten wissen";
	strings[CHANGE_POINT_SELECTION] = "Veranderen van de selectie";
	strings[NUDGE_CONTROL_POINT] = "Controlepunt verplaatsen";
	strings[NUDGE_CONTROL_POINTS] = "Controlepunten verplaatsen";
	strings[SELECT_PATH]		= "Pad uitkiezen";
	strings[CREATE_NEW_PATH]	= "Nieuw Pad";
	strings[DELETE_PATH]		= "Pad verwijderen";
	strings[TEXT_TO_SHAPE]		= "Tekst naar Vorm";
	strings[TEXTS_TO_SHAPES]	= "Teksten naar Vormen";
	strings[TRANSFORM_CONTROL_POINTS] = "Controlepunten transformeren";
	strings[MERGE_SHAPE_OBJECTS] = "Vormen samensmelten";
	
	strings[FILLING_RULE]		= "Vulregel";
	strings[NON_ZERO]			= "Linksdraaiend";
	strings[EVEN_ODD]			= "Afwisselend";	
	// text tool history
	strings[CHANGE_FONT_SIZE]	= "Tekstgrootte veranderen";
	strings[CHANGE_TEXT_ROTATION] = "Tekstrotatie veranderen";
	strings[MOVE_TEXT]			= "Tekst verschuiven";
	strings[CHANGE_FONT]		= "Lettertype wijzigen";
	strings[CHANGE_TEXT]		= "Tekst veranderen";
	strings[CHANGE_FONT_ADVANCE_SCALE] = "Karakterafstanden veranderen";
	strings[ALIGNMENT]			= "Uitlijning";
	strings[CHANGE_ALIGNMENT]	= "Uitlijning veranderen";
	strings[ALIGNMENT_LEFT]		= "Links";
	strings[ALIGNMENT_CENTER]	= "Centrum";
	strings[ALIGNMENT_RIGHT]	= "Rechts";
	strings[ALIGNMENT_JUSTIFY]	= "Uitvullen";
	strings[CHANGE_TEXT_WIDTH]	= "Verander Tekstbreedte";
	strings[PARAGRAPH_INSET]	= "Inspringen";
	strings[CHANGE_PARAGRAPH_INSET]	= "Verander Inspringen";
	strings[PARAGRAPH_SPACING]	= "P. Afstand";		// additional spacing between paragraphs	
	// crop tool
	strings[MODIFY_CROP_RECT]	= "Uitsnijvierkant veranderen";
	// select tool
	strings[MODIFY_SELECT_RECT]	= "Selectievierkant veranderen";
	// transformations
	strings[TRANSFORMATION]		= "Verschuiving";
	strings[BEGIN_TRANSFORMATION] = "Verschuiving starten";
	strings[FINISH_TRANSFORMATION]	= "Verschuiving beëindigen";
	strings[MOVE]				= "Verschuiven";
	strings[ROTATE]				= "Roteren";
	strings[SCALE]				= "Schalen";
	strings[MOVE_PIVOT]			= "Draaipunt verschuiven";
	strings[TRANSFORM_OBJECT]	= "Object verschuiven";
	strings[TRANSFORM_OBJECTS]	= "Objecten verschuiven";
	// guide tool
	strings[SHOW_GUIDES]		= "Toon hulplijnen";
	strings[HORIZONTAL]			= "Horizontaal";
	strings[VERTICAL]			= "Verticaal";
	strings[UNIT]				= "Eenheid";
	strings[UNIT_PERCENT]		= "%";
	strings[UNIT_PIXEL]			= "px";
	strings[UNIT_CM]			= "cm";
	strings[UNIT_MM]			= "mm";
	strings[UNIT_INCH]			= "inch";
	strings[UNIT_DPI]			= "dpi";
	// general
	strings[OK]					= "Ok";
	strings[CONFIRM]			= "Bevestigen";
	strings[CANCEL]				= "Annuleren";
	strings[REVERT]				= "Ongedaan maken";
	strings[REMOVE]				= "Verwijderen";
	strings[NAME]				= "Naam";
	strings[UNNAMED]			= "Niet benoemd";
	strings[DEFAULT]			= "Standaard";
	strings[MERGED]				= "Versmolten";
	strings[TOOL]				= "Werktuig";
	strings[ANY_MODIFIER]		= "Sneltoets";
	
	strings[RETURN]				= "Enter";
	strings[ESCAPE]				= "Escape";
	strings[SET]				= "Instellen";
	strings[COUNT]				= "Aantal";
	strings[INTERPOLATION]		= "Interpolatie";
	strings[NEAREST_NEIGHBOR]	= "Herhalen";
	strings[BILINEAR]			= "Bilineair";
	strings[RETRY]				= "Corrigeren";
	strings[OVERWRITE]			= "Overschrijven";
	// panels
	strings[PICK_COLOR]			= "Kleur kiezen";
	strings[WIDTH]				= "Breedte";
	strings[HEIGHT]				= "Hoogte";
	strings[BRUSH_PANEL]		= "Penseel kiezen";
	strings[BRUSH_NAME]			= "Penseelnaam";
	strings[PROGRAM_SETTINGS]	= "Programma instellingen";
	// program settings
	strings[ON_STARTUP]			= "Bij het opstarten";
	strings[DO_NOTHING]			= "Niets doen";
	strings[ASK_OPEN]			= "Open dialoog laten zien";
	strings[ASK_NEW]			= "Nieuw dialoog laten zien";
	strings[AUTO_SIZE]			= "Een nieuw document met de volgende grootte aanmaken";
	strings[LIVE_UPDATES]		= "Navigator en laagpictogrammen live bijwerken";
	strings[USE_SYSTEM_LANGUAGE] = "Zelfde taal als het systeem gebruiken";
	strings[STARTUP_ACTION]		= "Start actie";
	strings[INTERFACE]			= "Interface";
	// requestors
	strings[AKS_SAVE_CHANGES]	= "Document sluiten en onopgeslagen wijzigingen verwerpen?";
	strings[DISCARD]			= "Verwerpen";
	strings[SAVING_FAILED]		= "Fout bij opslaan!";
	strings[ERROR]				= "Fout";
	strings[BLIP]				= "Verdorie!";
	strings[DEMO_MODE]			= "WonderBrush draait in demomode. Het opslaan "
								  "van een document is niet mogelijk.\n\n"
								  "Registreert u het produkt alstublieft bij de "
								  "Kagi Online Store, om een registratiesleutel te krijgen.";
	strings[NEVER_MIND]			= "Negeren";
	strings[OPEN_TRANSLATIONKIT_ERROR]	= "Het openen van het bestand '%s' met behulp van de "
								  "Translation Kit mislukte.\n\nOf het "
								  "bestand bevat geen afbeelding, of er is geen "
								  "Translator geïnstalleerd, die het formaat "
								  "ondersteunt.";
	strings[OPEN_NATIVE_ERROR]	= "Het openen van het bestand '%s' als WonderBrush "
								  "document mislukte.\n\n";
	strings[UNSUPPORTED_CURSOR_SIZE] = "Het document moet 16 x 16 Pixel "
								  "groot zijn. Andere cursorgrootten worden "
								  "niet ondersteund.";
	strings[SELECTED_AREA_EMPTY] = "De selectie was leeg.";
	strings[ENTRY_DOES_NOT_EXIST] = "'%s' kon niet geladen worden, omdat het bestand niet bestaat.";
	strings[ENTRY_IS_A_FOLDER]	= "'%s' kon niet geladen worden, omdat het een map is.";
	strings[WARN_SVG_DATA_LOSS]	= "Het kan zijn dat WonderBrush "
								  "niet alle data uit het SVG bestand gelezen heeft "
								  "toen het geopend werd. "
								  "Door het originele bestand te overschrijven "
								  "zal deze informatie nu "
								  "verloren gaan. ";
	// tool tips
	strings[TOOL_TIPS]			= "Tooltips tonen";
	strings[ALPHA_TIP]			= "Stelt de globale doorschijnendheid van het actieve gebied in.";
	strings[VISIBILITY_TIP]		= "Schakelt de zichtbaarheid van het actieve vlak aan of uit.";
	strings[PRESSURE_CONTROL_TIP]	= "Activeert de besturing door middel van stiftdruk.";
	strings[BRUSH_PREVIEW_TIP]	= "Bekijk penseel vooraf. Klikken opent het penseelkeuze venster.";
	strings[CURRENT_COLOR_TIP]	= "Actuele kleur. Klikken opent het kleurkeuzevenster.";
	strings[NAVIGATOR_TIP]		= "Documentoverzicht. Klikken zet het zichtbare deel.\n"
								  "Scrollwieltje zoomt in/uit.";
	strings[LAYER_LIST_TIP]		= "Sorteren door Drag'N'Drop. Klikken op het oog\n"
								  "(de-)activeert de zichtbaarheid. Hernoemen met behulp van dubbelklik.";
	strings[HISTORY_LIST_TIP]	= "Sorteren met behulp van Drag'N'Drop. Het droppen van een\n"
								  "kleur stelt de kleur in van de geselecteerde objecten.";
	strings[PROPERTY_LIST_TIP]	= "Bewerken, kopiëren en invoegen van objecteigenschappen.";
	strings[SUB_PIXEL_TIP]		= "Muis-/penpositie volgen met subpixel-precisie.";
	strings[SOLID_TIP]			= "Ondoorschijnendheid niet per pixel variëren.";
	strings[TILT_TIP]			= "Penseelvorm overeenkomst stifthelling vervormen.";
	strings[FULLSCREEN_TIP]		= "Wisselt tussen volledig scherm en venstermodus.";
    strings[GRADIENT_CONTROL_TIP] = "Ter toevoeging of verandering van kleurverlooppunten kleuren droppen.\n";
	strings[GRADIENT_INHERITS_TRANSFORM_TIP] = "Transformatie van het object overerven";
	// quick help
	strings[READY]				= "Gereed.";
	strings[NO_CANVAS_HELP]		= "Creëer een nieuw document of laadt een document of afbeelding.";
	strings[NO_HELP]			= "Geen extra trucs.";
	strings[CROP_HELP]			= "Verander het uitsnijvierkant, druk <Enter> om toe te passen. <Shift> legt de verhouding tussen de zijden vast.";
	strings[TRANSLATE_HELP]		= "Kies de te verschuiven objecten uit. <Shift> legt de richting vast.";
	strings[EDIT_GRADIENT_HELP]	= "Bewerkt kleurverlooppunten, -type en -transformatie.";
	strings[STROKE_HELP]		= "Gebruik <Shift>, om aan een voorgaande penseelstreek aan te koppelen en een rechte lijn te trekken.";
	strings[CLONE_HELP]			= "De eerste klik definiëert het startpunt, de tweede het eindpunt. Druk <Command> om het eindpunt terug te zetten.";
	strings[BUCKET_FILL_HELP]	= "\"Zachtheid\" beïnvloedt de vastheid van de vulkleur.";
	strings[TEXT_HELP]			= "Druk <Enter> als U klaar bent.";
	strings[SHAPE_HELP]			= "Druk <Command> of <Alt> voor meer opties. Druk <Enter> om toe te passen.";
	strings[SELECT_HELP]		= "Selecteer een gebied om naar het klembord te kopiëren.";
	strings[ELLIPSE_HELP]		= "Klikken en slepen maakt een Ellips.\n<Shift> fixeert de verhouding tussen de zijden.";
	strings[ROUND_RECT_HELP]	= "Klikken en slepen maakt een Rechthoek.\n<Shift> fixeert de verhouding tussen de zijden.";
	strings[PICK_OBJECTS_HELP]	= "Selecteer Objecten door op ze te klikken of door een rechthoek om ze heen te trekken.\n<Shift> behoudt de vorige selectie.";
	strings[GUIDES_HELP]		= "Creëer, bewerk of verwijder hulplijnen.";

	strings[ENTER_SERIAL_NUMBER] = "Voer nu de toegangssleutel in die "
								   "met uw kopie van WonderBrush meegeleverd werd.";
	strings[WRONG_SERIAL_NUMBER] = "U heeft een verkeerde toegangssleutel ingevoerds.";
}