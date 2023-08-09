#pragma once

#include "CoreMinimal.h"
#include "SlateFwd.h"
#include "HAL/IConsoleManager.h"
#include "Input/Reply.h"
#include "Framework/Commands/Commands.h"

class FRendererToolCommands : public TCommands<FRendererToolCommands>
{
public:

	FRendererToolCommands();

	TSharedPtr< FUICommandInfo > CreateDisplayer;

	virtual void RegisterCommands() override;

};

class FRendererToolActionCallbacks
{
public:

	static void CreateNewDisplayer();

};