#include "Renderer/DisplayerMeshPassProcessor.h"

FDisplayerMeshPassProcessor::FDisplayerMeshPassProcessor()
{
}

FDisplayerMeshPassProcessor::~FDisplayerMeshPassProcessor()
{
}

DisplayerMeshPassCreateFunction FDisplayerMeshPassProcessorManager::FunctionJumpTable[(uint32)EDisplayerShadingPath::Num][EDisplayerMeshPass::Num] = {};