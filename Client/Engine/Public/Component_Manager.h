#pragma once

/* 컴포넌트들의 원형을 생성하여 레벨별로 보관한다. */
#include "Model.h"
#include "Shader.h"
#include "Texture.h"
#include "Renderer.h"
#include "Transform.h"
#include "VIBuffer_Rect.h"
#include "Navigation.h"

#include "VIBuffer_Cube.h"
#include "VIBuffer_Terrain.h"
#include "VIBuffer_SwordTrail.h"
#include "Collider.h"

BEGIN(Engine)

class CComponent_Manager final : public CBase
{
	DECLARE_SINGLETON(CComponent_Manager)
private:
	CComponent_Manager();
	virtual ~CComponent_Manager() = default;

public:
	HRESULT Reserve_Container(_uint iNumLevels);
	HRESULT Add_Prototype(_uint iLevelIndex, const _tchar* pPrototypeTag, class CComponent* pPrototype);
	HRESULT	Check_Prototype(_int iLevelIndex, const _tchar* pPrototypeTag); // 추가
	class CComponent* Clone_Component(_uint iLevelIndex, const _tchar* pPrototypeTag, void* pArg = nullptr);
	void Clear(_uint iLevelIndex);
private:
	_uint												m_iNumLevels = 0;
	map<const _tchar*, class CComponent*>*				m_pPrototypes = nullptr;
	typedef map<const _tchar*, class CComponent*>		PROTOTYPES;

private:
	class CComponent* Find_Component(_uint iLevelIndex, const _tchar* pPrototypeTag);

public:
	virtual void Free() override;
};

END