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

#ifndef __D_CONSTRAINT_H__
#define __D_CONSTRAINT_H__

#include "ndCollisionStdafx.h"

#define D_MAX_BOUND				dFloat32 (1.0e15f)
#define D_MIN_BOUND				(-D_MAX_BOUND)
#define D_INDEPENDENT_ROW		-1 
#define D_CONSTRAINT_MAX_ROWS	(3 * 16)
#define MIN_JOINT_PIN_LENGTH	dFloat32 (50.0f)

class ndBody;
class ndContact;
class ndConstraint;
class ndLeftHandSide;
class ndRightHandSide;
class ndBodyKinematic;
class ndJointBilateralConstraint;

D_MSV_NEWTON_ALIGN_32
class ndConstraintDebugCallback: public dClassAlloc
{
	public:
	ndConstraintDebugCallback()
	{
		m_debugScale = dFloat32(1.0f);
	}

	virtual ~ndConstraintDebugCallback() 
	{
	}

	virtual void DrawLine(const dVector& p0, const dVector& p1, const dVector& color) = 0;

	virtual void SetScale(dFloat32 scale)
	{
		m_debugScale = scale;
	}

	virtual void DrawFrame(const dMatrix& matrix)
	{
		dVector x(matrix.m_posit + matrix.RotateVector(dVector(m_debugScale, dFloat32(0.0f), dFloat32(0.0f), dFloat32(0.0f))));
		DrawLine(matrix.m_posit, x, dVector(dFloat32(1.0f), dFloat32(0.0f), dFloat32(0.0f), dFloat32(0.0f)));

		dVector y(matrix.m_posit + matrix.RotateVector(dVector(dFloat32(0.0f), m_debugScale, dFloat32(0.0f), dFloat32(0.0f))));
		DrawLine(matrix.m_posit, y, dVector(dFloat32(0.0f), dFloat32(1.0f), dFloat32(0.0f), dFloat32(0.0f)));

		dVector z(matrix.m_posit + matrix.RotateVector(dVector(dFloat32(0.0f), dFloat32(0.0f), m_debugScale, dFloat32(0.0f))));
		DrawLine(matrix.m_posit, z, dVector(dFloat32(0.0f), dFloat32(0.0f), dFloat32(1.0f), dFloat32(0.0f)));
	}

	//virtual void DrawPoint(const dVector& p0, dFloat32 thinckness = dFloat32(1.0f)) = 0;
	//virtual void SetOrthRendering() {};
	//virtual void ResetOrthRendering() {};
	//
	//dFloat32 GetScale() const
	//{ 
	//	return m_debugScale; 
	//} 
	//
	//void SetScale(dFloat32 scale) 
	//{ 
	//	m_debugScale = scale; 
	//}
	//
	//void DrawFrame(const dMatrix& matrix) 
	//{ 
	//	DrawFrame(matrix, m_debugScale); 
	//}
	//
	//void DrawFrame(const dMatrix& matrix, dFloat32 scale)
	//{
	//	dVector x(matrix.m_posit + matrix.RotateVector(dVector(scale, dFloat32(0.0f), dFloat32(0.0f), dFloat32(0.0f))));
	//	SetColor(dVector(dFloat32(1.0f), dFloat32(0.0f), dFloat32(0.0f), dFloat32(0.0f)));
	//	DrawLine(matrix.m_posit, x);
	//
	//	dVector y(matrix.m_posit + matrix.RotateVector(dVector(dFloat32(0.0f), scale, dFloat32(0.0f), dFloat32(0.0f))));
	//	SetColor(dVector(dFloat32(0.0f), dFloat32(1.0f), dFloat32(0.0f), dFloat32(0.0f)));
	//	DrawLine(matrix.m_posit, y);
	//
	//	dVector z(matrix.m_posit + matrix.RotateVector(dVector(dFloat32(0.0f), dFloat32(0.0f), scale, dFloat32(0.0f))));
	//	SetColor(dVector(dFloat32(0.0f), dFloat32(0.0f), dFloat32(1.0f), dFloat32(0.0f)));
	//	DrawLine(matrix.m_posit, z);
	//}
	
	dFloat32 m_debugScale;
	//int m_width;
	//int m_height;


} D_GCC_NEWTON_ALIGN_32;

