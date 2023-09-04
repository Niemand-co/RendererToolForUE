#include "Components/DisplayerLightComponent.h"
#include "Components/DisplayerDirectionalLightComponent.h"

FDisplayerLightSceneProxy* CreateDisplayerSceneProxy(ULightComponent* Light)
{
	FDisplayerLightSceneProxy* LightProxy = nullptr;
	if (UDisplayerDirectionalLightComponent* DirectionalLightComponent = Cast<UDisplayerDirectionalLightComponent>(Light))
	{
		LightProxy = DirectionalLightComponent->CreateDisplayerSceneProxy();
		DirectionalLightComponent->DisplayerSceneProxy = LightProxy;
	}
	else
	{
		check(false);
	}

	return LightProxy;
}

FDisplayerLightSceneProxy* GetDisplayerSceneProxy(ULightComponent* Light)
{
	FDisplayerLightSceneProxy* LightProxy = nullptr;
	if (UDisplayerDirectionalLightComponent* DirectionalLightComponent = Cast<UDisplayerDirectionalLightComponent>(Light))
	{
		LightProxy = DirectionalLightComponent->DisplayerSceneProxy;
	}
	else
	{
		check(false);
	}

	return LightProxy;
}

void EmptyDisplayerSceneProxy(ULightComponent* Light)
{
	if (UDisplayerDirectionalLightComponent* DirectionalLightComponent = Cast<UDisplayerDirectionalLightComponent>(Light))
	{
		DirectionalLightComponent->DisplayerSceneProxy = nullptr;
	}
	else
	{
		check(false);
	}
}
