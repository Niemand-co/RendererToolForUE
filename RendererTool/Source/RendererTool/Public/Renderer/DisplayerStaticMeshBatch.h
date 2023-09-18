#pragma once

#include "MeshBatch.h"

class FDisplayerStaticMeshBatch : public FMeshBatch
{
public:

	FDisplayerStaticMeshBatch(const class FDisplayerPrimitiveSceneInfo* SceneInfo, const FMeshBatch& MeshBatch);
	virtual ~FDisplayerStaticMeshBatch();

};