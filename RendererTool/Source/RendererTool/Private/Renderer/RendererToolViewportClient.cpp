#include "Renderer/RendererToolViewportClient.h"
#include "Renderer/RendererToolViewport.h"
#include "CanvasTypes.h"
#include "EngineModule.h"
#include "Widgets/SViewport.h"
#include "CanvasItem.h"
#include "Engine/Classes/Engine/TextureRenderTarget2D.h"

FRendererToolViewportClient::FRendererToolViewportClient(UWorld* InWorld, TSharedPtr<SViewport> InViewportWidget)
	: World(InWorld)
	, Viewport(MakeShareable(new FRendererToolViewport(this, InViewportWidget)))
	, EngineShowFlags(ESFIM_Game)
{
	InViewportWidget->SetViewportInterface(Viewport.ToSharedRef());
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
	FGameTime Time;
	if (!World || GetScene() != World->Scene)
	{
		Time = FGameTime::GetTimeSinceAppStart();
	}
	else
	{
		Time = World->GetTime();
	}

	FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
		InCanvas->GetRenderTarget(),
		GetScene(),
		EngineShowFlags)
		.SetTime(Time)
		.SetViewModeParam(VMI_Lit, "Lit"));

	FSceneView *View = CalcSceneView(&ViewFamily);

	GetRendererModule().BeginRenderingViewFamily(InCanvas, &ViewFamily);
}

FSceneView* FRendererToolViewportClient::CalcSceneView(FSceneViewFamily* InViewFamily)
{
	FSceneViewInitOptions ViewInitOption;

	ViewInitOption.bUseFauxOrthoViewPos = Viewport->GetDesiredAspectRatio();

	return nullptr;
}
