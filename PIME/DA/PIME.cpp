void DAMain();

#include "../Engine/PIMEEngine.h"

void DAMain()
{
	PIMEEnginePtr engine = PIME_OpenEngine();
	
	PIME_Run(engine);
	
	PIME_CloseEngine(engine);
}
