/*

Copyright (c) 2012-2013 Hai Nguyen
All rights reserved.

Distributed under the Boost Software License, Version 1.0.
http://www.boost.org/LICENSE_1_0.txt
http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt

*/

#include "ParticleSoup.h"
#include "cinder/Rand.h"
#include "cinder/gl/gl.h"
using namespace cinder;

const float kBorder = 1.0f;
const float kDampen = 0.94f;
const int	kMaxParticles = 1000;// 25000;
const float kPointSize = 2.25f;
const float kNumParticleStreams = 20;

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
	mPos += vel * simDt;
	mPos += mAccel * simDt * simDt;
	mAccel *= kDampen;
	mPrevPos = mPos;
	mAge += ageDt;
}

void ParticleSoup::setup( const Rectf& aBounds, Fluid2D* aFluid )
{
	
	mFluid = aFluid;
	mColor = Colorf(1.0, 1.0, 1.0);
	mDir = Vec2f(0, 5.0);
    
	Rectf bounds = aBounds;
	for( int n = 0; n < kMaxParticles; ++n ) {
		Vec2f P;
        
		//P.x = Rand::randInt(0, kNumParticleStreams) * ( bounds.x2 / kNumParticleStreams );
		//P.y = 2;//Rand::randFloat( bounds.y1 + 5.0f, bounds.y2 - 5.0f );

		P.x = Rand::randFloat( bounds.x1 + 5.0f, bounds.x2 - 5.0f );
		P.y = Rand::randFloat( bounds.y1 + 5.0f, bounds.y2 - 5.0f );

		float life = Rand::randFloat( 0.0f, 1.0f );
		mParticles.push_back( Particle( P, life, mColor ) );
	}
}

void ParticleSoup::update(Timer* timer )
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
			Vec2f P;
			P.x = Rand::randFloat( bounds.x1 + 5.0f, bounds.x2 - 5.0f );
			P.y = Rand::randFloat( bounds.y1 + 5.0f, bounds.y2 - 5.0f );
            //P.x = Rand::randInt(0, kNumParticleStreams) * ( bounds.x2 / kNumParticleStreams );
			//P.y = 2;
			float life = Rand::randFloat( 2.0f, 3.0f );
			part.reset( P, life, mColor );
		}

		float x = part.pos().x * dx + 2.0f;
		float y = part.pos().y * dy + 2.0f;

		Vec2f vel = mFluid->velocity().bilinearSampleChecked( x, y, Vec2f( 0.0f, 0.0f ) );
		part.addForce( vel );
		//part.addForce( mDir );

		part.update( mFluid->dt(), dt );
	}
}

void ParticleSoup::draw()
{
	glBegin( GL_POINTS );
	for( int i = 0; i < numParticles(); ++i ) {
		const Particle& part = mParticles.at( i );
		float alpha = 1.0;//std::min( part.age() / 1.0f, 0.75f );
		glColor4f( ColorAf( part.color(), alpha ) );
		glVertex2f( part.pos() );
	}
	glEnd();
}
