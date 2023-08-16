#include "Editor/RendererDisplayer.h"
#include "Widgets/SViewport.h"
#include "Renderer/RendererToolViewport.h"
#include "Renderer/RendererToolViewportClient.h"

#define LOCTEXT_NAMESPACE "RendererDisplayer"

TSharedPtr<FRendererDisplayer> FRendererDisplayerModule::CreateDefaultRendererDisplayer()
{
	TSharedPtr<SViewport> ViewportWidget;

	TSharedPtr<SWidget> DisplayerContent = SNew( SOverlay )
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
					SAssignNew( ViewportWidget, SViewport )
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

	TSharedRef<FRendererDisplayer> Displayer = SNew(FRendererDisplayer, ViewportWidget)
		.AutoCenter(EAutoCenter::PreferredWorkArea)
		.Title(LOCTEXT("RendererDisplayerHeading", "Renderer Displayer"))
		.SupportsMinimize(true)
		.SupportsMaximize(true)
		.MinHeight(100)
		.MinWidth(100)
		.ClientSize(FVector2D(1920.0, 1080.0));

	//Displayer->SetOnWindowClosed(FOnWindowClosed::CreateLambda([=]()
	//{
	//	FRendererDisplayerSystem::AddToDestroyList(Displayer);
	//}));

	Displayer->SetContent(DisplayerContent.ToSharedRef());

	return Displayer;
}

FRendererDisplayer::FRendererDisplayer(TSharedPtr<SViewport> InViewportWidget)
{
	World = UWorld::CreateWorld(EWorldType::Editor, true);

	ViewportClient = MakeShareable(new FRendererToolViewportClient(World, InViewportWidget));
}

FRendererDisplayer::~FRendererDisplayer()
{
	UE_LOG(LogTemp, Warning, TEXT("Displayer Die"))
}

void FRendererDisplayer::Construct(const FArguments& Arguments, TSharedPtr<SViewport> InViewportWidget)
{
	FWorldContext& RendererWorldContext = GEngine->CreateNewWorldContext(EWorldType::Editor);
	RendererWorldContext.SetCurrentWorld(UWorld::CreateWorld(EWorldType::Editor, true));
	this->World = RendererWorldContext.World();
	this->ViewportClient = MakeShareable(new FRendererToolViewportClient(World, InViewportWidget));

	SWindow::Construct(Arguments);
}

void FRendererDisplayer::Tick(float InDeltaTime)
{
	UE_LOG(LogTemp, Warning, TEXT("Test"));

	World->Tick(ELevelTick::LEVELTICK_All, InDeltaTime);

	ViewportClient->Tick(InDeltaTime);

	ViewportClient->GetViewport()->Draw();
}

void FRendererDisplayerSystem::Tick(float InDeltaTime)
{
	for (TSharedPtr<FRendererDisplayer> Displayer : Displayers)
	{
		Displayer->Tick(InDeltaTime);
	}

	while (!DisplayersToDestroy.IsEmpty())
	{
		TSharedPtr<FRendererDisplayer> Displayer;
		check(DisplayersToDestroy.Dequeue(Displayer));

		Displayers.Remove(Displayer);
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
	TSharedPtr<FRendererDisplayer> Displayer = FRendererDisplayerModule::CreateDefaultRendererDisplayer();
	Get().Displayers.Add(Displayer);

	return Displayer;
}

void FRendererDisplayerSystem::RemoveRendererDisplayer(TSharedPtr<FRendererDisplayer> InDisplayer)
{
	Get().Displayers.Remove(InDisplayer);
}

void FRendererDisplayerSystem::AddToDestroyList(TSharedPtr<FRendererDisplayer> InDisplayer)
{
	Get().DisplayersToDestroy.Enqueue(InDisplayer);
}

#undef LOCTEXT_NAMESPACE