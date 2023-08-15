#include "Renderer/RendererToolViewportClient.h"
#include "Renderer/RendererToolViewport.h"
#include "CanvasTypes.h"
#include "EngineModule.h"
#include "Widgets/SViewport.h"
#include "CanvasItem.h"
#include "SceneViewExtension.h"
#include "LegacyScreenPercentageDriver.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

FViewportTransform::FViewportTransform()
	: Location( FVector::ZeroVector )
	, Rotation( FRotator::ZeroRotator )
{

}

FRendererToolViewportClient::FRendererToolViewportClient(UWorld* InWorld, TSharedPtr<SViewport> InViewportWidget)
	: World(InWorld)
	, Viewport(MakeShareable(new FRendererToolViewport(this, InViewportWidget)))
	, ViewState()
	, ViewType(LVT_Perspective)
	, EngineShowFlags(ESFIM_Game)
	, ViewIndex(INDEX_NONE)
	, ViewFOV(EditorViewportDefs::DefaultPerspectiveFOVAngle)
	, AspectRatio(1.777777f)
	, NearPlane(-1.0f)
	, FarPlane(1.0f)
	, CurrentCursorPos(FIntPoint(-1, -1))
{
	InViewportWidget->SetViewportInterface(Viewport.ToSharedRef());

	FSceneInterface *Scene = GetScene();
	ViewState.Allocate((Scene != nullptr) ? Scene->GetFeatureLevel() : GMaxRHIFeatureLevel);

	AStaticMeshActor *Sphere = GetWorld()->SpawnActor<AStaticMeshActor>(FVector(), FRotator());
	ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	UStaticMesh *SphereMesh = SphereMeshAsset.Object;
	Sphere->GetComponentByClass<UStaticMeshComponent>()->SetStaticMesh(SphereMesh);
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

	ViewFamily.ViewMode = GetViewMode();

	if (ViewFamily.GetScreenPercentageInterface() == nullptr)
	{
		float GlobalResolutionFraction = 1.0f;

		ViewFamily.SetScreenPercentageInterface(new FLegacyScreenPercentageDriver(ViewFamily, 1.0f));
	}

	GetRendererModule().BeginRenderingViewFamily(InCanvas, &ViewFamily);

	//if (View)
	//{
	//	DrawCanvas(*Viewport, *View, *InCanvas);

	//	DrawSafeFrames(*Viewport, *View, *InCanvas);
	//}
}

void FRendererToolViewportClient::AddReferencedObjects(FReferenceCollector& Collector)
{
	if (ViewState.GetReference())
	{
		ViewState.GetReference()->AddReferencedObjects(Collector);
	}
}

FString FRendererToolViewportClient::GetReferencerName() const
{
	return TEXT("FRendererToolViewportClient");
}

FSceneView* FRendererToolViewportClient::CalcSceneView(FSceneViewFamily* InViewFamily)
{
	FSceneViewInitOptions ViewInitOption;

	FViewportTransform& ViewTransform = GetViewTransform();
	ViewInitOption.ViewOrigin = ViewTransform.GetLocation();

	FIntPoint ViewportSize = Viewport->GetSizeXY();
	EAspectRatioAxisConstraint AspectRatioAxisConstraint = GetDefault<ULevelEditorViewportSettings>()->AspectRatioAxisConstraint;

	ViewInitOption.SetViewRectangle(FIntRect(FIntPoint(), ViewportSize));

	if (ViewType == ELevelViewportType::LVT_Perspective)
	{
		float MinZ = GetNearPlane();
		float MaxZ = MinZ;

		ViewInitOption.ViewRotationMatrix = FInverseRotationMatrix(ViewTransform.GetRotation());

		float XAxisMultiplier;
		float YAxisMultiplier;

		if ((ViewportSize.X > ViewportSize.Y) && ((AspectRatioAxisConstraint == AspectRatio_MajorAxisFOV) || (AspectRatioAxisConstraint == AspectRatio_MaintainXFOV)))
		{
			XAxisMultiplier = 1.0f;
			YAxisMultiplier = ((float)ViewportSize.X / (float)ViewportSize.Y);
		}
		else
		{
			XAxisMultiplier = ((float)ViewportSize.Y / (float)ViewportSize.X);
			YAxisMultiplier = 1.0f;
		}

		if ((bool)ERHIZBuffer::IsInverted)
		{
			ViewInitOption.ProjectionMatrix = FReversedZPerspectiveMatrix(ViewFOV, ViewFOV, XAxisMultiplier, YAxisMultiplier, MinZ, MaxZ);
		}
		else
		{
			ViewInitOption.ProjectionMatrix = FPerspectiveMatrix(ViewFOV, ViewFOV, XAxisMultiplier, YAxisMultiplier, MinZ, MaxZ);
		}
	}
	else
	{

	}

	if (!ViewInitOption.IsValidViewRectangle())
	{
		ViewInitOption.SetViewRectangle(FIntRect(0, 0, 1, 1));
	}

	ViewInitOption.ViewFamily = InViewFamily;
	ViewInitOption.SceneViewStateInterface = ViewState.GetReference();
	ViewInitOption.StereoViewIndex = INDEX_NONE;

	ViewInitOption.ViewElementDrawer = this;

	ViewInitOption.BackgroundColor = FColor(255, 55, 55);

	ViewInitOption.EditorViewBitflag = 1;

	ViewInitOption.OverrideLODViewOrigin = FVector::ZeroVector;
	ViewInitOption.bUseFauxOrthoViewPos = true;

	ViewInitOption.FOV = ViewFOV;

	ViewInitOption.CursorPos = CurrentCursorPos;

	FSceneView *View = new FSceneView(ViewInitOption);

	View->ViewLocation = ViewTransform.GetLocation();
	View->ViewRotation = ViewTransform.GetRotation();

	View->StartFinalPostprocessSettings(View->ViewLocation);

	InViewFamily->Views.Add(View);

	for (int ViewExt = 0; ViewExt < InViewFamily->ViewExtensions.Num(); ViewExt++)
	{
		InViewFamily->ViewExtensions[ViewExt]->SetupView(*InViewFamily, *View);
	}

	return View;
}

bool FRendererToolViewportClient::SupportsPreviewResolutionFraction() const
{
	switch (GetViewMode())
	{
	case VMI_BrushWireframe:
	case VMI_Wireframe:
	case VMI_LightComplexity:
	case VMI_LightmapDensity:
	case VMI_LitLightmapDensity:
	case VMI_ReflectionOverride:
	case VMI_StationaryLightOverlap:
	case VMI_CollisionPawn:
	case VMI_CollisionVisibility:
	case VMI_LODColoration:
	case VMI_PrimitiveDistanceAccuracy:
	case VMI_MeshUVDensityAccuracy:
	case VMI_HLODColoration:
	case VMI_GroupLODColoration:
	case VMI_VisualizeGPUSkinCache:
		return false;
	}

	// Don't do preview screen percentage in certain cases.
	if (EngineShowFlags.VisualizeBuffer || EngineShowFlags.VisualizeNanite || EngineShowFlags.VisualizeVirtualShadowMap)
	{
		return false;
	}

	return true;
}