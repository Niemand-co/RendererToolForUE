#pragma once

#include "LightSceneProxy.h"

class FDisplayerLightSceneProxy
{
public:

	class FDisplayerLightSceneInfo* LightSceneInfo;

public:

	FDisplayerLightSceneProxy(const ULightComponent* InLightComponent);

	virtual ~FDisplayerLightSceneProxy();

	void SetTransform(const FMatrix& InLocalToWorld, const FVector& InPosition);

protected:

	const class ULightComponent* LightComponent;

	class FSceneInterface *Scene;

	FMatrix WorldToLocal;

	FMatrix LocalToWorld;

	FVector Position;

};