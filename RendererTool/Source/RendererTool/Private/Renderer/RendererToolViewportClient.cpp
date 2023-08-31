#include "Renderer/RendererToolViewportClient.h"
#include "Renderer/RendererToolViewport.h"
#include "CanvasTypes.h"
#include "EngineModule.h"
#include "Widgets/SViewport.h"
#include "CanvasItem.h"
#include "SceneViewExtension.h"
#include "LegacyScreenPercentageDriver.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/DirectionalLight.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "EditorModeManager.h"
#include "Renderer/RendererToolRenderer.h"

FRendererToolViewportClient::FRendererToolViewportClient(UWorld* InWorld, TSharedPtr<SViewport> InViewportWidget)
	: FEditorViewportClient(&GLevelEditorModeTools())
	, World(InWorld)
	, LocalViewport(MakeShareable(new FRendererToolViewport(this, InViewportWidget)))
{
	InViewportWidget->SetViewportInterface(LocalViewport.ToSharedRef());

	Viewport = LocalViewport.Get();

	const FStringAssetReference SphereAssetPath(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	UStaticMesh* SphereMesh = Cast<UStaticMesh>(SphereAssetPath.TryLoad());
	if (SphereMesh)
	{
		AStaticMeshActor* Sphere = World->SpawnActor<AStaticMeshActor>();
		Sphere->GetComponentByClass<UStaticMeshComponent>()->SetStaticMesh(SphereMesh);
		const FStringAssetReference SphereMaterialAsset(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
		UMaterialInterface* SphereMaterial = Cast<UMaterialInterface>(SphereMaterialAsset.TryLoad());
		if (SphereMaterial)
		{
			Sphere->GetComponentByClass<UStaticMeshComponent>()->SetMaterial(0, SphereMaterial);
		}

		AActor *Light = World->SpawnActor<AActor>();
		Light->AddComponentByClass(UDirectionalLightComponent::StaticClass(), false, FTransform(), false);
		
		const FStringAssetReference SkySphereAsset(TEXT("/Engine/EngineSky/SM_SkySphere.SM_SkySphere"));
		UStaticMesh* SkySphereMesh = Cast<UStaticMesh>(SkySphereAsset.TryLoad());
		if (SkySphereMesh)
		{
			AStaticMeshActor* SkySphere = World->SpawnActor<AStaticMeshActor>();
			SkySphere->GetComponentByClass<UStaticMeshComponent>()->SetStaticMesh(SkySphereMesh);
			const FStringAssetReference SkyMaterialAsset(TEXT("/Engine/EngineSky/M_SimpleSkyDome.M_SimpleSkyDome"));
			UMaterialInterface *Material = Cast<UMaterialInterface>(SkyMaterialAsset.TryLoad());
			if (Material)
			{
				SkySphere->GetComponentByClass<UStaticMeshComponent>()->SetMaterial(0, Material);
			}
		}
	}
}

FRendererToolViewportClient::~FRendererToolViewportClient()
{
	Viewport = NULL;
	World = NULL;
}

static void RenderInternal(FRHICommandListImmediate& RHICmdList, const TArray<class FDisplayerSceneRenderer*>& SceneRenderers)
{
	FDisplayerSceneRenderer::RenderThreadBegin(RHICmdList, SceneRenderers);

	for (FDisplayerSceneRenderer* SceneRenderer : SceneRenderers)
	{
		const ERHIFeatureLevel::Type FeatureLevel = SceneRenderer->FeatureLevel;

		FDisplayerViewFamilyInfo& ViewFamily = SceneRenderer->ViewFamily;

		FRDGBuilder GraphBuilder(
			RHICmdList,
			RDG_EVENT_NAME("SceneRenderer_%s(ViewFamily=%s)",
				ViewFamily.EngineShowFlags.HitProxies ? TEXT("RenderHitProxies") : TEXT("Render"),
				TEXT("Primary")
			),
			FSceneRenderer::GetRDGParalelExecuteFlags(FeatureLevel)
		);

		//const uint64 FamilyRenderStart = FPlatformTime::Cycles64();

		if (ViewFamily.EngineShowFlags.HitProxies)
		{
			// Render the scene's hit proxies.
			SceneRenderer->RenderHitProxies(GraphBuilder);
		}
		else
		{
			// Render the scene.
			SceneRenderer->Render(GraphBuilder);
		}

		GraphBuilder.Execute();

		//if (ViewFamily->ProfileSceneRenderTime)
		//{
		//	*ViewFamily->ProfileSceneRenderTime = (float)FPlatformTime::ToSeconds64(FPlatformTime::Cycles64() - FamilyRenderStart);
		//}
	}

	FDisplayerSceneRenderer::RenderThreadEnd(RHICmdList, SceneRenderers);
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

	ViewFamily.ViewMode = GetViewMode();

	if (ViewFamily.GetScreenPercentageInterface() == nullptr)
	{
		float GlobalResolutionFraction = 1.0f;

		ViewFamily.SetScreenPercentageInterface(new FLegacyScreenPercentageDriver(ViewFamily, 1.0f));
	}

	TArray<FDisplayerSceneRenderer*> SceneRenderers;

	TArray<const FSceneViewFamily*> ViewFamilies;
	ViewFamilies.Add(&ViewFamily);

	FDisplayerSceneRenderer::CreateDisplayerSceneRenderers(ViewFamilies, InCanvas->GetHitProxyConsumer(), SceneRenderers);

	const uint64 DrawSceneEnqueue = FPlatformTime::Cycles64();
	ENQUEUE_RENDER_COMMAND(FDrawSceneCommand)(
		[LocalSceneRenderers = CopyTemp(SceneRenderers), DrawSceneEnqueue](FRHICommandListImmediate& RHICmdList)
		{
			uint64 SceneRenderStart = FPlatformTime::Cycles64();
			const float StartDelayMillisec = FPlatformTime::ToMilliseconds64(SceneRenderStart - DrawSceneEnqueue);
			CSV_CUSTOM_STAT_GLOBAL(DrawSceneCommand_StartDelay, StartDelayMillisec, ECsvCustomStatOp::Set);
			RenderInternal(RHICmdList, LocalSceneRenderers);
			FlushPendingDeleteRHIResources_RenderThread();
		});

	if (View)
	{
		DrawCanvas(*Viewport, *View, *InCanvas);

		//DrawSafeFrames(*Viewport, *View, *InCanvas);
	}

	const bool bShouldDrawAxes = (bDrawAxes && !ViewFamily.EngineShowFlags.Game) || (bDrawAxesGame && ViewFamily.EngineShowFlags.Game);
	if (bShouldDrawAxes && !GLevelEditorModeTools().IsViewportUIHidden() && !IsVisualizeCalibrationMaterialEnabled())
	{
		switch (GetViewportType())
		{
		case LVT_OrthoXY:
		{
			const FRotator XYRot(-90.0f, -90.0f, 0.0f);
			DrawAxes(Viewport, InCanvas, &XYRot, EAxisList::XY);
			if (View)
			{
				DrawScaleUnits(Viewport, InCanvas, *View);
			}
			break;
		}
		case LVT_OrthoXZ:
		{
			const FRotator XZRot(0.0f, -90.0f, 0.0f);
			DrawAxes(Viewport, InCanvas, &XZRot, EAxisList::XZ);
			if (View)
			{
				DrawScaleUnits(Viewport, InCanvas, *View);
			}
			break;
		}
		case LVT_OrthoYZ:
		{
			const FRotator YZRot(0.0f, 0.0f, 0.0f);
			DrawAxes(Viewport, InCanvas, &YZRot, EAxisList::YZ);
			if (View)
			{
				DrawScaleUnits(Viewport, InCanvas, *View);
			}
			break;
		}
		case LVT_OrthoNegativeXY:
		{
			const FRotator XYRot(90.0f, 90.0f, 0.0f);
			DrawAxes(Viewport, InCanvas, &XYRot, EAxisList::XY);
			if (View)
			{
				DrawScaleUnits(Viewport, InCanvas, *View);
			}
			break;
		}
		case LVT_OrthoNegativeXZ:
		{
			const FRotator XZRot(0.0f, 90.0f, 0.0f);
			DrawAxes(Viewport, InCanvas, &XZRot, EAxisList::XZ);
			if (View)
			{
				DrawScaleUnits(Viewport, InCanvas, *View);
			}
			break;
		}
		case LVT_OrthoNegativeYZ:
		{
			const FRotator YZRot(0.0f, 180.0f, 0.0f);
			DrawAxes(Viewport, InCanvas, &YZRot, EAxisList::YZ);
			if (View)
			{
				DrawScaleUnits(Viewport, InCanvas, *View);
			}
			break;
		}
		default:
		{
			DrawAxes(Viewport, InCanvas);
			break;
		}
		}
	}
}

void FRendererToolViewportClient::Tick(float DeltaSeconds)
{
	FEditorViewportClient::Tick(DeltaSeconds);
}
