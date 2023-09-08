#include "Renderer/DisplayerPrimitiveSceneInfo.h"

FDisplayerPrimitiveSceneInfo::FDisplayerPrimitiveSceneInfo(UPrimitiveComponent* InComponent, FDisplayerScene* InScene)
	: PrimitiveId(InComponent->ComponentId)
	, Scene(InScene)
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