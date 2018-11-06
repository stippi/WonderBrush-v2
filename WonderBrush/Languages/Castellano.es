// "Castellano.es". a WonderBrush translation to Castillian
// (what you may know as "Spanish")
// Author(s): Oscar Lesta <oscar.lesta@gmail.com>

// addon stuff
extern "C" {
_EXPORT void init_strings(const char* strings[]);
}

// init_strings
void
init_strings(const char* strings[])
{
	strings[FILE_MENU]			= "Archivo";
	strings[NEW]				= "Nuevo";				// will show up in Canvas menu
	strings[NEW_CANVAS]			= "Nuevo Lienzo";		// tool tip
	strings[NEW_LAYER]			= "Nueva";				// these strings were added
	strings[NEW_BRUSH]			= "Nuevo";				// by request to be able to
	strings[NEW_PALETTE]		= "Nueva";				// reflect different genders in "New"
	strings[OPEN]				= "Abrir";
	strings[LOAD]				= "Cargar";
	strings[OPEN_CANVAS]		= "Abrir Lienzo";
	strings[EXPORT_CANVAS]		= "Exportar Lienzo";	// title of Export panel
	strings[EXPORT]				= "Exportar";
	strings[EXPORT_AS]			= "Exportar Como";
	strings[SAVE_CANVAS]		= "Guardar Lienzo";		// title of Save panel
	strings[SAVE]				= "Guardar";
	strings[SAVE_AS]			= "Guardar Como";
	strings[SETUP_PAGE]			= "Configurar Página";
	strings[PRINT]				= "Imprimir";
	strings[CLOSE]				= "Cerrar";
	strings[CLOSE_CANVAS]		= "Cerrar Lienzo";		// tool tip
	strings[ABOUT]				= "Acerca de";
	strings[QUIT]				= "Salir";

	strings[LOADING_FILE]		= "Cargando Archivo:";

	strings[EDIT]				= "Edición";
	strings[UNDO]				= "Deshacer";
	strings[REDO]				= "Rehacer";

	strings[SETTINGS]			= "Configuración";		// use "Opciones" (Options) instead?
	strings[FULLSCREEN]			= "Pantalla Completa";
	strings[SHOW_GRID]			= "Mostrar Grilla";
	strings[LANGUAGE]			= "Idioma";

	strings[CANVAS]				= "Lienzo";				// ie project, document
	strings[IMAGE]				= "Imagen";				// ie bitmap
	strings[CLEAR]				= "Limpiar";
	strings[RESIZE]				= "Cambiar Tamaño";		// resize canvas (new resolution)
	strings[ROTATE_90]			= "90°";
	strings[ROTATE_180]			= "180°";
	strings[ROTATE_270]			= "-90°";

	strings[FORMAT]				= "Formato";			// color space of the canvas (RGB, Lab etc)

	strings[EXPORT_FORMAT]		= "Formato";				// ie PNG, SVG etc
	strings[CURSOR_SOURCE]		= "Cursor (Código fuente)";
	strings[BITMAP_SOURCE]		= "Bitmap (Código fuente)";
	strings[SVG_FORMAT]			= "SVG (no todos los tipos de objetos)";
	strings[AI_FORMAT]			= "Adobe Illustrator (no todos los tipos de objetos)";
	strings[RDEF_FORMAT]		= "RDef (Código fuente)";

	strings[LAYER]				= "Capa";
	strings[DUPLICATE]			= "Duplicar";
	strings[MERGE_DOWN]			= "Combinar con Capas Inferiores";	// combining of current layer and the
																	// one underneath it
	strings[MERGE_VISIBLE]		= "Combinar Visibles";	// combining of all visible layers
	strings[RENAME]				= "Renombrar";
	strings[DELETE]				= "Eliminar";
	strings[DELETE_ALL]			= "Eliminar Todos";

	// layer blending modes
	strings[MODE]				= "Modo";
	strings[NORMAL]				= "Normal";
	strings[ERASE]				= "Borrar";
	strings[MULTIPLY]			= "Multiplicar";
	strings[INVERSE_MULTIPLY]	= "Multiplicación Inversa";
	strings[LUMINANCE]			= "Luminancia";
	strings[MULTIPLY_ALPHA]		= "Canal Alfa";
	strings[MULTIPLY_INVERSE_ALPHA]	= "Canal Alfa Inverso";
	strings[REPLACE_RED]		= "Reemplazar Rojo";
	strings[REPLACE_GREEN]		= "Reemplazar Verde";
	strings[REPLACE_BLUE]		= "Reemplazar Azul";
	strings[DARKEN]				= "Obscurecer";
	strings[LIGHTEN]			= "Aclarar";

	strings[HISTORY]			= "Historial";			// as in "modifier stack"
	strings[OBJECT]				= "Objeto";				// new name for the "History"
	strings[FREEZE]				= "Congelar";			// renders the result of the modifier stack
														// into one "bitmap modifier" and replaces
														// all previous modifiers (ie, you cannot edit
														// them anymore) by the single bitmap modifier
	strings[CUT]				= "Cortar";
	strings[COPY]				= "Copiar";
	strings[PASTE]				= "Pegar";
	strings[MULTI_PASTE]		= "Pegado Múltiple";	// paste one ore more properties to
														// *multiple* objects at once
	strings[REMOVE_ALL]			= "Eliminar Todo";

	strings[OBJECT_SELECTION]	= "Seleccionar";		// change selection shortcuts
	strings[SELECT_ALL_OBJECTS]	= "Todo";				// in Object and Property list
	strings[SELECT_NO_OBJECTS]	= "Ninguno";
	strings[INVERT_SELECTION]	= "Invertir Selección";
	strings[ALL_OBJECTS]		= "Todos";
	strings[PROPERTY_SELECTION]	= "Seleccionar";
	strings[SELECT_ALL_PROPERTIES] = "Todas";
	strings[SELECT_NO_PROPERTIES] = "Ninguna";
	strings[SPECIAL]			= "Especial";

	// filters
	strings[ADD_FILTER]			= "Filtro";				// the actual menu
	strings[DROP_SHADOW]		= "Sombra";
	strings[GAUSSIAN_BLUR]		= "Difusión Gaussiana";
	strings[INVERT]				= "Inversión";
	strings[BRIGHTNESS]			= "Brillo";
	strings[CONTRAST]			= "Contraste";
	strings[SATURATION]			= "Saturación";
	strings[MOTION_BLUR]		= "Difusión por Movimiento";
	strings[HALFTONE]			= "Medios Tonos";
	strings[DOT]				= "Punto";				// mode options of Halftone
	strings[LINE]				= "Línea";
	strings[DITHER]				= "Difuminado";
	strings[DITHER_FS]			= "Difuminado (FS)";
	strings[DITHER_NC]			= "Difuminado (NK)";
	strings[NOISE]				= "Ruido";
	strings[HUE_SHIFT]			= "Desplazamiento de Color";	// filter not yet written
	strings[DISPERSION]			= "Dispersión";
	strings[STRENGTH]			= "Fuerza";
	strings[LUMINANCE_ONLY]		= "Sólo Luminancia";
	strings[WARPSHARP]			= "Warpsharp";
	strings[LAMBDA]				= "Lamda";
	strings[MU]					= "µ";
	strings[NON_MAXIMAL_SUPPRESSION] = "NMS";
	strings[SOLID_AREA]			= "Área Sólida";

	strings[PROPERTY]			= "Propiedad";
	strings[UNKOWN_PROPERTY]	= "<desconocida>";

	strings[SWATCHES]			= "Paletas";			// color palette
	strings[ZOOM]				= "Acercamiento";
	strings[ALPHA]				= "Opacidad";			// keep this as short as possible
	strings[ALPHA_CHANNEL]		= "Canal Alfa";			// used for properties
	strings[COLOR]				= "Color";				// used for properties
	strings[GRADIENT]			= "Gradiente";			// used for properties (?)

	// pick objects tool
	strings[SELECTABLE]			= "Seleccionable";		// which object types are "selectable"

	// gradient options
	strings[GRADIENT_TYPE]					= "Tipo";
	strings[GRADIENT_TYPE_LINEAR]			= "Lineal";
	strings[GRADIENT_TYPE_RADIAL]			= "Radial";
	strings[GRADIENT_TYPE_DIAMONT]			= "Diamante";
	strings[GRADIENT_TYPE_CONIC]			= "Cónico";
	strings[GRADIENT_TYPE_XY]				= "X-Y";
	strings[GRADIENT_TYPE_XY_SQRT]			= "Raíz Cuadrada(X-Y)";
	strings[GRADIENT_INTERPOLATION]			= "Interpolación";
	strings[GRADIENT_INTERPOLATION_LINEAR]	= "Linear";
	strings[GRADIENT_INTERPOLATION_SMOOTH]	= "Suave";
	strings[GRADIENT_INHERITS_TRANSFORM]	= "Heredar Transformación";

	// brush options
	strings[OPACITY]			= "Opacidad";
	strings[MIN_OPACITY]		= "Op. mín.";			// property label (keep short)
	strings[DYN_OPACITY]		= "Op. din";			// property label (keep short)
	strings[RADIUS]				= "Radio";
	strings[MIN_RADIUS]			= "Radio mín.";			// property label (keep short)
	strings[DYN_RADIUS]			= "Radio din";			// property label (keep short)
	strings[HARDNESS]			= "Dureza";				// controls brush shape (soft...hard)
	strings[MIN_HARDNESS]		= "Dureza mín";			// property label (keep short)
	strings[DYN_HARDNESS]		= "Dureza din";			// property label (keep short)
	strings[SPACING]			= "Espaciado";			// distance in % of brush diameter at wich brush
														// shapes will be put into the bitmap along the
														// line of a stroke,
														// ! also used for Text tool (spacing between chars)
	strings[MIN_SPACING]		= "Espaciado min";		// property label (keep short), not yet used
	strings[DYN_SPACING]		= "Espaciado din";		// property label (keep short), not yet used
	strings[SUBPIXELS]			= "Subpixeles";			// allow subpixel precise positioning of tools
	strings[SOLID]				= "Sólido";				// solid (fixed) opacity for all pixels
	strings[TILT]				= "Inclinación";		// tablet specific, tilt of pen
	strings[BLUR_RADIUS]		= "Radio de Dif.";		// property label (keep short), also used for Dispersion filter
	// translate options
	strings[X_LABEL]			= "X";
	strings[Y_LABEL]			= "Y";
	strings[X_SCALE]			= "Escala X";
	strings[Y_SCALE]			= "Escala Y";
	strings[TRANSLATION_X]		= "X";
	strings[TRANSLATION_Y]		= "Y";
	strings[SCALE_X]			= "X";
	strings[SCALE_Y]			= "Y";
	strings[ANGLE]				= "Ángulo";

	strings[X_OFFSET]			= "Desplazamiento X";
	strings[Y_OFFSET]			= "Desplazamiento Y";
	// crop & select options
	strings[LEFT]				= "Izquierda";
	strings[TOP]				= "Top";
	strings[ENTIRE_CANVAS]		= "All";
	// bucket fill options
	strings[TOLERANCE]			= "Tolerancia";
	strings[SOFTNESS]			= "Suavidad";
	strings[CONTIGUOUS_AREA]	= "Area Contigua";
	// shape options
	strings[PATH]				= "Trayectoria";			// property label
	strings[OUTLINE]			= "Delineado";
	strings[TRANSFORM]			= "Transformar";
	strings[CLOSED]				= "Cerrada";			// ie first and last point of path
														// are connected

	strings[CAP_MODE]			= "Puntas";
	strings[BUTT_CAP]			= "Lisas";
	strings[SQUARE_CAP]			= "Cuadradas";
	strings[ROUND_CAP]			= "Redondeadas";
	strings[JOIN_MODE]			= "Uniones";
	strings[MITER_JOIN]			= "Rectas";
	strings[ROUND_JOIN]			= "Redondeadas";
	strings[BEVEL_JOIN]			= "En ángulo";

	strings[CIRCLE]				= "Círculo";				// not yet used
	strings[RECT]				= "Rectángulo";				// not yet used
	strings[ROUND_CORNERS]		= "Esquínas Redondeadas";		// not yet used
	strings[TOGGLE_ROUND_CORNERS]		= "Alternar Esquínas Redondeadas";		// not yet used
	strings[ROUND_CORNER_RADIUS]		= "Radio de las Esquínas Redondeadas";	// not yet used
	strings[CHANGE_ROUND_CORNER_RADIUS]	= "Cambia el Radio de las Esquínas Redondeadas";	// not yet used
	strings[REVERSE_PATH]		= "Invertir Trayectoria";

	// text options
	strings[FONT]				= "Fuente";
	strings[SIZE]				= "Tamaño";
	strings[ROTATION]			= "Rotación";
	strings[TEXT_INPUT]			= "Texto";				// label for text input control (not used anymore)
	strings[ADVANCE_SCALE]		= "Espaciado";			// additional spacing between chars
														// (not used anymore)
	strings[HINTING]			= "Hinting";			// hinting means that the shapes
														// of chars are modified to more directly hit pixels,
														// enhances readability of text on screen
	strings[KERNING]			= "Kerning";			// kerning means that the distance between two
														// glyphs is individual per their combination
	strings[TEXT_LINE_SPACING]	= "Espaciado de Línea";	// font height * line spacing = vertical line dist
	// color picker tool
	strings[TIP_SIZE]			= "Tamaño de la Punta";
	strings[PIXEL_1x1]			= "1x1 pixeles";
	strings[PIXEL_3x3]			= "3x3 pixeles";
	strings[PIXEL_5x5]			= "5x5 pixeles";
	strings[INCLUDE_ALL_LAYERS]	= "Incluir Todas las Capas";
	// tools
	strings[BRUSH]				= "Pincel";
	strings[CLONE]				= "Clonar";				// clone brush (transfers parts of an image to
														// another place)
	strings[ERASER]				= "Borrador";
	strings[PEN]				= "Lápiz";
	strings[PEN_ERASER]			= "Lápiz Borrador";
	strings[RESTORE]			= "Reestablecer";		// opposite of eraser (restores alpha channel)
	strings[DROPPER]			= "Gotero";				// color picker
	strings[TEXT]				= "Texto";
	strings[BLUR]				= "Difuminado";
	strings[BITMAP]				= "Mapa de Bits";
	strings[SELECT]				= "Seleccionar";
	strings[TRANSLATE]			= "Transladar";
	strings[CROP]				= "Recortar";
	strings[BUCKET_FILL]		= "Rellenar";
	strings[SHAPE]				= "Forma";
	strings[ELLIPSE]			= "Elipse";
	strings[ROUND_RECT]			= "Rectángulo (Redondeado)";	// the tool also creates non-round rectangles
	strings[EDIT_GRADIENT]		= "Editar Gradiente";
	strings[GUIDES]				= "Guías";
	strings[PICK_OBJECTS]		= "Seleccionar Objetos";
	strings[CLIPBOARD]			= "Portapapeles";

	// zoom
	strings[ZOOM_IN]			= "Acercar";			// tool tips
	strings[ZOOM_OUT]			= "Alejar";

	// actions
	strings[ADD_LAYER]			= "Agregar Capa";		// the following strings are for display in the
														// undo/redo menu
	strings[REMOVE_LAYER]		= "Eliminar Capa";		// a "modifier" is a step of changing the bitmap,
														// ie a brush stroke or something else.
	strings[REMOVE_LAYERS]		= "Eliminar Capas";
	strings[MOVE_LAYER]			= "Mover Capa";			// ie drag sorting to another pos in list
	strings[MOVE_LAYERS]		= "Mover Capas";
	strings[MERGE_LAYERS]		= "Combinar Capas";
	strings[UNKOWN_MODIFIER]	= "Objeto Desconocido";
	strings[ADD_MODIFIER]		= "Agregar Objeto";
	strings[ADD_MODIFIERS]		= "Agregar Objetos";
	strings[CHANGE_COLOR]		= "Cambiar Color";
	strings[CHANGE_COLORS]		= "Cambiar Colores";
	strings[REMOVE_MODIFIER]	= "Eliminar Objeto";
	strings[REMOVE_MODIFIERS]	= "Eliminar Objeto";
	strings[MOVE_MODIFIER]		= "Mover Objeto";
	strings[MOVE_MODIFIERS]		= "Mover Objetos";
	strings[TRANSLATE_MODIFIER]	= "Trasladar Objeto";
	strings[TRANSLATE_MODIFIERS]	= "Trasladar Objetos";
	strings[CHANGE_LAYER_SETTINGS]	= "Cambiar Opciones de la Capa";
	strings[CROP_CANVAS]		= "Recortar Lienzo";
	strings[RESIZE_CANVAS]		= "Cambiar el Tamaño del Lienzo";
	strings[ROTATE_CANVAS]		= "Rotar Lienzo";
	strings[INSERT_CANVAS]		= "Insertar Lienzo";
	strings[CHANGE_OPACITY]		= "Cambiar Opacidad";
	strings[EDIT_MODIFIER]		= "Editar Modificador";

	// pasting properties
	strings[PASTE_PROPERTY]		= "Pegar Propiedad";				// paste one property to one object
	strings[PASTE_PROPERTIES]	= "Pegar Propiedades";			// paste multiple properties to one object
	strings[MULTI_PASTE_PROPERTY] = "Pegar Propiedad Múltiple";		// paste one property to multiple objects
	strings[MUTLI_PASTE_PROPERTIES] = "Pegar Propiedades Múltiples";	// paste multiple property to multiple objects

	// shape tool history
	strings[ADD_CONTROL_POINT]	= "Agregar Punto de Control";
	strings[TOGGLE_OUTLINE]		= "Alternar Delineado";				// "Toggle Outline"
	strings[CHANGE_OUTLINE_WIDTH] = "Cambiar Ancho del Delineado";
	strings[CHANGE_CAP_MODE]	= "Cambiar Puntas";
	strings[CHANGE_JOIN_MODE]	= "Cambiar Uniones";
	strings[MODIFY_CONTROL_POINT] = "Modificar Punto de Control";
	strings[CLOSE_PATH_ACTION]	= "Cerrar Trayectoria";
	strings[INSERT_CONTROL_POINT] = "Insertar Punto de Control";
	strings[REMOVE_CONTROL_POINT] = "Eliminar Punto de Control";
	strings[REMOVE_CONTROL_POINTS] = "Eliminar Puntos de Control";
	strings[CHANGE_POINT_SELECTION] = "Cambiar Selección";
	strings[NUDGE_CONTROL_POINT] = "Nudge Punto de Control";
	strings[NUDGE_CONTROL_POINTS] = "Nudge Puntos de Control";
	strings[SELECT_PATH]		= "Seleccionar Trayectoria";
	strings[CREATE_NEW_PATH]	= "Nueva Trayectoria";
	strings[DELETE_PATH]		= "Eliminar Trayectoria";
	strings[TEXT_TO_SHAPE]		= "Texto a Forma";
	strings[TEXTS_TO_SHAPES]	= "Textos a Formas";
	strings[TRANSFORM_CONTROL_POINTS] = "Transformar Puntos de Control";
	strings[MERGE_SHAPE_OBJECTS] = "Combinar Formas";

	strings[FILLING_RULE]		= "Modo de Relleno";
	strings[NON_ZERO]			= "Distinto de Cero";
	strings[EVEN_ODD]			= "Par-Impar";

	// text tool history
	strings[CHANGE_FONT_SIZE]	= "Cambiar Tamaño de Fuente";
	strings[CHANGE_TEXT_ROTATION] = "Cambiar Rotación del Texto";
	strings[MOVE_TEXT]			= "Mover Texto";
	strings[CHANGE_FONT]		= "Cambiar Fuente";
	strings[CHANGE_TEXT]		= "Cambiar Texto";
	strings[CHANGE_FONT_ADVANCE_SCALE] = "Cambiar Espaciado";
	strings[ALIGNMENT]			= "Alineación";
	strings[CHANGE_ALIGNMENT]	= "Cambiar Alineación";
	strings[ALIGNMENT_LEFT]		= "Izquierda";
	strings[ALIGNMENT_CENTER]	= "Centrado";
	strings[ALIGNMENT_RIGHT]	= "Derecha";
	strings[ALIGNMENT_JUSTIFY]	= "Justificado";
	strings[CHANGE_TEXT_WIDTH]	= "Cambiar Ancho del Texto";
	strings[PARAGRAPH_INSET]	= "Intercalado";
	strings[CHANGE_PARAGRAPH_INSET]	= "Cambiar Intercalado";
	strings[PARAGRAPH_SPACING]	= "Esp. entre Párrafos";		// additional spacing between paragraphs
	// crop tool
	strings[MODIFY_CROP_RECT]	= "Modificar Rectángulo de Corte";
	// select tool
	strings[MODIFY_SELECT_RECT]	= "Modificar Rectángulo de Selección";
	// transformations
	strings[TRANSFORMATION]		= "Transformación";
	strings[BEGIN_TRANSFORMATION] = "Comenzar Transformación";
	strings[FINISH_TRANSFORMATION]	= "Finalizar Transformación";
	strings[MOVE]				= "Mover";
	strings[ROTATE]				= "Rotar";
	strings[SCALE]				= "Escalar";
	strings[MOVE_PIVOT]			= "Mover Pivote";
	strings[TRANSFORM_OBJECT]	= "Transformar Objeto";
	strings[TRANSFORM_OBJECTS]	= "Transformar Objetos";
	// guide tool
	strings[SHOW_GUIDES]		= "Mostrar Guías";
	strings[HORIZONTAL]			= "Horizontal";
	strings[VERTICAL]			= "Vertical";
	strings[UNIT]				= "Unidad";
	strings[UNIT_PERCENT]		= "%";
	strings[UNIT_PIXEL]			= "px";
	strings[UNIT_CM]			= "cm";
	strings[UNIT_MM]			= "mm";
	strings[UNIT_INCH]			= "pulgada";
	strings[UNIT_DPI]			= "ppp";
	// general
	strings[OK]					= "Ok";
	strings[CONFIRM]			= "Confirmar";			// apply changes
	strings[CANCEL]				= "Cancelar";
	strings[REVERT]				= "Revertir";			// revert changes made in panel
	strings[REMOVE]				= "Eliminar";
	strings[NAME]				= "Nombre";				// name of project
	strings[UNNAMED]			= "Sin Nombre";
	strings[DEFAULT]			= "Default";
	strings[MERGED]				= "Combinado";				// default name for merged layer
	strings[TOOL]				= "Herramienta";			// for tool tips
	strings[ANY_MODIFIER]		= "Cualquier Modificador";	// as in modifier key on the keyboard (shift,
															// option, alt etc)
	strings[RETURN]				= "Entrar";
	strings[ESCAPE]				= "Escape";
	strings[SET]				= "Aceptar";
	strings[COUNT]				= "Cuenta";
	strings[INTERPOLATION]		= "Interpolación";
	strings[NEAREST_NEIGHBOR]	= "Repetir";
	strings[BILINEAR]			= "Bilineal";
	strings[RETRY]				= "Reintentar";
	strings[OVERWRITE]			= "Sobreescribir";
	// panels
	strings[PICK_COLOR]			= "Selector de Color";			// window title of color picker
	strings[WIDTH]				= "Ancho";				// pixel count
	strings[HEIGHT]				= "Alto";
	strings[BRUSH_PANEL]		= "Selector de Pincel";
	strings[BRUSH_NAME]			= "Nombre del Pincel";
	strings[PROGRAM_SETTINGS]	= "Opciones del Programa";
	// program settings
	strings[ON_STARTUP]			= "Al Iniciar";
	strings[DO_NOTHING]			= "No Hacer Nada";
	strings[ASK_OPEN]			= "Mostrar Cuadro de Diálogo \"Abrir\"";
	strings[ASK_NEW]			= "Mostrar Cuadro de Diálogo \"Nuevo\"";
	strings[AUTO_SIZE]			= "Crear Nuevo Lienzo de Tamaño";
	strings[LIVE_UPDATES]		= "Actualización constante de los íconos Navegador y Capas";
	strings[USE_SYSTEM_LANGUAGE] = "Utilizar el idioma del Sistema";
	strings[STARTUP_ACTION]		= "Acción Inicial";
	strings[INTERFACE]			= "Interfaz";

	// requestors
	strings[AKS_SAVE_CHANGES]	= "¿Cerrar el documento y descartar los cambios realizados?";
	strings[DISCARD]			= "Descartar";
	strings[SAVING_FAILED]		= "¡Error al guardar!";
	strings[ERROR]				= "Error";				// alias reason for above
	strings[BLIP]				= "¡Blip!";				// As in "Mission Earth"

	strings[DEMO_MODE]			= "WonderBrush se está ejecutando en modo demostración. "
								  "La función Guardar Lienzo está deshabilitada.\n\n"
								  "Por favor regístrese en una de las siguientes "
								  "tiendas virtuales para recibir un archivo de"
								  "desbloqueo.";
	strings[NEVER_MIND]			= "No, gracias";
	strings[OPEN_TRANSLATIONKIT_ERROR]	= "Se produjo un error al abrir el archivo "
	                              "'%s' usando el Kit de Traducción.\n\n"
	                              "Puede suceder que el archivo no contenga "
	                              "datos de una imagen, o que no se encuentre "
	                              "instalado un Traductor capaz de manejar este formato.";
	strings[OPEN_NATIVE_ERROR]	= "No pudo abrirse el archivo '%s' "
								  "como documento de WonderBrush.\n\n";		// yes, that's two \n
	strings[UNSUPPORTED_CURSOR_SIZE] = "El Lienzo debe tener un tamaño"
									" de 16 x 16 pixeles. Sólo ese tamaño de cursor "
									"es admitido.";
	strings[SELECTED_AREA_EMPTY] = "El área seleccionada esta vacia.";
	strings[ENTRY_DOES_NOT_EXIST] = "'%s' no puede abrirse, porque no existe.";
	strings[ENTRY_IS_A_FOLDER]	= "'%s' no puede abrirse, porque es un directorio.";
	strings[WARN_SVG_DATA_LOSS]	= "WonderBrush puede no haber leído todos los datos desde el"
								  "archivo SVG al ser cargado."
								  "Al sobreescribir el archivo original, dicha información"
								  " se perderá.";

	// tool tips										// please test the appearance of the tips and
														// try to align them manually by putting the
														// '\n' in the right spot!
	strings[TOOL_TIPS]			= "Mostrar Sugerencias";
	strings[ALPHA_TIP]			= "Establece el valor global del canal alfa para la capa activa.";
	strings[VISIBILITY_TIP]		= "Alterna la visibilidad de la capa activa.";
	strings[PRESSURE_CONTROL_TIP]	= "Habilita el control por medio de la presión del lápiz.";
	strings[BRUSH_PREVIEW_TIP]	= "Previsualización del Pincel. Cliquée para abrir el panel de pinceles.";
	strings[CURRENT_COLOR_TIP]	= "Color Actual. Cliquée aquí para abrir el selector de colores.";
	strings[NAVIGATOR_TIP]		= "Vista general del Lienzo. Cliquée para mover el área visible.\n"
								  "Utilice la rueda del ratón para acercar o alejar la imagen.";
	strings[LAYER_LIST_TIP]		= "Ordene mediante \"Arrastrar y Soltar\". Cliquée el ícono del ojo para\n"
								  "alternar su visibilidad. Haga doble click para renombrar.";
	strings[HISTORY_LIST_TIP]	= "Ordene mediante \"Arrastrar y Soltar\". Suelte un color\npara cambiar"
								  "el color de los modificadores seleccionados.";
	strings[PROPERTY_LIST_TIP]	= "Edite, copie o pegue propiedades de objetos.";
	strings[SUB_PIXEL_TIP]		= "Respetar la posición del ratón/lápiz a nivel de subpixeles.";
	strings[SOLID_TIP]			= "No altera la opacidad por cada pixel.";
	strings[TILT_TIP]			= "Distorsiona la forma del pincel de acuerdo a la inclinación del lápiz.";
	strings[FULLSCREEN_TIP]		= "Alterna entre modos Ventana y Pantalla Completa.";
	strings[GRADIENT_CONTROL_TIP] = "Suelte colores aquí para agregar o cambiar un color puntual.\nHaga doble click en un punto para elegir su color.";
	strings[GRADIENT_INHERITS_TRANSFORM_TIP] = "Hereda la trasformación desde Objeto";

	// quick help
	strings[READY]				= "Listo.";
	strings[NO_CANVAS_HELP]		= "Cree un nuevo lienzo, o cargue un proyecto o una imagen.";
	strings[NO_HELP]			= "No hay sugerencias adicionales.";
	strings[CROP_HELP]			= "Ajuste los límites del rectángulo, presione <Entrar> para aplicar.\n<Mayús> limita la relación de aspecto.";
	strings[TRANSLATE_HELP]		= "Selecione los Objetos a ser transformados.\n<Mayús> limita la dirección o la relación de aspecto.";
	strings[EDIT_GRADIENT_HELP]	= "Edite los colores, tipo y transformación de los gradientes.";
	strings[STROKE_HELP]		= "Utilice <Mayús> para extender un trazo anterior y dibujar una línea recta.";
	strings[CLONE_HELP]			= "El primer click define el origen, el segundo el desplazamiento.\nPresione <Ctrl> para reestablecer el desplazamiento.";
	strings[BUCKET_FILL_HELP]	= "\"Suavidad\" controla la solidez del color de relleno.";
	strings[TEXT_HELP]			= "Cuando esté listo, presione <Entrar> para aplicar la herramienta.";
	strings[SHAPE_HELP]			= "Mantenga presionado <Ctrl>, <Mayús>, <Alt> u <Opción>\npara acceder a opciones adicionales de edición de puntos.\nPresione <Entrar> cuando termine.";
	strings[SELECT_HELP]		= "Defina un área. Presione <Entrar> para copiar dicha area al portapapeles del sistema.";
	strings[ELLIPSE_HELP]		= "Cliquée y arrastre para crear una elipse.\n<Mayús> limita la relación de aspecto.";
	strings[ROUND_RECT_HELP]	= "Cliquée y arrastre para crear un rectángulo.\n<Mayús> limita la relación de aspecto.";
	strings[PICK_OBJECTS_HELP]	= "Seleccione Objetos haciendo clic en ellos,\no enmarcándolos en un rectángulo.\n<Mayús> mantiene la selección previa.";
	strings[GUIDES_HELP]		= "Cree, edite o elimine guías.";

	strings[ENTER_SERIAL_NUMBER] = "Por favor, ingrese el número de serie incluído en su copia de WonderBrush.";
	strings[WRONG_SERIAL_NUMBER] = "Ha ingresado un número de serie no válido.";
}
