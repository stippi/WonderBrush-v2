// AllocationChecker.cpp

#include <stdio.h>
#include <stdlib.h>

#include <Autolock.h>

#include "AllocationChecker.h"
#include "AVLTree.h"
#include "ElfSymbolPatcher.h"

// amount of memory used for storing allocation infos
static const size_t kAllocationInfoSpace = 10 * 1024 * 1024;

// number of return addresses from the malloc()/new caller stack to be stored
static const int kAllocCallerInfoSize = 4;

// stackframe
struct stack_frame {
	stack_frame*	previous;
	void*			return_address;
};

// get_parent_stack_frame
stack_frame*
get_parent_stack_frame(int level = 1)
{
	stack_frame* stackFrame = (stack_frame*)get_stack_frame();
	if (stackFrame)
		stackFrame = stackFrame->previous;
	for (; stackFrame && level > 0; level--)
		stackFrame = stackFrame->previous;
	return stackFrame;
}

// dump_stack
void
dump_stack()
{
	stack_frame* stackFrame = get_parent_stack_frame();
	for (int i = 0; i < 10 && stackFrame; i++) {
		if (i == 0)
			printf("caller stack: ");
		else
			printf("              ");
		SymbolTable* symbols
			= AllocationChecker::GetDefault()->GetSymbolTable();
		uint32 offset;
		const char* symbol = symbols->FindSymbol(stackFrame->return_address,
												 &offset);
		if (symbol)
			printf("  %s + 0x%lx\n", symbol, offset);
		else
			printf("  %p (symbol not found)\n", stackFrame->return_address);

		stackFrame = stackFrame->previous;
	}
}


// AllocationChecker

// Info
class AllocationChecker::Info {
public:
	Info() : address(NULL), size(0), array(false), cpp(false), used(false),
			 track(false) {}
	Info(void *address, size_t size, bool array, bool cpp)
		: address(address), size(size), array(array), cpp(cpp), used(true),
		  track(false) {}

	void Init(void *address, size_t size, bool array, bool cpp,
			  void* _stackFrame)
	{
		stack_frame* stackFrame = (stack_frame*)_stackFrame;
		allocation_time = system_time();
		this->address = address;
		this->size = size;
		this->array = array;
		this->cpp = cpp;
		memset(caller, 0, sizeof(caller));
		for (int i = 0; i < kAllocCallerInfoSize && stackFrame; i++) {
			caller[i] = stackFrame->return_address;
			stackFrame = stackFrame->previous;
		}
		used = true;
		track = false;
	}

	void Free(Info *next)
	{
		this->next = next;
		size = 0;
		array = false;
		cpp = false;
		memset(caller, 0, sizeof(caller));
		used = false;
		track = false;
	}

	void Dump(bool detailed = false)
	{
		printf("address: %10p, size: %9lu, array: %d, cpp: %d, time: %lld\n",
			   address, size, array, cpp, allocation_time);
		if (detailed) {
			for (int i = 0; i < kAllocCallerInfoSize && caller[i]; i++) {
				if (i == 0)
					printf("  allocated from: ");
				else
					printf("                  ");
				SymbolTable* symbols
					= AllocationChecker::GetDefault()->GetSymbolTable();
				uint32 offset;
				const char* symbol = symbols->FindSymbol(caller[i], &offset);
				if (symbol)
					printf("%s + 0x%lx\n", symbol, offset);
				else
					printf("%p (symbol not found)\n", caller[i]);
			}
		}
	}

	// member variables
	bigtime_t	allocation_time;
	union {
		void*	address;
		Info*	next;
	};
	size_t		size;
	void*		caller[kAllocCallerInfoSize];
	bool		array;
	bool		cpp;
	bool		used;
	bool		track;
	// AVLTree node interface
	Info		*parent;
	Info		*left;
	Info		*right;
	int			balance_factor;
};

// GetInfoKey
class AllocationChecker::GetInfoKey {
public:
	inline void *operator()(const Info *a) const
	{
		return (void*)a->address;
	}

	inline void *operator()(Info *a) const
	{
		return (void*)a->address;
	}
};

// InfoNodeAllocator
class AllocationChecker::InfoNodeAllocator {
public:
	inline Info *Allocate(Info *a) const
	{
		a->parent = NULL;
		a->left = NULL;
		a->right = NULL;
		a->balance_factor = 0;
		return a;
	}

