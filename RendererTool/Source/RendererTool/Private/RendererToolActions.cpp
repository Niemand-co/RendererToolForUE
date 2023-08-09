#include "RendererToolActions.h"
#include "Editor/RendererDisplayer.h"
#include "Toolkits/GlobalEditorCommonCommands.h"

#define LOCTEXT_NAMESPACE "RendererToolCommands"

FRendererToolCommands::FRendererToolCommands()
	: TCommands<FRendererToolCommands>(
		TEXT("RendererTool"),
		LOCTEXT("RendererTool", "Renderer Tool"),
		NAME_None,
		FAppStyle::GetAppStyleSetName()
	)
{
}

void FRendererToolCommands::RegisterCommands()
{
	UI_COMMAND(CreateDisplayer, "New A Displayer", "Create a new dispalyer viewport to display rendering result", EUserInterfaceActionType::Button, FInputChord());
}

void FRendererToolActionCallbacks::CreateNewDisplayer()
{
	TSharedPtr<FRendererDisplayer> Displayer = FRendererDisplayerSystem::CreateRendererDisplayer();
	FSlateApplication::Get().AddWindow(Displayer.ToSharedRef(), true);
}

#undef LOCTEXT_NAMESPACE