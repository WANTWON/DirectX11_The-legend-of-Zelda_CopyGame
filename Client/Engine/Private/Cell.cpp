#include "..\Public\Cell.h"
#include "VIBuffer_Navigation.h"

CCell::CCell(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: m_pDevice(pDevice)
	, m_pContext(pContext)
{	
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);	
}

HRESULT CCell::Initialize(const _float3 * pPoints, _int iIndex)
{
	m_iIndex = iIndex;

	memcpy(m_vPoints, pPoints, sizeof(_float3) * POINT_END);

	_float3		vLines[LINE_END];

	XMStoreFloat3(&vLines[LINE_AB], XMLoadFloat3(&m_vPoints[POINT_B]) - XMLoadFloat3(&m_vPoints[POINT_A]));
	m_vNormals[LINE_AB] = _float3(vLines[LINE_AB].z * -1.f, 0.f, vLines[LINE_AB].x);

	XMStoreFloat3(&vLines[LINE_BC], XMLoadFloat3(&m_vPoints[POINT_C]) - XMLoadFloat3(&m_vPoints[POINT_B]));
	m_vNormals[LINE_BC] = _float3(vLines[LINE_BC].z * -1.f, 0.f, vLines[LINE_BC].x);

	XMStoreFloat3(&vLines[LINE_CA], XMLoadFloat3(&m_vPoints[POINT_A]) - XMLoadFloat3(&m_vPoints[POINT_C]));
	m_vNormals[LINE_CA] = _float3(vLines[LINE_CA].z * -1.f, 0.f, vLines[LINE_CA].x);

#ifdef _DEBUG
	m_pVIBuffer = CVIBuffer_Navigation::Create(m_pDevice, m_pContext, m_vPoints);
	if (nullptr == m_pVIBuffer)
		return E_FAIL;
#endif // _DEBUG
	
	return S_OK;
}

_bool CCell::Compare_Points(const _float3 * pSourPoint, const _float3 * pDestPoint)
{
	if (XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT_A]), XMLoadFloat3(pSourPoint)))	
	{
		if (XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT_B]), XMLoadFloat3(pDestPoint)))
			return true;
	
		if (XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT_C]), XMLoadFloat3(pDestPoint)))
			return true;		
	}

	if (XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT_B]), XMLoadFloat3(pSourPoint)))
	{
		if (XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT_C]), XMLoadFloat3(pDestPoint)))
			return true;

		if (XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT_A]), XMLoadFloat3(pDestPoint)))
			return true;
	}

	if (XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT_C]), XMLoadFloat3(pSourPoint)))
	{
		if (XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT_A]), XMLoadFloat3(pDestPoint)))
			return true;

		if (XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT_B]), XMLoadFloat3(pDestPoint)))
			return true;
	}

	return false;
}

_bool CCell::isIn(_fvector vPosition, _int* pNeighborIndex, _float3* vLastNormal)
{
	for (_uint i = 0; i < LINE_END; ++i)
	{
		_vector		vDir = XMVector3Normalize(vPosition - XMLoadFloat3(&m_vPoints[i]));
		if (0 < XMVectorGetX(XMVector3Dot(vDir, XMVector3Normalize(XMLoadFloat3(&m_vNormals[i])))))
		{
			*pNeighborIndex = m_iNeighborIndices[i];
			if(vLastNormal != nullptr)
				*vLastNormal = m_vNormals[i];
			return false;
		}
		if (vLastNormal != nullptr)
			*vLastNormal = m_vNormals[i];
	}

	return true;
}


_vector CCell::Get_Center()
{

	_float PositionX = (m_vPoints[POINT_A].x + m_vPoints[POINT_B].x + m_vPoints[POINT_C].x) / 3.f;
	_float PositionY = (m_vPoints[POINT_A].y + m_vPoints[POINT_B].y + m_vPoints[POINT_C].y) / 3.f;
	_float PositionZ = (m_vPoints[POINT_A].z + m_vPoints[POINT_B].z + m_vPoints[POINT_C].z) / 3.f;
	return XMVectorSet(PositionX, PositionY, PositionZ, 1.f);
}


#ifdef _DEBUG
HRESULT CCell::Render()
{
	if (nullptr == m_pVIBuffer)
		return E_FAIL;	

	m_pVIBuffer->Render();

	return S_OK;
}
#endif // _DEBUG

CCell * CCell::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const _float3 * pPoints, _int  iIndex)
{
	CCell*	pInstance = new CCell(pDevice, pContext);

	if (FAILED(pInstance->Initialize(pPoints, iIndex)))
	{
		ERR_MSG(TEXT("Failed to Created : CCell"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCell::Free()
{
#ifdef _DEBUG
	Safe_Release(m_pVIBuffer);	
#endif // _DEBUG

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
