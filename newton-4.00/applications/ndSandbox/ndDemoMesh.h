/* Copyright (c) <2003-2019> <Newton Game Dynamics>
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely
*/

#ifndef _D_MESH_H_
#define _D_MESH_H_

#include "ndSandboxStdafx.h"

class ndDemoMesh;
class ndDemoEntity;
class ndShaderPrograms;
class ndDemoEntityManager;

struct ndMeshVector
{
	ndMeshVector() {}
	ndMeshVector(GLfloat x, GLfloat y, GLfloat z)
		:m_x(x), m_y(y), m_z(z)
	{
	}

	GLfloat m_x;
	GLfloat m_y;
	GLfloat m_z;
};

struct ndMeshVector4: public ndMeshVector
{
	GLfloat m_w;
};

struct ndMeshUV
{
	GLfloat m_u;
	GLfloat m_v;
};

struct ndPointNormal
{
	ndMeshVector m_posit;
	ndMeshVector m_normal;
};

struct ndMeshPointUV: public ndPointNormal
{
	ndMeshUV m_uv;
};

class ndDemoSubMeshMaterial
{
	public:
	ndDemoSubMeshMaterial();
	~ndDemoSubMeshMaterial();

	dVector m_ambient;
	dVector m_diffuse;
	dVector m_specular;
	dFloat32 m_opacity;
	dFloat32 m_shiness;
	dUnsigned32 m_textureHandle;
	char  m_textureName[32];
};

class ndDemoSubMesh
{
	public:
	ndDemoSubMesh();
	~ndDemoSubMesh();
	void SetOpacity(dFloat32 opacity);

	ndDemoSubMeshMaterial m_material;
	dInt32 m_indexCount;
	dInt32 m_segmentStart;
	bool m_hasTranparency;
};

class ndDemoMeshInterface: public dClassAlloc, public dRefCounter<ndDemoMeshInterface>
{
	public:
	ndDemoMeshInterface();
	~ndDemoMeshInterface();
	const dString& GetName () const;

	bool GetVisible () const;
	void SetVisible (bool visibilityFlag);

	virtual ndDemoMeshInterface* Clone(ndDemoEntity* const owner) { dAssert(0); return nullptr; }

	virtual void Render (ndDemoEntityManager* const scene, const dMatrix& modelMatrix) = 0;
	virtual void RenderTransparency(ndDemoEntityManager* const scene, const dMatrix& modelMatrix) {}
	//virtual NewtonMesh* CreateNewtonMesh(NewtonWorld* const world, const dMatrix& meshMatrix) = 0;

	dString m_name;
	bool m_isVisible;
};

class ndDemoMesh: public ndDemoMeshInterface, public dList<ndDemoSubMesh>
{
	public:
	ndDemoMesh(const char* const name);
	ndDemoMesh(const ndDemoMesh& mesh, const ndShaderPrograms& shaderCache);
	ndDemoMesh(const char* const name, dMeshEffect* const meshNode, const ndShaderPrograms& shaderCache);
	ndDemoMesh(const char* const name, const ndShaderPrograms& shaderCache, dFloat32* const elevation, int size, dFloat32 cellSize, dFloat32 texelsDensity, int tileSize);
	ndDemoMesh(const char* const name, const ndShaderPrograms& shaderCache, const ndShapeInstance* const collision, const char* const texture0, const char* const texture1, const char* const texture2, dFloat32 opacity = 1.0f, const dMatrix& uvMatrix = dGetIdentityMatrix());

	virtual ndDemoMeshInterface* Clone(ndDemoEntity* const owner) 
	{ 
		AddRef(); 
		return this;
	}

	ndDemoSubMesh* AddSubMesh();
	virtual const char* GetTextureName (const ndDemoSubMesh* const subMesh) const;

	virtual void Render (ndDemoEntityManager* const scene, const dMatrix& modelMatrix);
	virtual void RenderTransparency(ndDemoEntityManager* const scene, const dMatrix& modelMatrix);
	virtual void RenderNormals ();
	
	//virtual NewtonMesh* CreateNewtonMesh(NewtonWorld* const world, const dMatrix& meshMatrix);

	protected:
	virtual ~ndDemoMesh();

	void  SpliteSegment(dListNode* const node, int maxIndexCount);

	public:
	void ResetOptimization();
	void OptimizeForRender(const dArray<ndMeshPointUV>& points, const dArray<dInt32>& indices);

	dInt32 m_indexCount;
	dInt32 m_vertexCount;
	dInt32 m_textureLocation;
	dInt32 m_transparencyLocation;
	dInt32 m_normalMatrixLocation;
	dInt32 m_projectMatrixLocation;
	dInt32 m_viewModelMatrixLocation;
	dInt32 m_directionalLightDirLocation;

