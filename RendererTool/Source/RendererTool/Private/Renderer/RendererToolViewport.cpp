#include "Renderer/RendererToolViewport.h"
#include "CanvasTypes.h"
#include "EngineModule.h"
#include "Widgets/SViewport.h"
#include "CanvasItem.h"
#include "Engine/Classes/Engine/TextureRenderTarget2D.h"

FRendererToolViewport::FRendererToolViewport(FViewportClient* InViewportClient, TSharedPtr<SViewport> InViewportWidget)
	: FSceneViewport(InViewportClient, InViewportWidget)
{
	SetInitialSize(FIntPoint(1920, 1080));
}

FRendererToolViewport::~FRendererToolViewport()
{
}

FRendererToolViewportClient::FRendererToolViewportClient(UWorld* InWorld, TSharedPtr<SViewport> InViewportWidget)
	: World(InWorld)
	, Viewport(MakeShareable(new FRendererToolViewport(this, InViewportWidget)))
{
	InViewportWidget->SetViewportInterface(Viewport.ToSharedRef());

	RenderTarget = NewObject<UTextureRenderTarget2D>(GetTransientPackage(), NAME_None, RF_Transient);
	RenderTarget->RenderTargetFormat = RTF_RGBA8;
	RenderTarget->InitAutoFormat(1920, 1080);
	RenderTarget->ClearColor = FLinearColor::Red;
	RenderTarget->UpdateResource();
}

FRendererToolViewportClient::~FRendererToolViewportClient()
{
}

FSceneInterface* FRendererToolViewportClient::GetScene() const
{
	if (World)
	{
		return World->Scene;
	}
	return nullptr;
}

void FRendererToolViewportClient::Draw(FViewport* InViewport, FCanvas* InCanvas)
{
	//FEditorViewportClient::Draw(Viewport, Canvas);

	//FGameTime Time;
	//if (!World || GetScene() != World->Scene)
	//{
	//	Time = FGameTime::GetTimeSinceAppStart();
	//}
	//else
	//{
	//	Time = World->GetTime();
	//}

	//FEngineShowFlags UseEngineShowFlags;

	//FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
	//	InCanvas->GetRenderTarget(),
	//	GetScene(),
	//	UseEngineShowFlags)
	//	.SetTime(Time)
	//	.SetViewModeParam(VMI_Lit, "Lit"));

	//GetRendererModule().BeginRenderingViewFamily(InCanvas, &ViewFamily);

	FCanvasTileItem Tile(FVector2D::ZeroVector, RenderTarget->Resource, InViewport->GetSizeXY(), FLinearColor(1, 1, 1, 1));
	InCanvas->DrawItem(Tile);
}
