// ShapeStroke.h

#ifndef SHAPE_STROKE_H
#define SHAPE_STROKE_H

#include <List.h>

#include "Stroke.h"

class VectorPath;

enum {
	FILL_MODE_NON_ZERO = 0,
	FILL_MODE_EVEN_ODD,
};

class ShapeStroke : public Stroke {
 public:
								ShapeStroke(rgb_color color);
								ShapeStroke(const ShapeStroke& other);
								ShapeStroke(BMessage* archive);
	virtual						~ShapeStroke();

	virtual	Stroke*				Clone() const;
	virtual	bool				SetTo(const Stroke* from);

								// BArchivable protocoll
	virtual	status_t			Archive(BMessage* into, bool deep = true) const;
	static	BArchivable*		Instantiate(BMessage* archive);

	virtual	bool				DrawLastLine(BBitmap* bitmap,
											 BRect& updateRect);
	virtual	void				Draw(BBitmap* bitmap);
	virtual	void				Draw(BBitmap* bitmap, BRect constrainRect);

	virtual	BRect				Bounds() const;

	virtual	const char*			Name() const;

	virtual	int32				ToolID() const;

	virtual	PropertyObject*		MakePropertyObject() const;
	virtual	bool				SetToPropertyObject(PropertyObject* object);

								// ShapeStroke
			void				SetPath(const VectorPath* from);
			bool				SetPath(const VectorPath* from, int32 index);
			bool				AddPath(VectorPath* path);
			bool				AddPath(VectorPath* path, int32 index);
			bool				RemovePath(VectorPath* path);
			VectorPath*			PathAt(int32 index) const;
			int32				IndexOf(VectorPath* path) const;
			int32				CountPaths() const;
			bool				HasPath(VectorPath* path) const;

			void				SetCurrentPath(VectorPath* path);
			VectorPath*			Path() const
									{ return fPath; }

			template<class VertexSource>
			void				AddVertexSource(VertexSource& source,
											    int32 pathID);

			void				FreezeTransformations();

			void				SetAlpha(uint8 alpha);
			uint8				Alpha() const
									{ return fAlpha; }

			void				SetOutline(bool outline);
			bool				IsOutline() const
									{ return fOutline; }

			void				SetOutlineWidth(float width);
			float				OutlineWidth() const
									{ return fOutlineWidth; }

			void				SetClosed(bool closed);
			bool				IsClosed() const;

			void				SetCapMode(uint32 mode);
			uint32				CapMode() const
									{ return fCapMode; }

			void				SetJoinMode(uint32 mode);
			uint32				JoinMode() const
									{ return fJoinMode; }

			void				SetFillingRule(uint32 rule);
			uint32				FillingRule() const
									{ return fFillMode; }

			void				SetSolid(bool solid);
			bool				Solid() const
									{ return fSolid; }

			void				UpdateBounds();
	virtual	void				TransformationChanged();

 protected:
			void				_MakeEmpty();

			template<class Rasterizer, class Renderer, class Scanline>
			void				_DrawPaths(Rasterizer& rasterizer,
										   Scanline& scanline,
										   Renderer& renderer,
										   bool outline) const;

			VectorPath*			fPath;
			BList				fSubPaths;

			uint8				fAlpha;
			bool				fOutline;
			float				fOutlineWidth;
			uint32				fCapMode;
			uint32				fJoinMode;
			bool				fSolid;
			uint32				fFillMode;
};

// AddVertexSource
template<class VertexSource>
void
ShapeStroke::AddVertexSource(VertexSource& source, int32 index)
{
//printf("ShapeStroke::AddVertexSource(pathID = %ld)\n", index);
	_MakeEmpty();

	// start with the first path
	fPath = new VectorPath();
	AddPath(fPath);

	VectorPath* path = fPath;

	source.rewind(index);
	double x1, y1;
	unsigned cmd = source.vertex(&x1, &y1);
	bool keepGoing = true;
	int32 subPath = 0;
	while (keepGoing) {
		if (agg::is_next_poly(cmd)) {
//printf("next polygon\n");
			if (agg::is_end_poly(cmd)) {
//printf("  end polygon\n");
				path->SetClosed(true);
				subPath++;
			} else {
//printf("  not end polygon\n");
			}

 			if (agg::is_stop(cmd)) {
//printf("  stop = true\n");
 				keepGoing = false;
			} else {
				if (subPath > 0) {
//printf("  new subpath\n");
					path->CleanUp();
					if (path->CountPoints() == 0) {
//printf("  path no points! (%ld)", fSubPaths.IndexOf((void*)path));
						RemovePath(path);
						delete path;
					}
					path = new VectorPath();
					AddPath(path);
				}
			}
		}
		switch (cmd) {
			case agg::path_cmd_move_to:
//printf("move to (%.2f, %.2f) (subPath: %ld)\n", x1, y1, subPath);
				if (path->CountPoints() > 0) {
					// cannot MoveTo on a path that has already points!
					path->CleanUp();
					path = new VectorPath();
					AddPath(path);
				}
				path->AddPoint(BPoint(x1, y1));
				path->SetInOutConnected(path->CountPoints() - 1, false);
				break;

			case agg::path_cmd_line_to:
//printf("line to (%.2f, %.2f) (subPath: %ld)\n", x1, y1, subPath);
				path->AddPoint(BPoint(x1, y1));
				path->SetInOutConnected(path->CountPoints() - 1, false);
				break;

			case agg::path_cmd_curve3: {
				double x2, y2;
				cmd = source.vertex(&x2, &y2);
//printf("curve3 (%.2f, %.2f)\n", x1, y1);
//printf("	   (%.2f, %.2f)\n", x2, y2);

				// convert to curve4 for easier editing
				int32 start = path->CountPoints() - 1;
				BPoint from;
				path->GetPointAt(start, from);
				
				double cx2 = (1.0/3.0) * from.x + (2.0/3.0) * x1;
				double cy2 = (1.0/3.0) * from.y + (2.0/3.0) * y1;
				double cx3 = (2.0/3.0) * x1 + (1.0/3.0) * x2;
				double cy3 = (2.0/3.0) * y1 + (1.0/3.0) * y2;
				
				path->SetPointOut(start, BPoint(cx2, cy2));

				path->AddPoint(BPoint(x2, y2));

				int32 end = path->CountPoints() - 1;
				path->SetInOutConnected(end, false);
				path->SetPointIn(end, BPoint(cx3, cy3));
				break;
			}

			case agg::path_cmd_curve4: {
				double x2, y2;
				double x3, y3;
				cmd = source.vertex(&x2, &y2);
				cmd = source.vertex(&x3, &y3);

				path->AddPoint(BPoint(x3, y3));

				int32 start = path->CountPoints() - 2;
				int32 end = path->CountPoints() - 1;

//printf("curve4 [%ld] (%.2f, %.2f) -> [%ld] (%.2f, %.2f) -> (%.2f, %.2f)\n", start, x1, y1, end, x2, y2, x3, y3);

				path->SetInOutConnected(end, false);
				path->SetPointOut(start, BPoint(x1, y1));
				path->SetPointIn(end, BPoint(x2, y2));
				break;
			}
			default:
//printf("unkown command\n");
				break;
		}
		cmd = source.vertex(&x1, &y1);
	}
//path->PrintToStream();
	path->CleanUp();
	if (path->CountPoints() == 0) {
//printf("path no points! (%ld)", fSubPaths.IndexOf((void*)path));
		RemovePath(path);
		delete path;
	}
}

#endif	// SHAPE_STROKE_H
