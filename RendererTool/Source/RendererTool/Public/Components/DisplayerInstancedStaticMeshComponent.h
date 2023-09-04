#pragma once

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/DisplayerMeshComponent.h"

class UDisplayerInstancedStaticMeshComponent : public UInstancedStaticMeshComponent
{
public:

	class FDisplayerPrimitiveSceneProxy* DisplayerSceneProxy;

public:

	UDisplayerInstancedStaticMeshComponent(const FObjectInitializer& ObjectInitializer);

	virtual ~UDisplayerInstancedStaticMeshComponent();

	FDisplayerPrimitiveSceneProxy* CreateDisplayerSceneProxy();

};