#pragma once

#include "CoreMinimal.h"
#include "SceneInterface.h"

class FDisplayerPrimitiveSceneProxy;
class FDisplayerPrimitiveSceneInfo;
class FDisplayerLightSceneProxy;
class FDisplayerLightSceneInfo;

class FDisplayerScene : public FSceneInterface
{
public:

	UWorld* World;

	struct FUpdateTransformCommand
	{
		FMatrix LocalToWorld;
		FBoxSphereBounds WorldBounds;
		FBoxSphereBounds LocalBounds;
		FVector AttachmentRootPosition;
	};

	struct FUpdateLightCommand
	{
		FMatrix LocalToWorld;
		FVector LightPosition;
		FLinearColor LightColor;
	};

	Experimental::TRobinHoodHashSet<FDisplayerPrimitiveSceneInfo*> AddedPrimitiveSceneInfos;
	Experimental::TRobinHoodHashSet<FDisplayerPrimitiveSceneInfo*> RemovedPrimitiveSceneInfos;
	Experimental::TRobinHoodHashMap<FDisplayerPrimitiveSceneProxy*, FUpdateTransformCommand> UpdateTransforms;

	Experimental::TRobinHoodHashSet<FDisplayerLightSceneInfo*> AddedLightSceneInfos;
	Experimental::TRobinHoodHashSet<FDisplayerLightSceneInfo*> RemovedLightSceneInfos;
	Experimental::TRobinHoodHashMap<FDisplayerLightSceneProxy*, FUpdateLightCommand> UpdateLights;

public:

	FDisplayerScene(UWorld* InWorld, ERHIFeatureLevel::Type InFeatureLevel);
	virtual ~FDisplayerScene();

	// FSceneInterface interface.
	virtual void AddPrimitive(UPrimitiveComponent* Primitive) override;
	virtual void RemovePrimitive(UPrimitiveComponent* Primitive) override;
	virtual void ReleasePrimitive(UPrimitiveComponent* Primitive) override;
	virtual void UpdateAllPrimitiveSceneInfos(FRDGBuilder& GraphBuilder, EUpdateAllPrimitiveSceneInfosAsyncOps AsyncOps = EUpdateAllPrimitiveSceneInfosAsyncOps::None) override;
	virtual void UpdatePrimitiveTransform(UPrimitiveComponent* Primitive) override;
	virtual void UpdatePrimitiveInstances(UInstancedStaticMeshComponent* Primitive) override;
	virtual void UpdatePrimitiveOcclusionBoundsSlack(UPrimitiveComponent* Primitive, float NewSlack) override;
	virtual void UpdatePrimitiveAttachment(UPrimitiveComponent* Primitive) override;
	virtual void UpdateCustomPrimitiveData(UPrimitiveComponent* Primitive) override;
	virtual void UpdatePrimitiveDistanceFieldSceneData_GameThread(UPrimitiveComponent* Primitive) override;

	virtual FPrimitiveSceneInfo* GetPrimitiveSceneInfo(int32 PrimitiveIndex) override;
	virtual FPrimitiveSceneInfo* GetPrimitiveSceneInfo(const FPersistentPrimitiveIndex& PersistentPrimitiveIndex) override;

