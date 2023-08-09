#include "Editor/RendererDisplayer.h"
#include "Widgets/SViewport.h"

#define LOCTEXT_NAMESPACE "RendererDisplayer"

TSharedRef<SWidget> FRendererDisplayerModule::GetDispalyerContent()
{
	return SNew( SOverlay )
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Top)
			[
				SNew( SBox )
				.HeightOverride(20.0f)
				[
					SNew( SOverlay )
					+ SOverlay::Slot()
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Fill)
					.Padding(0, 0, 0, 0)
					[
						SNew( SButton )
						.DesiredSizeScale(FVector2D(1, 1))
						.Text(LOCTEXT("FileButton_Heading", "File"))
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Fill)
					.Padding(60.0f, 0, 0, 0)
					[
						SNew(SButton)
						.DesiredSizeScale(FVector2D(1, 1))
						.Text(LOCTEXT("EditButton_Heading", "Edit"))
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Fill)
					.Padding(120.0f, 0, 0, 0)
					[
						SNew(SButton)
						.DesiredSizeScale(FVector2D(1, 1))
						.Text(LOCTEXT("ToolButton_Heading", "Tool"))
					]
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Padding(0, 25.0f, 0, 0)
			[
				SNew( SSplitter )
				.ResizeMode(ESplitterResizeMode::Fill)
				.Orientation(EOrientation::Orient_Horizontal)
				+ SSplitter::Slot()
				.Resizable(true)
				.MinSize(25)
				[
					SNew( SViewport )
				]
				+ SSplitter::Slot()
				.Resizable(true)
				.MinSize(25)
				[
					SNew( SSplitter )
					.ResizeMode(ESplitterResizeMode::Fill)
					.Orientation(EOrientation::Orient_Vertical)
					+ SSplitter::Slot()
					[
						SNew( SSpacer )
					]
					+ SSplitter::Slot()
					[
						SNew( SSpacer )
					]
				]
			];
}

FRendererDisplayer::FRendererDisplayer()
{

}

FRendererDisplayer::~FRendererDisplayer()
{
	FRendererDisplayerSystem::RemoveRendererDisplayer(MakeShareable(this));
}

void FRendererDisplayer::Tick(float InDeltaTime)
{
	UE_LOG(LogTemp, Warning, TEXT("Test"));
}

void FRendererDisplayerSystem::Tick(float InDeltaTime)
{
	for (TSharedPtr<FRendererDisplayer> Displayer : Displayers)
	{
		Displayer->Tick(InDeltaTime);
	}
}

ETickableTickType FRendererDisplayerSystem::GetTickableTickType() const
{
	return ETickableTickType::Always;
}

TStatId FRendererDisplayerSystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FRendererDisplayerSystem, STATGROUP_Tickables);
}

TSharedPtr<FRendererDisplayer> FRendererDisplayerSystem::CreateRendererDisplayer()
{
	TSharedRef<FRendererDisplayer> Displayer = SNew(FRendererDisplayer)
		.AutoCenter(EAutoCenter::PreferredWorkArea)
		.Title(LOCTEXT("RendererDisplayerHeading", "Renderer Displayer"))
		.SupportsMinimize(true)
		.SupportsMaximize(true)
		.MinHeight(100)
		.MinWidth(100)
		.ClientSize(FVector2D(1920.0, 1080.0));

	TSharedRef<SWidget> DisplayerContent = FRendererDisplayerModule::GetDispalyerContent();
	Displayer->SetContent(DisplayerContent);

	Get().Displayers.Add(Displayer);

	return Displayer;
}

void FRendererDisplayerSystem::RemoveRendererDisplayer(TSharedPtr<FRendererDisplayer> InDisplayer)
{
	Get().Displayers.Remove(InDisplayer);
}

#undef LOCTEXT_NAMESPACE