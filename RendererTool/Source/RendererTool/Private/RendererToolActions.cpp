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
	TSharedRef<FRendererDisplayer> Displayer = SNew( FRendererDisplayer )
		.AutoCenter(EAutoCenter::PreferredWorkArea)
		.Title(LOCTEXT("RendererDisplayerHeading", "Renderer Displayer"))
		.SupportsMinimize(true)
		.SupportsMaximize(true)
		.MinHeight(100)
		.MinWidth(100)
		.ClientSize(FVector2D(1920.0, 1080.0));

	TSharedRef<SWidget> DisplayerContent = FRendererDisplayerModule::GetDispalyerContent();
	Displayer->SetContent(DisplayerContent);

	FSlateApplication::Get().AddWindow(Displayer, true);
}

#undef LOCTEXT_NAMESPACE