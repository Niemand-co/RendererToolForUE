#pragma once

#include "DisplayerPrimitiveSceneProxy.h"

class FDisplayerStaticMeshSceneProxy : public FDisplayerPrimitiveSceneProxy
{
public:

	FStaticMeshRenderData* RenderData;

	int32 ForcedLodModel;

	int32 ClampedMinLod;

public:

	class FLODInfo : public FLightCacheInterface
	{
		struct FSectionInfo
		{
			int32 MaterialIndex;

			FSectionInfo()
				: MaterialIndex(-1)
			{
			}
		};

		TArray<FSectionInfo, TInlineAllocator<1>> Sections;

		FLODInfo(UStaticMeshComponent* InComponent, int32 LODIndex);
	};

public:

	FDisplayerStaticMeshSceneProxy(const class UStaticMeshComponent* InComponent);

	virtual ~FDisplayerStaticMeshSceneProxy();

	virtual SIZE_T GetTypeHash() const override;

	virtual int32 GetNumMeshBatches() const override;

	virtual void DrawStaticMeshElements() override;

	virtual bool GetMeshElements(int32 LODIndex, FMeshBatch& OutMeshBatch) override;

};