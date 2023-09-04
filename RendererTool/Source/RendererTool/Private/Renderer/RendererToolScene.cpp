#include "Renderer/RendererToolScene.h"
#include "Renderer/DisplayerPrimitiveSceneInfo.h"
#include "Renderer/DisplayerPrimitiveSceneProxy.h"
#include "Components/DisplayerStaticMeshComponent.h"
#include "Components/DisplayerInstancedStaticMeshComponent.h"
#include "Components/DisplayerDirectionalLightComponent.h"
#include "Renderer/DisplayerLightSceneInfo.h"
#include "Renderer/DisplayerLightSceneProxy.h"

FDisplayerScene::FDisplayerScene(UWorld* InWorld, ERHIFeatureLevel::Type InFeatureLevel)
	: FSceneInterface(InFeatureLevel)
	, World(InWorld)
{
}

FDisplayerScene::~FDisplayerScene()
{
}

void FDisplayerScene::AddPrimitive(UPrimitiveComponent* Primitive)
{
	checkf(Primitive->IsUnreachable(), TEXT("The primitive %s is unreachable."), Primitive->GetFullName());

	const float WorldTime = GetWorld()->GetTimeSeconds();
	const float DeltaTime = WorldTime - Primitive->LastSubmitTime;
	if (DeltaTime < -0.0001f || Primitive->LastSubmitTime < 0.0001f)
	{
		Primitive->LastSubmitTime = WorldTime;
	}
	else
	{
		Primitive->LastSubmitTime = WorldTime;
	}

	checkf(Primitive->SceneProxy, TEXT("The primitive is already added to the scene."))

	FDisplayerPrimitiveSceneProxy* PrimitiveProxy = CreateDisplayerSceneProxy(Primitive);

	if (!PrimitiveProxy)
	{
		return;
	}

	FDisplayerPrimitiveSceneInfo* PrimitiveInfo = new FDisplayerPrimitiveSceneInfo(Primitive, this);
	PrimitiveProxy->SceneInfo = PrimitiveInfo;

	FDisplayerScene* Scene = this;
	FMatrix RenderMatrix = Primitive->GetRenderMatrix();
	FVector AttachmentRootPosition = Primitive->GetActorPositionForRenderer();

	struct FCreateRenderThreadParameters
	{
		FDisplayerPrimitiveSceneProxy* PrimitiveSceneProxy;
		FMatrix RenderMatrix;
		FBoxSphereBounds WorldBounds;
		FVector AttachmentRootPosition;
		FBoxSphereBounds LocalBounds;
	};
	FCreateRenderThreadParameters Params =
	{
		PrimitiveProxy,
		RenderMatrix,
		Primitive->Bounds,
		AttachmentRootPosition,
		Primitive->GetLocalBounds()
	};

	ENQUEUE_RENDER_COMMAND(AddPrimitiveCommand)(
		[Params = MoveTemp(Params), Scene, PrimitiveInfo](FRHICommandListImmediate& RHICmdList)
		{
			FDisplayerPrimitiveSceneProxy* SceneProxy = Params.PrimitiveSceneProxy;
			FScopeCycleCounter Context(SceneProxy->GetStatId());
			//SceneProxy->SetTransform(Params.RenderMatrix, Params.WorldBounds, Params.LocalBounds, Params.AttachmentRootPosition);

			Scene->AddPrimitiveSceneInfo_RenderThread(PrimitiveInfo);
		});
}

void FDisplayerScene::RemovePrimitive(UPrimitiveComponent* Primitive)
{
	FDisplayerPrimitiveSceneProxy* PrimitiveProxy = GetDisplayerSceneProxy(Primitive);

	if (PrimitiveProxy)
	{
		FDisplayerPrimitiveSceneInfo *PrimitiveInfo = PrimitiveProxy->SceneInfo;

		EmptyDisplayerSceneProxy(Primitive);

		// Send a command to the rendering thread to remove the primitive from the scene.
		FDisplayerScene* Scene = this;
		FThreadSafeCounter* AttachmentCounter = &Primitive->AttachmentCounter;
		ENQUEUE_RENDER_COMMAND(FRemovePrimitiveCommand)(
			[Scene, PrimitiveInfo, AttachmentCounter](FRHICommandList&)
			{
				Scene->RemovePrimitiveSceneInfo_RenderThread(PrimitiveInfo);
				AttachmentCounter->Decrement();
			});
	}
}

void FDisplayerScene::ReleasePrimitive(UPrimitiveComponent* Primitive)
{
}

