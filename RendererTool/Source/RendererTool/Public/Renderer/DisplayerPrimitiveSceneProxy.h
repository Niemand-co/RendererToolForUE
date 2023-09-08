#pragma once

#include "PrimitiveSceneProxy.h"

class FDisplayerPrimitiveSceneInfo;

class FDisplayerPrimitiveSceneProxy
{
	friend class FDisplayerScene;

public:

	FDisplayerPrimitiveSceneProxy(const UPrimitiveComponent* InComponent, FName ResourceName = NAME_None);

	FDisplayerPrimitiveSceneProxy(FDisplayerPrimitiveSceneProxy const&) = default;

	virtual ~FDisplayerPrimitiveSceneProxy();

	TStatId GetStatId() const { return StatId; }

private:

	FDisplayerPrimitiveSceneInfo* SceneInfo;

	TStatId StatId;

private:

	FVector ActorPosition;

	FMatrix LocalToWorld;

	FBoxSphereBounds WorldBounds;

	FBoxSphereBounds LocalBounds;

private:

	void SetTransform(const FMatrix& InLocalToWorld, const FBoxSphereBounds& InBounds, const FBoxSphereBounds& InLocalBounds, FVector InActorPosition);

	bool WouldSetTransformBeInNeed(const FMatrix& InLocalToWorld, const FBoxSphereBounds& InWorldBounds, const FBoxSphereBounds& InLocalBounds, FVector InActorPosition);

};