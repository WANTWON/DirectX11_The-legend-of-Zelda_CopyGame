#include "..\Public\MeshContainer.h"
#include "HierarchyNode.h"
#include "Picking.h"
#include "Transform.h"

CMeshContainer::CMeshContainer(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CVIBuffer(pDevice, pContext)
{
}

CMeshContainer::CMeshContainer(const CMeshContainer & rhs)
	: CVIBuffer(rhs)
	, m_pAIMesh(rhs.m_pAIMesh)
	, m_iMaterialIndex(rhs.m_iMaterialIndex)
	, m_iNumBones(rhs.m_iNumBones)
{
	strcpy_s(m_szName, rhs.m_szName);
}

void CMeshContainer::Get_BoneMatrices(_float4x4 * pBoneMatrices, _fmatrix PivotMatrix)
{
	if (0 == m_iNumBones)
	{
		XMStoreFloat4x4(&pBoneMatrices[0], XMMatrixIdentity());
		return;
	}

	_uint		iNumBones = 0;

	for (auto& pBoneNode : m_Bones)
	{
		/* Offset * Combined * Pivot */
		XMStoreFloat4x4(&pBoneMatrices[iNumBones++], /*XMMatrixTranspose*/(pBoneNode->Get_OffsetMatrix() * pBoneNode->Get_CombinedTransformationMatrix() * PivotMatrix));
	}
}

HRESULT CMeshContainer::Initialize_Prototype(CModel::TYPE eModelType, const aiMesh * pAIMesh, CModel* pModel, _fmatrix PivotMatrix)
{	
	strcpy_s(m_szName, pAIMesh->mName.data);
	m_iMaterialIndex = pAIMesh->mMaterialIndex;
	m_pAIMesh = pAIMesh;

#pragma region VERTICES

	HRESULT			hr = 0;

	if (CModel::TYPE_NONANIM == eModelType)
		hr = Create_VertexBuffer_NonAnimModel(pAIMesh, PivotMatrix);
	else
		hr = Create_VertexBuffer_AnimModel(pAIMesh, pModel);

	if (FAILED(hr))
		return E_FAIL;

	
#pragma endregion


#pragma region Indices
	ZeroMemory(&m_BufferDesc, sizeof(D3D11_BUFFER_DESC));

	m_iIndicesByte = sizeof(FACEINDICES32);
	m_iNumPrimitive = m_pAIMesh->mNumFaces;
	m_iNumIndicesPerPrimitive = 3;

	m_BufferDesc.ByteWidth = m_iIndicesByte * m_iNumPrimitive;
	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT; /* 정적버퍼를 생성한다. */
	m_BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = sizeof(_ushort);

	FACEINDICES32*			pIndices = new FACEINDICES32[m_iNumPrimitive];

	for (_uint i = 0; i < m_iNumPrimitive; ++i)
	{
		aiFace		AIFace = m_pAIMesh->mFaces[i];

		pIndices[i]._0 = AIFace.mIndices[0];
		pIndices[i]._1 = AIFace.mIndices[1];
		pIndices[i]._2 = AIFace.mIndices[2];
	}

	ZeroMemory(&m_SubResourceData, sizeof(D3D11_SUBRESOURCE_DATA));
	m_SubResourceData.pSysMem = pIndices;

	/* 정점을 담기 위한 공간을 할당하고, 내가 전달해준 배열의 값들을 멤카피한다. */
	if (FAILED(__super::Create_IndexBuffer()))
		return E_FAIL;

	Safe_Delete_Array(pIndices);
#pragma endregion



	return S_OK;
}

HRESULT CMeshContainer::Initialize(void * pArg)
{
	return S_OK;
}

HRESULT CMeshContainer::SetUp_Bones(CModel * pModel)
{
	for (_uint i = 0; i < m_iNumBones; ++i)
	{
		aiBone*		pAIBone = m_pAIMesh->mBones[i];

		CHierarchyNode*	pBoneNode = pModel->Get_BonePtr(pAIBone->mName.data);
		if (nullptr == pBoneNode)
			return E_FAIL;		

		m_Bones.push_back(pBoneNode);

		Safe_AddRef(pBoneNode);

		_float4x4		OffsetMatrix;

		memcpy(&OffsetMatrix, &pAIBone->mOffsetMatrix, sizeof(_float4x4));

		pBoneNode->Set_OffsetMatrix(XMMatrixTranspose(XMLoadFloat4x4(&OffsetMatrix)));
	}


	if (0 == m_iNumBones)
	{
		CHierarchyNode*		pNode = pModel->Get_BonePtr(m_szName);
		if (nullptr == pNode)
			return S_OK;

		m_iNumBones = 1;

		m_Bones.push_back(pNode);
		Safe_AddRef(pNode);
	}

	return S_OK;
}

_bool CMeshContainer::Picking(CTransform * pTransform, _float3 * pOut)
{
	CPicking*		pPicking = GET_INSTANCE(CPicking);

	_matrix		WorldMatrixInv = pTransform->Get_WorldMatrixInverse();
	_float3			vTempRayDir, vTempRayPos;
	XMVECTOR		vRayDir, vRayPos;
	pPicking->Compute_LocalRayInfo(&vTempRayDir, &vTempRayPos, pTransform);

	vRayPos = XMLoadFloat3(&vTempRayPos);
	vRayPos = XMVectorSetW(vRayPos, 1.f);
	vRayDir = XMLoadFloat3(&vTempRayDir);
	vRayDir = XMVector3Normalize(vRayDir);

	FACEINDICES32*		pIndices = new FACEINDICES32[m_iNumPrimitive];

	for (_uint i = 0; i < m_iNumPrimitive / 3; ++i)
	{
		_matrix	WorldMatrix = pTransform->Get_WorldMatrix();

		aiFace		AIFace = m_pAIMesh->mFaces[i];

		_uint i0 = AIFace.mIndices[0];
		_uint i1 = AIFace.mIndices[1];
		_uint i2 = AIFace.mIndices[2];

		_float3 vPosition0 = _float3(0.f, 0.f, 0.f);
		_float3 vPosition1 = _float3(0.f, 0.f, 0.f);
		_float3 vPosition2 = _float3(0.f, 0.f, 0.f);

		memcpy(&vPosition0, &m_pAIMesh->mVertices[i0], sizeof(_float3));
		_vector vTemp_1 = XMLoadFloat3(&vPosition0);
		vTemp_1 = XMVectorSetW(vTemp_1, 1.f);
		memcpy(&vPosition1, &m_pAIMesh->mVertices[i1], sizeof(_float3));
		_vector vTemp_2 = XMLoadFloat3(&vPosition1);
		vTemp_2 = XMVectorSetW(vTemp_2, 1.f);
		memcpy(&vPosition2, &m_pAIMesh->mVertices[i2], sizeof(_float3));
		_vector vTemp_3 = XMLoadFloat3(&vPosition2);
		vTemp_3 = XMVectorSetW(vTemp_3, 1.f);

		_float fDist = 0;

		if (true == TriangleTests::Intersects((FXMVECTOR)vRayPos, (FXMVECTOR)vRayDir, (FXMVECTOR)vTemp_1, (GXMVECTOR)vTemp_2, (HXMVECTOR)vTemp_3, fDist))
		{
			_vector	vPickPos = vRayPos + vRayDir * fDist;

			XMStoreFloat3(pOut, XMVector3TransformCoord(vPickPos, WorldMatrix));

			Safe_Delete_Array(pIndices);
			RELEASE_INSTANCE(CPicking);
			return true;
		}

	}

	Safe_Delete_Array(pIndices);
	RELEASE_INSTANCE(CPicking);
	return false;
}




HRESULT CMeshContainer::Create_VertexBuffer_NonAnimModel(const aiMesh* pAIMesh, _fmatrix PivotMatrix)
{
	ZeroMemory(&m_BufferDesc, sizeof(D3D11_BUFFER_DESC));

	m_iStride = sizeof(VTXMODEL);
	m_iNumVertices = pAIMesh->mNumVertices;
	m_iNumVertexBuffers = 1;
	m_eFormat = DXGI_FORMAT_R32_UINT;
	m_eTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_BufferDesc.ByteWidth = m_iStride * m_iNumVertices;
	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT; /* 정적버퍼를 생성한다. */
	m_BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = m_iStride;

	VTXMODEL*			pVertices = new VTXMODEL[m_iNumVertices];

	for (_uint i = 0; i < m_iNumVertices; ++i)
	{
		memcpy(&pVertices[i].vPosition, &m_pAIMesh->mVertices[i], sizeof(_float3));
		XMStoreFloat3(&pVertices[i].vPosition, XMVector3TransformCoord(XMLoadFloat3(&pVertices[i].vPosition), PivotMatrix));

		memcpy(&pVertices[i].vNormal, &m_pAIMesh->mNormals[i], sizeof(_float3));
		XMStoreFloat3(&pVertices[i].vNormal, XMVector3TransformCoord(XMLoadFloat3(&pVertices[i].vNormal), PivotMatrix));

	
		memcpy(&pVertices[i].vTexUV, &m_pAIMesh->mTextureCoords[0][i], sizeof(_float2));
		memcpy(&pVertices[i].vTangent, &m_pAIMesh->mTangents[i], sizeof(_float3));
		
	}

	ZeroMemory(&m_SubResourceData, sizeof(D3D11_SUBRESOURCE_DATA));
	m_SubResourceData.pSysMem = pVertices;

	/* 정점을 담기 위한 공간을 할당하고, 내가 전달해준 배열의 값들을 멤카피한다. */
	if (FAILED(__super::Create_VertexBuffer()))
		return E_FAIL;

	Safe_Delete_Array(pVertices);

	return S_OK;
}

HRESULT CMeshContainer::Create_VertexBuffer_AnimModel(const aiMesh* pAIMesh, CModel* pModel)
{
	ZeroMemory(&m_BufferDesc, sizeof(D3D11_BUFFER_DESC));


	m_iStride = sizeof(VTXANIMMODEL);
	m_iNumVertices = pAIMesh->mNumVertices;
	m_iNumVertexBuffers = 1;
	m_eFormat = DXGI_FORMAT_R32_UINT;
	m_eTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_BufferDesc.ByteWidth = m_iStride * m_iNumVertices;
	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT; /* 정적버퍼를 생성한다. */
	m_BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = m_iStride;

	VTXANIMMODEL*			pVertices = new VTXANIMMODEL[m_iNumVertices];
	ZeroMemory(pVertices, sizeof(VTXANIMMODEL) * m_iNumVertices);

	for (_uint i = 0; i < m_iNumVertices; ++i)
	{
		memcpy(&pVertices[i].vPosition, &m_pAIMesh->mVertices[i], sizeof(_float3));
		memcpy(&pVertices[i].vNormal, &m_pAIMesh->mNormals[i], sizeof(_float3));
		memcpy(&pVertices[i].vTexUV, &m_pAIMesh->mTextureCoords[0][i], sizeof(_float2));
		memcpy(&pVertices[i].vTangent, &m_pAIMesh->mTangents[i], sizeof(_float3));
	}

	m_iNumBones = pAIMesh->mNumBones;

	for (_uint i = 0; i < m_iNumBones; ++i)
	{
		aiBone*		pAIBone = pAIMesh->mBones[i];

		//CHierarchyNode*	pBoneNode = pModel->Get_BonePtr(pAIBone->mName.data);
		//if (nullptr == pBoneNode)
		//	return E_FAIL;		

		//m_Bones.push_back(pBoneNode);

		//Safe_AddRef(pBoneNode);

		/*_float4x4		OffsetMatrix; 

		memcpy(&OffsetMatrix, &pAIBone->mOffsetMatrix, sizeof(_float4x4));		

		pBoneNode->Set_OffsetMatrix(XMMatrixTranspose(XMLoadFloat4x4(&OffsetMatrix)));*/

		// pAIBone->mNumWeights : 몇개의 정점에 영향을 주고 있는지? 
		for (_uint j = 0; j < pAIBone->mNumWeights; ++j)
		{
			/* pAIBone->mWeights[j].mVertexId : 이 뼈가 j번째로 영향을 주는 정점의 인덱스다. */
			_uint		iVertexIndex = pAIBone->mWeights[j].mVertexId;			

			if (0 == pVertices[iVertexIndex].vBlendWeight.x)
			{
				pVertices[iVertexIndex].vBlendWeight.x = pAIBone->mWeights[j].mWeight;
				pVertices[iVertexIndex].vBlendIndex.x = i;
			}

			else if (0 == pVertices[iVertexIndex].vBlendWeight.y)
			{
				pVertices[iVertexIndex].vBlendWeight.y = pAIBone->mWeights[j].mWeight;
				pVertices[iVertexIndex].vBlendIndex.y = i;
			}

			else if (0 == pVertices[iVertexIndex].vBlendWeight.z)
			{
				pVertices[iVertexIndex].vBlendWeight.z = pAIBone->mWeights[j].mWeight;
				pVertices[iVertexIndex].vBlendIndex.z = i;
			}

			else if (0 == pVertices[iVertexIndex].vBlendWeight.w)
			{
				pVertices[iVertexIndex].vBlendWeight.w = pAIBone->mWeights[j].mWeight;
				pVertices[iVertexIndex].vBlendIndex.w = i;
			}
		}
	}


	ZeroMemory(&m_SubResourceData, sizeof(D3D11_SUBRESOURCE_DATA));
	m_SubResourceData.pSysMem = pVertices;

	/* 정점을 담기 위한 공간을 할당하고, 내가 전달해준 배열의 값들을 멤카피한다. */
	if (FAILED(__super::Create_VertexBuffer()))
		return E_FAIL;

	Safe_Delete_Array(pVertices);


	/*if (0 == m_iNumBones)
	{
		CHierarchyNode*		pNode = pModel->Get_BonePtr(m_szName);
		if (nullptr == pNode)
			return S_OK;

		m_iNumBones = 1;

		m_Bones.push_back(pNode);
		Safe_AddRef(pNode);
	}*/

	return S_OK;
}



CMeshContainer * CMeshContainer::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, CModel::TYPE eModelType, const aiMesh * pAIMesh, CModel * pModel, _fmatrix PivotMatrix)
{
	CMeshContainer*	pInstance = new CMeshContainer(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(eModelType, pAIMesh, pModel, PivotMatrix)))
	{
		ERR_MSG(TEXT("Failed to Created : CMeshContainer"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent * CMeshContainer::Clone(void * pArg)
{
	CMeshContainer*	pInstance = new CMeshContainer(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		ERR_MSG(TEXT("Failed to Cloned : CTransform"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMeshContainer::Free()
{
	__super::Free();

	for (auto& pBoneNode : m_Bones)
		Safe_Release(pBoneNode);

	m_Bones.clear();

}
