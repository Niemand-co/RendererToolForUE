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
	UE_LOG(LogTemp, Warning, TEXT("Client Die"))
}

//void FRendererToolViewportClient::Draw(FViewport* InViewport, FCanvas* InCanvas)
//{
//	FGameTime Time;
//	if (!World || GetScene() != World->Scene)
//	{
//		Time = FGameTime::GetTimeSinceAppStart();
//	}
//	else
//	{
//		Time = World->GetTime();
//	}
//
//	FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
//		InCanvas->GetRenderTarget(),
//		GetScene(),
//		EngineShowFlags)
//		.SetTime(Time)
//		.SetViewModeParam(VMI_Lit, "Lit"));
//
//	FSceneView *View = CalcSceneView(&ViewFamily);
//
//	ViewFamily.ViewMode = GetViewMode();
//
//	if (ViewFamily.GetScreenPercentageInterface() == nullptr)
//	{
//		float GlobalResolutionFraction = 1.0f;
//
//		ViewFamily.SetScreenPercentageInterface(new FLegacyScreenPercentageDriver(ViewFamily, 1.0f));
//	}
//
//	GetRendererModule().BeginRenderingViewFamily(InCanvas, &ViewFamily);
//
//	//if (View)
//	//{
//	//	DrawCanvas(*Viewport, *View, *InCanvas);
//
//	//	DrawSafeFrames(*Viewport, *View, *InCanvas);
//	//}
//}

void FRendererToolViewportClient::Tick(float DeltaSeconds)
{
	FEditorViewportClient::Tick(DeltaSeconds);
}
