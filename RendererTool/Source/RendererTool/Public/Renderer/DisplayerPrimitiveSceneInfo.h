#pragma once

#include "RenderDeferredCleanup.h"

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

	TArray<class FStaticMeshBatch> StaticMeshes;

public:

	FDisplayerPrimitiveSceneInfo(UPrimitiveComponent* InComponent, FDisplayerScene* InScene);
	virtual ~FDisplayerPrimitiveSceneInfo();

};