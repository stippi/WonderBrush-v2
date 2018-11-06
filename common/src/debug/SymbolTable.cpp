// SymbolTable.cpp

#include <new>
#include <stdio.h>
#include <string.h>

#include "AVLTree.h"
#include "SymbolTable.h"

// Entry
class SymbolTable::Entry {
public:
								~Entry();

	static	Entry*				Create(void* address, const char* symbol,
									   bool clone = true);

			void*				GetAddress() const;
			const char*			GetSymbol() const;

private:
								Entry(void* address, const char* symbol);

private:
			void*				fAddress;
			const char*			fSymbol;

public:
	// AVLTree node interface
	Entry		*parent;
	Entry		*left;
	Entry		*right;
	int			balance_factor;
};

// constructor
SymbolTable::Entry::Entry(void* address, const char* symbol)
	: fAddress(address),
	  fSymbol(symbol)
{
}

// destructor
SymbolTable::Entry::~Entry()
{
	if (fSymbol)
		free((void*)fSymbol);
}

// Create
SymbolTable::Entry*
SymbolTable::Entry::Create(void* address, const char* _symbol, bool clone)
{
	// clone the symbol name, if requested
	if (!_symbol)
		return NULL;
	const char* symbol = (clone ? strdup(_symbol) : _symbol);
	if (!symbol)
		return NULL;
	// create the entry
	Entry* entry = new(nothrow) Entry(address, symbol);
	if (!entry && clone)
		free((void*)symbol);
	return entry;
}

// GetAddress
void*
SymbolTable::Entry::GetAddress() const
{
	return fAddress;
}

// GetSymbol
const char*
SymbolTable::Entry::GetSymbol() const
{
	return fSymbol;
}


// GetEntryKey
class SymbolTable::GetEntryKey {
public:
	inline void *operator()(const Entry* a) const
	{
		return a->GetAddress();
	}

	inline void *operator()(Entry* a) const
	{
		return a->GetAddress();
	}
};

// EntryNodeAllocator
class SymbolTable::EntryNodeAllocator {
public:
	inline Entry* Allocate(Entry* a) const
	{
		a->parent = NULL;
		a->left = NULL;
		a->right = NULL;
		a->balance_factor = 0;
		return a;
	}

	inline void Free(Entry* node) const
	{
	}
};

// EntryGetValue
class SymbolTable::EntryGetValue {
public:
	inline Entry*& operator()(Entry* node) const
	{
		return node;
	}
};

// EntryTree
class SymbolTable::EntryTree
	: public AVLTree<Entry*, void*, Entry, AVLTreeStandardCompare<void*>,
					 GetEntryKey, EntryNodeAllocator, EntryGetValue > {
};


// SymbolTable

// constructor
SymbolTable::SymbolTable()
	: fEntries(NULL),
	  fImages()
{
	fEntries = new(nothrow) EntryTree;
}

// destructor
SymbolTable::~SymbolTable()
{
	delete fEntries;
}

// InitCheck
status_t
SymbolTable::InitCheck() const
{
	return (fEntries ? B_OK : B_BAD_VALUE);
}

// AddAllImages
status_t
SymbolTable::AddAllImages()
{
//printf("SymbolTable::AddAllImages()...\n");
	image_info info;
	int32 cookie = 0;
	while (get_next_image_info(0, &cookie, &info) == B_OK) {
		status_t error = _AddImage(info);
		if (error != B_OK)
{
//printf("  failed: %s\n", strerror(error));
			return error;
}
	}
	return B_OK;
}

// AddImage
status_t
SymbolTable::AddImage(image_id id)
{
	image_info info;
	status_t error = get_image_info(id, &info);
	if (error == B_OK)
		error = _AddImage(info);
	return error;
}

// RemoveImage
status_t
SymbolTable::RemoveImage(image_id id)
{
	image_info* info = _FindImage(id);
	return _RemoveImage(info);
}

