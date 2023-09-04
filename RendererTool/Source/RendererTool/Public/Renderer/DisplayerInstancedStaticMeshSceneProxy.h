#pragma once

#include "Renderer/DisplayerStaticMeshSceneProxy.h"

class FDisplayerInstancedStaticMeshSceneProxy : public FDisplayerStaticMeshSceneProxy
{
public:

	FDisplayerInstancedStaticMeshSceneProxy(const class UInstancedStaticMeshComponent* InComponent);

	virtual ~FDisplayerInstancedStaticMeshSceneProxy();

};