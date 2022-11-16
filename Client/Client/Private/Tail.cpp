#include "stdafx.h"
#include "..\Public\Tail.h"
#include "Player.h"

CTail::CTail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CMonster(pDevice, pContext)
{
}

HRESULT CTail::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CTail::Initialize(void * pArg)
{
	if (pArg != nullptr)
		memcpy(&m_TailDesc, pArg, sizeof(TAILDESC));

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_tInfo.iMaxHp = 3;
	m_tInfo.iDamage = 4;
	m_tInfo.iCurrentHp = m_tInfo.iMaxHp;

	m_fAttackRadius = 1.5f;
	m_eMonsterID = MONSTER_TAIL;

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_TailDesc.InitPostion);
	m_pNavigationCom->Compute_CurrentIndex(m_pTransformCom->Get_State(CTransform::STATE_POSITION));
	CCollision_Manager::Get_Instance()->Add_CollisionGroup(CCollision_Manager::COLLISION_MONSTER, this);


	Create_Tail(m_TailDesc.eTailType);



	return S_OK;
}

int CTail::Tick(_float fTimeDelta)
{
	if (__super::Tick(fTimeDelta))
		return OBJ_DEAD;


	AI_Behaviour(fTimeDelta);
	Check_Navigation(fTimeDelta);

	if (m_TailDesc.eTailType != TAIL1)
		m_eState = dynamic_cast<CTail*>(m_TailDesc.pParent)->Get_AnimState();

	m_pModelCom->Set_NextAnimIndex(m_eState);
	Change_Animation(fTimeDelta);


	return OBJ_NOEVENT;
}

void CTail::Late_Tick(_float fTimeDelta)
{
	__super::Late_Tick(fTimeDelta);

	CBaseObj* pCollisionBlock = nullptr;
	if (CCollision_Manager::Get_Instance()->CollisionwithGroup(CCollision_Manager::COLLISION_BLOCK, m_pSPHERECom, &pCollisionBlock))
	{
		_vector vDirection = m_pTransformCom->Get_State(CTransform::STATE_POSITION) - pCollisionBlock->Get_TransformState(CTransform::STATE_POSITION);
		if (fabs(XMVectorGetX(vDirection)) > fabs(XMVectorGetZ(vDirection)))
			vDirection = XMVectorSet(XMVectorGetX(vDirection), 0.f, 0.f, 0.f);
		else
			vDirection = XMVectorSet(0.f, 0.f, XMVectorGetZ(vDirection), 0.f);
		m_pTransformCom->Go_PosDir(fTimeDelta*1.5f, vDirection);

		m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), 180);
		m_fTurnAngle = (rand() % 2000) * 0.001f + 0.1f;
		m_fTurnAngle = rand() % 2 == 0 ? m_fTurnAngle : -m_fTurnAngle;
	}

}

HRESULT CTail::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

#ifdef _DEBUG
	//m_pOBBCom->Render();
	//m_pAABBCom->Render();
	m_pSPHERECom->Render();
#endif

	return S_OK;
}

void CTail::Check_Navigation(_float fTimeDelta)
{
	if (m_pNavigationCom->Get_CurrentCelltype() == CCell::DROP)
	{
		if (m_eState == DEAD)
		{
			m_pTransformCom->Go_PosDir(fTimeDelta * 2, XMVectorSet(0.f, -1.f, 0.f, 0.f), m_pNavigationCom);
		}
		if (m_eState == DAMAGE)
		{
			m_eState = DEAD;
		}
		else
		{
			m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), 180);
			m_fTurnAngle = (rand() % 2000) * 0.001f + 0.1f;
			m_fTurnAngle = rand() % 2 == 0 ? m_fTurnAngle : -m_fTurnAngle;
		}
	}
	else if (m_pNavigationCom->Get_CurrentCelltype() == CCell::ACCESSIBLE)
	{
		_vector vPosition = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		_float fHeight = m_pNavigationCom->Compute_Height(vPosition, Get_Scale().y*0.25f);
		vPosition = XMVectorSetY(vPosition, fHeight);
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPosition);

	}


}