	inline void Free(Info *node) const
	{
	}
};

// InfoGetValue
class AllocationChecker::InfoGetValue {
public:
	inline Info *&operator()(Info *node) const
	{
		return node;
	}
};

// InfoTree
class AllocationChecker::InfoTree
	: public AVLTree<Info*, void*, Info, AVLTreeStandardCompare<void*>,
					 GetInfoKey, InfoNodeAllocator, InfoGetValue > {
};

// CreateDefault
AllocationChecker*
AllocationChecker::CreateDefault()
{
	if (!fDefaultChecker)
		fDefaultChecker = new(nothrow) AllocationChecker;
	return fDefaultChecker;
}

// DeleteDefault
void
AllocationChecker::DeleteDefault()
{
	if (fDefaultChecker) {
		delete fDefaultChecker;
		fDefaultChecker = NULL;
	}
}

// GetDefault
AllocationChecker*
AllocationChecker::GetDefault()
{
	return fDefaultChecker;
}

// New
void*
AllocationChecker::New(size_t size, bool array, bool cpp, bool clear)
{
	if (!this)
		return malloc(size);
	BAutolock locker(fLock);
	if (!locker.IsLocked())
		return malloc(size);
	_UpdateCurrentStackFrame();
	if (size == 0)
		printf("WARNING: Allocation of 0 bytes.\n");
	void *address = (*fOldMallocHook)(size);
	if (address) {
		_Allocated(address, size, array, cpp);
		if (clear)
			memset(address, 0, size);
	}
	return address;
}

// Delete
void
AllocationChecker::Delete(void *block, bool array, bool cpp)
{
	if (!this)
		return free(block);
	BAutolock locker(fLock);
	if (!locker.IsLocked())
		return free(block);
	_UpdateCurrentStackFrame();
	if (!block)
		return;
	_Deleted(block, array, cpp);
	(*fOldFreeHook)(block);
}

// Realloc
void*
AllocationChecker::Realloc(void *block, size_t size)
{
	if (!this)
		return realloc(block, size);
	BAutolock locker(fLock);
	if (!locker.IsLocked())
		return realloc(block, size);
	_UpdateCurrentStackFrame();
	if (size == 0)
		printf("WARNING: Re-allocation of 0 bytes.\n");
	void* result = (*fOldReallocHook)(block, size);
	if (block)
		_Deleted(block, false, false);
	if (result)
		_Allocated(result, size, false, false);
	return result;
}

// ImplicitAllocation
void
AllocationChecker::ImplicitAllocation(void *address, size_t size, bool array,
									  bool cpp)
{
	if (!this)
		return;
	BAutolock locker(fLock);
	if (!locker.IsLocked())
		return;
	_UpdateCurrentStackFrame();
	if (!address) {
		printf("WARNING: Ignoring implicit NULL allocation.\n");
		return;
	}
	if (size == 0) {
		printf("WARNING: Ignoring allocation of 0 bytes.\n");
		return;
	}
	_Allocated(address, size, array, cpp);
}

// TrackAllocation
bool
AllocationChecker::TrackAllocation(void* address, bool track)
{
	if (!this)
		return false;
	BAutolock locker(fLock);
	if (!locker.IsLocked())
		return false;
	_UpdateCurrentStackFrame();
	Info *info = _FindInfo(address);
	if (info)
		info->track = track;
	return info;
}

// Dump
void
AllocationChecker::Dump(bool detailed, bigtime_t after)
{
	if (!this)
		return;
	BAutolock locker(fLock);
	if (!locker.IsLocked())
		return;
	printf("AllocationChecker: allocations after %lld\n", after);
	int32 count = 0;
	for (InfoTree::Iterator it(fUsedInfos); it.GetCurrent(); it.GetNext()) {
		Info *info = *it.GetCurrent();
		if (info->allocation_time >= after) {
			count++;
			info->Dump(detailed);
		}
	}
	printf("%ld allocations\n", count);
}

