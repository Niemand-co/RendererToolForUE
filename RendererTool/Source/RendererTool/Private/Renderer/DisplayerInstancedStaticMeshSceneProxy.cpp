#include "Renderer/DisplayerInstancedStaticMeshSceneProxy.h"
#include "Components/DisplayerInstancedStaticMeshComponent.h"

FDisplayerInstancedStaticMeshSceneProxy::FDisplayerInstancedStaticMeshSceneProxy(const UInstancedStaticMeshComponent* InComponent)
	: FDisplayerStaticMeshSceneProxy(InComponent)
{
}

FDisplayerInstancedStaticMeshSceneProxy::~FDisplayerInstancedStaticMeshSceneProxy()
{
}

SIZE_T FDisplayerInstancedStaticMeshSceneProxy::GetTypeHash() const
{
	static SIZE_T UniquePointer;
	return (SIZE_T)&UniquePointer;
}

void FDisplayerInstancedStaticMeshSceneProxy::DrawStaticMeshElements()
{
}