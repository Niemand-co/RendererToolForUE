#include "Components/DisplayerMeshComponent.h"
#include "Components/DisplayerStaticMeshComponent.h"
#include "Components/DisplayerInstancedStaticMeshComponent.h"

FDisplayerPrimitiveSceneProxy* CreateDisplayerSceneProxy(UPrimitiveComponent* Primitive)
{
	FDisplayerPrimitiveSceneProxy* PrimitiveProxy = nullptr;
	if (UDisplayerStaticMeshComponent* StaticMeshComponent = Cast<UDisplayerStaticMeshComponent>(Primitive))
	{
		PrimitiveProxy = StaticMeshComponent->CreateDisplayerSceneProxy();
		StaticMeshComponent->DisplayerSceneProxy = PrimitiveProxy;
	}
	else if (UDisplayerInstancedStaticMeshComponent* InstancedStaticMeshComponent = Cast<UDisplayerInstancedStaticMeshComponent>(Primitive))
	{
		PrimitiveProxy = InstancedStaticMeshComponent->CreateDisplayerSceneProxy();
		InstancedStaticMeshComponent->DisplayerSceneProxy = PrimitiveProxy;
	}
	else
	{
		check(false)
	}

	return PrimitiveProxy;
}

FDisplayerPrimitiveSceneProxy* GetDisplayerSceneProxy(UPrimitiveComponent* Primitive)
{
	FDisplayerPrimitiveSceneProxy* PrimitiveProxy = nullptr;
	if (UDisplayerStaticMeshComponent* StaticMeshComponent = Cast<UDisplayerStaticMeshComponent>(Primitive))
	{
		PrimitiveProxy = StaticMeshComponent->DisplayerSceneProxy;
	}
	else if (UDisplayerInstancedStaticMeshComponent* InstancedStaticMeshComponent = Cast<UDisplayerInstancedStaticMeshComponent>(Primitive))
	{
		PrimitiveProxy = InstancedStaticMeshComponent->DisplayerSceneProxy;
	}
	else
	{
		check(false)
	}

	return PrimitiveProxy;
}

void EmptyDisplayerSceneProxy(UPrimitiveComponent* Primitive)
{
	if (UDisplayerStaticMeshComponent* StaticMeshComponent = Cast<UDisplayerStaticMeshComponent>(Primitive))
	{
		StaticMeshComponent->DisplayerSceneProxy = nullptr;
	}
	else if (UDisplayerInstancedStaticMeshComponent* InstancedStaticMeshComponent = Cast<UDisplayerInstancedStaticMeshComponent>(Primitive))
	{
		InstancedStaticMeshComponent->DisplayerSceneProxy = nullptr;
	}
	else
	{
		check(false)
	}
}
