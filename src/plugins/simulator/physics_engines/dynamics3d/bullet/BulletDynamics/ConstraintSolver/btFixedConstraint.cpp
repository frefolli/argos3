/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2013 Erwin Coumans  http://bulletphysics.org

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#include <argos3/plugins/simulator/physics_engines/dynamics3d/bullet/BulletDynamics/ConstraintSolver/btFixedConstraint.h>
#include <argos3/plugins/simulator/physics_engines/dynamics3d/bullet/BulletDynamics/Dynamics/btRigidBody.h>
#include <argos3/plugins/simulator/physics_engines/dynamics3d/bullet/LinearMath/btTransformUtil.h>
#include <new>

btFixedConstraint::btFixedConstraint(btRigidBody& rbA, btRigidBody& rbB, const btTransform& frameInA, const btTransform& frameInB)
	: btGeneric6DofSpring2Constraint(rbA, rbB, frameInA, frameInB)
{
	setAngularLowerLimit(btVector3(0, 0, 0));
	setAngularUpperLimit(btVector3(0, 0, 0));
	setLinearLowerLimit(btVector3(0, 0, 0));
	setLinearUpperLimit(btVector3(0, 0, 0));
}

btFixedConstraint::~btFixedConstraint()
{
}
