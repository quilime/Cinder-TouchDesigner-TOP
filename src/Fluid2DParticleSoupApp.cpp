/*

Copyright (c) 2012-2013 Hai Nguyen
All rights reserved.

Distributed under the Boost Software License, Version 1.0.
http://www.boost.org/LICENSE_1_0.txt
http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt

*/

#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/params/Params.h"
#include "cinder/Rand.h"

#include "cinderfx/Fluid2D.h"

#include "ParticleSoup.h"

class Fluid2DParticleSoupApp : public ci::app::AppNative {
public:
	void prepareSettings( ci::app::AppNative::Settings *settings );
	void setup();
	void keyDown( ci::app::KeyEvent event );
	void mouseDown( ci::app::MouseEvent event );	
	void mouseMove( ci::app::MouseEvent event );
	void mouseDrag( ci::app::MouseEvent event );
	void update();
	void draw();

private:
	float					mVelScale;
	float					mDenScale;
	float					mRgbScale;

	ci::Vec2f				mPrevPos;
	ci::Vec2f				mPos;

	cinderfx::Fluid2D		mFluid2D;
	ci::gl::Texture			mTex;
	ci::params::InterfaceGl	mParams;
	ParticleSoup			mParticleSoup;
	ci::Colorf				mColor;
	int						mCount;
	ci::Area				wallBounds;


};

using namespace ci;
using namespace ci::app;
using namespace cinderfx;
using namespace std;

void Fluid2DParticleSoupApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 700, 700 );
   	settings->setResizable( false ); 
	settings->setFrameRate( 1000 );
	settings->enableMultiTouch();
}

void Fluid2DParticleSoupApp::setup()
{
	glEnable( GL_TEXTURE_2D );

	mDenScale = 50;
	mRgbScale = 40;

	mFluid2D.set( 192, 192 );
   	mFluid2D.setDensityDissipation( 0.99f );
	mFluid2D.setRgbDissipation( 0.99f ); 
	mVelScale = 3.0f*std::max( mFluid2D.resX(), mFluid2D.resY() );

	mFluid2D.setRgbDissipation( 0.9930f );
	mFluid2D.setVelocityDissipation( 1.0f );
	mFluid2D.enableDensity();
	mFluid2D.enableRgb();
	mFluid2D.enableVorticityConfinement();
	mFluid2D.setBoundaryType(Fluid2D::BOUNDARY_TYPE_NONE);
	mFluid2D.setGravityDir(Vec2f(0, -1.0));
	//mFluid2D.setBoundaryType();
//	mFluid2D.setGravityDir(  );
	//mFluid2D.initSimData();
	
	mParams = params::InterfaceGl( "Params", Vec2i( 300, 400 ) );
	mParams.addParam( "Stam Step", mFluid2D.stamStepAddr() );
	mParams.addSeparator();
	mParams.addParam( "Velocity Input Scale", &mVelScale, "min=0 max=10000 step=1" );
	mParams.addParam( "Density Input Scale", &mDenScale, "min=0 max=1000 step=1" );
	mParams.addParam( "Rgb Input Scale", &mRgbScale, "min=0 max=1000 step=1" );
	mParams.addSeparator();
	mParams.addParam( "Velocity Dissipation", mFluid2D.velocityDissipationAddr(), "min=0.0001 max=1 step=0.0001" );
	mParams.addParam( "Density Dissipation", mFluid2D.densityDissipationAddr(), "min=0.0001 max=1 step=0.0001" );
	mParams.addParam( "Rgb Dissipation", mFluid2D.rgbDissipationAddr(), "min=0.0001 max=1 step=0.0001" );     
	mParams.addSeparator();
	mParams.addParam( "Velocity Viscosity", mFluid2D.velocityViscosityAddr(), "min=0.000001 max=10 step=0.000001" );
	mParams.addParam( "Density Viscosity", mFluid2D.densityViscosityAddr(), "min=0.000001 max=10 step=0.000001" );
	mParams.addParam( "Rgb Viscosity", mFluid2D.rgbViscosityAddr(), "min=0.000001 max=10 step=0.000001" );
	mParams.addSeparator();
	mParams.addParam( "Vorticity Confinement", mFluid2D.enableVorticityConfinementAddr() );
	mParams.addSeparator();
	std::vector<std::string> boundaries;
	boundaries.push_back( "None" ); boundaries.push_back( "Wall" ); boundaries.push_back( "Wrap" );
	mParams.addParam( "Boundary Type", boundaries, mFluid2D.boundaryTypeAddr() );
	mParams.addSeparator();
	mParams.addParam( "Enable Buoyancy", mFluid2D.enableBuoyancyAddr() );
	mParams.addParam( "Buoyancy Scale", mFluid2D.buoyancyScaleAddr(), "min=0 max=100 step=0.001" );
	mParams.addParam( "Vorticity Scale", mFluid2D.vorticityScaleAddr(), "min=0 max=1 step=0.001" );

	mParticleSoup.setup( &mFluid2D );

	mCount = 0;
	mColor = Colorf( 1.0f, 1.0f, 1.0f );
}