void FDisplayerScene::UpdateAllPrimitiveSceneInfos(FRDGBuilder& GraphBuilder, EUpdateAllPrimitiveSceneInfosAsyncOps AsyncOps)
{
	//FScene::UpdateAllPrimitiveSceneInfos(GraphBuilder, AsyncOps);
}

void FDisplayerScene::UpdatePrimitiveTransform(UPrimitiveComponent* Primitive)
{
	const float WorldTime = GetWorld()->GetTimeSeconds();
	const float DeltaTime = WorldTime - Primitive->LastSubmitTime;
	if (DeltaTime < -0.0001f || Primitive->LastSubmitTime < 0.0001f)
	{
		Primitive->LastSubmitTime = WorldTime;
	}
	else if (DeltaTime > 0.0001f)
	{
		Primitive->LastSubmitTime = WorldTime;
	}

	FDisplayerPrimitiveSceneProxy* Proxy = GetDisplayerSceneProxy(Primitive);

	if (Proxy)
	{
		if (Primitive->ShouldRecreateProxyOnUpdateTransform())
		{
			RemovePrimitive(Primitive);
			AddPrimitive(Primitive);
		}
		else
		{
			struct FUpdateTransformParameters
			{
				FDisplayerScene* Scene;
				FDisplayerPrimitiveSceneProxy* Proxy;
				FVector AttachmentRootPosition;
				FMatrix LocalToWorld;
				FBoxSphereBounds WorldBounds;
				FBoxSphereBounds LocalBounds;
			};

			FUpdateTransformParameters Params;
			Params.Scene = this;
			Params.Proxy = Proxy;
			Params.AttachmentRootPosition = Primitive->GetActorPositionForRenderer();
			Params.LocalToWorld = Primitive->GetRenderMatrix();
			Params.WorldBounds = Primitive->Bounds;
			Params.LocalBounds = Primitive->GetLocalBounds();

			bool bPerformUpdate = false;
			if (Proxy->WouldSetTransformBeInNeed(Params.LocalToWorld, Params.WorldBounds, Params.LocalBounds, Params.AttachmentRootPosition))
			{
				bPerformUpdate = true;
			}
			else
			{

			}

			if (bPerformUpdate)
			{
				ENQUEUE_RENDER_COMMAND(UpdateTransformCommand)(
					[&Params](FRHICommandListImmediate& RHICmdList)
					{
						Params.Scene->UpdatePrimitiveTransform_RenderThread(
							Params.Proxy,
							Params.LocalToWorld,
							Params.WorldBounds,
							Params.LocalBounds,
							Params.AttachmentRootPosition
						);
					}
				);
			}
		}
	}
	else
	{
		AddPrimitive(Primitive);
	}
}

void FDisplayerScene::UpdatePrimitiveInstances(UInstancedStaticMeshComponent* Primitive)
{
}

void FDisplayerScene::UpdatePrimitiveOcclusionBoundsSlack(UPrimitiveComponent* Primitive, float NewSlack)
{
}

void FDisplayerScene::UpdatePrimitiveAttachment(UPrimitiveComponent* Primitive)
{
}

void FDisplayerScene::UpdateCustomPrimitiveData(UPrimitiveComponent* Primitive)
{
}

void FDisplayerScene::UpdatePrimitiveDistanceFieldSceneData_GameThread(UPrimitiveComponent* Primitive)
{
}

FPrimitiveSceneInfo* FDisplayerScene::GetPrimitiveSceneInfo(int32 PrimitiveIndex)
{
	return nullptr;
}

FPrimitiveSceneInfo* FDisplayerScene::GetPrimitiveSceneInfo(const FPersistentPrimitiveIndex& PersistentPrimitiveIndex)
{
	return nullptr;
}

bool FDisplayerScene::GetPreviousLocalToWorld(const FPrimitiveSceneInfo* PrimitiveSceneInfo, FMatrix& OutPreviousLocalToWorld) const
{
	return false;
}

void FDisplayerScene::AddLight(ULightComponent* Light)
{
	FDisplayerLightSceneProxy* LightSceneProxy = CreateDisplayerSceneProxy(Light);
	if (LightSceneProxy)
	{
		LightSceneProxy->SetTransform(Light->GetComponentTransform().ToMatrixNoScale(), Light->GetLightPosition());

		LightSceneProxy->LightSceneInfo = new FDisplayerLightSceneInfo(LightSceneProxy, true);

		ENQUEUE_RENDER_COMMAND(FAddLightCommand)(
			[this, LightSceneInfo = LightSceneProxy->LightSceneInfo](FRHICommandListImmediate& RHICmdList)
			{
				AddLightSceneInfo_RenderThread(LightSceneInfo);
			}
		);
	}

}

