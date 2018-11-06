// FlattenableProtocol.h

#ifndef FLATTENABLE_PROTOCOL_H
#define FLATTENABLE_PROTOCOL_H

// an entry consists of the tag (4 bytes), the tag size (4 bytes) and the data

// Tags
enum {
	TAG_CLASS		= 'clss',
	TAG_DATA		= 'data',	// untyped data
	TAG_BITMAP		= 'btmp',	// 4 bytes Width, 4 bytes Height,
								// 4 bytes ColorSpace, 4 bytes BytesPerRow
								// N bytes Data (N = tagSize - space for above (16))
};

#endif // FLATTENABLE_PROTOCOL_H