D_MSV_NEWTON_ALIGN_32
class dgPointParam
{
	public:
	dVector m_r0;
	dVector m_r1;
	dVector m_posit0;
	dVector m_posit1;
	dFloat32 m_defualtDiagonalRegularizer;
} D_GCC_NEWTON_ALIGN_32;

D_MSV_NEWTON_ALIGN_32
class ndJacobian
{
	public:
	dVector m_linear;
	dVector m_angular;
} D_GCC_NEWTON_ALIGN_32;

D_MSV_NEWTON_ALIGN_32
class ndJacobianPair
{
	public:
	ndJacobian m_jacobianM0;
	ndJacobian m_jacobianM1;
} D_GCC_NEWTON_ALIGN_32;

class ndForceImpactPair
{
	public:
	void Clear()
	{
		m_force = dFloat32(dFloat32(0.0f));
		m_impact = dFloat32(dFloat32(0.0f));
		for (dInt32 i = 0; i < sizeof(m_initialGuess) / sizeof(m_initialGuess[0]); i++) 
		{
			m_initialGuess[i] = dFloat32(dFloat32(0.0f));
		}
	}

	void Push(dFloat32 val)
	{
		for (dInt32 i = 1; i < sizeof(m_initialGuess) / sizeof(m_initialGuess[0]); i++) 
		{
			m_initialGuess[i - 1] = m_initialGuess[i];
		}
		m_initialGuess[sizeof(m_initialGuess) / sizeof(m_initialGuess[0]) - 1] = val;
	}

	dFloat32 GetInitiailGuess() const
	{
		dFloat32 value = dFloat32(dFloat32(0.0f));
		dFloat32 smallest = dFloat32(1.0e15f);
		for (dInt32 i = 0; i < sizeof(m_initialGuess) / sizeof(m_initialGuess[0]); i++)
		{
			dFloat32 mag = dAbs(m_initialGuess[i]);
			if (mag < smallest) 
			{
				smallest = mag;
				value = m_initialGuess[i];
			}
		}
		return value;
	}

	dFloat32 m_force;
	dFloat32 m_impact;
	dFloat32 m_initialGuess[4];
};

class ndJointAccelerationDecriptor
{
	public:
	dInt32 m_rowsCount;
	dFloat32 m_timestep;
	dFloat32 m_invTimeStep;
	dFloat32 m_firstPassCoefFlag;
	ndRightHandSide* m_rightHandSide;
	const ndLeftHandSide* m_leftHandSide;
};

class ndBilateralBounds
{
	public:
	ndForceImpactPair* m_jointForce;
	dFloat32 m_low;
	dFloat32 m_upper;
	dInt32 m_normalIndex;
};

D_MSV_NEWTON_ALIGN_32
class ndConstraintDescritor
{
	public:
	ndJacobianPair m_jacobian[D_CONSTRAINT_MAX_ROWS];
	ndBilateralBounds m_forceBounds[D_CONSTRAINT_MAX_ROWS];
	dFloat32 m_jointAccel[D_CONSTRAINT_MAX_ROWS];
	dFloat32 m_restitution[D_CONSTRAINT_MAX_ROWS];
	dFloat32 m_penetration[D_CONSTRAINT_MAX_ROWS];
	dFloat32 m_diagonalRegularizer[D_CONSTRAINT_MAX_ROWS];
	dFloat32 m_penetrationStiffness[D_CONSTRAINT_MAX_ROWS];
	dFloat32 m_zeroRowAcceleration[D_CONSTRAINT_MAX_ROWS];
	dInt32 m_flags[D_CONSTRAINT_MAX_ROWS];
	dFloat32 m_timestep;
	dFloat32 m_invTimestep;
	dInt32 m_rowsCount;
} D_GCC_NEWTON_ALIGN_32;

D_MSV_NEWTON_ALIGN_32
class ndLeftHandSide
{
	public:
	ndJacobianPair m_Jt;
	ndJacobianPair m_JMinv;
} D_GCC_NEWTON_ALIGN_32;

D_MSV_NEWTON_ALIGN_32
class ndRightHandSide
{
	public:
	dFloat32 m_force;
	dFloat32 m_diagDamp;
	dFloat32 m_invJinvMJt;
	dFloat32 m_coordenateAccel;

	dFloat32 m_lowerBoundFrictionCoefficent;
	dFloat32 m_upperBoundFrictionCoefficent;
	dFloat32 m_deltaAccel;
	dFloat32 m_restitution;

