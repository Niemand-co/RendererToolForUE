#include "Renderer/RendererToolScene.h"
#include "Renderer/DisplayerPrimitiveSceneInfo.h"
#include "Renderer/DisplayerPrimitiveSceneProxy.h"
#include "Components/DisplayerStaticMeshComponent.h"
#include "Components/DisplayerInstancedStaticMeshComponent.h"
#include "Components/DisplayerDirectionalLightComponent.h"
#include "Renderer/DisplayerLightSceneInfo.h"
#include "Renderer/DisplayerLightSceneProxy.h"

struct FPrimitiveInfoArraySortKey
{
	inline bool operator()(const FDisplayerPrimitiveSceneInfo& A, const FDisplayerPrimitiveSceneInfo& B)
	{
		SIZE_T HashA = A.Proxy->GetTypeHash();
		SIZE_T HashB = B.Proxy->GetTypeHash();

		if (HashA == HashB)
		{
			return A.RegistrationSerialNumber < B.RegistrationSerialNumber;
		}
		else
		{
			return HashA < HashB;
		}
	}
};

template<typename T>
static void TArraySwapElements(TArray<T>& Array, int32 i1, int32 i2)
{
	T tmp = Array[i1];
	Array[i1] = Array[i2];
	Array[i2] = tmp;
}