	virtual bool GetPreviousLocalToWorld(const FPrimitiveSceneInfo* PrimitiveSceneInfo, FMatrix& OutPreviousLocalToWorld) const override;
	virtual void AddLight(ULightComponent* Light) override;
	virtual void RemoveLight(ULightComponent* Light) override;
	virtual void AddInvisibleLight(ULightComponent* Light) override;
	virtual void SetSkyLight(FSkyLightSceneProxy* Light) override;
	virtual void DisableSkyLight(FSkyLightSceneProxy* Light) override;
	virtual bool HasSkyLightRequiringLightingBuild() const override;
	virtual bool HasAtmosphereLightRequiringLightingBuild() const override;
	virtual void AddDecal(UDecalComponent* Component) override;
	virtual void RemoveDecal(UDecalComponent* Component) override;
	virtual void UpdateDecalTransform(UDecalComponent* Decal) override;
	virtual void UpdateDecalFadeOutTime(UDecalComponent* Decal) override;
	virtual void UpdateDecalFadeInTime(UDecalComponent* Decal) override;
	virtual void BatchUpdateDecals(TArray<FDeferredDecalUpdateParams>&& UpdateParams) override;
	virtual void AddReflectionCapture(UReflectionCaptureComponent* Component) override;
	virtual void RemoveReflectionCapture(UReflectionCaptureComponent* Component) override;
	virtual void GetReflectionCaptureData(UReflectionCaptureComponent* Component, class FReflectionCaptureData& OutCaptureData) override;
	virtual void UpdateReflectionCaptureTransform(UReflectionCaptureComponent* Component) override;
	virtual void ReleaseReflectionCubemap(UReflectionCaptureComponent* CaptureComponent) override;
	virtual void AddPlanarReflection(class UPlanarReflectionComponent* Component) override;
	virtual void RemovePlanarReflection(UPlanarReflectionComponent* Component) override;
	virtual void UpdatePlanarReflectionTransform(UPlanarReflectionComponent* Component) override;
	virtual void UpdateSceneCaptureContents(class USceneCaptureComponent2D* CaptureComponent) override;
	virtual void UpdateSceneCaptureContents(class USceneCaptureComponentCube* CaptureComponent) override;
	virtual void UpdatePlanarReflectionContents(UPlanarReflectionComponent* CaptureComponent, FSceneRenderer& MainSceneRenderer) override;
	virtual void AllocateReflectionCaptures(const TArray<UReflectionCaptureComponent*>& NewCaptures, const TCHAR* CaptureReason, bool bVerifyOnlyCapturing, bool bCapturingForMobile, bool bInsideTick) override;
	virtual void ResetReflectionCaptures(bool bOnlyIfOOM) override;
	virtual void UpdateSkyCaptureContents(const USkyLightComponent* CaptureComponent, bool bCaptureEmissiveOnly, UTextureCube* SourceCubemap, FTexture* OutProcessedTexture, float& OutAverageBrightness, FSHVectorRGB3& OutIrradianceEnvironmentMap, TArray<FFloat16Color>* OutRadianceMap, FLinearColor* SpecifiedCubemapColorScale) override;
	virtual void AllocateAndCaptureFrameSkyEnvMap(FRDGBuilder& GraphBuilder, FSceneRenderer& SceneRenderer, FViewInfo& MainView, bool bShouldRenderSkyAtmosphere, bool bShouldRenderVolumetricCloud, FInstanceCullingManager& InstanceCullingManager, FRDGExternalAccessQueue& ExternalAccessQueue) override;
	virtual void ValidateSkyLightRealTimeCapture(FRDGBuilder& GraphBuilder, const FViewInfo& View, FRDGTextureRef SceneColorTexture) override;
	virtual void ProcessAndRenderIlluminanceMeter(FRDGBuilder& GraphBuilder, TArrayView<FViewInfo> Views, FRDGTextureRef SceneColorTexture);
	virtual void AddPrecomputedLightVolume(const class FPrecomputedLightVolume* Volume) override;
	virtual void RemovePrecomputedLightVolume(const class FPrecomputedLightVolume* Volume) override;
	virtual bool HasPrecomputedVolumetricLightmap_RenderThread() const override;
	virtual void AddPrecomputedVolumetricLightmap(const class FPrecomputedVolumetricLightmap* Volume, bool bIsPersistentLevel) override;
	virtual void RemovePrecomputedVolumetricLightmap(const class FPrecomputedVolumetricLightmap* Volume) override;
	virtual void AddRuntimeVirtualTexture(class URuntimeVirtualTextureComponent* Component) override;
	virtual void RemoveRuntimeVirtualTexture(class URuntimeVirtualTextureComponent* Component) override;
	virtual void GetRuntimeVirtualTextureHidePrimitiveMask(uint8& bHideMaskEditor, uint8& bHideMaskGame) const override;
	virtual void InvalidateRuntimeVirtualTexture(class URuntimeVirtualTextureComponent* Component, FBoxSphereBounds const& WorldBounds) override;
	virtual void InvalidatePathTracedOutput() override;
	virtual void InvalidateLumenSurfaceCache_GameThread(UPrimitiveComponent* Component) override;
	virtual void GetPrimitiveUniformShaderParameters_RenderThread(const FPrimitiveSceneInfo* PrimitiveSceneInfo, bool& bHasPrecomputedVolumetricLightmap, FMatrix& PreviousLocalToWorld, int32& SingleCaptureIndex, bool& bOutputVelocity) const override;
	virtual void UpdateLightTransform(ULightComponent* Light) override;
	virtual void UpdateLightColorAndBrightness(ULightComponent* Light) override;
	virtual void AddExponentialHeightFog(UExponentialHeightFogComponent* FogComponent) override;
	virtual void RemoveExponentialHeightFog(UExponentialHeightFogComponent* FogComponent) override;
	virtual bool HasAnyExponentialHeightFog() const override;