void CTail::Change_Animation(_float fTimeDelta)
{
	switch (m_eState)
	{
	case Client::CTail::DAMAGE:
		m_fAnimSpeed = 3.f;
		m_bIsLoop = false;
		if (m_TailDesc.eTailType == TAIL1)
		{
			m_pTarget = dynamic_cast<CBaseObj*>(CGameInstance::Get_Instance()->Get_Object(LEVEL_STATIC, TEXT("Layer_Player")));
			_vector vDir = m_pTransformCom->Get_State(CTransform::STATE_POSITION) - m_pTarget->Get_TransformState(CTransform::STATE_POSITION);
			m_pTransformCom->Go_PosDir(fTimeDelta, vDir, m_pNavigationCom);
		}
		if (m_pModelCom->Play_Animation(fTimeDelta*m_fAnimSpeed, m_bIsLoop))
		{
			m_bIsAttacking = false;
			m_eState = WAIT_MOVE;

			//m_fTurnAngle += 1.f;
			m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), 180);
			m_fTurnAngle = rand() % 2 == 0 ? m_fTurnAngle : -m_fTurnAngle;
		}
		break;
	case Client::CTail::PIYO:
		m_fAnimSpeed = 3.f;
		m_bIsLoop = false;
		if (m_pModelCom->Play_Animation(fTimeDelta*m_fAnimSpeed, m_bIsLoop))
		{
			m_bIsAttacking = false;
			m_eState = WAIT_MOVE;
		}
		break;
	case Client::CTail::DEAD:
		m_bIsLoop = false;
		if (m_pModelCom->Play_Animation(fTimeDelta*m_fAnimSpeed, m_bIsLoop))
		{
			m_bDead = true;
		}
		break;
	case Client::CTail::WAIT_MOVE:
		m_fAnimSpeed = 2.f;
		m_bIsLoop = true;
		m_pModelCom->Play_Animation(fTimeDelta*m_fAnimSpeed, m_bIsLoop);
		break;
		break;
	default:
		break;
	}
}

