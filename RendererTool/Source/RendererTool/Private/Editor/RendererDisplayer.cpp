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

	World = UWorld::CreateWorld( EWorldType::Editor, true, TEXT("Renderer Tool World"));

	
}

FRendererDisplayer::~FRendererDisplayer()
{
}

void FRendererDisplayer::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SWindow::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	World->Tick(ELevelTick::LEVELTICK_All, InDeltaTime);
}

#undef LOCTEXT_NAMESPACE