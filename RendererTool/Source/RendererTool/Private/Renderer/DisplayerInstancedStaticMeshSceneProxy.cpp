#include "Renderer/DisplayerInstancedStaticMeshSceneProxy.h"
#include "Components/DisplayerInstancedStaticMeshComponent.h"

FDisplayerInstancedStaticMeshSceneProxy::FDisplayerInstancedStaticMeshSceneProxy(const UInstancedStaticMeshComponent* InComponent)
	: FDisplayerStaticMeshSceneProxy(InComponent)
{
}

FDisplayerInstancedStaticMeshSceneProxy::~FDisplayerInstancedStaticMeshSceneProxy()
{
}
