/*

Copyright (c) 2012-2013 Hai Nguyen
All rights reserved.

Distributed under the Boost Software License, Version 1.0.
http://www.boost.org/LICENSE_1_0.txt
http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt

*/

#include "Particles.h"
//
//#include "cinder/app/App.h"
//#include "cinder/gl/gl.h"

#include "cinder/Rand.h"
using namespace cinder;

const float kSimDt			= 0.1f;
const float kSimDt2			= kSimDt*kSimDt;
const float kPointSize		= 2.5f;
const float kBorder			= kPointSize;
const float kDampen         = 0.93f;
const int   kMaxParticles   = 50000;

void Particle::reset( const Vec2f& aPos, float aLife, const Colorf& aColor )
{
	mPos = mPrevPos = aPos;
	mAccel = Vec2f( 0, 0 );
	mLife = aLife;
	mAge = 0;
	mColor = aColor;
}

void Particle::update( float simDt, float ageDt )
{
	Vec2f vel = mPos - mPrevPos;
	mPrevPos = mPos;
	mPos += vel * simDt;
	mPos += mAccel * simDt * simDt;
	mAccel *= kDampen;
	mAge += ageDt;
}

void ParticleSystem::setup( const Rectf& aBounds, Fluid2D* aFluid )
{
	mBounds = aBounds;
	mFluid = aFluid;
	Rectf bounds = aBounds;
	for( int n = 0; n < kMaxParticles; ++n ) {
		mParticles.push_back( Particle( newParticleVec(), Rand::randFloat( 0.0f, 1.0f ), mColor ) );
	}
}

void ParticleSystem::resize(int aNumParticles)
{
	if (aNumParticles > (int) mParticles.size()) {
		for( int n = mParticles.size(); n < aNumParticles; ++n ) {
			mParticles.push_back( Particle( newParticleVec(), Rand::randFloat( 0.0f, 1.0f ), mColor ) );
		}
	} else {
		mParticles.resize(aNumParticles);
	}
	
}

Vec2f ParticleSystem::newParticleVec()
{
	Vec2f P;
	if (mUseParticleStreams) {
		P.x = Rand::randInt(0, mNumParticleStreams) * ( mBounds.x2 / mNumParticleStreams );
		P.y = mBounds.getHeight() - 2;
	} else {
		P.x = Rand::randFloat( mBounds.x1 + 5.0f, mBounds.x2 - 5.0f );
		P.y = Rand::randFloat( mBounds.y1 + 5.0f, mBounds.y2 - 5.0f );
	}
	return P;
}

void ParticleSystem::append( const Particle& aParticle )
{
	if ( mPartPos >= mParticles.size() ) {
		mPartPos = 0;
	}
	std::vector<Particle>::iterator it = mParticles.begin() + mPartPos;
	for( ; it != mParticles.end(); ++it ) {
		mPartPos++;
		if( ! it->alive() ) {
			*it = aParticle;
			return;
		}
	}
}

void ParticleSystem::update(Timer* timer )
{
	static float prevTime = (float) timer->getSeconds();
	float curTime = (float) timer->getSeconds();
	float dt = curTime - prevTime;
	prevTime = curTime;

	Rectf bounds = mBounds;
	float minX = -kBorder;
	float minY = -kBorder;
	float maxX = bounds.getWidth();
	float maxY = bounds.getHeight();

	// Avoid the borders in the remap because the velocity might be zero.
	// Nothing interesting happens where velocity is zero.
	float dx = (float)(mFluid->resX() - 4) / (float) bounds.getWidth();
	float dy = (float)(mFluid->resY() - 4) / (float) bounds.getHeight();
	for( int i = 0; i < numParticles(); ++i ) {
		Particle& part = mParticles.at( i );
		if( part.pos().x < minX || part.pos().y < minY || part.pos().x >= maxX || part.pos().y >= maxY ) {
			part.reset( newParticleVec(), Rand::randFloat( 0.0f, 1.0f ), mColor );
		}

		float x = part.pos().x * dx + 2.0f;
		float y = part.pos().y * dy + 2.0f;

		Vec2f vel = mFluid->velocity().bilinearSampleChecked( x, y, Vec2f( 0.0f, 0.0f ) );
		part.addForce( vel );

		part.update( mFluid->dt(), dt );
	}
}

void ParticleSystem::draw()
{
	/*

	glPointSize( kPointSize );
	glBegin( GL_POINTS );
	for( int i = 0; i < numParticles(); ++i ) {
		const Particle& part = mParticles.at( i );
//		if( ! part.alive() )
//			continue;
		float alpha = part.age()*part.invLife();
		alpha = 1.0f;// - std::min( alpha, 1.0f );
		alpha = std::min( alpha, 0.8f );
		glColor4f( ColorAf( part.color(), alpha ) );
		glVertex2f( part.pos() );
	}
	glEnd();
	*/

}
