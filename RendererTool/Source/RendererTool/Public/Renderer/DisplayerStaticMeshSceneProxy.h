#pragma once

#include "DisplayerPrimitiveSceneProxy.h"

class FDisplayerStaticMeshSceneProxy : public FDisplayerPrimitiveSceneProxy
{
public:

	

public:

	FDisplayerStaticMeshSceneProxy(const class UStaticMeshComponent* InComponent);

	virtual ~FDisplayerStaticMeshSceneProxy();

};