/*
 * mutex.cpp
 *
 *  Created on: Mar 15, 2015
 *      Author: gz
 */

#include <lib/primitives/AbstractMutex.h>

AbstractMutex::AbstractMutex(void)
	: m_mutex(MutexState::UNLOCKED)
{

}

void AbstractMutex::lock(void)
{
	MutexState oldValue = MutexState::UNLOCKED;
	while (!m_mutex.compare_exchange_weak(oldValue, MutexState::LOCKED)) {
		this->onLockFailed();
	}
}

void AbstractMutex::unlock(void)
{
	m_mutex.store(MutexState::UNLOCKED);

	// notify something if needed
	this->onUnlock();
}
