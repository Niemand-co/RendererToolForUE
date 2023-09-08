#pragma once

#include "LightSceneProxy.h"

class FSceneInterface;

class FDisplayerLightSceneProxy
{
public:

	class FDisplayerLightSceneInfo* LightSceneInfo;

public:

	FDisplayerLightSceneProxy(const ULightComponent* InLightComponent);

	virtual ~FDisplayerLightSceneProxy();

	void SetTransform(const FMatrix& InLocalToWorld, const FVector& InPosition);

	FSceneInterface* GetScene() { return Scene; }
	const FSceneInterface* GetScene() const { return Scene; }

protected:

	const class ULightComponent* LightComponent;

	FSceneInterface *Scene;

	FMatrix WorldToLocal;

	FMatrix LocalToWorld;

	FVector Position;

};