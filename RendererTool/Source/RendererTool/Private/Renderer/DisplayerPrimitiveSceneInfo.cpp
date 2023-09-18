#include "Renderer/DisplayerPrimitiveSceneInfo.h"
#include "Renderer/DisplayerPrimitiveSceneProxy.h"

FDisplayerPrimitiveSceneInfo::FDisplayerPrimitiveSceneInfo(UPrimitiveComponent* InComponent, FDisplayerScene* InScene)
	: PrimitiveId(InComponent->ComponentId)
	, Scene(InScene)
	, RegistrationSerialNumber(InComponent->RegistrationSerialNumber)
{
	check(Proxy)
	check(PrimitiveId.IsValid())

	//DefaultDynamicHitProxy = Proxy->CreateHitProxies(InComponent, HitProxies);
	//if (DefaultDynamicHitProxy)
	//{
	//	DefaultDynamicHitProxyId = DefaultDynamicHitProxy->Id;
	//}
}

FDisplayerPrimitiveSceneInfo::~FDisplayerPrimitiveSceneInfo()
{
}

void FDisplayerPrimitiveSceneInfo::AddToScene(FDisplayerScene* Scene, TArray<FDisplayerPrimitiveSceneInfo*>& SceneInfos, EPrimitiveAddToSceneOps AddOps)
{
	if (EnumHasAllFlags(AddOps, EPrimitiveAddToSceneOps::AddStaticMeshes))
	{
		AddStaticMeshes(Scene, SceneInfos, EnumHasAllFlags(AddOps, EPrimitiveAddToSceneOps::CacheDrawCommands));
	}
}

void FDisplayerPrimitiveSceneInfo::AddStaticMeshes(FDisplayerScene* Scene, TArrayView<FDisplayerPrimitiveSceneInfo*> SceneInfos, bool bCacheDrawCommands)
{
	ParallelForTemplate(SceneInfos.Num(), [&SceneInfos](int32 Index)
		{
			FDisplayerPrimitiveSceneInfo& SceneInfo = *SceneInfos[Index];
			SceneInfo.Proxy->DrawStaticMeshElements();
			SceneInfo.StaticMeshes.Shrink();
			SceneInfo.StaticMeshRelevances.Shrink();
		});

	if (bCacheDrawCommands)
	{
		CacheDrawCommands(Scene, SceneInfos);
	}
}

void FDisplayerPrimitiveSceneInfo::CacheDrawCommands(FDisplayerScene* Scene, TArrayView<FDisplayerPrimitiveSceneInfo*> SceneInfos)
{
	static SIZE_T NumBatches = WITH_EDITOR ? 1 : 64;

	auto DoLambda = [](int32 Index)
		{
			
		};

	for (int32 BatchIndex = 0; BatchIndex < NumBatches; ++BatchIndex)
	{
		DoLambda
	}
}