void FDisplayerScene::RemoveLight(ULightComponent* Light)
{
}

void FDisplayerScene::AddInvisibleLight(ULightComponent* Light)
{
}

void FDisplayerScene::SetSkyLight(FSkyLightSceneProxy* Light)
{
}

void FDisplayerScene::DisableSkyLight(FSkyLightSceneProxy* Light)
{
}

bool FDisplayerScene::HasSkyLightRequiringLightingBuild() const
{
	return false;
}

bool FDisplayerScene::HasAtmosphereLightRequiringLightingBuild() const
{
	return false;
}

void FDisplayerScene::AddDecal(UDecalComponent* Component)
{
}

void FDisplayerScene::RemoveDecal(UDecalComponent* Component)
{
}

void FDisplayerScene::UpdateDecalTransform(UDecalComponent* Decal)
{
}

void FDisplayerScene::UpdateDecalFadeOutTime(UDecalComponent* Decal)
{
}

void FDisplayerScene::UpdateDecalFadeInTime(UDecalComponent* Decal)
{
}

void FDisplayerScene::BatchUpdateDecals(TArray<FDeferredDecalUpdateParams>&& UpdateParams)
{
}

void FDisplayerScene::AddReflectionCapture(UReflectionCaptureComponent* Component)
{
}

void FDisplayerScene::RemoveReflectionCapture(UReflectionCaptureComponent* Component)
{
}

void FDisplayerScene::GetReflectionCaptureData(UReflectionCaptureComponent* Component, FReflectionCaptureData& OutCaptureData)
{
}

void FDisplayerScene::UpdateReflectionCaptureTransform(UReflectionCaptureComponent* Component)
{
}

void FDisplayerScene::ReleaseReflectionCubemap(UReflectionCaptureComponent* CaptureComponent)
{
}

void FDisplayerScene::AddPlanarReflection(UPlanarReflectionComponent* Component)
{
}

void FDisplayerScene::RemovePlanarReflection(UPlanarReflectionComponent* Component)
{
}

void FDisplayerScene::UpdatePlanarReflectionTransform(UPlanarReflectionComponent* Component)
{
}

void FDisplayerScene::UpdateSceneCaptureContents(USceneCaptureComponent2D* CaptureComponent)
{
}

void FDisplayerScene::UpdateSceneCaptureContents(USceneCaptureComponentCube* CaptureComponent)
{
}

void FDisplayerScene::UpdatePlanarReflectionContents(UPlanarReflectionComponent* CaptureComponent, FSceneRenderer& MainSceneRenderer)
{
}

void FDisplayerScene::AllocateReflectionCaptures(const TArray<UReflectionCaptureComponent*>& NewCaptures, const TCHAR* CaptureReason, bool bVerifyOnlyCapturing, bool bCapturingForMobile, bool bInsideTick)
{
}

void FDisplayerScene::ResetReflectionCaptures(bool bOnlyIfOOM)
{
}

void FDisplayerScene::UpdateSkyCaptureContents(const USkyLightComponent* CaptureComponent, bool bCaptureEmissiveOnly, UTextureCube* SourceCubemap, FTexture* OutProcessedTexture, float& OutAverageBrightness, FSHVectorRGB3& OutIrradianceEnvironmentMap, TArray<FFloat16Color>* OutRadianceMap, FLinearColor* SpecifiedCubemapColorScale)
{
}

void FDisplayerScene::AllocateAndCaptureFrameSkyEnvMap(FRDGBuilder& GraphBuilder, FSceneRenderer& SceneRenderer, FViewInfo& MainView, bool bShouldRenderSkyAtmosphere, bool bShouldRenderVolumetricCloud, FInstanceCullingManager& InstanceCullingManager, FRDGExternalAccessQueue& ExternalAccessQueue)
{
}

void FDisplayerScene::ValidateSkyLightRealTimeCapture(FRDGBuilder& GraphBuilder, const FViewInfo& View, FRDGTextureRef SceneColorTexture)
{
}

void FDisplayerScene::ProcessAndRenderIlluminanceMeter(FRDGBuilder& GraphBuilder, TArrayView<FViewInfo> Views, FRDGTextureRef SceneColorTexture)
{
}

void FDisplayerScene::AddPrecomputedLightVolume(const FPrecomputedLightVolume* Volume)
{
}

void FDisplayerScene::RemovePrecomputedLightVolume(const FPrecomputedLightVolume* Volume)
{
}

bool FDisplayerScene::HasPrecomputedVolumetricLightmap_RenderThread() const
{
	return false;
}

