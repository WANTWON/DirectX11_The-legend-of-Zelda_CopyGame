#include "stdafx.h"
#include "..\Public\Camera_Dynamic.h"
#include "GameInstance.h"

CCamera_Dynamic::CCamera_Dynamic(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCamera(pDevice, pContext)
{
}

CCamera_Dynamic::CCamera_Dynamic(const CCamera_Dynamic & rhs)
	: CCamera(rhs)
{
}

HRESULT CCamera_Dynamic::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CCamera_Dynamic::Initialize(void* pArg)
{
	

	if (FAILED(__super::Initialize(&((CAMERADESC_DERIVED*)pArg)->CameraDesc)))
		return E_FAIL;
	

	return S_OK;
}

int CCamera_Dynamic::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	if (GetKeyState('W') < 0)
	{
		m_pTransform->Go_Straight(fTimeDelta);
	}

	if (GetKeyState('S') < 0)
	{
		m_pTransform->Go_Backward(fTimeDelta);
	}

	if (GetKeyState('A') < 0)
	{
		
		m_pTransform->Go_Left(fTimeDelta);
	}

	if (GetKeyState('D') < 0)
	{
		
		m_pTransform->Go_Right(fTimeDelta);
	}

	if (GetKeyState('Q') < 0)
	{
		_vector Axis = XMVectorSet(0.f, 1.f, 0.f, 0.f);
		m_pTransform->Turn(Axis, -fTimeDelta);
	}

	if (GetKeyState('E') < 0)
	{
		_vector Axis = XMVectorSet(0.f, 1.f, 0.f, 0.f);
		m_pTransform->Turn(Axis, fTimeDelta);
	}

	if (GetKeyState('1') < 0)
	{

		_vector Axis = m_pTransform->Get_State(CTransform::STATE_RIGHT);
		m_pTransform->Turn(Axis, -fTimeDelta);
	}

	if (GetKeyState('2') < 0)
	{
		_vector Axis = m_pTransform->Get_State(CTransform::STATE_RIGHT);
		m_pTransform->Turn(Axis, fTimeDelta);
	}

	/*CGameInstance*			pGameInstance = CGameInstance::Get_Instance();
	Safe_AddRef(pGameInstance);

	_long			MouseMove = 0;

	if (MouseMove = pGameInstance->Get_DIMMoveState(DIMM_X))
	{
		_vector Axis = XMVectorSet(0.f, 1.f, 0.f, 0.f);
		m_pTransform->Turn(Axis, fTimeDelta * MouseMove * 0.1f);
	}

	if (MouseMove = pGameInstance->Get_DIMMoveState(DIMM_Y))
	{
		m_pTransform->Turn(m_pTransform->Get_State(CTransform::STATE_RIGHT), fTimeDelta * MouseMove * 0.1f);
	}


	Safe_Release(pGameInstance);*/

	if (FAILED(Bind_OnPipeLine()))
		return OBJ_NOEVENT;

	return OBJ_NOEVENT;
}

void CCamera_Dynamic::Late_Tick(_float fTimeDelta)
{
	__super::Late_Tick(fTimeDelta);

}

HRESULT CCamera_Dynamic::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	
	return S_OK;
}

CCamera_Dynamic * CCamera_Dynamic::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCamera_Dynamic*	pInstance = new CCamera_Dynamic(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		ERR_MSG(TEXT("Failed to Created : CCamera_Dynamic"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CCamera_Dynamic::Clone(void* pArg)
{
	CCamera_Dynamic*	pInstance = new CCamera_Dynamic(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		ERR_MSG(TEXT("Failed to Cloned : CCamera_Dynamic"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCamera_Dynamic::Free()
{
	__super::Free();

	
}
