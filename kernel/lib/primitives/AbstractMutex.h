/*
 * mutex.h
 *
 *  Created on: Mar 15, 2015
 *      Author: gz
 */

#ifndef INCLUDE_PRIMITIVES_ABSTRACT_MUTEX_H_
#define INCLUDE_PRIMITIVES_ABSTRACT_MUTEX_H_

#include <atomic>
#include "lib/primitives/Mutex.h"

enum class MutexState {
	UNLOCKED 	= 0,
	LOCKED 		= 1
};

class AbstractMutex : public Mutex
{
public:
	AbstractMutex(void);
	virtual ~AbstractMutex(void) = default;

	virtual void lock(void) override;
	virtual void unlock(void) override;

	inline MutexState unsafe_get(void) {
		return this->m_mutex;
	}

protected:
	virtual void onLockFailed(void) = 0;
	virtual void onUnlock(void) = 0;

	std::atomic<MutexState> m_mutex;
};

#endif /* INCLUDE_PRIMITIVES_ABSTRACT_MUTEX_H_ */
