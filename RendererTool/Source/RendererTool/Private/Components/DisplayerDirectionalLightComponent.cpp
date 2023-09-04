#include "Components/DisplayerDirectionalLightComponent.h"
#include "Renderer/DisplayerLightSceneProxy.h"

class FDisplayerDirectionalLightSceneProxy : public FDisplayerLightSceneProxy
{
public:

    FDisplayerDirectionalLightSceneProxy(const ULightComponent* InLightComponent);

    virtual ~FDisplayerDirectionalLightSceneProxy();

};

UDisplayerDirectionalLightComponent::UDisplayerDirectionalLightComponent(const FObjectInitializer& ObjectInitializer)
{
}

UDisplayerDirectionalLightComponent::~UDisplayerDirectionalLightComponent()
{
}

FDisplayerLightSceneProxy* UDisplayerDirectionalLightComponent::CreateDisplayerSceneProxy() const
{
    return new FDisplayerDirectionalLightSceneProxy(this);
}
