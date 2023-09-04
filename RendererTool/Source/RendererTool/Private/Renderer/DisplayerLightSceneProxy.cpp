#include "Renderer/DisplayerLightSceneProxy.h"
#include "Components/LightComponent.h"

FDisplayerLightSceneProxy::FDisplayerLightSceneProxy(const ULightComponent* InLightComponent)
	: LightComponent(InLightComponent)
	, Scene(InLightComponent->GetScene())
{
}

FDisplayerLightSceneProxy::~FDisplayerLightSceneProxy()
{
}

void FDisplayerLightSceneProxy::SetTransform(const FMatrix& InLocalToWorld, const FVector& InPosition)
{
	LocalToWorld = InLocalToWorld;
	WorldToLocal = InLocalToWorld.InverseFast();
	Position = InPosition;
}
