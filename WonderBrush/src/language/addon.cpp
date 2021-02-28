// addon.cpp

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <Application.h>
#include <Entry.h>
#include <File.h>
#include <Path.h>
#include <List.h>
#include <Directory.h>
#include <Roster.h>

#include "Strings.h"

#include "addon.h"

#if defined(__x86_64__)
#define COMPILER_NAME			"gcc"
#define COMPILER_ADDON_OPTION	"-shared"
#define COMPILER_OPTIMIZATION	"-O1"
#define COMPILER_EXTRA_CRAP		""
#elif defined(__INTEL__)
#define COMPILER_NAME			"gcc"
#define COMPILER_ADDON_OPTION	"-nostart"
#define COMPILER_OPTIMIZATION	"-O1"
#define COMPILER_EXTRA_CRAP		""
#elif __POWERPC__
#define COMPILER_NAME			"mwcc"
#define COMPILER_ADDON_OPTION	"-export init_strings -xms"
#define COMPILER_OPTIMIZATION	"-O1"
#define COMPILER_EXTRA_CRAP	"/boot/develop/lib/ppc/glue-noinit.a " \
							"/boot/develop/lib/ppc/init_term_dyn.o " \
							"/boot/develop/lib/ppc/start_dyn.o " \
							"-lroot"
#else
#error Running on unsupported processors is unsupported.
#endif

static int
find_template_file(char *buffer)
{
	app_info ai;
	BPath p;

	be_app->GetAppInfo(&ai);
	BEntry e(&ai.ref);
	e.GetPath(&p);
	p.GetParent(&p);
	p.Append("template");

	strcpy(buffer, p.Path());

	return 1;
}

static int32 addons_made = 0;

// build_addon
language_addon*
build_addon(const char* path)
{
	if (addons_made++ % CLEANING_FREQUENCY == 1)
		clean_temporary_files();

	FILE* tfp = fopen(path, "r");
	if (!tfp)
		return NULL;

	// open a temporary source file, define the string indices,
	// and tack the language file onto the end of it.
	char srcfile[64];
	char outfile[64];

	strcpy(outfile, "/tmp/WonderBrush_languageXXXXXX");
	mktemp(outfile);
	strcpy(srcfile, outfile);
	strcat(srcfile, ".cpp");

	FILE* fp = fopen(srcfile, "w");
	if (!fp) {
		fclose(tfp);
		return NULL;
	}

	print_header(fp);

	char buffer[1024];
	while (fgets(buffer, sizeof(buffer), tfp))
		fputs(buffer, fp);

	fclose(fp);
	fclose(tfp);

	// invoke the compiler
	sprintf(buffer, "%s %s %s -o %s %s %s",
			COMPILER_NAME, COMPILER_OPTIMIZATION, COMPILER_ADDON_OPTION,
			outfile, srcfile, COMPILER_EXTRA_CRAP);

	if (system(buffer) != 0) {
		printf("couldn't execute compiler\n");
		return NULL;
	}

	// load the add-on it produced, find the symbol, populate the
	// language_addon, and give it back.
	language_addon* a = (language_addon *)malloc(sizeof(language_addon));
	if (!a) {
		printf("malloc failed!\n");
		return NULL;
	}

	a->image = load_add_on(outfile);
	if (a->image <= 0) {
		printf("couldn't load addon\n");
		free(a);
		return NULL;
	}

	if (get_image_symbol(a->image, "init_strings", B_SYMBOL_TYPE_TEXT,
						(void **)&(a->function)) != B_OK) {
		printf("couldn't find init_strings() in addon\n");
		unload_add_on(a->image);
		free(a);
		return NULL;
	}

	strcpy(a->path, outfile);
	strcpy(a->name, DEFAULT_NAME);

	return a;
}

// destroy_addon
void
destroy_addon(language_addon *addon)
{
	// sanity check
	if (addon == NULL)
		return;

	unload_add_on(addon->image);
	free(addon);
}

// load_addon
language_addon*
load_addon(const char* path)
{
	language_addon* a = (language_addon *)malloc(sizeof(language_addon));
	if (!a)
		return NULL;

	strcpy(a->path, path);

	char* cp = strrchr(a->path, '/');
	if (cp == NULL)
		strcpy(a->name, DEFAULT_NAME);
	else
		strcpy(a->name, cp+1);

	if ((a->image = load_add_on(a->path)) <= 0)
		goto bail;

	if (get_image_symbol(a->image, "init_strings", B_SYMBOL_TYPE_TEXT,
						(void **)&(a->function)) != B_OK)
		goto bail2;

	return a;

bail2:
	unload_add_on(a->image);
bail:
	free(a);
	return NULL;
}

// save_addon
status_t
save_addon(language_addon *addon, const char *directory)
{
	BDirectory oldDir, newDir;
	BEntry entry;
	char file[B_FILE_NAME_LENGTH];
	status_t err;

	BPath path(addon->path);
	strcpy(file, path.Leaf());
	path.GetParent(&path);

	if ((err = oldDir.SetTo(path.Path())) < B_OK)
		return err;

	if ((err = newDir.SetTo(directory)) < B_OK)
		return err;

	if ((err = oldDir.FindEntry(file, &entry)) < B_OK)
		return err;

	if ((err = entry.MoveTo(&newDir)) < B_OK) {
		if (err == B_CROSS_DEVICE_LINK) {
			// we compiled the addon in the /tmp folder
			// wich is on another volume (the boot volume)
			// and WonderBrush is running on some other
			// volume, so we just copy the file instead
			BFile src(&entry, B_READ_ONLY);
			BFile dst(&newDir, file, B_CREATE_FILE | B_ERASE_FILE | B_WRITE_ONLY);
			ssize_t size = 1024;
			char buffer[size];
			ssize_t bytesRead = src.Read(buffer, size);
			if (bytesRead < B_OK)
				err = bytesRead;
			while (bytesRead > 0) {
				err = dst.Write(buffer, bytesRead);
				if (err != bytesRead) {
					// just to make sure we don't return success
					if (err >= B_OK)
						err = B_ERROR;
					break;
				}
				bytesRead = src.Read(buffer, size);
				if (bytesRead < B_OK) {
					err = bytesRead;
					break;
				}
			}
			if (err >= B_OK) {
				// now let entry point to the correct file
				err = entry.SetTo(&newDir, file);
			}
		} else
			return err;
	}

	err = entry.Rename(addon->name, true);

	newDir.GetEntry(&entry);
	entry.GetPath(&path);

	sprintf(addon->path, "%s/%s", path.Path(), addon->name);

	return err;
}

// clean_temporary_files
void
clean_temporary_files()
{
	BDirectory dir("/tmp");
	BEntry entry;
	char name[B_FILE_NAME_LENGTH];

	while (dir.GetNextEntry(&entry) == B_OK) {
		entry.GetName(name);
		if (strncmp(name, "WonderBrush_language",
					strlen("WonderBrush_language")) == 0)
			entry.Remove();
	}
}
