#pragma once

#include "Components/DirectionalLightComponent.h"
#include "Components/DisplayerLightComponent.h"

class UDisplayerDirectionalLightComponent : public UDirectionalLightComponent
{
public:

	class FDisplayerLightSceneProxy* DisplayerSceneProxy;

public:

	UDisplayerDirectionalLightComponent(const FObjectInitializer& ObjectInitializer);

	virtual ~UDisplayerDirectionalLightComponent();

	class FDisplayerLightSceneProxy* CreateDisplayerSceneProxy() const;

};