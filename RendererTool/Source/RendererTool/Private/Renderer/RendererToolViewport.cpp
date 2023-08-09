#include "Renderer/RendererToolViewport.h"

FRendererToolViewport::FRendererToolViewport(FViewportClient* InViewportClient, TSharedPtr<SViewport> InViewportWidget)
	: FSceneViewport(InViewportClient, InViewportWidget)
{
}

FRendererToolViewport::~FRendererToolViewport()
{
}

FRendererToolViewportClient::FRendererToolViewportClient(UWorld* InWorld, TSharedPtr<SViewport> InViewportWidget)
	: World(InWorld)
	, Viewport(MakeShareable(new FRendererToolViewport(this, InViewportWidget)))
{
}

FRendererToolViewportClient::~FRendererToolViewportClient()
{
}
