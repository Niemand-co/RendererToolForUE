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
		Displayer->SetContent
		(
			SNew( SVerticalBox )
			+ SVerticalBox::Slot()
			.FillHeight(80.0f / Displayer->GetClientSizeInScreen().Y)
			.HAlign(EHorizontalAlignment::HAlign_Fill)
			.VAlign(EVerticalAlignment::VAlign_Fill)
			[
				SNew( SHorizontalBox )
				+ SHorizontalBox::Slot().MaxWidth(80.0f)
				[
					SNew( SButton )
					.Text(LOCTEXT("FileHeading", "File"))
				]
				+ SHorizontalBox::Slot().MaxWidth(80.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("EditHeading", "Edit"))
				]
			]
			+ SVerticalBox::Slot()
			.FillHeight((Displayer->GetClientSizeInScreen().Y - 80.0f) / Displayer->GetClientSizeInScreen().Y)
			.HAlign(EHorizontalAlignment::HAlign_Fill)
			.VAlign(EVerticalAlignment::VAlign_Fill)
			[
				SNew( SSplitter )
				.Orientation(EOrientation::Orient_Horizontal)
				+ SSplitter::Slot()
				.MinSize(20)
				[
					SNew( SImage )
				]
				+ SSplitter::Slot()
				.MinSize(20)
				[
					SNew( SSplitter )
					.Orientation(EOrientation::Orient_Vertical)
					+ SSplitter::Slot()
					.MinSize(20)
					[
						SNew( SImage )
					]
					+ SSplitter::Slot()
					.MinSize(20)
					[
						SNew( SImage )
					]
				]
			]
		);

	FSlateApplication::Get().AddWindow(Displayer, false);

	Displayer->ShowWindow();
}

#undef LOCTEXT_NAMESPACE