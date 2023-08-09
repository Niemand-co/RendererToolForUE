#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FRendererToolModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:

	void RegisterRendererToolMenu();

	void UnregisterRendererToolMenu();

private:

	TSharedPtr<FUICommandList> ActionList;

};
