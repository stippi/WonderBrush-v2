// PSDFile.h

#ifndef PSD_FILE_H
#define PSD_FILE_H

#include <DataIO.h>

class BPositionIO;
class Canvas;

struct file_header;

class PSDFile {
 public:
								PSDFile();
	virtual						~PSDFile();

			status_t			Read(BPositionIO* stream,
									 Canvas** canvas);

			status_t			Write(BPositionIO* stream,
									  const Canvas* canvas);

 private:
			status_t			_WriteHeader(BPositionIO* stream,
											 file_header& header) const;
};

#endif // PSD_FILE_H
