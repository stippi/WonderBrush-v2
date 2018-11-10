// load.cpp

#include <stdio.h>

#include <Alert.h>
#include <Bitmap.h>
#include <Entry.h>
#include <File.h>
#include <Message.h>
#include <Messenger.h>
#include <Path.h>
#include <Roster.h>
#include <String.h>
#include <TranslatorRoster.h>
#include <TranslationUtils.h>

#include "bitmap_support.h"

#include "AppendCanvasAction.h"
#include "Canvas.h"
#include "CanvasView.h"
#include "DocumentBuilder.h"
#include "LanguageManager.h"
#include "Layer.h"
#include "MainWindow.h"
#include "TranslatorExporter.h"
#include "SVGExporter.h"
#include "SVGParser.h"

#include "load.h"

// load_from_message
Canvas*
load_from_message(entry_ref* ref)
{
	Canvas* canvas = NULL;
	BFile file(ref, B_READ_ONLY);
	if (file.InitCheck() >= B_OK) {
		// try to load the file as our own native document format
		BMessage archive;
		status_t status = archive.Unflatten(&file);
		if (status >= B_OK) {
			canvas = new Canvas(BRect(0.0, 0.0, -1.0, -1.0));
			status = canvas->Unarchive(&archive);
			if (status < B_OK) {
				// loading failed
				delete canvas;
				canvas = NULL;
				// report error
				printf("error opening flattened BMessage as native "
					   "document: %s\n", strerror(status));
				printf("dumping message to stream:\n\n");
				archive.PrintToStream();
				// additional GUI alert
				LanguageManager* manager = LanguageManager::Default();
				char error[1024];
				sprintf(error, manager->GetString(OPEN_NATIVE_ERROR,
										   		  "Failed to open the file "
										   		  "'%s' as WonderBrush "
										   		  "document.\n\n"), ref->name);
				BString helper(error);
				helper << manager->GetString(ERROR, "Error") << ": "
					<< strerror(status);
				BAlert* alert = new BAlert("load error",
										   helper.String(),
										   manager->GetString(OK, "Ok"),
										   NULL, NULL, B_WIDTH_AS_USUAL,
										   B_WARNING_ALERT);
				alert->Go(NULL);
			} else {
				// loading successful
				canvas->SetName(ref->name);
				canvas->SetDocumentRef(ref);
				be_roster->AddToRecentDocuments(ref);
			}
		}
	}
	return canvas;
}

// load_from_bitmap
Canvas*
load_from_bitmap(entry_ref* ref)
{
	Canvas* canvas = NULL;
	BBitmap* bitmap = NULL;
	Layer* layer = NULL;

	// load the bitmap using the Translation Kit and
	// convert the color space if necessary
	if ((bitmap = BTranslationUtils::GetBitmap(ref))
		&& bitmap->IsValid()) {
		if (bitmap->ColorSpace() != B_RGBA32) {
			// convert to native color space first
			BBitmap* oldBitmap = bitmap;
			bitmap = get_bgra32_bitmap(oldBitmap);
			delete oldBitmap;
		}
		// create just the layer by itself from the bitmap
		layer = new Layer(bitmap);
	}

	if (layer && layer->InitCheck() >= B_OK) {
		be_roster->AddToRecentDocuments(ref);
		// we have the layer containing the bitmap,
		// now we have to create the document/canvas to put the layer in
		canvas = new Canvas(layer->Bitmap()->Bounds());
		canvas->AddLayer(layer);
		canvas->SetName(ref->name);
		canvas->SetExportRef(ref);
		layer->SetName(ref->name);
		// remember translator info in canvas
		BTranslatorRoster* roster = BTranslatorRoster::Default();
		BFile file(ref, B_READ_ONLY);
		if (roster && file.InitCheck() >= B_OK) {
			translator_info info;
			if (roster->Identify(&file, NULL, &info, 0, NULL,
								 B_TRANSLATOR_BITMAP) >= B_OK) {
				TranslatorExporter* exporter = new TranslatorExporter();
				exporter->SetTranslatorInfo(info.translator, info.type);
				canvas->SetExporter(exporter);
			}
		}
	} else {
		// we failed for some reason, inform the user
		delete layer;
		LanguageManager* manager = LanguageManager::Default();
		char error[1024];
		sprintf(error, manager->GetString(OPEN_TRANSLATIONKIT_ERROR,
								   		  "Failed to open the file '%s' "
								   		  "using the Translation Kit.\n\n"
								   		  "Either the file does not contain "
								   		  "an image, or there is no "
								   		  "Translator installed to "
								   		  "handle its format."), ref->name);
		BAlert* alert = new BAlert("load error",
								   error, manager->GetString(OK, "Ok"),
								   NULL, NULL, B_WIDTH_AS_USUAL,
								   B_WARNING_ALERT);
		alert->Go(NULL);
	}
	return canvas;
}