	dFloat32 m_maxImpact;
	dFloat32 m_penetration;
	dFloat32 m_diagonalRegularizer;
	dFloat32 m_penetrationStiffness;

	ndForceImpactPair* m_jointFeebackForce;
	dInt32 m_normalForceIndex;
} D_GCC_NEWTON_ALIGN_32;

D_MSV_NEWTON_ALIGN_32
class ndConstraint
{
	public:
	virtual ~ndConstraint();

	virtual ndContact* GetAsContact() { return nullptr; }
	virtual ndJointBilateralConstraint* GetAsBilateral() { return nullptr; }

	virtual bool IsBilateral() const { return false; }

	virtual const dUnsigned32 GetRowsCount() const = 0;
	virtual ndBodyKinematic* GetBody0() const { return nullptr; }
	virtual ndBodyKinematic* GetBody1() const { return nullptr; }
	virtual void JacobianDerivative(ndConstraintDescritor& desc) = 0;
	virtual void JointAccelerations(ndJointAccelerationDecriptor* const desc) = 0;

	void InitPointParam(dgPointParam& param, dFloat32 stiffness, const dVector& p0Global, const dVector& p1Global) const;

	virtual void DebugJoint(ndConstraintDebugCallback& debugCallback) const {}

	dFloat32 m_preconditioner0;
	dFloat32 m_preconditioner1;
	dInt32 m_rowCount;
	dInt32 m_rowStart;
	bool m_jointFeebackForce;
	bool m_isInSkeletonLoop;
	protected:
	ndConstraint();
} D_GCC_NEWTON_ALIGN_32 ;

#if 0

D_INLINE ndConstraint::~ndConstraint()
{
}

D_INLINE ConstraintsForceFeeback ndConstraint::GetUpdateFeedbackFunction ()
{
	return m_updaFeedbackCallback;
}

D_INLINE void ndConstraint::SetUpdateFeedbackFunction (ConstraintsForceFeeback function)
{
	m_updaFeedbackCallback = function;
}

D_INLINE bool ndConstraint::IsBilateral() const
{
	return m_isBilateral ? true : false;
}

D_INLINE bool ndConstraint::IsSkeleton () const
{
	return m_isInSkeleton ? true : false;
}

D_INLINE bool ndConstraint::IsSkeletonLoop () const
{
	return m_isInSkeletonLoop ? true : false;
}

D_INLINE bool ndConstraint::IsCollidable () const
{
	return m_enableCollision ? true : false;
}

D_INLINE void ndConstraint::SetCollidable (bool state)
{
	m_enableCollision = dUnsigned32 (state);
}

D_INLINE dUnsigned32 ndConstraint::GetId () const
{
	return m_constId;
}

D_INLINE ntBody* ndConstraint::GetBody0 () const
{
	return m_body0;
}

D_INLINE ntBody* ndConstraint::GetBody1 () const
{
	return m_body1;
}

//D_INLINE void ndConstraint::SetBodies (ntBody* const body0, ntBody* const body1)
//{
//	m_body0 = body0;
//	m_body1 = body1;
//}

D_INLINE dgBodyMasterListRow::dListNode* ndConstraint::GetLink0()	const
{
	return m_link0;
}
D_INLINE dgBodyMasterListRow::dListNode* ndConstraint::GetLink1()	const
{
	return m_link1;
}


D_INLINE dFloat32 ndConstraint::GetStiffness() const
{
	return dFloat32 (dFloat32(1.0f));
}

D_INLINE void ndConstraint::SetStiffness(dFloat32 stiffness)
{
}


D_INLINE void ndConstraint::ResetMaxDOF()
{
}

D_INLINE void ndConstraint::SetImpulseContactSpeed(dFloat32 speed)
{
}

D_INLINE dFloat32 ndConstraint::GetImpulseContactSpeed() const
{
	return dFloat32 (dFloat32(0.0f));
}

D_INLINE dInt32 ndConstraint::GetMaxDOF() const
{
	return dInt32 (m_maxDOF);
}

D_INLINE bool ndConstraint::IsActive() const
{
	return m_isActive ? true : false;
}

D_INLINE void ndConstraint::SetIndex (dInt32 index)
{
	m_index = index;
}

D_INLINE void ndConstraint::GetInfo(dgConstraintInfo* const info) const
{
	dAssert(0);
}
#endif

inline ndConstraint::~ndConstraint()
{
}


#endif 