	virtual void AddHairStrands(FHairStrandsInstance* Proxy) override;
	virtual void RemoveHairStrands(FHairStrandsInstance* Proxy) override;

	virtual void GetLightIESAtlasSlot(const FLightSceneProxy* Proxy, FLightRenderParameters* Out) override;
	virtual void GetRectLightAtlasSlot(const FRectLightSceneProxy* Proxy, FLightRenderParameters* Out) override;

	virtual void AddSkyAtmosphere(FSkyAtmosphereSceneProxy* SkyAtmosphereSceneProxy, bool bStaticLightingBuilt) override;
	virtual void RemoveSkyAtmosphere(FSkyAtmosphereSceneProxy* SkyAtmosphereSceneProxy) override;
	virtual FSkyAtmosphereRenderSceneInfo* GetSkyAtmosphereSceneInfo() override { return nullptr; }
	virtual const FSkyAtmosphereRenderSceneInfo* GetSkyAtmosphereSceneInfo() const override { return nullptr; }

	virtual void AddSparseVolumeTextureViewer(FSparseVolumeTextureViewerSceneProxy* SVTV) override;
	virtual void RemoveSparseVolumeTextureViewer(FSparseVolumeTextureViewerSceneProxy* SVTV) override;

	virtual void SetPhysicsField(class FPhysicsFieldSceneProxy* PhysicsFieldSceneProxy) override;
	virtual void ResetPhysicsField() override;
	virtual void ShowPhysicsField() override;
	virtual void UpdatePhysicsField(FRDGBuilder& GraphBuilder, FViewInfo& View) override;

	virtual void AddVolumetricCloud(FVolumetricCloudSceneProxy* VolumetricCloudSceneProxy) override;
	virtual void RemoveVolumetricCloud(FVolumetricCloudSceneProxy* VolumetricCloudSceneProxy) override;
	virtual FVolumetricCloudRenderSceneInfo* GetVolumetricCloudSceneInfo() override { return nullptr; }
	virtual const FVolumetricCloudRenderSceneInfo* GetVolumetricCloudSceneInfo() const override { return nullptr; }

	virtual void AddWindSource(UWindDirectionalSourceComponent* WindComponent) override;
	virtual void RemoveWindSource(UWindDirectionalSourceComponent* WindComponent) override;
	virtual void UpdateWindSource(UWindDirectionalSourceComponent* WindComponent) override;
	virtual const TArray<FWindSourceSceneProxy*>& GetWindSources_RenderThread() const override;
	virtual void GetWindParameters(const FVector& Position, FVector& OutDirection, float& OutSpeed, float& OutMinGustAmt, float& OutMaxGustAmt) const override;
	virtual void GetWindParameters_GameThread(const FVector& Position, FVector& OutDirection, float& OutSpeed, float& OutMinGustAmt, float& OutMaxGustAmt) const override;
	virtual void GetDirectionalWindParameters(FVector& OutDirection, float& OutSpeed, float& OutMinGustAmt, float& OutMaxGustAmt) const override;
	virtual void AddSpeedTreeWind(FVertexFactory* VertexFactory, const UStaticMesh* StaticMesh) override;
	virtual void RemoveSpeedTreeWind_RenderThread(FVertexFactory* VertexFactory, const UStaticMesh* StaticMesh) override;
	virtual void UpdateSpeedTreeWind(double CurrentTime) override;
	virtual FRHIUniformBuffer* GetSpeedTreeUniformBuffer(const FVertexFactory* VertexFactory) const override;
	virtual void DumpUnbuiltLightInteractions(FOutputDevice& Ar) const override;
	virtual void UpdateParameterCollections(const TArray<FMaterialParameterCollectionInstanceResource*>& InParameterCollections) override;

