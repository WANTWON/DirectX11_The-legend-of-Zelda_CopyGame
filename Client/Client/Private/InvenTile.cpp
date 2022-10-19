#include "stdafx.h"
#include "..\Public\InvenTile.h"

#include "GameInstance.h"


CInvenTile::CInvenTile(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CObj_UI(pDevice, pContext)
{
}

CInvenTile::CInvenTile(const CInvenTile & rhs)
	: CObj_UI(rhs)
{
}

HRESULT CInvenTile::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CInvenTile::Initialize(void * pArg)
{
	if (pArg != nullptr)
		memcpy(&m_InvenDesc, pArg, sizeof(INVENDESC));

	switch (m_InvenDesc.eTileType)
	{
	case INEVEN_TILE:
		m_fSizeX = 110;
		m_fSizeY = 110;
		break;
	case EQUIP_TILE:
		m_fSizeX = 100;
		m_fSizeY = 100;
		break;
	}

	m_fX = m_InvenDesc.vPosition.x;
	m_fY = m_InvenDesc.vPosition.y;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

int CInvenTile::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	return OBJ_NOEVENT;
}

void CInvenTile::Late_Tick(_float fTimeDelta)
{
	__super::Late_Tick(fTimeDelta);
}

HRESULT CInvenTile::Render()
{
	if (!CUI_Manager::Get_Instance()->Get_UI_Open() && m_InvenDesc.eTileType != EQUIP_TILE)
		return E_FAIL;
		
	if (nullptr == m_pShaderCom ||
		nullptr == m_pVIBufferCom)
		return E_FAIL;

	if (FAILED(SetUp_ShaderResources()))
		return E_FAIL;

	m_pShaderCom->Begin();

	m_pVIBufferCom->Render();


	return S_OK;
}

HRESULT CInvenTile::Ready_Components()
{
	/* For.Com_Renderer */
	if (FAILED(__super::Add_Components(TEXT("Com_Renderer"), LEVEL_STATIC, TEXT("Prototype_Component_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	/* For.Com_Transform */
	if (FAILED(__super::Add_Components(TEXT("Com_Transform"), LEVEL_STATIC, TEXT("Prototype_Component_Transform"), (CComponent**)&m_pTransformCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Components(TEXT("Com_Shader"), LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxTex"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	switch (m_InvenDesc.eTileType)
	{
	case INEVEN_TILE:
		/* For.Com_Texture */
		if (FAILED(__super::Add_Components(TEXT("Com_Texture"), LEVEL_STATIC, TEXT("Prototype_Component_Texture_InvenTile"), (CComponent**)&m_pTextureCom)))
			return E_FAIL;
		break;
	case EQUIP_TILE:
		/* For.Com_Texture */
		if (FAILED(__super::Add_Components(TEXT("Com_Texture"), LEVEL_STATIC, TEXT("Prototype_Component_Texture_EquipTile"), (CComponent**)&m_pTextureCom)))
			return E_FAIL;
		break;
	}

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Components(TEXT("Com_VIBuffer"), LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CInvenTile::SetUp_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_World4x4_TP(), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix", &m_ViewMatrix, sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ProjMatrix", &m_ProjMatrix, sizeof(_float4x4))))
		return E_FAIL;
	
	if (m_InvenDesc.eTileType == EQUIP_TILE)
	{
		if (CUI_Manager::Get_Instance()->Get_UI_Open())
			m_InvenDesc.eState = STATE_EQUIP;
		else
			m_InvenDesc.eState = STATE_DEFAULT;
	}
	
	if (FAILED(m_pShaderCom->Set_ShaderResourceView("g_DiffuseTexture", m_pTextureCom->Get_SRV(m_InvenDesc.eState))))
		return E_FAIL;

	return S_OK;
}

CInvenTile * CInvenTile::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CInvenTile*	pInstance = new CInvenTile(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		ERR_MSG(TEXT("Failed to Created : CInvenTile"));
		Safe_Release(pInstance);
	}

	return pInstance;
}


CGameObject * CInvenTile::Clone(void * pArg)
{
	CInvenTile*	pInstance = new CInvenTile(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		ERR_MSG(TEXT("Failed to Cloned : CInvenTile"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CInvenTile::Free()
{
	__super::Free();
}