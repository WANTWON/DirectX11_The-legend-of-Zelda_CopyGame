#pragma once

#include "Client_Defines.h"
#include "Base.h"
#include "UI_Manager.h"
#include "Collision_Manger.h"
#include "Data_Manager.h"

BEGIN(Engine)
class CRenderer;
class CGameInstance;
END

BEGIN(Client)

class CMainApp final : public CBase
{
private:
	CMainApp();
	virtual ~CMainApp() = default;

public:
	HRESULT Initialize();
	void Tick(_float fTimeDelta);
	HRESULT Render();

private:
	CGameInstance*			m_pGameInstance = nullptr;

private:
	ID3D11Device*			m_pDevice = nullptr;
	ID3D11DeviceContext*	m_pContext = nullptr;
	CRenderer*				m_pRenderer = nullptr;
	CUI_Manager*			m_pUI_Manager = nullptr;
	CCollision_Manager*		m_pCollisionManager = nullptr;
	CData_Manager*			m_pDataManager = nullptr;
#ifdef _DEBUG
private:
	_uint					m_iNumRender = 0;
	_tchar					m_szFPS[MAX_PATH] = TEXT("");
	_float					m_fTimeAcc = 0.f;
#endif // _DEBUG

private:
	HRESULT Open_Level(LEVEL eLevel);
	HRESULT Ready_Prototype_Component();	

public:
	static CMainApp* Create();
	virtual void Free();
};

END

