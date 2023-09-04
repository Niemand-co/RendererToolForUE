#pragma once

class FDisplayerLightSceneInfo
{
public:

	class FDisplayerLightSceneProxy* Proxy;

	int32 Id;

	bool bVisible;

	class FDisplayerScene *Scene;

public:

	FDisplayerLightSceneInfo(FDisplayerLightSceneProxy* InProxy, bool InbVisible);

	~FDisplayerLightSceneInfo();

};