// constructor
AllocationChecker::AllocationChecker()
	: fLock(),
	  fAppImage(-1),
	  fAppTextArea(-1),
	  fPatchGroup(NULL),
	  fCurrentStackFrame(NULL),
	  fArea(-1),
	  fUsedInfos(NULL),
	  fInfos(NULL),
	  fInfoCount(0),
	  fNextUnusedIndex(0),
	  fNextFreeInfo(NULL),
	  fDontLog(false),
	  fOldCallocHook(NULL),
	  fOldFreeHook(NULL),
	  fOldMallocHook(NULL),
	  fOldReallocHook(NULL),
	  fOldNewHook(NULL),
	  fOldNewNothrowHook(NULL),
	  fOldNewVecHook(NULL),
	  fOldNewVecNothrowHook(NULL),
	  fOldDeleteHook(NULL),
	  fOldDeleteNothrowHook(NULL),
	  fOldDeleteVecHook(NULL),
	  fOldDeleteVecNothrowHook(NULL),
	  fOldStrdupHook(NULL),
	  fOldLoadAddOnHook(NULL),
	  fOldUnloadAddOnHook(NULL)
{
	void *address = NULL;
	fArea = create_area("AllocationChecker", &address, B_ANY_ADDRESS,
						kAllocationInfoSpace, B_NO_LOCK,
						B_WRITE_AREA | B_READ_AREA);
	if (fArea >= 0) {
		fUsedInfos = new(address) InfoTree;
		address = (uint8*)address + sizeof(InfoTree);
		size_t size = kAllocationInfoSpace - sizeof(InfoTree);
		fInfoCount = size / sizeof(Info);
		fInfos = new(address) Info[fInfoCount];
	}
	// find the app image
	image_info info;
	int32 cookie = 0;
	while (get_next_image_info(0, &cookie, &info) == B_OK) {
		if (info.type == B_APP_IMAGE) {
			fAppImage = info.id;
			fAppTextArea = area_for(info.text);
			break;
		}
	}
	// init symbol table
	fSymbols.AddAllImages();
	// override the allocation hooks
	_InstallMallocHooks();
}

// destructor
AllocationChecker::~AllocationChecker()
{
	fLock.Lock();
	_RestoreMallocHooks();
	if (fArea >= 0)
		delete_area(fArea);
}

// _Allocated
void
AllocationChecker::_Allocated(void *address, size_t size, bool array, bool cpp)
{
	if (fDontLog)
		return;
	if (address) {
		if (Info *info = _AllocateInfo()) {
			info->Init(address, size, array, cpp, fCurrentStackFrame);
			// check for intersections with older allocations
			InfoTree::Iterator it;
			if (fUsedInfos->FindClose(address, false, &it)) {
				while (Info **nextInfoP = it.GetCurrent()) {
					Info *nextInfo = *nextInfoP;
					if ((uint32)nextInfo->address  < (uint32)address + size) {
						printf("WARNING: New allocation (%p, %lu) intersects "
							   "with older allocation (%p, %lu). Assuming "
							   "implicit deletion.\n", address, size,
							   nextInfo->address, nextInfo->size);
						it.GetNext();
						_Deleted(nextInfo->address, nextInfo->array,
								 nextInfo->cpp);
					} else
						break;
				}
			}
			// insert the info
			if (fUsedInfos->Insert(info) != B_OK) {
				printf("WARNING: Failed to insert allocation info!\n");
				_Debugger("Failed to insert allocation info!\n");
			}
		} else
			printf("WARNING: Ran out of allocation infos!\n");
	}
}

// _Deleted
void
AllocationChecker::_Deleted(void *block, bool array, bool cpp)
{
	if (fDontLog)
		return;
	if (Info *info = _FindInfo(block)) {
		if (info->track) {
			printf("deleting: ");
			_Debugger("Tracking memory.", info);
		}
		// check C/C++ allocation
		if (info->cpp != cpp) {
			const char *texts[] = { "C", "C++" };
			const char *text1 = (info->cpp ? texts[0] : texts[1]);
			const char *text2 = (cpp ? texts[0] : texts[1]);
			printf("WARNING: Deleting %s allocation as %s allocation (%p)\n",
				   text1, text2, block);
			_Debugger("Wrong C/C++ deletion.", info);
		}
		// check array/non-array allocation
		if (info->array != array) {
			const char *texts[] = { "an array", "a non-array" };
			const char *text1 = (info->array ? texts[0] : texts[1]);
			const char *text2 = (array ? texts[0] : texts[1]);
			printf("WARNING: Deleting %s like %s (%p)\n", text1, text2, block);
			_Debugger("Wrong array/non-array deletion.", info);
		}
		fUsedInfos->Remove(block);
		_FreeInfo(info);
	} else {
		printf("WARNING: Couldn't find allocation info for %p!\n", block);
		_Debugger("Couldn't find allocation info.");
	}
}

