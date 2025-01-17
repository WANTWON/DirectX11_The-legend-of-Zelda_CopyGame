#include "..\Public\GameObject.h"
#include "GameInstance.h"
#include "PipeLine.h"

CGameObject::CGameObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice(pDevice), m_pContext(pContext)
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

CGameObject::CGameObject(const CGameObject & rhs)
	: m_pDevice(rhs.m_pDevice), m_pContext(rhs.m_pContext)
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CGameObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CGameObject::Initialize(void* pArg)
{
	return S_OK;
}

int CGameObject::Tick(_float fTimeDelta)
{
	return OBJ_NOEVENT;
}

void CGameObject::Late_Tick(_float fTimeDelta)
{
}

HRESULT CGameObject::Render()
{
	return S_OK;
}


HRESULT CGameObject::Add_Components(const _tchar * pComponentTag, _uint iLevelIndex, const _tchar * pPrototypeTag, CComponent** ppOut, void * pArg)
{
	if (nullptr != Find_Component(pComponentTag))
		return E_FAIL;

	CGameInstance*		pGameInstance = CGameInstance::Get_Instance();
	Safe_AddRef(pGameInstance);

	CComponent*		pComponent = pGameInstance->Clone_Component(iLevelIndex, pPrototypeTag, pArg);
	if (nullptr == pComponent)
		return E_FAIL;

	m_Components.emplace(pComponentTag, pComponent);

	*ppOut = pComponent;

	Safe_AddRef(pComponent);

	Safe_Release(pGameInstance);

	return S_OK;
}

void CGameObject::Compute_CamDistance(_vector vWorldPos)
{
	_float4x4			ViewMatrix;

	CPipeLine*			pPipeLine = GET_INSTANCE(CPipeLine);

	m_fCamDistance = XMVectorGetX(XMVector3Length(vWorldPos - XMLoadFloat4(&pPipeLine->Get_CamPosition())));

	RELEASE_INSTANCE(CPipeLine);

}

CComponent * CGameObject::Find_Component(const _tchar * pComponentTag)
{
	auto	iter = find_if(m_Components.begin(), m_Components.end(), CTag_Finder(pComponentTag));
	if (iter == m_Components.end())
		return nullptr;

	return iter->second;
}

void CGameObject::Free()
{
	for (auto& Pair : m_Components)
		Safe_Release(Pair.second);
	m_Components.clear();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
