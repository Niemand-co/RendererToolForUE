#include "Renderer/DisplayerStaticMeshSceneProxy.h"
#include "Components/DisplayerStaticMeshComponent.h"

FDisplayerStaticMeshSceneProxy::FDisplayerStaticMeshSceneProxy(const UStaticMeshComponent* InComponent)
	: FDisplayerPrimitiveSceneProxy(InComponent, InComponent->GetStaticMesh()->GetFName())
{
}

FDisplayerStaticMeshSceneProxy::~FDisplayerStaticMeshSceneProxy()
{
}
