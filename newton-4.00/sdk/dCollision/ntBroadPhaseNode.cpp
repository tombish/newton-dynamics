/* Copyright (c) <2003-2019> <Julio Jerez, Newton Game Dynamics>
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 
* 3. This notice may not be removed or altered from any source distribution.
*/

#include "ndCollisionStdafx.h"
#include "ntBodyKinematic.h"
#include "ntBroadPhaseNode.h"

#define D_AABB_QUANTIZATION		dFloat32 (8.0f)
#define D_AABB_INV_QUANTIZATION	(dFloat32 (1.0f) / D_AABB_QUANTIZATION)

dVector ntBroadPhaseNode::m_aabbQuantization(D_AABB_QUANTIZATION, D_AABB_QUANTIZATION, D_AABB_QUANTIZATION, dFloat32 (0.0f));
dVector ntBroadPhaseNode::m_aabbInvQuantization(D_AABB_INV_QUANTIZATION, D_AABB_INV_QUANTIZATION, D_AABB_INV_QUANTIZATION, dFloat32(0.0f));

ntBroadPhaseBodyNode::ntBroadPhaseBodyNode(ntBodyKinematic* const body)
	:ntBroadPhaseNode(nullptr)
	,m_body(body)
{
	SetAABB(body->m_minAABB, body->m_maxAABB);
	m_body->SetBroadPhaseBodyNode(this);
}

ntBroadPhaseBodyNode::~ntBroadPhaseBodyNode()
{
	m_body->SetBroadPhaseBodyNode(nullptr);
}

void ntBroadPhaseNode::SetAABB(const dVector& minBox, const dVector& maxBox)
{
	dAssert(minBox.m_x <= maxBox.m_x);
	dAssert(minBox.m_y <= maxBox.m_y);
	dAssert(minBox.m_z <= maxBox.m_z);

	dVector p0(minBox * m_aabbQuantization);
	dVector p1(maxBox * m_aabbQuantization + dVector::m_one);

	m_minBox = p0.Floor() * m_aabbInvQuantization;
	m_maxBox = p1.Floor() * m_aabbInvQuantization;

	dAssert(m_minBox.m_w == dFloat32(0.0f));
	dAssert(m_maxBox.m_w == dFloat32(0.0f));

	dVector size(m_maxBox - m_minBox);
	m_surfaceArea = size.DotProduct(size.ShiftTripleRight()).GetScalar();
}

ntBroadPhaseTreeNode::ntBroadPhaseTreeNode(ntBroadPhaseNode* const sibling, ntBroadPhaseNode* const myNode)
	:ntBroadPhaseNode(sibling->m_parent)
	,m_left(sibling)
	,m_right(myNode)
	,m_fitnessNode(nullptr)
{
	if (m_parent) 
	{
		ntBroadPhaseTreeNode* const myParent = (ntBroadPhaseTreeNode*)m_parent;
		if (myParent->m_left == sibling) 
		{
			myParent->m_left = this;
		} 
		else 
		{
			dAssert(myParent->m_right == sibling);
			myParent->m_right = this;
		}
	}

	sibling->m_parent = this;
	myNode->m_parent = this;

	ntBroadPhaseNode* const left = m_left;
	ntBroadPhaseNode* const right = m_right;

	m_minBox = left->m_minBox.GetMin(right->m_minBox);
	m_maxBox = left->m_maxBox.GetMax(right->m_maxBox);
	dVector side0(m_maxBox - m_minBox);
	m_surfaceArea = side0.DotProduct(side0.ShiftTripleRight()).m_x;
}

ntBroadPhaseTreeNode::~ntBroadPhaseTreeNode()
{
	if (m_left) 
	{
		delete m_left;
	}
	if (m_right) 
	{
		delete m_right;
	}
}
