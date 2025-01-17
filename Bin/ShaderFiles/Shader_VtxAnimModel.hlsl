
#include "Client_Shader_Defines.hpp"

matrix			g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
texture2D		g_DiffuseTexture;
texture2D		g_NormalTexture;
texture2D		g_DissolveTexture;
texture2D		g_SpecularTexture;

float			g_HitRed = 0.2f;
float			g_fAlpha = 1.f;
float			g_fColorPercent = 0.f;
float			g_DissolveSize = 1.5f;
vector			g_DissolveColor = vector(1.f, 0.7f, 0.f, 1);
vector			g_vColor = vector(1.f, 1.f, 1.f, 1);

float			g_fMinRange = 5.f;
float			g_fMaxRange = 10.f;
float4			g_PlayerPosition;



/* 정점들에게 곱해져야할 행렬. */
/* 정점들은 메시에게 소속. 이때 곱해져야하는 뼈의 행렬 == 이 메시에 영향을 주는 뼈다. */
/* 모델 전체에 정의된 뼈다(xxxxx) */
matrix			g_BoneMatrices[256];

struct VS_IN
{
	float3		vPosition : POSITION;
	float3		vNormal : NORMAL;
	float2		vTexUV : TEXCOORD0;
	float3		vTangent : TANGENT;
	uint4		vBlendIndex : BLENDINDEX;
	float4		vBlendWeight : BLENDWEIGHT;
};

struct VS_OUT
{
	float4		vPosition : SV_POSITION;
	float3		vNormal : NORMAL;
	float2		vTexUV : TEXCOORD0;
	float4		vProjPos : TEXCOORD1;

	float3		vTangent : TANGENT;
	float3		vBinormal : BINORMAL;

	float4		vWorldPos : TEXCOORD2;
};

/* DrawIndexed함수를 호출하면. */
/* 인덱스버퍼에 담겨있는 인덱스번째의 정점을 VS_MAIN함수에 인자로 던진다. VS_IN에 저장된다. */
/* 일반적으로 TriangleList로 그릴경우, 정점 세개를 각각 VS_MAIN함수의 인자로 던진다. */
VS_OUT VS_MAIN(VS_IN In)
{
	VS_OUT Out = (VS_OUT)0;

	matrix matWV, matWVP;
	matWV = mul(g_WorldMatrix, g_ViewMatrix);
	matWVP = mul(matWV, g_ProjMatrix);

	float fW = 1.f - (In.vBlendWeight.x + In.vBlendWeight.y + In.vBlendWeight.z);

	matrix BoneMatrix = g_BoneMatrices[In.vBlendIndex.x] * In.vBlendWeight.x +
		g_BoneMatrices[In.vBlendIndex.y] * In.vBlendWeight.y +
		g_BoneMatrices[In.vBlendIndex.z] * In.vBlendWeight.z +
		g_BoneMatrices[In.vBlendIndex.w] * fW;

	vector vPosition = mul(vector(In.vPosition, 1.f), BoneMatrix);
	vector vNormal = mul(vector(In.vNormal, 0.f), BoneMatrix);
	vNormal = normalize(mul(vNormal, g_WorldMatrix));

	/* 정점의 위치에 월드 뷰 투영행렬을 곱한다. 현재 정점은 ViewSpace에 존재하낟. */
	/* 투영행렬까지 곱하면 정점위치의 w에 뷰스페이스 상의 z를 보관한다. == Out.vPosition이 반드시 float4이어야하는 이유. */
	Out.vPosition = mul(vPosition, matWVP);
	Out.vNormal = vNormal.xyz;
	Out.vTexUV = In.vTexUV;
	Out.vProjPos = Out.vPosition;
	Out.vTangent = normalize(mul(vector(In.vTangent, 0.f), g_WorldMatrix)).xyz;
	Out.vBinormal = cross(Out.vNormal, Out.vTangent);

	return Out;
}


