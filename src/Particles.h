/*

Copyright (c) 2012-2013 Hai Nguyen
All rights reserved.

Distributed under the Boost Software License, Version 1.0.
http://www.boost.org/LICENSE_1_0.txt
http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt

*/

#pragma once



#include <vector>
//
#include "cinder/Color.h"
#include "cinder/Rect.h"
#include "cinder/Vector.h"
#include "cinder/Timer.h"

using ci::Colorf;
using ci::Rectf;
using ci::Vec2f;
//
#include "cinderfx/Fluid2D.h"
using cinderfx::Fluid2D;

/**
 *
 *
 */
class Particle {
public:

	Particle() 
		:	mAge( 0 ), 
			mLife( 0 ) {}

	Particle( const Vec2f& aPos, float aLife, const Colorf& aColor ) 
		:	mPos( aPos ), 
			mPrevPos( aPos ), 
			mAccel( 0, 0 ), 
			mAge( 0 ), 
			mLife( aLife ), 
			mInvLife( 1.0f/aLife ), 
			mColor( aColor ) {}

	Vec2f&			pos() { return mPos; }
	const Vec2f&	pos() const { return mPos; }
	Vec2f&			prevPos() { return mPrevPos; }
	const Vec2f&	prevPos() const { return mPrevPos; }
	Vec2f&			getAccel() { return mAccel; }
	const	Vec2f&	getAccel() const { return mAccel; }
	void			setPos( const Vec2f& aPos ) { mPos = aPos; mPrevPos = aPos; }
	void			addForce( const Vec2f& aForce ) { mAccel += aForce; }
	void			clearForce() { mAccel = Vec2f( 0, 0 ); }

	float			age() const { return mAge; }
	float			life() const { return mLife; }
	float			invLife() const { return mInvLife; }
	void			setLife( float aLife ) { mAge = 0; mLife = aLife; }

	bool			alive() const { return mAge < mLife; }
	void			kill() { mAge = mLife; }

	const Colorf&	color() const { return mColor; }
	void			setColor( const Colorf& aColor ) { mColor = aColor; }

	void			reset( const Vec2f& aPos, float aLife, const Colorf& aColor );

	//void set( const Vec2f& aPos, float aLife, const Colorf& aColor ) {
	//	setPos( aPos );
	//	clearForce();
	//	setLife( aLife );
	//	setColor( aColor );
	//}

	//void			update( float dt );
	void			update( float simDt, float ageDt );

private:
	Vec2f		mPos;
	Vec2f		mPrevPos;
	Vec2f		mAccel;

	float		mAge;
	float		mLife;
	float		mInvLife;

	Colorf		mColor;
	
};

/**
 *
 *
 */
class ParticleSystem {
public:

	ParticleSystem() 
		:	mPartPos( 0 ),
			mColor(1.0, 1.0, 1.0)
	{}

	int				numParticles() const { return (int) mParticles.size(); }
	Particle&		at( int n ) { return *( mParticles.begin() + (size_t)n ); }
	const Particle&	at( int n ) const { return *( mParticles.begin() + (size_t)n ); }
	void			append( const Particle& aParticle );

	const Colorf&	color() const { return mColor; }
	void			setColor( const Colorf& aColor ) { mColor = aColor; }
	void			setDirection( Vec2f& dir ) { mDir = dir; } 
	void			setNumParticleStreams( int n ) { mNumParticleStreams = n; } 
	void			setBounds( Rectf aBounds ) { mBounds = aBounds; } 
	void			resize( int numParticles );
	void			useParticleStreams( bool v = true ) { mUseParticleStreams = v; } 
	Vec2f			newParticleVec();

	void			setup( const Rectf& aBounds, Fluid2D* aFluid );
	void			update(ci::Timer* timer );
	void			draw();

private:
	Rectf					mBounds;
	Fluid2D*				mFluid;
	int						mMaxParticles;
	Colorf					mColor;
	Vec2f					mDir;

	int						mNumParticleStreams;
	bool					mUseParticleStreams;
	bool					mDrawParticleVectors;
	std::vector<Particle>	mParticles;
	
	size_t					mPartPos;
};
