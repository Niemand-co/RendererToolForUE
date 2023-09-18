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

	virtual SIZE_T GetTypeHash() const = 0;

	//Accessors
	inline const FVector GetPosition() const { return ActorPosition; }
	inline const FMatrix GetLocalToWorld() const { return LocalToWorld; }
	inline const FBoxSphereBounds GetWorldBounds() const { return WorldBounds; }
	inline const FBoxSphereBounds GetLocalBounds() const { return LocalBounds; }

	virtual int32 GetNumMeshBatches() const = 0;

	virtual void DrawStaticMeshElements() = 0;

	virtual bool GetMeshElements(int32 LODIndex, FMeshBatch& OutMeshBatch) = 0;

protected:

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