// _AllocateInfo
AllocationChecker::Info*
AllocationChecker::_AllocateInfo()
{
	Info *info = NULL;
	if (fNextFreeInfo) {
		info = fNextFreeInfo;
		fNextFreeInfo = info->next;
	} else if (fNextUnusedIndex < fInfoCount) {
		info = fInfos + fNextUnusedIndex;
		fNextUnusedIndex++;
	}
	return info;
}

// _FreeInfo
void
AllocationChecker::_FreeInfo(Info* info)
{
	if (info) {
		info->Free(fNextFreeInfo);
		fNextFreeInfo = info;
	}
}

// _FindInfo
AllocationChecker::Info*
AllocationChecker::_FindInfo(void* block)
{
	Info** info = fUsedInfos->Find(block);
	return (info ? *info : NULL);
}

// _UpdateCurrentStackFrame
void
AllocationChecker::_UpdateCurrentStackFrame()
{
	fCurrentStackFrame = get_parent_stack_frame(2);
}

// _Debugger
void
AllocationChecker::_Debugger(const char* message, Info* info)
{
	if (info)
		info->Dump(true);
	dump_stack();
	if (info && info->caller[0] && area_for(info->caller[0]) == fAppTextArea
		|| !info && fCurrentStackFrame && fCurrentStackFrame->return_address
		   && area_for(fCurrentStackFrame->return_address) == fAppTextArea) {
		debugger(message);
	}
}

// _CallocHook
void*
AllocationChecker::_CallocHook(size_t nmemb, size_t size)
{
	return GetDefault()->New(nmemb * size, false, false, true);
}

// _FreeHook
void
AllocationChecker::_FreeHook(void* ptr)
{
	GetDefault()->Delete(ptr, false, false);
}

// _MallocHook
void*
AllocationChecker::_MallocHook(size_t size)
{
	return GetDefault()->New(size, false, false);
}

// _ReallocHook
void*
AllocationChecker::_ReallocHook(void* ptr, size_t size)
{
	return GetDefault()->Realloc(ptr, size);
}

// _NewHook
void*
AllocationChecker::_NewHook(size_t size)
{
	return GetDefault()->New(size, false, true);
}

// _NewNothrowHook
void*
AllocationChecker::_NewNothrowHook(size_t size, const nothrow_t&)
{
	return GetDefault()->New(size, false, true);
}

// _NewVecHook
void*
AllocationChecker::_NewVecHook(size_t size)
{
	return GetDefault()->New(size, true, true);
}

// _NewVecNothrowHook
void*
AllocationChecker::_NewVecNothrowHook(size_t size, const nothrow_t&)
{
	return GetDefault()->New(size, true, true);
}

// _DeleteHook
void
AllocationChecker::_DeleteHook(void *ptr)
{
	GetDefault()->Delete(ptr, false, true);
}

// _DeleteNothrowHook
void
AllocationChecker::_DeleteNothrowHook(void *ptr, const nothrow_t&)
{
	GetDefault()->Delete(ptr, false, true);
}

// _DeleteVecHook
void
AllocationChecker::_DeleteVecHook(void *ptr)
{
	GetDefault()->Delete(ptr, true, true);
}

// _DeleteVecNothrowHook
void
AllocationChecker::_DeleteVecNothrowHook(void *ptr, const nothrow_t&)
{
	GetDefault()->Delete(ptr, true, true);
}

// _StrdupHook
char*
AllocationChecker::_StrdupHook(const char* str)
{
	char* result = (*GetDefault()->fOldStrdupHook)(str);
	if (result) {
		GetDefault()->ImplicitAllocation(result, strlen(result) + 1, false,
										 false);
	}
	return result;
}

// _LoadAddOnHook
image_id
AllocationChecker::_LoadAddOnHook(const char* path)
{
	return GetDefault()->_LoadAddOn(path);
}

// _UnloadAddOnHook
status_t
AllocationChecker::_UnloadAddOnHook(image_id image)
{
	return GetDefault()->_UnloadAddOn(image);
}