void FDisplayerScene::AddPrecomputedVolumetricLightmap(const FPrecomputedVolumetricLightmap* Volume, bool bIsPersistentLevel)
{
}

void FDisplayerScene::RemovePrecomputedVolumetricLightmap(const FPrecomputedVolumetricLightmap* Volume)
{
}

void FDisplayerScene::AddRuntimeVirtualTexture(URuntimeVirtualTextureComponent* Component)
{
}

void FDisplayerScene::RemoveRuntimeVirtualTexture(URuntimeVirtualTextureComponent* Component)
{
}

void FDisplayerScene::GetRuntimeVirtualTextureHidePrimitiveMask(uint8& bHideMaskEditor, uint8& bHideMaskGame) const
{
}

void FDisplayerScene::InvalidateRuntimeVirtualTexture(URuntimeVirtualTextureComponent* Component, FBoxSphereBounds const& WorldBounds)
{
}

void FDisplayerScene::InvalidatePathTracedOutput()
{
}

void FDisplayerScene::InvalidateLumenSurfaceCache_GameThread(UPrimitiveComponent* Component)
{
}

void FDisplayerScene::GetPrimitiveUniformShaderParameters_RenderThread(const FPrimitiveSceneInfo* PrimitiveSceneInfo, bool& bHasPrecomputedVolumetricLightmap, FMatrix& PreviousLocalToWorld, int32& SingleCaptureIndex, bool& bOutputVelocity) const
{
}

void FDisplayerScene::UpdateLightTransform(ULightComponent* Light)
{
}

void FDisplayerScene::UpdateLightColorAndBrightness(ULightComponent* Light)
{
}

void FDisplayerScene::AddExponentialHeightFog(UExponentialHeightFogComponent* FogComponent)
{
}

void FDisplayerScene::RemoveExponentialHeightFog(UExponentialHeightFogComponent* FogComponent)
{
}

bool FDisplayerScene::HasAnyExponentialHeightFog() const
{
	return false;
}

void FDisplayerScene::AddHairStrands(FHairStrandsInstance* Proxy)
{
}

void FDisplayerScene::RemoveHairStrands(FHairStrandsInstance* Proxy)
{
}

void FDisplayerScene::GetLightIESAtlasSlot(const FLightSceneProxy* Proxy, FLightRenderParameters* Out)
{
}

void FDisplayerScene::GetRectLightAtlasSlot(const FRectLightSceneProxy* Proxy, FLightRenderParameters* Out)
{
}

void FDisplayerScene::AddSkyAtmosphere(FSkyAtmosphereSceneProxy* SkyAtmosphereSceneProxy, bool bStaticLightingBuilt)
{
}

void FDisplayerScene::RemoveSkyAtmosphere(FSkyAtmosphereSceneProxy* SkyAtmosphereSceneProxy)
{
}

void FDisplayerScene::AddSparseVolumeTextureViewer(FSparseVolumeTextureViewerSceneProxy* SVTV)
{
}

void FDisplayerScene::RemoveSparseVolumeTextureViewer(FSparseVolumeTextureViewerSceneProxy* SVTV)
{
}

void FDisplayerScene::SetPhysicsField(FPhysicsFieldSceneProxy* PhysicsFieldSceneProxy)
{
}

void FDisplayerScene::ResetPhysicsField()
{
}

void FDisplayerScene::ShowPhysicsField()
{
}

void FDisplayerScene::UpdatePhysicsField(FRDGBuilder& GraphBuilder, FViewInfo& View)
{
}

void FDisplayerScene::AddVolumetricCloud(FVolumetricCloudSceneProxy* VolumetricCloudSceneProxy)
{
}

void FDisplayerScene::RemoveVolumetricCloud(FVolumetricCloudSceneProxy* VolumetricCloudSceneProxy)
{
}

void FDisplayerScene::AddWindSource(UWindDirectionalSourceComponent* WindComponent)
{
}

void FDisplayerScene::RemoveWindSource(UWindDirectionalSourceComponent* WindComponent)
{
}

void FDisplayerScene::UpdateWindSource(UWindDirectionalSourceComponent* WindComponent)
{
}

const TArray<FWindSourceSceneProxy*>& FDisplayerScene::GetWindSources_RenderThread() const
{
	// TODO: insert return statement here
}

void FDisplayerScene::GetWindParameters(const FVector& Position, FVector& OutDirection, float& OutSpeed, float& OutMinGustAmt, float& OutMaxGustAmt) const
{
}

void FDisplayerScene::GetWindParameters_GameThread(const FVector& Position, FVector& OutDirection, float& OutSpeed, float& OutMinGustAmt, float& OutMaxGustAmt) const
{
}