HRESULT CTail::Ready_Components(void * pArg)
{
	/* For.Com_Renderer */
	if (FAILED(__super::Add_Components(TEXT("Com_Renderer"), LEVEL_STATIC, TEXT("Prototype_Component_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	/* For.Com_Transform */
	CTransform::TRANSFORMDESC		TransformDesc;
	ZeroMemory(&TransformDesc, sizeof(CTransform::TRANSFORMDESC));

	TransformDesc.fSpeedPerSec = 5.f;
	TransformDesc.fRotationPerSec = XMConvertToRadians(1.0f);
	if (FAILED(__super::Add_Components(TEXT("Com_Transform"), LEVEL_STATIC, TEXT("Prototype_Component_Transform"), (CComponent**)&m_pTransformCom, &TransformDesc)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Components(TEXT("Com_Shader"), LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	switch (m_TailDesc.eTailType)
	{
	case TAIL1:
		/* For.Com_Model*/
		if (FAILED(__super::Add_Components(TEXT("Com_Model"), LEVEL_TAILCAVE, TEXT("Prototype_Component_Model_Tail1"), (CComponent**)&m_pModelCom)))
			return E_FAIL;
		break;
	case TAIL2:
		/* For.Com_Model*/
		if (FAILED(__super::Add_Components(TEXT("Com_Model"), LEVEL_TAILCAVE, TEXT("Prototype_Component_Model_Tail2"), (CComponent**)&m_pModelCom)))
			return E_FAIL;
		break;
	case TAIL3:
		/* For.Com_Model*/
		if (FAILED(__super::Add_Components(TEXT("Com_Model"), LEVEL_TAILCAVE, TEXT("Prototype_Component_Model_Tail3"), (CComponent**)&m_pModelCom)))
			return E_FAIL;
		break;
	default:
		break;
	}


	///* For.Com_OBB*/
	CCollider::COLLIDERDESC		ColliderDesc;
	//ColliderDesc.vScale = _float3(1.5f, 1.5f, 1.5f);
	//ColliderDesc.vRotation = _float3(0.f, XMConvertToRadians(0.0f), 0.f);
	//ColliderDesc.vPosition = _float3(0.f, 0.0f, 0.f);
	//if (FAILED(__super::Add_Components(TEXT("Com_OBB"), LEVEL_TAILCAVE, TEXT("Prototype_Component_Collider_OBB"), (CComponent**)&m_pOBBCom, &ColliderDesc)))
	//	return E_FAIL;

	/* For.Com_SHPERE */
	ColliderDesc.vScale = _float3(0.5f, 0.5f, 0.5f);
	ColliderDesc.vRotation = _float3(0.f, 0.f, 0.f);
	ColliderDesc.vPosition = _float3(0.f, 0.f, 0.f);
	if (FAILED(__super::Add_Components(TEXT("Com_SPHERE"), LEVEL_TAILCAVE, TEXT("Prototype_Component_Collider_SPHERE"), (CComponent**)&m_pSPHERECom, &ColliderDesc)))
		return E_FAIL;

	/* For.Com_Navigation */
	CNavigation::NAVIDESC			NaviDesc;
	ZeroMemory(&NaviDesc, sizeof NaviDesc);
	NaviDesc.iCurrentCellIndex = 0;
	if (FAILED(__super::Add_Components(TEXT("Com_Navigation"), LEVEL_STATIC, TEXT("Prototype_Component_Navigation_TailCave"), (CComponent**)&m_pNavigationCom, &NaviDesc)))
		return E_FAIL;


	return S_OK;
}

HRESULT CTail::SetUp_ShaderResources()
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

_bool CTail::IsDead()
{
	if (m_bDead && m_eState == STATE::DEAD)
		return true;
	else if (m_bDead && m_eState != STATE::DEAD)
	{
		m_dwDeathTime = GetTickCount();
		m_eState = STATE::DEAD;
	}

	return false;
}




void CTail::AI_Behaviour(_float fTimeDelta)
{
	if (!m_bMove || m_eState == DEAD || m_eState == PIYO)
		return;


	if (m_TailDesc.eTailType == TAIL1)
		Behaviour_Head(fTimeDelta);
	else
		Behaviour_Tail(fTimeDelta);

}

void CTail::Behaviour_Head(_float fTimeDelta)
{
	if (m_bDead || m_eState == DAMAGE )
		return;

	Find_Target();
	if (!m_bStart && m_fDistanceToTarget < 5.f)
	{
		m_bStart = true;
		m_eState = WAIT_MOVE;
	}

	if (!m_bIsAttacking && m_pSPHERECom->Collision(m_pTarget->Get_Collider()) == true)
	{
		m_bAggro = true;
		m_eState = STATE::PIYO;
		m_bIsAttacking = true;
	}
	else
	{
		m_bChangeDirection = m_pTransformCom->Go_Straight(fTimeDelta, m_pNavigationCom);
		if (m_bChangeDirection == false)
		{
			m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), 180);
			m_fTurnAngle = rand() % 2 == 0 ? m_fTurnAngle : -m_fTurnAngle;
		}

		m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), m_fTurnAngle);
	}
}

void CTail::Behaviour_Tail(_float fTimeDelta)
{
	if (m_eState == DEAD || m_bDead)
		return;

	if (m_TailDesc.pParent == nullptr)
		return;

	if (m_TailDesc.eTailType != TAIL1)
	{ 
		_vector vPosition;
		if(m_TailDesc.eTailType == TAIL2)
			 vPosition = m_TailDesc.pParent->Get_TransformState(CTransform::STATE_POSITION) - m_TailDesc.pParent->Get_TransformState(CTransform::STATE_LOOK)*0.4f;
		else if (m_TailDesc.eTailType == TAIL3)
			vPosition = m_TailDesc.pParent->Get_TransformState(CTransform::STATE_POSITION) - m_TailDesc.pParent->Get_TransformState(CTransform::STATE_LOOK)*0.3f;
		m_pTransformCom->LookAt(vPosition);
		m_pTransformCom->Fix_Look_byFloor(XMVectorSet(0.f, 1.f, 0.f, 0.f));
		//m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPosition);
		m_pTransformCom->Go_Straight(fTimeDelta, m_pNavigationCom);

	}

}

HRESULT CTail::Create_Tail(TAILTYPE eType)
{
	if (eType == TAIL3)
		return E_FAIL;

	CGameInstance* pGameInstance = GET_INSTANCE(CGameInstance)

		CTail::TAILDESC TailDesc;
	TailDesc.eTailType = TAILTYPE(eType + 1);
	TailDesc.InitPostion = m_TailDesc.InitPostion;
	TailDesc.pParent = this;
	if (FAILED(CGameInstance::Get_Instance()->Add_GameObject(TEXT("Prototype_GameObject_Tail"), LEVEL_GAMEPLAY, TEXT("Layer_Monster"), &TailDesc)))
		return E_FAIL;


	RELEASE_INSTANCE(CGameInstance);
	return S_OK;
}



_uint CTail::Take_Damage(float fDamage, void * DamageType, CBaseObj * DamageCauser)
{
	if (m_eState == DEAD || m_eState == STATE::DAMAGE)
		return 0;

	_uint fHp = __super::Take_Damage(fDamage, DamageType, DamageCauser);

	if (fHp > 0)
	{
		if (!m_bDead)
		{
			m_bHit = true;
			m_eState = STATE::DAMAGE;
			m_bMove = true;
		}

		m_bAggro = true;
		m_bIsAttacking = false;
		m_dwAttackTime = GetTickCount();
		Change_Parent_State(m_eState);
		return fHp;
	}
	else
		m_eState = STATE::DEAD;

	return 0;
}

void CTail::Change_Parent_State(STATE TailState)
{
	if (m_TailDesc.pParent == nullptr)
		return;

	dynamic_cast<CTail*>(m_TailDesc.pParent)->Set_AnimState(m_eState);
	dynamic_cast<CTail*>(m_TailDesc.pParent)->Change_Parent_State(m_eState);
}

CTail * CTail::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CTail*	pInstance = new CTail(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		ERR_MSG(TEXT("Failed to Created : CTail"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CTail::Clone(void * pArg)
{
	CTail*	pInstance = new CTail(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		ERR_MSG(TEXT("Failed to Cloned : CTail"));
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CTail::Free()
{
	__super::Free();
}

