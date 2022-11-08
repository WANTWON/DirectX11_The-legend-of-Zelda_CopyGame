#include "stdafx.h"
#include "..\Public\MonsterBullet.h"


CMonsterBullet::CMonsterBullet(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CBaseObj(pDevice, pContext)
{
}

CMonsterBullet::CMonsterBullet(const CMonsterBullet & rhs)
	: CBaseObj(rhs)
{
}

HRESULT CMonsterBullet::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CMonsterBullet::Initialize(void * pArg)
{
	if (pArg != nullptr)
		memcpy(&m_BulletDesc, pArg, sizeof(BULLETDESC));

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;


	switch (m_BulletDesc.eBulletType)
	{
	case DEFAULT:
		Set_Scale(_float3(2, 1, 2));
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_BulletDesc.vInitPositon);
		break;
	case OCTOROCK:
		Set_Scale(_float3(1, 1, 1));
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_BulletDesc.vInitPositon);
		m_pTransformCom->LookDir(m_BulletDesc.vLook);
		break;
	case ROLA:
	{
		Set_Scale(_float3(1, 1, 1));
		_float PosY = XMVectorGetY(m_BulletDesc.vInitPositon);
		PosY += 1.f;
		m_BulletDesc.vInitPositon = XMVectorSetY(m_BulletDesc.vInitPositon, PosY);
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_BulletDesc.vInitPositon);
		m_pTransformCom->LookDir(m_BulletDesc.vLook);
		break;
	}
	default:
		break;
	}

	CCollision_Manager::Get_Instance()->Add_CollisionGroup(CCollision_Manager::COLLISION_MBULLET, this);
	return S_OK;
}

int CMonsterBullet::Tick(_float fTimeDelta)
{
	if (m_bDead)
	{
		CCollision_Manager::Get_Instance()->Out_CollisionGroup(CCollision_Manager::COLLISION_MBULLET, this);
		return OBJ_DEAD;
	}
		

	__super::Tick(fTimeDelta);

	switch (m_BulletDesc.eBulletType)
	{
	case DEFAULT:
		Moving_DefaultBullet(fTimeDelta);
		break;
	case OCTOROCK:
		Moving_OctorockBullet(fTimeDelta);
		break;
	case ROLA:
		Moving_RolaBullet(fTimeDelta);
		break;
	default:
		break;
	}


	return OBJ_NOEVENT;
}

void CMonsterBullet::Late_Tick(_float fTimeDelta)
{
	__super::Late_Tick(fTimeDelta);

	if (nullptr != m_pRendererCom)
		m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONALPHABLEND, this);
	SetUp_ShaderID();
}

HRESULT CMonsterBullet::Render()
{

#ifdef _DEBUG
	//m_pAABBCom->Render();
	m_pOBBCom->Render();
	/*m_pSPHERECom->Render();*/
#endif


	if (nullptr == m_pShaderCom ||
		nullptr == m_pModelCom)
		return E_FAIL;

	if (FAILED(SetUp_ShaderResources()))
		return E_FAIL;


	_uint		iNumMeshes = m_pModelCom->Get_NumMeshContainers();

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(m_pModelCom->SetUp_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(m_pShaderCom, i, m_eShaderID)))
			return E_FAIL;
	}



	return S_OK;
}

