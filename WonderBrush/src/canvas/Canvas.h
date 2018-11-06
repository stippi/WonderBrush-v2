// Canvas.h

#ifndef CANVAS_H
#define CANVAS_H

#include <String.h>
#include <TranslationDefs.h>

#include "LayerContainer.h"

struct entry_ref;

class BMessage;
class BMessenger;
class Exporter;
class HistoryManager;
class Layer;
class Guide;

class Canvas : public LayerContainer {
 public:
								Canvas(BRect frame);
								Canvas(BBitmap* bitmap);
	virtual						~Canvas();

								// LayerContainer
	virtual	void				Minimize(bool doIt);

	// canvas properties
			void				SetName(const char* name);
			const char*			Name() const;
	virtual	const char*			DefaultName() const;

			void				SetCurrentLayer(int32 index);
			int32				CurrentLayer() const;

			void				SetColorSpace(uint32 space);
			uint32				ColorSpace() const;

			void				SetDocumentRef(const entry_ref* ref);
			const entry_ref*	DocumentRef() const;

			void				SetExportRef(const entry_ref* ref);
			const entry_ref*	ExportRef() const;

			void				SetExporter(Exporter* exporter);
			Exporter*			GetExporter() const
									{ return fExporter; }

			void				SetZoomState(float zoom,
											 BPoint center);
			void				GetZoomState(float& zoom,
											 BPoint& center) const;

	// loading / saving
	virtual	status_t			Archive(BMessage* into) const;
	virtual	status_t			Unarchive(BMessage* archive);

			void				SetTemporaryHistory(HistoryManager* history);
			HistoryManager*		GetHistoryManager() const;
			bool				IsSaved() const;

	// printing
			void				SetPageSetupSettings(BMessage* message);
			BMessage*			PageSetupSettings() const;

	// observer (if WonderBrush is used as External Editor)
			void				SetExternalObserver(const BMessenger* messenger,
									const entry_ref* watchedRef = NULL);
			const BMessenger*	ExternalObserver() const;
			const entry_ref*	ExternalObserverRef() const
									{ return fEditObserverWatchedRef; }

	// guides
			bool				AddGuide(Guide* guide);
			Guide*				RemoveGuide(int32 index);
			bool				RemoveGuide(Guide* guide);

			int32				CountGuides() const;
			Guide*				GuideAt(int32 index) const;

			void				SetShowGuides(bool show);
			bool				ShowGuides() const
									{ return fShowGuides; }

 private:
 	// properties
	BString						fName;
	int32						fCurrentLayer;
	uint32						fColorSpace;
	// references to files and format
	entry_ref*					fDocumentRef;
	entry_ref*					fExportRef;

	Exporter*					fExporter;

	// BMessenger that is observing changes to the file
	BMessenger*					fEditObserver;
	entry_ref*					fEditObserverWatchedRef;

	// undo/redo stack
	HistoryManager*				fHistory;
	HistoryManager*				fTemporaryHistory;

	// selection
	Layer*						fSelection;

	// zoom state
	float						fZoomLevel;
	BPoint						fCenter;

	// printing
	BMessage*					fPageSetupSettings;

	// guides
	BList						fGuides;
	bool						fShowGuides;
};

#endif // CANVAS_H
