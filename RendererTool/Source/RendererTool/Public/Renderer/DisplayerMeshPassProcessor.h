#pragma once

#include "MeshPassProcessor.h"
#include "Renderer/RendererToolRenderer.h"

namespace EDisplayerMeshPass
{
	enum Type : uint8
	{
		EBasePass = 0,
		Num = 1
	};
}

class FDisplayerMeshPassDrawListContext
{

};

class FDisplayerCachedMeshPassDrawListContext : public FDisplayerMeshPassDrawListContext
{

};

class FDisplayerDynamicMeshPassDrawListContext : public FDisplayerMeshPassDrawListContext
{

};

class FDisplayerMeshPassProcessor : public IPSOCollector
{
public:

	FDisplayerMeshPassProcessor();
	virtual ~FDisplayerMeshPassProcessor();

};

typedef FDisplayerMeshPassProcessor* (*DisplayerMeshPassCreateFunction)(EDisplayerShadingPath ShadingPath, EDisplayerMeshPass::Type PassType, FDisplayerMeshPassDrawListContext* DrawList);

class FDisplayerMeshPassProcessorManager
{
public:

	static FDisplayerMeshPassProcessor* CreateMeshPassProcessor(EDisplayerShadingPath ShadingPath, EDisplayerMeshPass::Type PassType, FDisplayerMeshPassDrawListContext* DrawList)
	{
		if (FunctionJumpTable[(uint32)ShadingPath][PassType])
		{
			return FunctionJumpTable[(uint32)ShadingPath][PassType](ShadingPath, PassType, DrawList);
		}
		return nullptr;
	}


private:

	static DisplayerMeshPassCreateFunction FunctionJumpTable[(uint32)EDisplayerShadingPath::Num][EDisplayerMeshPass::Num];
	
};