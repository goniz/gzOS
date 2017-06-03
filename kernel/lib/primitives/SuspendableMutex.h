#ifndef PROCESSMUTEX_H
#define PROCESSMUTEX_H

#include "lib/primitives/AbstractMutex.h"
#include "lib/primitives/Suspendable.h"

class SuspendableMutex : public AbstractMutex, public Suspendable
{
public:
	SuspendableMutex(void) = default;
	virtual ~SuspendableMutex() = default;
	
private:
	virtual void onLockFailed(void);
	virtual void onUnlock(void);
};

#endif // PROCESSMUTEX_H
