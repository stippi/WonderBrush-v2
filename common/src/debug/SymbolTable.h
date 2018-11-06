// SymbolTable.cpp

#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <image.h>
#include <List.h>

class SymbolTable {
public:
								SymbolTable();
								~SymbolTable();

			status_t			InitCheck() const;

			status_t			AddAllImages();
			status_t			AddImage(image_id id);
			status_t			RemoveImage(image_id id);

			const char*			FindSymbol(void* address, uint32* offset);

private:
			class Entry;
			class GetEntryKey;
			class EntryNodeAllocator;
			class EntryGetValue;
			class EntryTree;

private:
			status_t			_AddImage(const image_info& info);
			status_t			_RemoveImage(image_info* info);
			image_info*			_FindImage(image_id id, bool remove = false);
			image_info*			_FindImage(const image_info& info,
										   bool remove = false);
			image_info*			_FindImage(dev_t device, ino_t node,
										   bool remove = false);

private:
			EntryTree*			fEntries;
			BList				fImages;
};

#endif SYMBOL_TABLE_H
