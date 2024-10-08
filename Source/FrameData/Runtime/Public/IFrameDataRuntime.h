#pragma once

#include "ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(FrameDataRuntime, Log, All);

/**
 * The public interface to this module
 */
class IFrameDataRuntime : public IModuleInterface
{

public:

	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static IFrameDataRuntime& Get()
	{
		return FModuleManager::LoadModuleChecked< IFrameDataRuntime >("FrameDataRuntime");
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded( "FrameDataRuntime" );
	}
};

