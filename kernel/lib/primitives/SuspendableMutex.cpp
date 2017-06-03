#include "lib/primitives/SuspendableMutex.h"

void SuspendableMutex::onLockFailed(void)
{
	this->wait();
}

void SuspendableMutex::onUnlock(void)
{
	// we use notifyOne here because only 1 can get the lock successfully
	// and if notifyAll will be called, then it will be the first one to get it
	// so we will notify only ONE task at a time..
	this->notifyOne(0);
}
