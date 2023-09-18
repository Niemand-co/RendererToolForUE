#include "Renderer/DisplayerStaticMeshBatch.h"

FDisplayerStaticMeshBatch::FDisplayerStaticMeshBatch(const class FDisplayerPrimitiveSceneInfo* SceneInfo, const FMeshBatch& MeshBatch)
	: FMeshBatch(MeshBatch)
{
}

FDisplayerStaticMeshBatch::~FDisplayerStaticMeshBatch()
{
}
