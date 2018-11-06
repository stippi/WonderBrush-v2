// Canvas.cpp

#include <stdio.h>

#include <Bitmap.h>
#include <Entry.h>
#include <Message.h>
#include <Messenger.h>

#include "defines.h"

#include "Exporter.h"
#include "Guide.h"
#include "LanguageManager.h"
#include "Layer.h"
#include "HistoryManager.h"

#include "Canvas.h"

// constructor
Canvas::Canvas(BRect frame)
	: LayerContainer(frame),
	  fName(""),
	  fCurrentLayer(-1),
	  fColorSpace(COLOR_SPACE_NONLINEAR_RGB),
	  fDocumentRef(NULL),
	  fExportRef(NULL),
	  fExporter(NULL),
	  fEditObserver(NULL),
	  fEditObserverWatchedRef(NULL),
	  fHistory(new HistoryManager()),
	  fTemporaryHistory(NULL),
	  fSelection(NULL),
//	  fSelection(new Layer(frame)),
	  fZoomLevel(100.0),
	  fCenter((frame.left + frame.right) / 2.0, (frame.top + frame.bottom) / 2.0),
	  fPageSetupSettings(NULL),
	  fGuides(4),
	  fShowGuides(true)
{
}

// constructor
Canvas::Canvas(BBitmap* bitmap)
	: LayerContainer(bitmap->Bounds().OffsetToCopy(0.0, 0.0)),
	  fName(""),
	  fCurrentLayer(-1),
	  fColorSpace(COLOR_SPACE_NONLINEAR_RGB),
	  fDocumentRef(NULL),
	  fExportRef(NULL),
	  fEditObserver(NULL),
	  fEditObserverWatchedRef(NULL),
	  fHistory(new HistoryManager()),
	  fTemporaryHistory(NULL),
	  fSelection(NULL),
	  fZoomLevel(100.0),
	  fCenter(-1.0, -1.0),
	  fPageSetupSettings(NULL),
	  fGuides(4),
	  fShowGuides(true)
{
	if (bitmap->IsValid()) {
		Layer* layer = new Layer(bitmap);
		if (!AddLayer(layer))
			delete layer;
		else {
			BRect r(Bounds());
			fCenter.x = (r.left + r.right) / 2.0;
			fCenter.y = (r.top + r.bottom) / 2.0;
//			fSelection = new Layer(bitmap->Bounds());
		}
	}
}

// destructor
Canvas::~Canvas()
{
	delete fDocumentRef;
	delete fExportRef;
	delete fExporter;
	delete fEditObserver;
	delete fEditObserverWatchedRef;
	delete fHistory;
	delete fSelection;
	delete fPageSetupSettings;

	for (int32 i = 0; Guide* guide = GuideAt(i); i++)
		delete guide;
}

// Minimize
void
Canvas::Minimize(bool doIt)
{
	if (fSelection)
		fSelection->Minimize(doIt);

	LayerContainer::Minimize(doIt);
}

// SetName
void
Canvas::SetName(const char* name)
{
	fName.SetTo(name);
}

// Name
const char*
Canvas::Name() const
{
	if (fName.CountChars() > 0)
		return fName.String();
	return DefaultName();
}

// DefaultName
const char*
Canvas::DefaultName() const
{
	return LanguageManager::Default()->GetString(UNNAMED, "<unnamed>");
}

// SetCurrentLayer
void
Canvas::SetCurrentLayer(int32 index)
{
	if (Layer* layer = LayerAt(fCurrentLayer))
		layer->SetCurrent(false);
	fCurrentLayer = index;
	if (Layer* layer = LayerAt(fCurrentLayer))
		layer->SetCurrent(true);
}

// CurrentLayer
int32
Canvas::CurrentLayer() const
{
	return fCurrentLayer;
}

// SetColorSpace
void
Canvas::SetColorSpace(uint32 space)
{
	fColorSpace = space;
}

// ColorSpace
uint32
Canvas::ColorSpace() const
{
	return fColorSpace;
}

// SetDocumentRef
void
Canvas::SetDocumentRef(const entry_ref* ref)
{
	if (ref) {
		if (!fDocumentRef)
			fDocumentRef = new entry_ref(*ref);
		else
			*fDocumentRef = *ref;
	}
}

// DocumentRef
const entry_ref*
Canvas::DocumentRef() const
{
	return fDocumentRef;
}

// SetExportRef
void
Canvas::SetExportRef(const entry_ref* ref)
{
	if (ref) {
		if (!fExportRef)
			fExportRef = new entry_ref(*ref);
		else
			*fExportRef = *ref;
	}
}

// ExportRef
const entry_ref*
Canvas::ExportRef() const
{
	return fExportRef;
}

// SetExporter
void
Canvas::SetExporter(Exporter* exporter)
{
	delete fExporter;
	fExporter = exporter;
}

// SetZoomState
void
Canvas::SetZoomState(float zoom, BPoint center)
{
	fZoomLevel = zoom;
	fCenter = center;
}

// GetZoomState
void
Canvas::GetZoomState(float& zoom, BPoint& center) const
{
	zoom = fZoomLevel;
	center = fCenter;
}

static const char*	COLORSPACE_KEY		= "color space";
static const char*	CURRENT_LAYER_KEY	= "current layer";
static const char*	PAGE_SETUP_KEY		= "page setup settings";
static const char*	ZOOM_LEVEL_KEY		= "zoom level";
static const char*	ZOOM_CENTER_KEY		= "zoom center";
static const char*	GUIDES_KEY			= "guide";
static const char*	SHOW_GUIDES_KEY		= "show guides";

