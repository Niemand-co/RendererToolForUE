#include "Components/DisplayerInstancedStaticMeshComponent.h"
#include "Renderer/DisplayerInstancedStaticMeshSceneProxy.h"
#include "Engine/Classes/Engine/InstancedStaticMesh.h"

UDisplayerInstancedStaticMeshComponent::UDisplayerInstancedStaticMeshComponent(const FObjectInitializer& ObjectInitializer)
	: UInstancedStaticMeshComponent(ObjectInitializer)
{
}

UDisplayerInstancedStaticMeshComponent::~UDisplayerInstancedStaticMeshComponent()
{
}

FDisplayerPrimitiveSceneProxy* UDisplayerInstancedStaticMeshComponent::CreateDisplayerSceneProxy()
{
	ProxySize = 0;

	// Verify that the mesh is valid before using it.
	const bool bMeshIsValid =
		// make sure we have instances
		PerInstanceSMData.Num() > 0 &&
		// make sure we have an actual static mesh
		GetStaticMesh() &&
		GetStaticMesh()->IsCompiling() == false &&
		GetStaticMesh()->HasValidRenderData() &&
		!IsPSOPrecaching();

	if (bMeshIsValid)
	{
		check(InstancingRandomSeed != 0);

		// if instance data was modified, update GPU copy
		// generally happens only in editor 
		if (InstanceUpdateCmdBuffer.NumTotalCommands() != 0)
		{
			FlushInstanceUpdateCommands(true);
		}

		ProxySize = PerInstanceRenderData->ResourceSize;

		return ::new FDisplayerInstancedStaticMeshSceneProxy(this);
	}
	else
	{
		return nullptr;
	}
}
