// main.cpp

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Application.h>
#include <Bitmap.h>
#include <String.h>

#include "support.h"

#include "MapImage.h"
#include "RGB8Image.h"
#include "WarpManager.h"

static bool useMap = false;
static bool saveMap = false;
static const char* fileName = "";
static const char* outFileName = "./sharped_image";
static const char* mapFileName = NULL;
static const char* translatorName = NULL;

// ------------------------------------------------------
// scroll down for main(), it is heavily commented
// ------------------------------------------------------

// get_file_name
char *
get_file_name(int argc, char *argv[])
{
	char* n = ((argc > 1) && (argv[argc - 1][0] != '-')) ? argv[argc - 1] : NULL;
	if (!n)
		return NULL;
	return strdup(n);
}

// print_help
void
print_help()
{
  cout<<"USAGE: warpsharp [options] IMAGEFILENAME\n"
      <<"Options:\n"
      <<" --help:     print this help message.\n"
      <<" -o FILE:    save output image in FILE, default is \"./sharped_image\".\n"
      <<" -t NAME:    NAME = image translator for output image, default is \"TGA image\".\n"
      <<" -rR:        R = <integer> radius of edge-detector, default is 2.\n"
      <<" -n:         Apply non-maximal supression, default is FALSE.\n"
      <<" -a MAPFILE: Save mapfile in MAPFILE, default is FALSE\n"
      <<" -d MAPFILE: Use mapfile from MAPFILE, default is FALSE\n"
      <<" -lL:        L = <float> lambda, default is 10.0.\n"
      <<" -mM:        M = <float> mu, default is 0.01.\n"
      <<" -sS:        S = <scale> scale, default is 1.0.\n";
}

// initialize_sharp
void
initialize_sharp(uint32 argc, char* argv[], WarpManager* manager)
{ 
	// do some hacky parsing of the command line
	// parameters
	for (uint32 k = 1; k < argc; k++) {
		if (!strcmp(argv[k], "--help"))
			print_help(); 

		if (!strcmp(argv[k], "-o"))
			outFileName = argv[++k];

		if (!strcmp(argv[k], "-t"))
			translatorName = argv[++k];

		if (!strcmp(argv[k], "-d")) {
			useMap = true;  
			mapFileName = argv[++k];
		}

		if (!strcmp(argv[k], "-a")) {
			saveMap = true; 
			mapFileName = argv[++k];
		}

		if (!strcmp(argv[k], "-n"))
			manager->SetUseNonMaximalSupression(true);

		if (!strncmp(argv[k], "-l", 2))
			manager->SetLambda(atof(argv[k] + 2));

		if (!strncmp(argv[k], "-m", 2))
			manager->SetMu(atof(argv[k] + 2));

		if (!strncmp(argv[k], "-r", 2))
			manager->SetEdgeRadius(uint32(atoi(argv[k] + 2)));

		if (!strncmp(argv[k], "-s", 2))
			manager->SetScale(atof(argv[k] + 2));
	}
	fileName = get_file_name(argc, argv);
}

// save_result
void
save_result(RGB8Image* image, const char* name)
{
	if (image && image->IsValid()) {
		// save_bitmap() takes a normal
		// BBitmap as input parameter,
		// which we can conviniently get
		// from the RGB8Image.
		// The bitmap is saved using the
		// BTranslationRoster and its friends.
		// You need to have a valid BApplication
		// to make use of this feature
		BBitmap* bitmap = image->GetBitmap();
		if (bitmap) {
			printf("saving bitmap (%s)....\n", name);
			// We leave it up to save_bitmap() to
			// call bitmap->IsValid() and all...
			save_bitmap(bitmap, name);
		} else
			fprintf(stderr, "no bitmap.\n");
		// The BBitmap returned from RGB8Image::GetBitmap()
		// is ours.
		delete bitmap;
	} else {
		fprintf(stderr, "no result.\n");
	}
}

