// load.h

#ifndef LOAD_H
#define LOAD_H

class BMessage;
class CanvasView;
class MainWindow;

struct load_info {
	MainWindow*			window;
	CanvasView*			canvas_view;
	BMessage*			message;
	int32				append_index;
};

int32	load(void* /*load_info*/ info);

#endif // LOAD_H
