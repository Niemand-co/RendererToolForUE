#pragma once

#include "RenderDeferredCleanup.h"

enum class EPrimitiveAddToSceneOps : uint8
{
	None = 0,
	AddStaticMeshes = 1,
	CacheDrawCommands = 2,
	CreatePrimitiveLightInteraction = 4,
	All = AddStaticMeshes | CacheDrawCommands | CreatePrimitiveLightInteraction,
};

class FDisplayerPrimitiveSceneInfo : public FDeferredCleanupInterface
{
	friend class FDisplayerSceneRenderer;

public:

	class FDisplayerPrimitiveSceneProxy* Proxy;

	FPrimitiveComponentId PrimitiveId;

	class FDisplayerScene* Scene;

	TArray<TRefCountPtr<HHitProxy>> HitProxies;

	HHitProxy* DefaultDynamicHitProxy;

	FHitProxyId DefaultDynamicHitProxyId;

	TArray<class FDisplayerStaticMeshBatch> StaticMeshes;

	TArray<class FStaticMeshBatchRelevance> StaticMeshRelevances;

	int32 PackedIndex;

	int32 RegistrationSerialNumber = -1;

public:

	FDisplayerPrimitiveSceneInfo(UPrimitiveComponent* InComponent, FDisplayerScene* InScene);
	virtual ~FDisplayerPrimitiveSceneInfo();

	static void AddToScene(FDisplayerScene* Scene, TArray<FDisplayerPrimitiveSceneInfo*>& SceneInfos, EPrimitiveAddToSceneOps AddOps);

	static void AddStaticMeshes(FDisplayerScene* Scene, TArrayView<FDisplayerPrimitiveSceneInfo*> SceneInfos, bool bCacheDrawCommands);

	static void CacheDrawCommands(FDisplayerScene* Scene, TArrayView<FDisplayerPrimitiveSceneInfo*> SceneInfos);
};