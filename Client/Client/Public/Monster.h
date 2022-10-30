#pragma once
#include "Client_Defines.h"
#include "BaseObj.h"
#include "GameInstance.h"

BEGIN(Engine)
class CModel;
END

BEGIN(Client)
class CMonster abstract : public CBaseObj
{
public:
	enum MONSTER_ID { MONSTER_OCTOROCK, MONSTER_MOBLINSWORD, MONSTER_END };

protected:
	CMonster(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMonster(const CMonster& rhs);
	virtual ~CMonster() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual int Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

protected:
	virtual HRESULT Ready_Components(void* pArg = nullptr) = 0;
	virtual HRESULT SetUp_ShaderResources() = 0;
	virtual HRESULT SetUp_ShaderID() { return S_OK; };
	virtual void Change_Animation(_float fTimeDelta) = 0;


public: // Get& Set
	_bool Get_Aggro(void) { return m_bAggro; }
	MONSTER_ID Get_MonsterID(void) { return m_eMonsterID; }
	_bool Get_Hited(void) { return m_bHit; }

protected:
	void Calculate_Direction(_vector vTargetPos);
	virtual void AI_Behaviour(_float fTimeDelta) { };
	virtual void Find_Target() { };
	virtual void Follow_Target(_float fTimeDelta) { };
	virtual _float Take_Damage(float fDamage, void* DamageType, CGameObject* DamageCauser);
	virtual HRESULT Drop_Items() { return S_OK; };
	virtual _bool IsDead() = 0;

protected:
	CGameObject* m_pTarget = nullptr;
	_float m_fAttackRadius = .5f;
	_float m_fDistanceToTarget = 0.f;
	_float m_fPatrolRadius = 3.f;
	_float3 m_fPatrolPosition = _float3(0.f, 0.f, 0.f);

	_bool m_bAggro = false;
	_bool m_bIsAttacking = false;
	//_bool m_bDidDamage = false;
	_bool m_bHit = false;
	_bool m_bMove = true;

//For Move Time
	DWORD m_dwDeathTime = GetTickCount();
	DWORD m_dwAttackTime = GetTickCount();
	DWORD m_dwIdleTime = GetTickCount();
	DWORD m_dwWalkTime = GetTickCount();

	MONSTER_ID	m_eMonsterID = MONSTER_END;
	OBJINFO  m_tInfo;

protected: /* For.Components */
	CModel*					m_pModelCom = nullptr;


public:
	virtual void Free() override;
};
END