// load_from_svg
Canvas*
load_from_svg(entry_ref* ref)
{
	Canvas* canvas = NULL;
	BPath path(ref);
	if (path.InitCheck() >= B_OK) {
		// TODO: See if this could be an SVG file
		bool isSVG = false;
		BFile file(path.Path(), B_READ_ONLY);
		if (file.InitCheck() >= B_OK) {
			ssize_t size = 5;
			char* buffer = new char[size + 1];
			if (file.Read(buffer, size) == size) {
				// 0 terminate
				buffer[size] = 0;
				if (strcasecmp(buffer, "<?xml") == 0) {
					// we're stretching it a bit, but what the heck
					isSVG = true;
				}
			}
			delete[] buffer;
		}
		if (isSVG) {
		    try {
				agg::svg::DocumentBuilder builder;
				agg::svg::Parser parser(builder);
				parser.parse(path.Path());
//				builder.arrange_orientations();
				canvas = builder.GetCanvas(ref->name);
				if (canvas) {
					canvas->SetExportRef(ref);
					SVGExporter* exporter = new SVGExporter();
					// if tags have been ignored, we remember the ref of
					// the original SVG, and when the user tries to overwrite
					// it by exporting SVG again, we can warn him that data
					// will be lost
					if (parser.TagsIgnored())
						exporter->SetOriginalEntry(ref);
					canvas->SetExporter(exporter);
				}
			} catch(agg::svg::exception& e) {
				LanguageManager* manager = LanguageManager::Default();
				char error[1024];
				sprintf(error, /*manager->GetString(OPEN_SVG_ERROR,*/
										   		  "Failed to open the "
										   		  "file '%s' as "
										   		  "an SVG Document.\n\n"
										   		  "Error: %s"/*)*/,
										   		  ref->name, e.msg());
				BAlert* alert = new BAlert("load error",
										   error, manager->GetString(OK, "Ok"),
										   NULL, NULL, B_WIDTH_AS_USUAL,
										   B_WARNING_ALERT);
				alert->Go(NULL);
			}
		}
	}
	return canvas;
}


// load
//
// tries to load a given file as BMessage archive or Translation bitmap
// and returns a Canvas object if successful
Canvas*
load(entry_ref* ref)
{
	// dereference
	BEntry entry(ref, true);
	entry.GetRef(ref);

	Canvas* canvas = load_from_message(ref);

	if (!canvas)
		canvas = load_from_svg(ref);

	if (!canvas)
		canvas = load_from_bitmap(ref);

	return canvas;
}

// load
int32
load(void* arg)
{
	load_info* info = (load_info*)arg;
	if (info && info->window && info->message) {
		// see if message contains a target (external messenger)
		// that wants to be notified of editing changes
		const BMessenger* externalObserver;
		BMessenger messenger;
		if (info->message->FindMessenger("target", &messenger) < B_OK)
			externalObserver = NULL;
		else
			externalObserver = &messenger;

		// try to load all the entries in the message
		entry_ref ref;
		LanguageManager* m = LanguageManager::Default();
		const char* loadingFile = m->GetString(LOADING_FILE, "Loading file:");
		BEntry testEntry;
		for (int32 i = 0; info->message->FindRef("refs", i, &ref) >= B_OK;
			 i++) {
			testEntry.SetTo(&ref, true);
			if (!testEntry.Exists()) {
				char error[2048];
				sprintf(error, m->GetString(ENTRY_DOES_NOT_EXIST,
											"'%s' could not be loaded, "
											"because it does not exist."),
											ref.name);
				info->window->SetStatus(error, 2);
				continue;
			}
			if (testEntry.IsDirectory()) {
				char error[2048];
				sprintf(error, m->GetString(ENTRY_IS_A_FOLDER,
											"'%s' could not be loaded, "
											"because it is a directory."),
											ref.name);
				info->window->SetStatus(error, 2);
				continue;
			}
			BString string(loadingFile);
			string << " \"" << ref.name << "\"";
			info->window->SetBusy(true, string.String());
			if (Canvas* canvas = load(&ref)) {
				if (info->window->Lock()) {
					Canvas* appendTo = info->canvas_view ?
						info->canvas_view->CurrentCanvas() : NULL;
					if (appendTo) {
						info->canvas_view->Perform(
							new AppendCanvasAction(appendTo, canvas,
												   info->append_index));
						delete canvas;
					} else {
						canvas->SetExternalObserver(externalObserver,
													externalObserver ?
														&ref : NULL);
						info->window->AddDocument(canvas);
						info->window->SetToDocument(canvas);
					}
					info->window->Unlock();
				}
			}
		}
		info->window->SetBusy(false);
	}
	if (info) {
		delete info->message;
		delete info;
	}
	return 0;
}