HRESULT CMonsterBullet::Ready_Components(void * pArg)
{
	LEVEL iLevel = (LEVEL)CGameInstance::Get_Instance()->Get_DestinationLevelIndex();

	/* For.Com_Renderer */
	if (FAILED(__super::Add_Components(TEXT("Com_Renderer"), LEVEL_STATIC, TEXT("Prototype_Component_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	/* For.Com_Transform */
	CTransform::TRANSFORMDESC		TransformDesc;
	ZeroMemory(&TransformDesc, sizeof(CTransform::TRANSFORMDESC));
	if(m_BulletDesc.eBulletType == OCTOROCK)
		TransformDesc.fSpeedPerSec = 5.0f;
	else
		TransformDesc.fSpeedPerSec = 2.0f;
	TransformDesc.fRotationPerSec = XMConvertToRadians(1.0f);
	if (FAILED(__super::Add_Components(TEXT("Com_Transform"), LEVEL_STATIC, TEXT("Prototype_Component_Transform"), (CComponent**)&m_pTransformCom, &TransformDesc)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Components(TEXT("Com_Shader"), LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Model*/
	switch (m_BulletDesc.eBulletType)
	{
	case OCTOROCK:
		/* For.Com_Model*/
		if (FAILED(__super::Add_Components(TEXT("Com_Model"), iLevel, TEXT("Prototype_Component_Model_OctorockBullet"), (CComponent**)&m_pModelCom)))
			return E_FAIL;
		break;
	case ROLA:
		/* For.Com_Model*/
		if (FAILED(__super::Add_Components(TEXT("Com_Model"), iLevel, TEXT("Prototype_Component_Model_RolaBullet"), (CComponent**)&m_pModelCom)))
			return E_FAIL;
		break;
	default:
		break;
	}

	/* For.Com_OBB*/
	CCollider::COLLIDERDESC		ColliderDesc;
	ColliderDesc.vScale = _float3(0.5f, 0.5f, 0.5f);
	ColliderDesc.vRotation = _float3(0.f, XMConvertToRadians(0.0f), 0.f);
	ColliderDesc.vPosition = _float3(0.f, 0.f, 0.f);
	if(m_BulletDesc.eBulletType == ROLA)
		ColliderDesc.vScale = _float3(5.f, 0.5f, 0.5f);
	if (FAILED(__super::Add_Components(TEXT("Com_OBB"), iLevel, TEXT("Prototype_Component_Collider_OBB"), (CComponent**)&m_pOBBCom, &ColliderDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CMonsterBullet::SetUp_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_World4x4_TP(), sizeof(_float4x4))))
		return E_FAIL;

	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);

	if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_VIEW), sizeof(_float4x4))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Set_RawValue("g_ProjMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_PROJ), sizeof(_float4x4))))
		return E_FAIL;

	RELEASE_INSTANCE(CGameInstance);

	return S_OK;
}

HRESULT CMonsterBullet::SetUp_ShaderID()
{
	return S_OK;
}

void CMonsterBullet::Change_Animation(_float fTimeDelta)
{
}


void CMonsterBullet::Moving_DefaultBullet(_float fTimeDelta)
{
	m_fDeadtime += fTimeDelta;

	if (m_BulletDesc.fDeadTime < m_fDeadtime)
		m_bDead = true;
}

void CMonsterBullet::Moving_OctorockBullet(_float fTimeDelta)
{

	m_pTransformCom->Go_Straight(fTimeDelta);
}

void CMonsterBullet::Moving_RolaBullet(_float fTimeDelta)
{
	_vector vAxis = m_pTransformCom->Get_State(CTransform::STATE_RIGHT);
	/*_float Y = XMVectorGetY(vAxis);
	Y += 1;
	vAxis = XMVectorSetY(vAxis, Y);*/

	m_pTransformCom->Turn(vAxis, 10.f);
	m_pTransformCom->Go_PosDir(fTimeDelta, m_BulletDesc.vLook);
}

CMonsterBullet * CMonsterBullet::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CMonsterBullet*	pInstance = new CMonsterBullet(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		ERR_MSG(TEXT("Failed to Created : CMonsterBullet"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CMonsterBullet::Clone(void * pArg)
{
	CMonsterBullet*	pInstance = new CMonsterBullet(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		ERR_MSG(TEXT("Failed to Cloned : CMonsterBullet"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMonsterBullet::Free()
{
	__super::Free();

	CCollision_Manager::Get_Instance()->Out_CollisionGroup(CCollision_Manager::COLLISION_MBULLET, this);

	Safe_Release(m_pTransformCom);
	Safe_Release(m_pRendererCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);

	Safe_Release(m_pAABBCom);
	Safe_Release(m_pOBBCom);
	Safe_Release(m_pSPHERECom);
}