#include "Renderer/DisplayerPrimitiveSceneProxy.h"

static FBoxSphereBounds PadLocalBounds(const FBoxSphereBounds& InBounds, const FMatrix& LocalToWorld, float PadAmount)
{
	FVector Scale = LocalToWorld.GetScaleVector();
	FVector Padder = FVector(
		PadAmount * (Scale.X > 0.0f ? Scale.X : 0.0f),
		PadAmount * (Scale.Y > 0.0f ? Scale.Y : 0.0f),
		PadAmount * (Scale.Y > 0.0f ? Scale.Y : 0.0f)
	);

	FBoxSphereBounds Result = InBounds;
	Result.BoxExtent += Scale + Padder;
	Result.SphereRadius += Padder.Length();
}

FDisplayerPrimitiveSceneProxy::FDisplayerPrimitiveSceneProxy(const UPrimitiveComponent* InComponent, FName ResourceName)
	: SceneInfo(nullptr)
	, StatId()
{
#if STATS
	{
		UObject const* StatObject = InComponent->AdditionalStatObject(); // prefer the additional object, this is usually the thing related to the component
		if (!StatObject)
		{
			StatObject = InComponent;
		}
		StatId = StatObject->GetStatID(true);
	}
#endif
}

FDisplayerPrimitiveSceneProxy::~FDisplayerPrimitiveSceneProxy()
{
}

void FDisplayerPrimitiveSceneProxy::SetTransform(const FMatrix& InLocalToWorld, const FBoxSphereBounds& InBounds, const FBoxSphereBounds& InLocalBounds, FVector InActorPosition)
{
}

bool FDisplayerPrimitiveSceneProxy::WouldSetTransformBeInNeed(const FMatrix& InLocalToWorld, const FBoxSphereBounds& InWorldBounds, const FBoxSphereBounds& InLocalBounds, FVector InActorPosition)
{
	if (ActorPosition != InActorPosition)
	{
		return true;
	}

	if (WorldBounds != InWorldBounds)
	{
		return true;
	}

	if (LocalBounds != InLocalBounds)
	{
		return true;
	}

	if (LocalToWorld != InLocalToWorld)
	{
		return true;
	}
}