void FDisplayerScene::GetDirectionalWindParameters(FVector& OutDirection, float& OutSpeed, float& OutMinGustAmt, float& OutMaxGustAmt) const
{
}

void FDisplayerScene::AddSpeedTreeWind(FVertexFactory* VertexFactory, const UStaticMesh* StaticMesh)
{
}

void FDisplayerScene::RemoveSpeedTreeWind_RenderThread(FVertexFactory* VertexFactory, const UStaticMesh* StaticMesh)
{
}

void FDisplayerScene::UpdateSpeedTreeWind(double CurrentTime)
{
}

FRHIUniformBuffer* FDisplayerScene::GetSpeedTreeUniformBuffer(const FVertexFactory* VertexFactory) const
{
	return nullptr;
}

void FDisplayerScene::DumpUnbuiltLightInteractions(FOutputDevice& Ar) const
{
}

void FDisplayerScene::UpdateParameterCollections(const TArray<FMaterialParameterCollectionInstanceResource*>& InParameterCollections)
{
}

bool FDisplayerScene::RequestGPUSceneUpdate(FPrimitiveSceneInfo& PrimitiveSceneInfo, EPrimitiveDirtyState PrimitiveDirtyState)
{
	return false;
}

void FDisplayerScene::RefreshNaniteRasterBins(FPrimitiveSceneInfo& PrimitiveSceneInfo)
{
}

void FDisplayerScene::GetRelevantLights(UPrimitiveComponent* Primitive, TArray<const ULightComponent*>* RelevantLights) const
{
}

void FDisplayerScene::SetPrecomputedVisibility(const FPrecomputedVisibilityHandler* InPrecomputedVisibilityHandler)
{
}

void FDisplayerScene::UpdateStaticDrawLists()
{
}

void FDisplayerScene::UpdateCachedRenderStates(FPrimitiveSceneProxy* SceneProxy)
{
}

void FDisplayerScene::UpdatePrimitiveSelectedState_RenderThread(const FPrimitiveSceneInfo* PrimitiveSceneInfo, bool bIsSelected)
{
}

void FDisplayerScene::UpdatePrimitiveVelocityState_RenderThread(FPrimitiveSceneInfo* PrimitiveSceneInfo, bool bIsBeingMoved)
{
}

void FDisplayerScene::Release()
{
}

void FDisplayerScene::OnWorldCleanup()
{
}

void FDisplayerScene::UpdateSceneSettings(AWorldSettings* WorldSettings)
{
}

void FDisplayerScene::SetFXSystem(FFXSystemInterface* InFXSystem)
{
}

FFXSystemInterface* FDisplayerScene::GetFXSystem()
{
	return nullptr;
}

void FDisplayerScene::AddPrimitiveSceneInfo_RenderThread(FDisplayerPrimitiveSceneInfo* PrimitiveSceneInfo)
{
	AddedPrimitiveSceneInfos.FindOrAdd(PrimitiveSceneInfo);
}

void FDisplayerScene::RemovePrimitiveSceneInfo_RenderThread(FDisplayerPrimitiveSceneInfo* PrimitiveSceneInfo)
{
	check(IsInRenderingThread());

	if (AddedPrimitiveSceneInfos.Remove(PrimitiveSceneInfo))
	{
		UpdateTransforms.Remove(PrimitiveSceneInfo->Proxy);

		delete PrimitiveSceneInfo->Proxy;
		delete PrimitiveSceneInfo;
	}
	else
	{
		check(RemovedPrimitiveSceneInfos.Find(PrimitiveSceneInfo) == nullptr);
		RemovedPrimitiveSceneInfos.FindOrAdd(PrimitiveSceneInfo);
	}
}

void FDisplayerScene::UpdatePrimitiveTransform_RenderThread(FDisplayerPrimitiveSceneProxy* PrimitiveSceneProxy, const FMatrix& InLocalToWorld, const FBoxSphereBounds& InWorldBounds, const FBoxSphereBounds& InLocalBounds, const FVector& InAttachmentRootPosition)
{
	check(IsInRenderingThread());

	UpdateTransforms.Update(PrimitiveSceneProxy, { InLocalToWorld, InWorldBounds, InLocalBounds, InAttachmentRootPosition });
}

void FDisplayerScene::AddLightSceneInfo_RenderThread(FDisplayerLightSceneInfo* LightSceneInfo)
{
	check(IsInRenderingThread());


}

void FDisplayerScene::RemoveLightSceneInfo_RenderThread(FDisplayerLightSceneInfo* LightSceneInfo)
{
}
