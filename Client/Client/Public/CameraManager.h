#pragma once
#include "Camera_Dynamic.h"
#include "Camera_2D.h"


BEGIN(Client)

class CCameraManager final : public CBase
{
	DECLARE_SINGLETON(CCameraManager)
public:
	enum CAM_STATE { CAM_DYNAMIC, CAM_2D, CAM_TARGET, CAM_END };

public:
	CCameraManager();
	virtual ~CCameraManager() = default;

public: /* Get*/ 
	CAM_STATE Get_CamState() { return m_eCamState; }
	CCamera* Get_CurrentCamera() { return m_pCurrentCamera; }

public:/*Set*/
	void Set_CamState(CAM_STATE _eState);
	void Set_CurrentCamera(CCamera* _pCamera) { m_pCurrentCamera = _pCamera; }

public:
	HRESULT Ready_Camera(LEVEL eLevelIndex);

private:
	CCamera*					m_pCurrentCamera = nullptr;
	CAM_STATE					m_eCamState = CAM_DYNAMIC;
	LEVEL						m_eCurrentLevel;

	

public:
	virtual void Free() override;
};

END