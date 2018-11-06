// main.cpp

#include <signal.h>
#include <stdio.h>

#include <Alert.h>
#include <Bitmap.h>
#include <File.h>

#include "support.h"
#include "blending.h"
#if USE_COMPRESSION
#include "minilzo.h"
#endif
#if USE_LCMS
#include "cms_support.h"
#endif
#include "lab_convert.h"

#if ALLOCATION_CHECK
	#include "AllocationChecker.h"
#endif
#include "BigIntGenerator.h"
#include "BubbleHelper.h"
#include "GlobalSettings.h"
#include "Layer.h"
#include "LanguageManager.h"
#include "PSDFile.h"
#include "WonderBrushApp.h"

static thread_id sMainThread;

// signalHandler
void
signalHandler(int signalNumber)
{
	switch (signalNumber) {
		case SIGINT:
		case SIGQUIT:
		case SIGTERM:
			// Note: There's probably a race condition here. The reason is that
			// the code executed in a signal handler can trick nested locks
			// that were held at the time the thread was interrupted. Since
			// even malloc() probably protects internal data with a nested
			// lock and sending a message might require memory allocation too,
			// there could be a chance of reentering critical code.
			//
			// The only clean solution would probably be to have a thread that
			// literally does nothing but snoozing, which would be the one
			// sending the B_QUIT_REQUESTED message at this point. Since it
			// normally does nothing, it can't possibly reenter critical code.
			if (find_thread(NULL) == sMainThread)
				be_app_messenger.SendMessage(B_QUIT_REQUESTED);
			break;
		default:
			break;
	}
}

#ifdef USE_SERIAL_NUMBERS

#include "rsa.h"
#include "serial_number.h"

#include "SerialPanel.h"

static const unsigned char kSerialNumberKey[] = {
	0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x78, 0xa7, 0xc9,
	0x57, 0x19, 0x7b, 0xa6, 0xd0, 0x70, 0x13, 0x1a, 0x4f, 0x86,
	0xa7, 0xfc, 0xcc, 0xc7, 0xbd, 0x95, 0x2c, 0x29, 0x06, 0xa6,
	0x3a, 0xcb, 0x01, 0xf2, 0x4f, 0xbe, 0xc7, 0xba, 0xde
};

// check_serial_number
static bool
check_serial_number()
{
	// prepare the rsa key
	rsa_key key;
	status_t error = rsa_unflatten_key(kSerialNumberKey,
		sizeof(kSerialNumberKey), key);
	if (error != B_OK) {
		fprintf(stderr, "Failed to unflatten serial number RSA key: %s\n",
			strerror(error));
		return false;
	}

	// get the serial number stored in the global settings
	const char* serialNumber = GlobalSettings::Default().SerialNumber();

	// check the serial number
	static const int firstAcceptedMajor = 2;
	static const int firstAcceptedMiddle = 0;
	if (serialNumber) {
		if (check_serial_number(serialNumber, MAJOR_APP_VERSION,
				MIDDLE_APP_VERSION, firstAcceptedMajor, firstAcceptedMiddle,
				key)) {
			return true;
		}
	}

	LanguageManager* m = LanguageManager::Default();

	while (true) {
		SerialPanel* panel = new SerialPanel();
		BString serialNumber;
		if (panel->Go(&serialNumber)) {
			// filter out dashes and newlines
			BString filteredSerialNumber(serialNumber);
			filteredSerialNumber.ReplaceAll("-", "");
			filteredSerialNumber.ReplaceAll("\n", "");
			serialNumber = filteredSerialNumber.String();
	
			// check the serial number
			if (check_serial_number(serialNumber.String(), MAJOR_APP_VERSION,
					MIDDLE_APP_VERSION, firstAcceptedMajor, firstAcceptedMiddle,
					key)) {
				GlobalSettings::Default().SetSerialNumber(serialNumber.String());
				return true;
			}
		} else {
			// user canceled the panel
			return false;
		}

		BAlert* alert = new BAlert("wrong serial",
								   m->GetString(WRONG_SERIAL_NUMBER,
								   				"You have entered an "
												"invalid Serial Number."),
								   m->GetString(CANCEL, "Cancel"),
								   m->GetString(RETRY, "Retry"));
		if (alert->Go() == 0)
			return false;
	}
}