	virtual bool RequestGPUSceneUpdate(FPrimitiveSceneInfo& PrimitiveSceneInfo, EPrimitiveDirtyState PrimitiveDirtyState) override;

	virtual void RefreshNaniteRasterBins(FPrimitiveSceneInfo& PrimitiveSceneInfo) override;

	virtual void GetRelevantLights(UPrimitiveComponent* Primitive, TArray<const ULightComponent*>* RelevantLights) const override;

	/** Sets the precomputed visibility handler for the scene, or NULL to clear the current one. */
	virtual void SetPrecomputedVisibility(const FPrecomputedVisibilityHandler* InPrecomputedVisibilityHandler) override;

	/** Updates all static draw lists. */
	virtual void UpdateStaticDrawLists() override;

	/** Update render states that possibly cached inside renderer, like mesh draw commands. More lightweight than re-registering the scene proxy. */
	virtual void UpdateCachedRenderStates(FPrimitiveSceneProxy* SceneProxy) override;

	/** Updates PrimitivesSelected array for this PrimitiveSceneInfo */
	virtual void UpdatePrimitiveSelectedState_RenderThread(const FPrimitiveSceneInfo* PrimitiveSceneInfo, bool bIsSelected) override;
	virtual void UpdatePrimitiveVelocityState_RenderThread(FPrimitiveSceneInfo* PrimitiveSceneInfo, bool bIsBeingMoved) override;

	virtual void Release() override;
	virtual UWorld* GetWorld() const override { return World; }

	virtual bool RequiresHitProxies() const override;

	/**
	* Return the scene to be used for rendering
	*/
	virtual class FScene* GetRenderScene() override
	{
		return nullptr;
	}
	virtual void OnWorldCleanup() override;


	virtual void UpdateSceneSettings(AWorldSettings* WorldSettings) override;

	virtual class FGPUSkinCache* GetGPUSkinCache() override
	{
		return nullptr;
	}

	virtual void GetComputeTaskWorkers(TArray<class IComputeTaskWorker*>& OutWorkers) const override
	{
		//OutWorkers = ComputeTaskWorkers;
	}

	/**
	 * Sets the FX system associated with the scene.
	 */
	virtual void SetFXSystem(class FFXSystemInterface* InFXSystem) override;

	/**
	 * Get the FX system associated with the scene.
	 */
	virtual class FFXSystemInterface* GetFXSystem() override;

	virtual bool HasAnyLights() const override
	{
		check(IsInGameThread());
		return true;
		//return NumVisibleLights_GameThread > 0 || NumEnabledSkylights_GameThread > 0;
	}

private:

	void AddPrimitiveSceneInfo_RenderThread(FDisplayerPrimitiveSceneInfo* PrimitiveSceneInfo);

	void RemovePrimitiveSceneInfo_RenderThread(FDisplayerPrimitiveSceneInfo* PrimitiveSceneInfo);

	void UpdatePrimitiveTransform_RenderThread(FDisplayerPrimitiveSceneProxy* PrimitiveSceneProxy, const FMatrix& InLocalToWorld, const FBoxSphereBounds& InWorldBounds, const FBoxSphereBounds& InLocalBounds, const FVector& InAttachmentRootPosition);

	void AddLightSceneInfo_RenderThread(FDisplayerLightSceneInfo* LightSceneInfo);

	void RemoveLightSceneInfo_RenderThread(FDisplayerLightSceneInfo* LightSceneInfo);

	void UpdateLight_RenderThread(FDisplayerLightSceneProxy* LightSceneProxy, const FMatrix& InLocalToWorld, const FVector& InLightPosition, const FLinearColor& InLightColor);

};