// FindSymbol
const char*
SymbolTable::FindSymbol(void* address, uint32* offset)
{
	if (Entry** entry = fEntries->FindClose(address, true)) {
		if (offset)
			*offset = (uint8*)address - (uint8*)(*entry)->GetAddress();
		return (*entry)->GetSymbol();
	}
	return NULL;
}

// _AddImage
status_t
SymbolTable::_AddImage(const image_info& _info)
{
//printf("SymbolTable::AddImage(%ld)...\n", _info.id);
	// check whether the image is already known
	if (image_info* oldInfo = _FindImage(_info)) {
		// It is: to be safe, we check the text and data addresses and
		// remove the old image, if they differ.
		if (oldInfo->text == _info.text && oldInfo->data == _info.data)
			return B_OK;
		_RemoveImage(oldInfo);
	}
	// clone and add the image info
	image_info* info = new(nothrow) image_info(_info);
	if (!info)
		return B_NO_MEMORY;
	if (!fImages.AddItem(info)) {
		delete info;
		return B_NO_MEMORY;
	}
	// iterate through the image symbols and add them to the tree
	image_id image = info->id;
	char name[1024];
	int32 symbolClass;
	void* symbolAddress = NULL;
	for (int32 i = 0, nameLen = sizeof(name);
		 get_nth_image_symbol(image, i, name, &nameLen, &symbolClass,
							  &symbolAddress) == B_OK;
		 i++, nameLen = sizeof(name)) {
		if (nameLen > (int32)sizeof(name)) {
			// The provided name buffer was too small: Allocate a sufficiently
			// large buffer and re-get the symbol.
			// allocate name buffer
			char* clonedName = (char*)malloc(nameLen);
			if (!clonedName)
				return B_NO_MEMORY;
			// get the symbol
			if (get_nth_image_symbol(image, i, clonedName, &nameLen,
									 &symbolClass, &symbolAddress) == B_OK) {
				// create and add the entry
				Entry* entry = Entry::Create(symbolAddress, name, false);
				if (!entry) {
					free(clonedName);
					return B_NO_MEMORY;
				}
				fEntries->Insert(entry);
			} else
				free(clonedName);
		} else {
			// name buffer was large enough: create and add the entry
			Entry* entry = Entry::Create(symbolAddress, name);
			if (!entry)
				return B_NO_MEMORY;
			fEntries->Insert(entry);
		}
	}
	return B_OK;
}

// _RemoveImage
status_t
SymbolTable::_RemoveImage(image_info* info)
{
	if (!info)
		return B_BAD_IMAGE_ID;
	void* textStart = info->text;
	void* textEnd = (uint8*)textStart + info->text_size;
	void* dataStart = info->data;
	void* dataEnd = (uint8*)dataStart + info->data_size;
	fImages.RemoveItem(info);
	delete info;
	// remove the text symbols
	EntryTree::Iterator it;
	for (fEntries->FindClose(textStart, false, &it); it.GetCurrent(); ) {
		Entry* entry = *it.GetCurrent();
		if (entry->GetAddress() >= textEnd)
			break;
		it.Remove();
		delete entry;
	}
	// remove the data symbols
	for (fEntries->FindClose(dataStart, false, &it); it.GetCurrent(); ) {
		Entry* entry = *it.GetCurrent();
		if (entry->GetAddress() >= dataEnd)
			break;
		it.Remove();
		delete entry;
	}
	return B_OK;
}

// _FindImage
image_info*
SymbolTable::_FindImage(image_id id, bool remove)
{
	image_info info;
	if (get_image_info(id, &info) == B_OK)
		return _FindImage(info);
	return NULL;
}

// _FindImage
image_info*
SymbolTable::_FindImage(const image_info& info, bool remove)
{
	return _FindImage(info.device, info.node);
}

// _FindImage
image_info*
SymbolTable::_FindImage(dev_t device, ino_t node, bool remove)
{
	for (int i = 0; image_info* info = (image_info*)fImages.ItemAt(i); i++) {
		if (info->device == device && info->node == node) {
			if (remove)
				fImages.RemoveItem(i);
			return info;
		}
	}
	return NULL;
}

