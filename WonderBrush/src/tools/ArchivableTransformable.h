// ArchivableTransformable.h

#ifndef ARCHIVABLE_TRANSFORMABLE_H
#define ARCHIVABLE_TRANSFORMABLE_H

#include <Archivable.h>

#include "Transformable.h"

class ArchivableTransformable : public BArchivable,
								public Transformable {
 public:
								ArchivableTransformable();
								ArchivableTransformable(const BMessage* archive);
	virtual						~ArchivableTransformable();

								// the BArchivable protocol
								// stores matrix directly to message, deep is ignored
	virtual	status_t			Archive(BMessage* into, bool deep = true) const;
};

#endif // ARCHIVABLE_TRANSFORMABLE_H