// main
int
main(int argc, char *argv[])
{
	// In order to use the BeOS translation system
	// we need a valid BApplication or we will make
	// a trip to the debugger when calling save_bitmap().
	// We don't call BApplication::Run(), because we
	// would block doing so, and it is not necessary.
	BApplication dummy("application/x.vnd-YellowBites.warpsharp");

	// A WarpManager holds all information and functions
	// to sharp an image using the "image dependant warping"
	// technique.
	// 
	// manager will be initialized with default values for
	// the edge detector radius, mu and lambda, the three
	// parameters that influence the resulting image the most.
	// It is also initialized with a scale of 1.0 and the
	// other parameters also have default values.
	// 
	// Note that depending on the scale, you will find
	// different values for mu and lambda giving you
	// more satisfying results.
	WarpManager manager;

	// This next call will parse the command line
	// options and will set different parameters of
	// the mananger accordingly.
	initialize_sharp(argc, argv, &manager);

	// If the last function did not find a file name
	// for the input file, we display the help text and quit.
	if (!fileName) {
		print_help();
		exit(0);
	}

	// A MapImage holds the actual warping information,
	// that is the information how much and in which
	// direction a pixel in the input image will be
	// moved for the resulting (morphed) image.
	// By default, the WarpManager will compute
	// this map using an edge detection algorithm.
	// 
	// We can load a previously generated map image
	// from disk, which is a command line option.
	MapImage* map = NULL;
	if (useMap) {
		// load map from the user supplied file name
		map = new MapImage(mapFileName);
		if (map->IsValid())
			// We supply the WarpManager with the loaded map.
			// 
			// Unless you change parameters later,
			// the manager will use this map instead
			// of generating its own to save time.
			manager.SetSharpMap(map);
	}

	printf("loading bitmap....\n");
	// An RGB8Image is an image representation
	// with independant components for red green and blue
	// with 8 bit resolution each. Such an image is unfit
	// for drawing on screen like a BBitmap would be, but
	// it allows faster access to the individual components
	// and all the algorithms in this library use it.
	//
	// This form of the constructor will first load a BBitmap
	// using the BTranslationUtils with the supplied file name,
	// and then copy the BBitmap contents into its own storage
	// format, finally deleting the temporary BBitmap.
	RGB8Image* input = new RGB8Image(fileName);

	if (!input || !input->IsValid()) {
		fprintf(stderr, "Error reading bitmap from file \"%s\"\n", fileName);
		delete input;
		delete map;
		exit(0);
	}

	// We tell the WarpManager what the input image will be.
	// It will keep a pointer to this image. Obviously, you
	// should never delete this input image if you are going
	// to use functionality from the manager yet!
	manager.SetInputImage(input);

	// It is important to set the translator at least once
	// in your application or saving a bitmap will fail.
	set_translator(translatorName);

	// Uncomment one of the next blocks to get some
	// comparision results of the influence the
	// different parameters have on the final image
	// you can also get performance information. Whenever
	// you change a parameter of the WarpManager, the next
	// time you call GetWarpedImage(), some or all of
	// the internal data has to be recomputed. The manager
	// tries to reuse existing (cached) data whenever
	// possible.

/*
	float finalScale = manager.Scale();
	for (uint32 i = 0; i < 5; i++) {
		BString name(outFileName);
		name << i;
		float scale = 1.0 + ((finalScale - 1.0) / 5) * i;
		manager.SetScale(scale);
		printf("warping....\n");
bigtime_t now = system_time();
		RGB8Image* res = manager.GetWarpedImage();
printf("time (scale = %f): %Ld\n", scale, system_time() - now);
		save_result(res, name.String());
		delete res;
	}
*/
/*
	float finalMu = manager.Mu();
	float baseMu = finalMu / 4.0;
	for (uint32 i = 0; i < 5; i++) {
		BString name(outFileName);
		name << i;
		float mu = baseMu + ((finalMu - baseMu) / 5) * i;
		manager.SetMu(mu);
		printf("warping....\n");
bigtime_t now = system_time();
		RGB8Image* res = manager.GetWarpedImage();
printf("time (mu = %f): %Ld\n", mu, system_time() - now);
		save_result(res, name.String());
		delete res;
	}
*/
/*
	float finalLambda = manager.Lambda();
	float baseLambda = finalLambda / 4.0;
	for (uint32 i = 0; i < 5; i++) {
		BString name(outFileName);
		name << i;
		float lambda = baseLambda + ((finalLambda - baseLambda) / 5) * i;
		manager.SetLambda(lambda);
		printf("warping....\n");
bigtime_t now = system_time();
		RGB8Image* res = manager.GetWarpedImage();
printf("time (lambda = %f): %Ld\n", lambda, system_time() - now);
		save_result(res, name.String());
		delete res;
	}
*/

	// We are now ready to warp the intput image.
	printf("warping....\n");
	// The image returned by GetWarpedImage() is
	// ours and we are responsible for deleting it.
	RGB8Image* result = manager.GetWarpedImage();
	// We don't need the input image anymore.
	//
	// Be careful not to call GetWarpedImage() from
	// here on! To be on the save side, you could
	// call WarpManager::UnsetInputImage().
	delete input;
	// The result can be saved...
	save_result(result, outFileName);
	// ...and deleted.
	delete result;

	// If the user specified to save the map image
	// we can ask the WarpManager for the MapImage
	// that holds the actual warping information.
	// This map will only exist, if we called
	// GetWarpedImage() or SetWarpMap() before,
	// that's why we check the returned pointer!
	if (saveMap)
		if (MapImage* usedMap = manager.SharpMap())
			usedMap->SaveAs(mapFileName, true);

	// In case we supplied the WarpManager with a
	// MapImage from disk using SetSharpMap(),
	// it will not assume ownership of the MapImage
	// so we need to free it ourself! Notice
	// that we don't use this pointer to save the
	// MapImage above, so we don't need to keep track
	// of wether we actually loaded it from disk or
	// not...
	delete map;

	return 0;
}