	dInt32 m_materialAmbientLocation;
	dInt32 m_materialDiffuseLocation;
	dInt32 m_materialSpecularLocation;

	GLuint m_shader;
	GLuint m_indexBuffer;
	GLuint m_vertexBuffer;
	GLuint m_vetextArrayBuffer;
	bool m_hasTransparency;
};

class ndDemoSkinMesh: public ndDemoMeshInterface
{
	public:
	struct dWeightBoneIndex
	{
		int m_boneIndex[4];
	};

	ndDemoSkinMesh(const ndDemoSkinMesh& clone, ndDemoEntity* const owner);
	//ndDemoSkinMesh(dScene* const scene, ndDemoEntity* const owner, dScene::dTreeNode* const meshNode, const dTree<ndDemoEntity*, dScene::dTreeNode*>& boneMap, const ndShaderPrograms& shaderCache);
	~ndDemoSkinMesh();

	void Render (ndDemoEntityManager* const scene, const dMatrix& modelMatrix);
	//NewtonMesh* CreateNewtonMesh(NewtonWorld* const world, const dMatrix& meshMatrix);

	protected: 
	virtual ndDemoMeshInterface* Clone(ndDemoEntity* const owner);
	int CalculateMatrixPalette(dMatrix* const bindMatrix) const;
	void ConvertToGlMatrix(int count, const dMatrix* const bindMatrix, GLfloat* const glMatrices) const;
	//dGeometryNodeSkinClusterInfo* FindSkinModifier(dScene* const scene, dScene::dTreeNode* const meshNode) const;
	void OptimizeForRender(const ndDemoSubMesh& segment, const dVector* const pointWeights, const dWeightBoneIndex* const pointSkinBone) const;

	ndDemoMesh* m_mesh;
	ndDemoEntity* m_entity; 
	dMatrix* m_bindingMatrixArray;
	int m_nodeCount; 
	int m_shader;
};

class ndDemoBezierCurve: public ndDemoMeshInterface
{
	public:
	ndDemoBezierCurve(const dBezierSpline& curve);
	//ndDemoBezierCurve(const dScene* const scene, dScene::dTreeNode* const meshNode);

	int GetRenderResolution() const;
	void SetRenderResolution(int breaks);

	virtual void Render(ndDemoEntityManager* const scene, const dMatrix& modelMatrix);


	//virtual NewtonMesh* CreateNewtonMesh(NewtonWorld* const world, const dMatrix& meshMatrix);

	dBezierSpline m_curve;
	int m_renderResolution;
};

class ndFlatShadedDebugMesh: public ndDemoMeshInterface
{
	public:
	ndFlatShadedDebugMesh(const ndShaderPrograms& shaderCache, const ndShapeInstance* const collision);
	~ndFlatShadedDebugMesh();

	void SetColor(const dVector& color)
	{
		m_color.m_x = GLfloat(color.m_x);
		m_color.m_y = GLfloat(color.m_y);
		m_color.m_z = GLfloat(color.m_z);
		m_color.m_w = GLfloat(color.m_w);
	}
	void Render(ndDemoEntityManager* const scene, const dMatrix& modelMatrix);

	ndMeshVector4 m_color;
	dInt32 m_indexCount;
	dInt32 m_shadeColorLocation;
	dInt32 m_normalMatrixLocation;
	dInt32 m_projectMatrixLocation;
	dInt32 m_viewModelMatrixLocation;

	GLuint m_shader;
	GLuint m_vertexBuffer;
	GLuint m_vetextArrayBuffer;
	GLuint m_triangleIndexBuffer;
};

class ndWireFrameDebugMesh: public ndDemoMeshInterface
{
	public:
	ndWireFrameDebugMesh(const ndShaderPrograms& shaderCache, const ndShapeInstance* const collision);
	~ndWireFrameDebugMesh();

	void SetColor(const dVector& color)
	{
		m_color.m_x = GLfloat(color.m_x);
		m_color.m_y = GLfloat(color.m_y);
		m_color.m_z = GLfloat(color.m_z);
		m_color.m_w = GLfloat(color.m_w);
	}

	void Render(ndDemoEntityManager* const scene, const dMatrix& modelMatrix);

	ndMeshVector4 m_color;
	dInt32 m_indexCount;
	dInt32 m_shadeColorLocation;
	dInt32 m_projectionViewModelMatrixLocation;

	GLuint m_shader;
	GLuint m_vertexBuffer;
	GLuint m_vetextArrayBuffer;
	GLuint m_lineIndexBuffer;
};

#endif 