#endif	// USE_SERIAL_NUMBERS


// main
int
main(int argc, char** argv)
{
#if ALLOCATION_CHECK
	AllocationChecker::CreateDefault();
#endif

	GlobalSettings::CreateDefault();

#if USE_COMPRESSION
	lzo_init();
	Layer::AllocateCompressionMem();
#endif

#if GAMMA_BLEND
	init_gamma_blending();
#endif

	if (argc > 1 && strcmp(argv[1], "--benchmark") == 0) {
		BApplication dummy("application/x.vnd-YellowBites.WonderBrush");
		BRect canvasFrame(0.0, 0.0, 799.0, 599.0);
		BBitmap* testBitmap1 = new BBitmap(canvasFrame, B_RGB32);
		memset(testBitmap1->Bits(), 50, testBitmap1->BitsLength());
		BBitmap* testBitmap2 = new BBitmap(canvasFrame, B_RGB32);
		if (argc > 2 && strcmp(argv[2], "worst") == 0)
			memset(testBitmap2->Bits(), 50, testBitmap2->BitsLength());
		else
			memset(testBitmap2->Bits(), 255, testBitmap2->BitsLength());
		Layer* testLayer = new Layer(testBitmap1);
		if (argc > 2) {
			if (strcmp(argv[2], "multiply") == 0)
				testLayer->SetMode(MODE_MULTIPLY);
			else if (strcmp(argv[2], "inv-multiply") == 0)
				testLayer->SetMode(MODE_INVERSE_MULTIPLY);
			else if (strcmp(argv[2], "luminance") == 0)
				testLayer->SetMode(MODE_LUMINANCE);
		}
		bigtime_t now = system_time();
		testLayer->Compose(testBitmap2, testBitmap2->Bounds());
		printf("blending (%ldx%ld): %lld µsecs\n",
			   canvasFrame.IntegerWidth() + 1,
			   canvasFrame.IntegerHeight() + 1,
			   system_time() - now);
		delete testLayer;
		delete testBitmap2;
	} else if (argc > 1 && strcmp(argv[1], "--testpsd") == 0) {
		BFile file("/boot/home/test.psd", B_CREATE_FILE | B_READ_WRITE | B_ERASE_FILE);
		if (file.InitCheck() >= B_OK) {
			PSDFile psd;
			psd.Write(&file, NULL);
		}
	} else if (argc > 1 && strcmp(argv[1], "--dumpstrings") == 0) {
		if (argc > 2) {
			BApplication dummy("application/x.vnd-YellowBites.WonderBrush");
			LanguageManager* manager = LanguageManager::Default();
			if (strcmp(argv[2], "all") == 0) {
				for (int32 i = 0; manager->SetLanguage(i); i++) {
					manager->DumpMissingStrings();
				}
			} else if (manager->SetLanguage(argv[2])) {
				manager->DumpMissingStrings();
			}
		} else {
			printf("Please specify the name of the language or \"all\" to dump the "
				   "missing strings of all languages.\n");
		}
	} else {
		BigIntGenerator::CreateDefault();

		// We need to install the signal handler early. Apparently
		// 1) CTRL-C causes a SIGINT to be sent to all threads of the
		//    application, so we need signal handlers for all of them, and
		// 2) signal handlers are inherited by child threads.
		// Hence, installing the signal handler as long as there's only the
		// main thread ensures that all threads get the signal handler.
		sMainThread = find_thread(NULL);
		signal(SIGINT, signalHandler);

		new WonderBrushApp();

		#ifdef USE_SERIAL_NUMBERS
			if (check_serial_number())
				be_app->Run();
		#else
			be_app->Run();
		#endif

		delete be_app;

		// delete global instances of some classes
		BubbleHelper::DeleteDefault();
		BigIntGenerator::DeleteDefault();
	}

#if ALLOCATION_CHECK
	AllocationChecker::DeleteDefault();
#endif

#if USE_COMPRESSION
	Layer::FreeCompressionMem();
#endif

	GlobalSettings::DeleteDefault();

	return 0;
}