void Fluid2DParticleSoupApp::keyDown( KeyEvent event )
{
	switch( event.getCode() ) {
	case KeyEvent::KEY_r:
		mFluid2D.initSimData();
		break;
	}
}

void Fluid2DParticleSoupApp::mouseDown( MouseEvent event )
{
}

void Fluid2DParticleSoupApp::mouseMove( MouseEvent event )
{
	mPos = event.getPos();
}

void Fluid2DParticleSoupApp::mouseDrag( MouseEvent event )
{
	mPos = event.getPos();
}



void Fluid2DParticleSoupApp::update()
{
	
    //float x = (mPos.x / (float) getWindowWidth()) * mFluid2D.resX();
    //float y = (mPos.y / (float) getWindowHeight()) * mFluid2D.resY();
    float mx = getWindowWidth() / 2.0f;
    float my = 2.0f;
   	float x = (mx / (float) getWindowWidth())  * mFluid2D.resX();
	float y = (my / (float) getWindowHeight()) * mFluid2D.resY();
    Vec2f p1(x, y);
    Vec2f p2(x + 5, y + 5);
    Vec2f p3(x - 5, y + 5);
	
	
	
	float velScale = 30;
	
	//mFluid2D.splatVelocity( p1.x, p1.y, velScale * Vec2f(0, -1.0));
	//mFluid2D.splatVelocity( p2.x, p2.y, velScale * Vec2f(1.0, -1.0) );
	//mFluid2D.splatVelocity( p3.x, p3.y, velScale * Vec2f(-1.0, -1.0) );
	//mFluid2D.splatRgb( p1.x, p1.y, mRgbScale * mColor );
	//mFluid2D.splatRgb( p2.x, p2.y, mRgbScale * mColor );
	//mFluid2D.splatRgb( p3.x, p3.y, mRgbScale * mColor );

//	if( mFluid2D.isBuoyancyEnabled() ) {
//		mFluid2D.splatDensity( x, y, mDenScale );
//	}
//    
	
	

	// wind
	Vec2f wind_vec =  Vec2f(0, 1.0) * 25;
	for (int i = 0; i < mFluid2D.resX(); i++) {
		mFluid2D.addVelocity( i,				   2, wind_vec );
		//mFluid2D.addVelocity( i, mFluid2D.resY() - 2, vv );
	}
	for (int i = 0; i < mFluid2D.resY(); i++) {
		//mFluid2D.addVelocity( 2,				  i, vv );
		//mFluid2D.addVelocity( mFluid2D.resX()-2,  i, vv );
	}



	
	// X Boundaries
	//int s = -1 * mVelScale;
	//int m = mFluid2D.resY() / 2;
	//int y0 = m;
	//int y1 = m + 1;
	//for( int j = 0; j < mFluid2D.resX() / 2; ++j ) {
	//	Vec2f v0 = mFluid2D.velocityAt( j, y0); 
	//	Vec2f v1 = mFluid2D.velocityAt( j, y1); 
	//	//mFluid2D.set



	//	mFluid2D.velocityAt(j, y0) = Vec2f(0,0);// Vec2f( v0.x, s * v0.y );
	//	mFluid2D.velocityAt(j, y1) = Vec2f(0,0);// Vec2f( v1.x, s * v1.y );
	//	//mFluid2D.set(j, y0, Vec2f( v0.x, s*v0.y ));
	//	//mFluid2D.set(j, y1, Vec2f( v1.x, s*v1.y ));
	//	//inOutVel.at( 0, j ) = Vec2<T>( s*v0.x, v0.y );
	//	//inOutVel.at( m, j ) = Vec2<T>( s*v1.x, v1.y );
	//}
	

	/*
	// RECT
	wallBounds.set(
                    (mFluid2D.resX()/2.0f) - 20,
                    (mFluid2D.resY()/2.0f) + 15,
                    (mFluid2D.resX()/2.0f) + 20,
                    (mFluid2D.resY()/2.0f) + 21);
	int32_t mmx = wallBounds.getX2() - 1;
	int32_t mmy = wallBounds.getY2() - 1;
	for (int32_t y = wallBounds.getY1(); y < wallBounds.getY2(); ++y) {
		for (int32_t x = wallBounds.getX1(); x < wallBounds.getX2(); ++x) {
			if (x == 0)        mFluid2D.velocityAt(x, y) = mFluid2D.velocityAt(x - 1, y)     * -2.0f;
			else if (x == mmx) mFluid2D.velocityAt(x, y) = mFluid2D.velocityAt(x + 1, y)     * -2.0f;
			else if (y == 0)   mFluid2D.velocityAt(x, y) = mFluid2D.velocityAt(x,     y - 1) * -2.0f;
			else if (y == mmy) mFluid2D.velocityAt(x, y) = mFluid2D.velocityAt(x,     y + 1) * -2.0f;
			else mFluid2D.velocityAt(x, y) = Vec2f(0, 0);
		}
	}
	*/




	// mouse pos
    float mousex = (mPos.x / (float) getWindowWidth()) * mFluid2D.resX();
    float mousey = (mPos.y / (float) getWindowHeight()) * mFluid2D.resY();

	// circle
	float radius = (30.0 / (float) getWindowWidth()) * mFluid2D.resX();
	Vec2f c_center(mFluid2D.resX() / 2.0f, mFluid2D.resX() / 2.0f);
	Vec2f cc(mousex, mousey);

	// circle limits
	cc.x = max((double) cc.x, (double) radius);
	cc.x = min((double) cc.x, (double) mFluid2D.resX() - radius);
	cc.y = max((double) cc.y, (double) radius);
	cc.y = min((double) cc.y, (double) mFluid2D.resY() - radius);

	// only check pixels in circle rec bounds
	Area ca(
		cc.x - radius - 2,
        cc.y - radius - 2,
        cc.x + radius + 2,
        cc.y + radius + 2);
	for (int32_t y = ca.getY1(); y < ca.getY2(); ++y) {
		for (int32_t x = ca.getX1(); x < ca.getX2(); ++x) {
			Vec2f d = Vec2f(x, y);
			Vec2f vv = d - cc;
			double dx = cc.x - x;
			double dy = cc.y - y;
			dx *= dx;
			dy *= dy;
			double distanceSquared = dx + dy;
			double radiusSquared = radius * radius;
			if(distanceSquared <= radiusSquared) {
				mFluid2D.velocityAt(x, y) = (vv * 0.25) + (mPos - mPrevPos);
			}
		}
	}


	mFluid2D.step();
	mParticleSoup.setColor( mColor );
	mParticleSoup.update();


	// previous position
	mPrevPos = mPos;
}

void Fluid2DParticleSoupApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
	gl::enableAdditiveBlending();
	
	//glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

	// Uncomment to see underlining density
	
	
	
	float* data = const_cast<float*>( (float*)mFluid2D.rgb().data() );
	Surface32f surf( data, mFluid2D.resX(), mFluid2D.resY(), mFluid2D.resX()*sizeof(Colorf), SurfaceChannelOrder::RGB );
	if ( ! mTex ) {
		mTex = gl::Texture( surf );
	} else {
		mTex.update( surf );
	}
	gl::draw( mTex, getWindowBounds() );
	mTex.unbind();
	
	

	mParticleSoup.draw();



	gl::color(1.0, 1.0, 1.0);
	gl::drawSolidCircle(mPos, 30 );


	mParams.draw();	
}

CINDER_APP_NATIVE( Fluid2DParticleSoupApp, RendererGl )