static void TBitArraySwapElements(TBitArray<>& Array, int32 i1, int32 i2)
{
	FBitReference BitRef1 = Array[i1];
	FBitReference BitRef2 = Array[i2];
	bool Bit1 = BitRef1;
	bool Bit2 = BitRef2;
	BitRef1 = Bit2;
	BitRef2 = Bit1;
}

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

	FThreadSafeCounter* AttachmentCounter = &Primitive->AttachmentCounter;
	ENQUEUE_RENDER_COMMAND(AddPrimitiveCommand)(
		[Params = MoveTemp(Params), Scene, PrimitiveInfo, AttachmentCounter](FRHICommandListImmediate& RHICmdList)
		{
			FDisplayerPrimitiveSceneProxy* SceneProxy = Params.PrimitiveSceneProxy;
			FScopeCycleCounter Context(SceneProxy->GetStatId());
			SceneProxy->SetTransform(Params.RenderMatrix, Params.WorldBounds, Params.LocalBounds, Params.AttachmentRootPosition);

			Scene->AddPrimitiveSceneInfo_RenderThread(PrimitiveInfo);
			AttachmentCounter->Increment();
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
	TArray<FDisplayerPrimitiveSceneInfo*> RemovedLocalPrimitiveSceneInfos;
	RemovedLocalPrimitiveSceneInfos.Reserve(RemovedPrimitiveSceneInfos.Num());
	for (FDisplayerPrimitiveSceneInfo* PrimitiveSceneInfo : RemovedPrimitiveSceneInfos)
	{
		RemovedLocalPrimitiveSceneInfos.Add(PrimitiveSceneInfo);
	}
	RemovedPrimitiveSceneInfos.Empty();

	RemovedLocalPrimitiveSceneInfos.Sort(FPrimitiveInfoArraySortKey());

	TArray<FDisplayerPrimitiveSceneInfo*> AddedLocalPrimitiveSceneInfos;
	AddedLocalPrimitiveSceneInfos.Reserve(AddedPrimitiveSceneInfos.Num());
	for (FDisplayerPrimitiveSceneInfo* PrimitiveSceneInfo : AddedPrimitiveSceneInfos)
	{
		AddedLocalPrimitiveSceneInfos.Add(PrimitiveSceneInfo);
	}
	AddedPrimitiveSceneInfos.Empty();

	AddedLocalPrimitiveSceneInfos.Sort(FPrimitiveInfoArraySortKey());

	TArray<FDisplayerPrimitiveSceneInfo*> SceneInfosWithStaticDrawListUpdate;
	TArray<FDisplayerPrimitiveSceneInfo*> SceneInfosWithoutStaticDrawListUpdate;

	SceneInfosWithStaticDrawListUpdate.Reserve(AddedLocalPrimitiveSceneInfos.Num() + UpdateTransforms.Num());
	SceneInfosWithoutStaticDrawListUpdate.Reserve(AddedLocalPrimitiveSceneInfos.Num() + UpdateTransforms.Num());

	while (RemovedLocalPrimitiveSceneInfos.Num())
	{
		int32 StartIndex = RemovedLocalPrimitiveSceneInfos.Num() - 1;
		SIZE_T ProxyHash = RemovedLocalPrimitiveSceneInfos[StartIndex]->Proxy->GetTypeHash();

		for (; StartIndex > 0 && RemovedLocalPrimitiveSceneInfos[StartIndex - 1]->Proxy->GetTypeHash() == ProxyHash; --StartIndex)
		{
		}

		/**
		* Example shows relationship between primitive info array and type offset table.
		* PrimitiveInfoArray: [0, 0, 0, 0, 1, 1, 1, 5, 2, 2, 3, 3, 3, 4, 6]
		* TypeOffsetTable: [4(0), 7(1), 8(5), 10(2), 13(3), 14(4), 15(6)] (PrefixSum(Hash))
		*/
		int32 BroadIndex = 0;
		for (int32 TypeIndex = 0; TypeIndex < TypeOffsetTable.Num(); ++TypeIndex)
		{
			if (TypeOffsetTable[TypeIndex].TypeHash == ProxyHash)
			{
				int32 InsertOffset = TypeOffsetTable[TypeIndex].Offset;
				int32 PreOffset = TypeIndex > 0 ? TypeOffsetTable[TypeIndex].Offset : 0;
				for (int32 CheckIndex = StartIndex; CheckIndex < RemovedLocalPrimitiveSceneInfos.Num(); ++CheckIndex)
				{
					int32 PrimitiveIndex = RemovedLocalPrimitiveSceneInfos[CheckIndex]->PackedIndex;
					check(PrimitiveIndex >= PreOffset && PrimitiveIndex < InsertOffset);
				}
				BroadIndex = TypeIndex;
				break;
			}
		}

		/**
		* Example of removing a primitive info.
		* [0, 0, X, 0, 1, 1, 1, 5, 2, 2, 3, 3, 3, 4, 6]
		* [0, 0, 0, X, 1, 1, 1, 5, 2, 2, 3, 3, 3, 4, 6]
		* [0, 0, 0, 1, 1, 1, X, 5, 2, 2, 3, 3, 3, 4, 6]
		* [0, 0, 0, 1, 1, 1, 5, X, 2, 2, 3, 3, 3, 4, 6]
		* [0, 0, 0, 1, 1, 1, 5, 2, 2, X, 3, 3, 3, 4, 6]
		* [0, 0, 0, 1, 1, 1, 5, 2, 2, 3, 3, 3, X, 4, 6]
		* [0, 0, 0, 1, 1, 1, 5, 2, 2, 3, 3, 3, 4, X, 6]
		* [0, 0, 0, 1, 1, 1, 5, 2, 2, 3, 3, 3, 4, 6, X]
		* [0, 0, 0, 1, 1, 1, 5, 2, 2, 3, 3, 3, 4, 6]
		*/
		for (int32 CheckIndex = StartIndex; CheckIndex < RemovedLocalPrimitiveSceneInfos.Num(); ++CheckIndex)
		{
			int32 SourceIndex = RemovedLocalPrimitiveSceneInfos[CheckIndex]->PackedIndex;

			for (int32 TypeIndex = SourceIndex == (TypeOffsetTable[BroadIndex].Offset - 1) ? BroadIndex + 1 : BroadIndex; TypeIndex < TypeOffsetTable.Num(); ++TypeIndex)
			{
				FTypeOffsetEntry& Entry = TypeOffsetTable[TypeIndex];
				int32 DestIndex = --Entry.Offset;

				Primitives[SourceIndex]->PackedIndex = DestIndex;
				Primitives[DestIndex]->PackedIndex = SourceIndex;

				TArraySwapElements(Primitives, SourceIndex, DestIndex);
				TArraySwapElements(PrimitiveTransforms, SourceIndex, DestIndex);
				TArraySwapElements(PrimitiveSceneProxies, SourceIndex, DestIndex);
				TArraySwapElements(PrimitiveBounds, SourceIndex, DestIndex);

				SourceIndex = DestIndex;
			}
		}

		int32 PreviousOffset = BroadIndex > 0 ? TypeOffsetTable[BroadIndex - 1].Offset : 0;
		int32 InsertionOffset = TypeOffsetTable[BroadIndex].Offset;
		if (PreviousOffset == InsertionOffset)
		{
			TypeOffsetTable.RemoveAt(BroadIndex);
		}

		for (int32 RemoveIndex = 0; RemoveIndex < RemovedLocalPrimitiveSceneInfos.Num(); ++RemoveIndex)
		{
			RemovedLocalPrimitiveSceneInfos[RemoveIndex]->PackedIndex = INDEX_NONE;
		}

		int32 RemoveCount = RemovedLocalPrimitiveSceneInfos.Num() - StartIndex;
		int32 SourceIndex = Primitives.Num() - RemoveCount;

		Primitives.RemoveAt(SourceIndex, RemoveCount, false);
		PrimitiveTransforms.RemoveAt(SourceIndex, RemoveCount, false);
		PrimitiveSceneProxies.RemoveAt(SourceIndex, RemoveCount, false);
		PrimitiveBounds.RemoveAt(SourceIndex, RemoveCount, false);

		RemovedLocalPrimitiveSceneInfos.RemoveAt(StartIndex, RemovedLocalPrimitiveSceneInfos.Num() - StartIndex, false);
	}

	Primitives.Reserve(Primitives.Num() + AddedLocalPrimitiveSceneInfos.Num());
	PrimitiveTransforms.Reserve(PrimitiveTransforms.Num() + AddedLocalPrimitiveSceneInfos.Num());
	PrimitiveBounds.Reserve(PrimitiveBounds.Num() + AddedLocalPrimitiveSceneInfos.Num());
	PrimitiveSceneProxies.Reserve(PrimitiveSceneProxies.Num() + AddedLocalPrimitiveSceneInfos.Num());

	while (AddedLocalPrimitiveSceneInfos.Num())
	{
		int32 StartIndex = AddedLocalPrimitiveSceneInfos.Num() - 1;
		SIZE_T ProxyHash = AddedLocalPrimitiveSceneInfos[StartIndex]->Proxy->GetTypeHash();

		for (; StartIndex > 0 && ProxyHash == AddedLocalPrimitiveSceneInfos[StartIndex - 1]->Proxy->GetTypeHash(); --StartIndex)
		{
		}

		int32 BroadIndex = -1;
		for (int32 TypeIndex = 0; TypeIndex < TypeOffsetTable.Num(); ++TypeIndex)
		{
			if (TypeOffsetTable[TypeIndex].TypeHash == ProxyHash)
			{
				BroadIndex = TypeIndex;
				break;
			}
		}

		for (int32 AddIndex = StartIndex; AddIndex < AddedLocalPrimitiveSceneInfos.Num(); ++AddIndex)
		{
			FDisplayerPrimitiveSceneInfo* Info = AddedLocalPrimitiveSceneInfos[AddIndex];
			Primitives.Add(Info);
			PrimitiveTransforms.Add(Info->Proxy->GetLocalToWorld());
			PrimitiveBounds.Add(Info->Proxy->GetWorldBounds());
			PrimitiveSceneProxies.Add(Info->Proxy);
		}

		if (BroadIndex == -1)
		{
			int32 Offset = TypeOffsetTable[TypeOffsetTable.Num() - 1].Offset;
			TypeOffsetTable.Add({ ProxyHash, Offset + AddedLocalPrimitiveSceneInfos.Num() });
			AddedLocalPrimitiveSceneInfos.RemoveAt(StartIndex, AddedLocalPrimitiveSceneInfos.Num() - StartIndex);
			continue;
		}

		/**
		* Example of adding a primitive info(X is type 0).
		* [0, 0, 0, 1, 1, 1, 5, 2, 2, 3, 3, 3, 4, 6, X]
		* [0, 0, 0, 1, 1, 1, 5, 2, 2, 3, 3, 3, 4, X, 6]
		* [0, 0, 0, 1, 1, 1, 5, 2, 2, 3, 3, 3, X, 4, 6]
		* [0, 0, 0, 1, 1, 1, 5, 2, 2, X, 3, 3, 3, 4, 6]
		* [0, 0, 0, 1, 1, 1, 5, X, 2, 2, 3, 3, 3, 4, 6]
		* [0, 0, 0, 1, 1, 1, X, 5, 2, 2, 3, 3, 3, 4, 6]
		* [0, 0, 0, X, 1, 1, 1, 5, 2, 2, 3, 3, 3, 4, 6]
		*/
		for (int32 AddIndex = StartIndex; AddIndex < AddedLocalPrimitiveSceneInfos.Num(); ++AddIndex)
		{
			int32 SourceIndex = AddIndex;

			for (int32 TypeIndex = TypeOffsetTable.Num(); TypeIndex >= BroadIndex; --TypeIndex)
			{
				FTypeOffsetEntry& Entry = TypeOffsetTable[TypeIndex];
				int32 DestIndex = Entry.Offset - 1;
				Entry.Offset++;

				Primitives[SourceIndex]->PackedIndex = DestIndex;
				Primitives[DestIndex]->PackedIndex = SourceIndex;

				TArraySwapElements(Primitives, SourceIndex, DestIndex);
				TArraySwapElements(PrimitiveTransforms, SourceIndex, DestIndex);
				TArraySwapElements(PrimitiveBounds, SourceIndex, DestIndex);
				TArraySwapElements(PrimitiveSceneProxies, SourceIndex, DestIndex);

				SourceIndex = DestIndex;
			}
		}

		for (int32 AddIndex = StartIndex; AddIndex < AddedLocalPrimitiveSceneInfos.Num(); ++AddIndex)
		{
			SceneInfosWithStaticDrawListUpdate.Push(AddedLocalPrimitiveSceneInfos[AddIndex]);
		}

		AddedLocalPrimitiveSceneInfos.RemoveAt(StartIndex, AddedLocalPrimitiveSceneInfos.Num() - StartIndex);
	}

	if (SceneInfosWithStaticDrawListUpdate.Num() > 0)
	{
		FDisplayerPrimitiveSceneInfo::AddToScene(this, SceneInfosWithStaticDrawListUpdate, EPrimitiveAddToSceneOps::All);
	}
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
	FDisplayerLightSceneProxy* LightSceneProxy = GetDisplayerSceneProxy(Light);

	if (LightSceneProxy)
	{
		FDisplayerLightSceneInfo* LightSceneInfo = LightSceneProxy->LightSceneInfo;

		EmptyDisplayerSceneProxy(Light);

		FDisplayerScene* Scene = this;
		ENQUEUE_RENDER_COMMAND(FRemoveLightCommand)(
			[Scene, LightSceneInfo](FRHICommandListImmediate& RHICmdList)
			{
				Scene->RemoveLightSceneInfo_RenderThread(LightSceneInfo);
			}
		);
	}
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
	FDisplayerLightSceneProxy* LightSceneProxy = GetDisplayerSceneProxy(Light);

	if (LightSceneProxy)
	{
		struct FUpdateParameters
		{
			FMatrix LocalToWorld;
			FVector4 Position;
		};
		FUpdateParameters Params = 
		{
			Light->GetComponentTransform().ToMatrixWithScale(),
			Light->GetLightPosition()
		};

		if (LightSceneProxy->LightSceneInfo->bVisible)
		{
			ENQUEUE_RENDER_COMMAND(FUpdateLightTransformCommand)(
				[Scene = this, &Params, LightSceneProxy](FRHICommandListImmediate& RHICmdList)
				{
					Scene->UpdateLightTransform_RenderThread(LightSceneProxy, Params.LocalToWorld, Params.Position);
				}
				);
		}
	}
}

void FDisplayerScene::UpdateLightColorAndBrightness(ULightComponent* Light)
{
	FDisplayerLightSceneProxy* LightSceneProxy = GetDisplayerSceneProxy(Light);

	if (LightSceneProxy)
	{
		struct FUpdateParameters
		{
			FLinearColor LightColor;
		};
		FUpdateParameters Params =
		{
			Light->GetColoredLightBrightness()
		};

		if (LightSceneProxy->LightSceneInfo->bVisible)
		{
			ENQUEUE_RENDER_COMMAND(FUpdateLightTransformCommand)(
				[Scene = this, &Params, LightSceneProxy](FRHICommandListImmediate& RHICmdList)
				{
					Scene->UpdateLightColorAndBrightness_RenderThread(LightSceneProxy, Params.LightColor);
				}
			);
		}
	}
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
	check(IsInRenderingThread());

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

	AddedLightSceneInfos.FindOrAdd(LightSceneInfo);
}

void FDisplayerScene::RemoveLightSceneInfo_RenderThread(FDisplayerLightSceneInfo* LightSceneInfo)
{
	check(IsInRenderingThread());

	if (AddedLightSceneInfos.Remove(LightSceneInfo))
	{
		UpdateLights.Remove(LightSceneInfo->Proxy);
	}
	else
	{
		check(RemovedLightSceneInfos.Find(LightSceneInfo) == nullptr)
		RemovedLightSceneInfos.FindOrAdd(LightSceneInfo);
	}
}

void FDisplayerScene::UpdateLightTransform_RenderThread(FDisplayerLightSceneProxy* LightSceneProxy, const FMatrix& InLocalToWorld, const FVector4& InLightPosition)
{
	check(IsInRenderingThread());

	FUpdateLightCommand* UpdateLightCommand = UpdateLights.Find(LightSceneProxy);
	if (!UpdateLightCommand)
	{
		UpdateLightCommand = UpdateLights.FindOrAdd(LightSceneProxy, FUpdateLightCommand({ InLocalToWorld, InLightPosition }));
	}
	else
	{
		UpdateLightCommand->Set({ InLocalToWorld, InLightPosition });
	}
}

void FDisplayerScene::UpdateLightColorAndBrightness_RenderThread(FDisplayerLightSceneProxy* LightSceneProxy, const FLinearColor& InLightColor)
{
	check(IsInRenderingThread());

	FUpdateLightCommand* UpdateLightCommand = UpdateLights.Find(LightSceneProxy);
	if (!UpdateLightCommand)
	{
		UpdateLightCommand = UpdateLights.FindOrAdd(LightSceneProxy, FUpdateLightCommand({ InLightColor }));
	}
	else
	{
		UpdateLightCommand->Set({ InLightColor });
	}
}