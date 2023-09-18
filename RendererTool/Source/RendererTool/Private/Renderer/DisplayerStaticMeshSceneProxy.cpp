#include "Renderer/DisplayerStaticMeshSceneProxy.h"
#include "Components/DisplayerStaticMeshComponent.h"
#include "Renderer/DisplayerPrimitiveSceneInfo.h"
#include "Renderer/DisplayerStaticMeshBatch.h"

FDisplayerStaticMeshSceneProxy::FDisplayerStaticMeshSceneProxy(const UStaticMeshComponent* InComponent)
	: FDisplayerPrimitiveSceneProxy(InComponent, InComponent->GetStaticMesh()->GetFName())
	, RenderData(InComponent->GetStaticMesh()->GetRenderData())
	, ForcedLodModel(InComponent->ForcedLodModel)
{
	ClampedMinLod = 0;
	int32 CurrentMinLod = InComponent->bOverrideMinLOD ? InComponent->MinLOD : InComponent->GetStaticMesh()->GetMinLODIdx();
	for (int32 FirstAvailableLodIdx = 0; FirstAvailableLodIdx < RenderData->LODResources.Num(); ++FirstAvailableLodIdx)
	{
		if (RenderData->LODResources[FirstAvailableLodIdx].GetNumVertices() > 0)
		{
			ClampedMinLod = FMath::Clamp(CurrentMinLod, FirstAvailableLodIdx, RenderData->LODResources.Num() - 1);
		}
	}
}

FDisplayerStaticMeshSceneProxy::~FDisplayerStaticMeshSceneProxy()
{
}

SIZE_T FDisplayerStaticMeshSceneProxy::GetTypeHash() const
{
	static SIZE_T UniquePointer;
	return (SIZE_T)&UniquePointer;
}

int32 FDisplayerStaticMeshSceneProxy::GetNumMeshBatches() const
{
	return 1;
}

void FDisplayerStaticMeshSceneProxy::DrawStaticMeshElements()
{
	if (ForcedLodModel > 0)
	{
		int32 LODIndex = FMath::Clamp(ForcedLodModel, ClampedMinLod + 1, RenderData->LODResources.Num()) - 1;

		const FStaticMeshLODResources& LodModel = RenderData->LODResources[LODIndex];
		for (int32 SectionIndex = 0; SectionIndex < LodModel.Sections.Num(); ++SectionIndex)
		{
			const FStaticMeshSection& Section = LodModel.Sections[SectionIndex];
			
			int32 NumBatches = GetNumMeshBatches();
			for (int32 BatchIndex = 0; BatchIndex < NumBatches; ++BatchIndex)
			{
				FMeshBatch Batch;
				
				if (GetMeshElements(LODIndex, Batch))
				{
					FDisplayerStaticMeshBatch* Mesh = new(SceneInfo->StaticMeshes) FDisplayerStaticMeshBatch(SceneInfo, Batch);
				}
			}
		}
	}
	else
	{
		for (int32 LODIndex = ClampedMinLod; LODIndex < RenderData->LODResources.Num(); ++LODIndex)
		{
			const FStaticMeshLODResources& LodModel = RenderData->LODResources[LODIndex];
			for (int32 SectionIndex = 0; SectionIndex < LodModel.Sections.Num(); ++SectionIndex)
			{
				const FStaticMeshSection& Section = LodModel.Sections[SectionIndex];

				int32 NumBatches = GetNumMeshBatches();
				for (int32 BatchIndex = 0; BatchIndex < NumBatches; ++BatchIndex)
				{
					FMeshBatch Batch;

					if (GetMeshElements(LODIndex, Batch))
					{
						FDisplayerStaticMeshBatch* Mesh = new(SceneInfo->StaticMeshes) FDisplayerStaticMeshBatch(SceneInfo, Batch);
					}
				}
			}
		}
	}
}

bool FDisplayerStaticMeshSceneProxy::GetMeshElements(int32 LODIndex, FMeshBatch& OutMeshBatch)
{
	return false;
}

FDisplayerStaticMeshSceneProxy::FLODInfo::FLODInfo(UStaticMeshComponent* InComponent, int32 LODIndex)
	: FLightCacheInterface()
{
	const FStaticMeshRenderData& RenderData = *(InComponent->GetStaticMesh()->GetRenderData());
	Sections.Empty(RenderData.LODResources.Num());
	for (int32 SectionIndex = 0; SectionIndex < Sections.Num(); ++SectionIndex)
	{

	}
}