struct PS_IN
{
	float4		vPosition : SV_POSITION;
	float3		vNormal : NORMAL;
	float2		vTexUV : TEXCOORD0;
	float4		vProjPos : TEXCOORD1;

	float3		vTangent : TANGENT;
	float3		vBinormal : BINORMAL;

	float4		vWorldPos : TEXCOORD2;
};

struct PS_OUT
{
	float4		vDiffuse : SV_TARGET0;
	float4		vNormal : SV_TARGET1;
	float4		vDepth : SV_TARGET2;
	float4		vSpecular : SV_TARGET3;
	vector		vLightDepth : SV_TARGET4;
	float4		vBlur : SV_TARGET5;
};

/* 이렇게 만들어진 픽셀을 PS_MAIN함수의 인자로 던진다. */
/* 리턴하는 색은 Target0 == 장치에 0번째에 바인딩되어있는 렌더타겟(일반적으로 백버퍼)에 그린다. */
/* 그래서 백버퍼에 색이 그려진다. */
PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;
	float4 vTextureNormal = g_NormalTexture.Sample(LinearSampler, In.vTexUV);
	float3 vNormal;

	vNormal = float3(vTextureNormal.x, vTextureNormal.y, sqrt(1 - vTextureNormal.x * vTextureNormal.x - vTextureNormal.y * vTextureNormal.y));

	float3x3 WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);
	vNormal = mul(vNormal, WorldMatrix);

	Out.vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexUV);
	Out.vNormal = vector(vNormal * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 500.f, 0.f, 0.f);
	Out.vSpecular = g_SpecularTexture.Sample(LinearSampler, In.vTexUV);

	////float4		vFogColor = vector(186, 218, 255, 0) / 255.f;
	//float4		vFogColor = vector(66, 116, 214, 0) / 255.f;
	//float		fDistance = length(g_PlayerPosition - In.vWorldPos);
	//float		fFogPower = max(fDistance - g_fMinRange, 0.f) / (g_fMaxRange - g_fMinRange);

	//Out.vDiffuse += vFogColor * fFogPower;

	if (Out.vDiffuse.a <= 0.1f)
		discard;


	Out.vBlur = Out.vDiffuse;
	return Out;
}


PS_OUT PS_MAIN_HIT(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	float4 vTextureNormal = g_NormalTexture.Sample(LinearSampler, In.vTexUV);
	float3 vNormal;

	vNormal = float3(vTextureNormal.x, vTextureNormal.y, sqrt(1 - vTextureNormal.x * vTextureNormal.x - vTextureNormal.y * vTextureNormal.y));

	float3x3 WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);
	vNormal = mul(vNormal, WorldMatrix);

	Out.vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexUV);
	Out.vNormal = vector(vNormal * 0.5f + 0.5f, 0.f);

	Out.vDiffuse.gb *= g_HitRed;
	Out.vDiffuse.r *= (g_HitRed + 0.2f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 500.f, 0.f, 0.f);
	Out.vSpecular = g_SpecularTexture.Sample(LinearSampler, In.vTexUV);

	//float4		vFogColor = vector(186, 218, 255, 0) / 255.f;
	float4		vFogColor = vector(66, 116, 214, 0) / 255.f;
	float		fDistance = length(g_PlayerPosition - In.vWorldPos);
	float		fFogPower = max(fDistance - g_fMinRange, 0.f) / (g_fMaxRange - g_fMinRange);

	Out.vDiffuse += vFogColor * fFogPower;

	if (Out.vDiffuse.a <= 0.3f)
		discard;

	Out.vBlur = Out.vDiffuse;
	return Out;
}


