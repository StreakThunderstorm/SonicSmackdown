#include "FrameDataRuntimePCH.h"

DEFINE_LOG_CATEGORY(FrameDataRuntime)

class FFrameDataRuntime : public IFrameDataRuntime
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE( FFrameDataRuntime, FrameDataRuntime )



void FFrameDataRuntime::StartupModule()
{
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
}


void FFrameDataRuntime::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}



