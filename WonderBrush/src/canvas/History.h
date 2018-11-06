// History.h

#ifndef HISTORY_H
#define HISTORY_H

#include <List.h>

class Stroke;

class History : public BList {
 public:
								History();
								History(const History& clone);
	virtual						~History();

			// list functionality
			bool				AddModifier(Stroke* stroke);
			bool				AddModifier(Stroke* stroke,
											int32 index);

			Stroke*				RemoveModifier(int32 index);
			bool				RemoveModifier(Stroke* stroke);

			Stroke*				ModifierAt(int32 index) const;
			int32				IndexOf(Stroke* stroke) const;
			int32				CountModifiers() const;

			bool				HasModifier(Stroke* stroke) const;

 private:
};

#endif // HISTORY_H