// Archive
status_t
Canvas::Archive(BMessage* into) const
{
	status_t status = LayerContainer::Archive(into);
	if (status >= B_OK)
		status = into->AddInt32(COLORSPACE_KEY, fColorSpace);
	if (status >= B_OK)
		status = into->AddInt32(CURRENT_LAYER_KEY, fCurrentLayer);
	if (status >= B_OK && fPageSetupSettings)
		status = into->AddMessage(PAGE_SETUP_KEY, fPageSetupSettings);
	if (status >= B_OK)
		status = into->AddFloat(ZOOM_LEVEL_KEY, fZoomLevel);
	if (status >= B_OK)
		status = into->AddPoint(ZOOM_CENTER_KEY, fCenter);

	// add guides
	if (status >= B_OK) {
		BMessage guideArchive;
		for (int32 i = 0; Guide* guide = GuideAt(i); i++) {
			guideArchive.MakeEmpty();
			status = guide->Archive(&guideArchive);
			if (status >= B_OK)
				status = into->AddMessage(GUIDES_KEY, &guideArchive);
			if (status < B_OK)
				break;
		}
	}

	if (status >= B_OK)
		status = into->AddBool(SHOW_GUIDES_KEY, fShowGuides);

	// we don't add our name, since it will always be corresponding
	// to the file name under which we were saved.
	return status;
}

// Unarchive
status_t
Canvas::Unarchive(BMessage* archive)
{
	status_t status = LayerContainer::Unarchive(archive);
	if (status >= B_OK) {
		// restore color space
		uint32 colorSpace;
		if (archive->FindInt32(COLORSPACE_KEY, (int32*)&colorSpace) >= B_OK)
			fColorSpace = colorSpace;
		// restore current layer
		int32 index;
		if (archive->FindInt32(CURRENT_LAYER_KEY, &index) >= B_OK)
			SetCurrentLayer(index);
		// restore page setup settings
		BMessage pageSetupSettings;
		if (archive->FindMessage(PAGE_SETUP_KEY, &pageSetupSettings) >= B_OK)
			fPageSetupSettings = new BMessage(pageSetupSettings);
		// restore zoom state
		float zoomLevel;
		if (archive->FindFloat(ZOOM_LEVEL_KEY, &zoomLevel) >= B_OK)
			fZoomLevel = zoomLevel;
		BPoint center;
		if (archive->FindPoint(ZOOM_CENTER_KEY, &center) >= B_OK)
			fCenter = center;
		// restore guides
		BMessage guideArchive;
		for (int32 i = 0; archive->FindMessage(GUIDES_KEY, i, &guideArchive) >= B_OK; i++) {
			Guide* guide = new Guide(&guideArchive);
			if (!AddGuide(guide)) {
				delete guide;
// this would be correct, but on the other hand, as a user, I would
// still want this canvas to load nevertheless...
//				status = B_NO_MEMORY;
				break;
			}
		}
		// restore show guides
		bool showGuides;
		if (archive->FindBool(SHOW_GUIDES_KEY, &showGuides) >= B_OK)
			fShowGuides = showGuides;

	}
	return status;
}

// SetTemporaryHistory
void
Canvas::SetTemporaryHistory(HistoryManager* history)
{
	fTemporaryHistory = history;
}

// GetHistoryManager
HistoryManager*
Canvas::GetHistoryManager() const
{
	if (fTemporaryHistory)
		return fTemporaryHistory;
	return fHistory;
}

// IsSaved
bool
Canvas::IsSaved() const
{
	bool saved = fHistory->IsSaved();
	if (saved && fTemporaryHistory)
		saved = fTemporaryHistory->IsSaved();
	return saved;
}

// SetPageSetupSettings
void
Canvas::SetPageSetupSettings(BMessage* message)
{
	delete fPageSetupSettings;
	fPageSetupSettings = message;
}

// PageSetupSettings
BMessage*
Canvas::PageSetupSettings() const
{
	return fPageSetupSettings;
}

// SetExternalObserver
void
Canvas::SetExternalObserver(const BMessenger* messenger,
							const entry_ref* watchedRef)
{
	delete fEditObserver;
	fEditObserver = NULL;
	if (messenger)
		fEditObserver = new BMessenger(*messenger);

	delete fEditObserverWatchedRef;
	fEditObserverWatchedRef = NULL;
	if (watchedRef)
		fEditObserverWatchedRef = new entry_ref(*watchedRef);
}

// ExternalObserver
const BMessenger*
Canvas::ExternalObserver() const
{
	return fEditObserver;
}

// AddGuide
bool
Canvas::AddGuide(Guide* guide)
{
	if (!guide)
		return false;
	return fGuides.AddItem((void*)guide);
}

// RemoveGuide
Guide*
Canvas::RemoveGuide(int32 index)
{
	return (Guide*)fGuides.RemoveItem(index);
}

// RemoveGuide
bool
Canvas::RemoveGuide(Guide* guide)
{
	return fGuides.RemoveItem((void*)guide);
}

// CountGuides
int32
Canvas::CountGuides() const
{
	return fGuides.CountItems();
}

// GuideAt
Guide*
Canvas::GuideAt(int32 index) const
{
	return (Guide*)fGuides.ItemAt(index);
}

// SetShowGuides
void
Canvas::SetShowGuides(bool show)
{
	fShowGuides = show;
}

