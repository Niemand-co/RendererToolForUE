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
	UE_LOG(LogTemp, Warning, TEXT("Viewport Die"))
}

void FRendererToolViewport::ProcessInput(float InDeltaTime)
{
	UE_LOG(LogTemp, Warning, TEXT("Tick"))
}