// _InstallMallocHooks
void
AllocationChecker::_InstallMallocHooks()
{
	// init the symbol patch group
	if (!fPatchGroup)
		fPatchGroup = new(nothrow) ElfSymbolPatchGroup;
	if (fPatchGroup) {
		fPatchGroup->RemoveAllPatches();
		if (// calloc()
			fPatchGroup->AddPatch("calloc", (void*)&_CallocHook,
								  (void**)&fOldCallocHook) == B_OK
			// free()
			&& fPatchGroup->AddPatch("free", (void*)&_FreeHook,
									 (void**)&fOldFreeHook) == B_OK
			// malloc()
			&& fPatchGroup->AddPatch("malloc", (void*)&_MallocHook,
									 (void**)&fOldMallocHook) == B_OK
			// realloc()
			&& fPatchGroup->AddPatch("realloc", (void*)&_ReallocHook,
									 (void**)&fOldReallocHook) == B_OK
			// new()
			&& fPatchGroup->AddPatch("__builtin_new", (void*)&_NewHook,
									 (void**)&fOldNewHook) == B_OK
			// new(const nothrow&)
			&& fPatchGroup->AddPatch("__nw__FUlRC9nothrow_t",
									 (void*)&_NewNothrowHook,
									 (void**)&fOldNewNothrowHook) == B_OK
			// new[]()
			&& fPatchGroup->AddPatch("__builtin_vec_new", (void*)&_NewVecHook,
									 (void**)&fOldNewVecHook) == B_OK
			// new[](const nothrow&)
			&& fPatchGroup->AddPatch("__vn__FUlRC9nothrow_t",
									 (void*)&_NewVecNothrowHook,
									(void**)&fOldNewVecNothrowHook) == B_OK
			// delete()
			&& fPatchGroup->AddPatch("__builtin_delete", (void*)&_DeleteHook,
									 (void**)&fOldDeleteHook) == B_OK
			// delete(const nothrow&)
			&& fPatchGroup->AddPatch("__dl__FPvRC9nothrow_t",
									 (void*)&_DeleteNothrowHook,
									 (void**)&fOldDeleteNothrowHook) == B_OK
			// delete[]()
			&& fPatchGroup->AddPatch("__builtin_vec_delete",
									 (void*)&_DeleteVecHook,
									 (void**)&fOldDeleteVecHook) == B_OK
			// delete[](const nothrow&)
			&& fPatchGroup->AddPatch("__vd__FPvRC9nothrow_t",
									 (void*)&_DeleteVecNothrowHook,
									 (void**)&fOldDeleteVecNothrowHook) == B_OK
			// strdup()
			&& fPatchGroup->AddPatch("strdup", (void*)&_StrdupHook,
									 (void**)&fOldStrdupHook) == B_OK
			// load_add_on()
			&& fPatchGroup->AddPatch("load_add_on", (void*)&_LoadAddOnHook,
									 (void**)&fOldLoadAddOnHook) == B_OK
			// unload_add_on()
			&& fPatchGroup->AddPatch("unload_add_on", (void*)&_UnloadAddOnHook,
									 (void**)&fOldUnloadAddOnHook) == B_OK
			) {
			// everything went fine
			fPatchGroup->Patch();
		} else {
			delete fPatchGroup;
			fPatchGroup = NULL;
		}
	}
}

// _RestoreMallocHooks
void
AllocationChecker::_RestoreMallocHooks()
{
printf("restoring malloc hooks...\n");
	if (fPatchGroup)
		fPatchGroup->Restore();
printf("restoring malloc hooks done\n");
}

// _LoadAddOn
image_id
AllocationChecker::_LoadAddOn(const char* path)
{
	if (!this)
		return load_add_on(path);
	BAutolock locker(fLock);
	if (!locker.IsLocked())
		return load_add_on(path);
	fDontLog = true;
	image_id result = (*fOldLoadAddOnHook)(path);
	if (fPatchGroup)
		fPatchGroup->Update();
	if (result >= 0)
		fSymbols.AddImage(result);
	fDontLog = false;
	return result;
}

// _UnloadAddOn
status_t
AllocationChecker::_UnloadAddOn(image_id image)
{
	if (!this)
		return unload_add_on(image);
	BAutolock locker(fLock);
	if (!locker.IsLocked())
		return unload_add_on(image);
	fDontLog = true;
	status_t result = (*fOldUnloadAddOnHook)(image);
	if (fPatchGroup)
		fPatchGroup->Update();
	fDontLog = false;
	return result;
}


// fDefaultChecker
AllocationChecker* AllocationChecker::fDefaultChecker = NULL;