PS_OUT PS_DEAD(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;
	float4 vTextureNormal = g_NormalTexture.Sample(LinearSampler, In.vTexUV);
	float3 vNormal;

	vNormal = float3(vTextureNormal.x, vTextureNormal.y, sqrt(1 - vTextureNormal.x * vTextureNormal.x - vTextureNormal.y * vTextureNormal.y));

	float3x3 WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);
	vNormal = mul(vNormal, WorldMatrix);

	Out.vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexUV);
	Out.vNormal = vector(vNormal * 0.5f + 0.5f, 0.f);
	Out.vSpecular = g_SpecularTexture.Sample(LinearSampler, In.vTexUV);
	vector vDissolve = g_DissolveTexture.Sample(LinearSampler, In.vTexUV);
	if (vDissolve.r >= g_fAlpha)
	{
		Out.vDiffuse.a = 1.f;
	}
	else
	{
		Out.vDiffuse.a = 0.f;
	}

	
	if (vDissolve.r < g_fAlpha * g_DissolveSize)
	{
		Out.vDiffuse.rgb =  g_DissolveColor.rgb;
	}


	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 500.f, 0.f, 0.f);


	//float4		vFogColor = vector(186, 218, 255, 0) / 255.f;
	float4		vFogColor = vector(66, 116, 214, 0) / 255.f;
	float		fDistance = length(g_PlayerPosition - In.vWorldPos);
	float		fFogPower = max(fDistance - g_fMinRange, 0.f) / (g_fMaxRange - g_fMinRange);

	Out.vDiffuse += vFogColor * fFogPower;

	if (Out.vDiffuse.a <= 0.0f)
		discard;

	Out.vBlur = Out.vDiffuse;
	return Out;
}


PS_OUT PS_MAIN_SHADOW(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vLightDepth.r = In.vProjPos.w / 500.f;

	Out.vLightDepth.a = 1.f;

	return Out;
}

PS_OUT PS_CHARGE(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;
	float4 vTextureNormal = g_NormalTexture.Sample(LinearSampler, In.vTexUV);
	float3 vNormal;

	vNormal = float3(vTextureNormal.x, vTextureNormal.y, sqrt(1 - vTextureNormal.x * vTextureNormal.x - vTextureNormal.y * vTextureNormal.y));

	float3x3 WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);
	vNormal = mul(vNormal, WorldMatrix);

	Out.vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexUV);
	Out.vNormal = vector(vNormal * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 500.f, 0.f, 0.f);
	Out.vSpecular = g_SpecularTexture.Sample(LinearSampler, In.vTexUV);

	vector GetColor = g_vColor / 255.f;

	Out.vDiffuse.rgb += GetColor.rgb * g_fColorPercent;

	////float4		vFogColor = vector(186, 218, 255, 0) / 255.f;
	//float4		vFogColor = vector(66, 116, 214, 0) / 255.f;
	//float		fDistance = length(g_PlayerPosition - In.vWorldPos);
	//float		fFogPower = max(fDistance - g_fMinRange, 0.f) / (g_fMaxRange - g_fMinRange);

	//Out.vDiffuse += vFogColor * fFogPower;

	if (Out.vDiffuse.a <= 0.1f)
		discard;
	Out.vBlur = Out.vDiffuse;
	return Out;
}

technique11 DefaultTechnique
{
	pass Default
	{
		SetRasterizerState(RS_Default);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 1.f), 0xffffffff);
		SetDepthStencilState(DSS_Default, 0);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN();
	}

	pass Hit
	{
		SetRasterizerState(RS_Default);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 1.f), 0xffffffff);
		SetDepthStencilState(DSS_Default, 0);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_HIT();
	}

	pass Dead
	{
		SetRasterizerState(RS_Default);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 1.f), 0xffffffff);
		SetDepthStencilState(DSS_Default, 0);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_DEAD();
	}

	pass Shadow
	{
		SetRasterizerState(RS_Default);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 1.f), 0xffffffff);
		SetDepthStencilState(DSS_Priority, 0);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_SHADOW();
	}

	pass Charge
	{
		SetRasterizerState(RS_Default);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 1.f), 0xffffffff);
		SetDepthStencilState(DSS_Default, 0);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_CHARGE();
	}
}