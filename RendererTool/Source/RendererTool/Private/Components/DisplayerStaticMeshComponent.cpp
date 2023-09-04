#include "Components/DisplayerStaticMeshComponent.h"
#include "Renderer/DisplayerStaticMeshSceneProxy.h"

UDisplayerStaticMeshComponent::UDisplayerStaticMeshComponent(const FObjectInitializer& ObjectInitializer)
	: UStaticMeshComponent(ObjectInitializer)
{
}

UDisplayerStaticMeshComponent::~UDisplayerStaticMeshComponent()
{
}

FDisplayerPrimitiveSceneProxy* UDisplayerStaticMeshComponent::CreateDisplayerSceneProxy()
{
	if (GetStaticMesh() == nullptr)
	{
		UE_LOG(LogStaticMesh, Verbose, TEXT("Skipping CreateSceneProxy for StaticMeshComponent %s (StaticMesh is null)"), *GetFullName());
		return nullptr;
	}

	// Prevent accessing the RenderData during async compilation. The RenderState will be recreated when compilation finishes.
	if (GetStaticMesh()->IsCompiling())
	{
		UE_LOG(LogStaticMesh, Verbose, TEXT("Skipping CreateSceneProxy for StaticMeshComponent %s (StaticMesh is not ready)"), *GetFullName());
		return nullptr;
	}

	if (GetStaticMesh()->GetRenderData() == nullptr)
	{
		UE_LOG(LogStaticMesh, Verbose, TEXT("Skipping CreateSceneProxy for StaticMeshComponent %s (RenderData is null)"), *GetFullName());
		return nullptr;
	}

	if (!GetStaticMesh()->GetRenderData()->IsInitialized())
	{
		UE_LOG(LogStaticMesh, Verbose, TEXT("Skipping CreateSceneProxy for StaticMeshComponent %s (RenderData is not initialized)"), *GetFullName());
		return nullptr;
	}

	if (IsPSOPrecaching())
	{
		UE_LOG(LogStaticMesh, Verbose, TEXT("Skipping CreateSceneProxy for StaticMeshComponent %s (Static mesh component PSOs are still compiling)"), *GetFullName());
		return nullptr;
	}

	const FStaticMeshLODResourcesArray& LODResources = GetStaticMesh()->GetRenderData()->LODResources;
	const int32 SMCurrentMinLOD = GetStaticMesh()->GetMinLODIdx();
	const int32 EffectiveMinLOD = bOverrideMinLOD ? MinLOD : SMCurrentMinLOD;
	if (LODResources.Num() == 0 || LODResources[FMath::Clamp<int32>(EffectiveMinLOD, 0, LODResources.Num() - 1)].VertexBuffers.StaticMeshVertexBuffer.GetNumVertices() == 0)
	{
		UE_LOG(LogStaticMesh, Verbose, TEXT("Skipping CreateSceneProxy for StaticMeshComponent %s (LOD problems)"), *GetFullName());
		return nullptr;
	}

	LLM_SCOPE(ELLMTag::StaticMesh);

	FDisplayerPrimitiveSceneProxy* Proxy = ::new FDisplayerStaticMeshSceneProxy(this);

	return Proxy;
}
