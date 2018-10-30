#ifndef __VRWorker_H__
#define __VRWorker_H__

#include "logger.h"

class VRWorker
{
public:
	VRWorker(Logger* logger);

	void Init();

private:
	Logger* _logger;
};

#endif //__VRWorker_H__