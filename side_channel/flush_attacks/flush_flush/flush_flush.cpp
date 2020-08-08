#include "flush_flush.hpp"

using FF=FlushFlush;

void FF::operation() const {

	flush();
}

FF::FlushFlush(const char* exec, unsigned int offset) :
	CacheTimingAttack(exec, offset){};

FF::FlushFlush(const FF& other) : CacheTimingAttack(other){};
