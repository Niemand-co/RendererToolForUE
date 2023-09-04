#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DisplayerMeshComponent.h"

class UDisplayerStaticMeshComponent : public UStaticMeshComponent
{
public:

	class FDisplayerPrimitiveSceneProxy* DisplayerSceneProxy;

public:

	UDisplayerStaticMeshComponent(const FObjectInitializer& ObjectInitializer);

	virtual ~UDisplayerStaticMeshComponent();

	FDisplayerPrimitiveSceneProxy* CreateDisplayerSceneProxy();

};