#include "cinder/Cinder.h"
#include "cinder/gl/gl.h"
#include "cinder/app/AppNative.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/Rand.h"
#include "cinder/Timer.h"
#include "cinderfx/Fluid2D.h"
#include "Particles.h"

#include "TOP_CPlusPlusBase.h"



class CinderTOP : public TOP_CPlusPlusBase {
	public:
		CinderTOP (const TOP_NodeInfo *info);
		virtual			~CinderTOP();
		virtual void	getGeneralInfo(TOP_GeneralInfo *);
		virtual bool	getOutputFormat(TOP_OutputFormat*);
		virtual void	execute(const TOP_OutputFormatSpecs*, const TOP_InputArrays*, void* reserved);
		virtual int		getNumInfoCHOPChans();
		virtual void	getInfoCHOPChan(int index, TOP_InfoCHOPChan *chan);
		virtual bool	getInfoDATSize(TOP_InfoDATSize *infoSize);
		virtual void	getInfoDATEntries(int index, int nEntries, TOP_InfoDATEntries *entries);
	private:
		// We don't need to store this pointer, but we do for the example.
		// The TOP_NodeInfo class store information about the node that's using
		// this instance of the class (like its name).
		const TOP_NodeInfo		*myNodeInfo;
		// In this example this value will be incremented each time the execute()
		// function is called, then passes back to the TOP 
		int							myExecuteCount;
		ci::Timer					mTimer;
		float						mVelScale;
		float						mDenScale;
		float						mRgbScale;
		ci::Vec2f					mPrevPos;
		cinderfx::Fluid2D			mFluid2D;
		ci::gl::Texture				mTex;
		ParticleSystem				mParticles;
		ci::Colorf					mColor;
};


using namespace ci;
using namespace cinderfx;
using namespace std;


// SETUP
CinderTOP::CinderTOP(const TOP_NodeInfo *info) : myNodeInfo(info) {

	myExecuteCount = 0;
	mTimer.start();

	mRgbScale = 50;
	mDenScale = 50;
	
	mFluid2D.set( 192, 192 );
   	mFluid2D.setDensityDissipation( 0.99f );
	mFluid2D.setRgbDissipation( 0.99f ); 
	mVelScale = 3.0f*std::max( mFluid2D.resX(), mFluid2D.resY() );

	/*
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
	*/

	mFluid2D.setDt( 0.1f );
	mFluid2D.enableDensity();
	mFluid2D.enableRgb();
	mFluid2D.enableVorticityConfinement();

	mParticles.setup(Rectf(0, 0, 500, 500), &mFluid2D );
}
CinderTOP::~CinderTOP() {}


// DRAW LOOP
void CinderTOP::execute(const TOP_OutputFormatSpecs* outputFormat , const TOP_InputArrays* arrays, void* reserved) {

	myExecuteCount++;

	// update
	mFluid2D.step();
	mParticles.update( mTimer );


	// draw

	// clear out the window with black
	//gl::clear( Color( 0, 0, 0 ) );

	//gl::color( ColorAf( 1.0f, 1.0f, 1.0f, 0.999f ) );
	float* data = const_cast<float*>( (float*) mFluid2D.rgb().data() );
	//Surface32f surf( data, mFluid2D.resX(), mFluid2D.resY(), mFluid2D.resX()*sizeof(Colorf), SurfaceChannelOrder::RGB );
	

	//if ( ! mTex ) {
	//	mTex = gl::Texture( surf );
	//} else {
	//	mTex.update( surf );
	//}
	//gl::draw( mTex, getWindowBounds() );
	//mTex.unbind();
	//mParticles.draw();
	//mParams.draw();
	


	//gl::drawColorCube(Vec3f(0,0,0), Vec3f(30,30,30));

	// Lets just draw a small red square in the lower left quadrant of the texture
	::glColor4f(
		arrays->floatInputs[0].values[0], 
		arrays->floatInputs[0].values[1], 
		arrays->floatInputs[0].values[2], 
		arrays->floatInputs[0].values[3]);
	::glMatrixMode(GL_MODELVIEW);
	::glPushMatrix();
	::glRotatef(myExecuteCount * 1.0f, 0.0f, 0.0f, 1.0f);
	::glBegin(GL_QUADS);
	::glVertex2i(0, 0);
	::glVertex2i(outputFormat->width / 2, 0);
	::glVertex2i(outputFormat->width / 2, outputFormat->height / 2);
	::glVertex2i(0, outputFormat->height / 2);
	::glEnd();
	::glPopMatrix();

	// Draw a diamond to test anti-aliasing (it will draw over part of the above square)
	::glPushMatrix();
	::glRotatef(-myExecuteCount * 1.0f, 0.0f, 0.0f, .150f);
	::glColor4f(
		arrays->floatInputs[1].values[0], 
		arrays->floatInputs[1].values[1], 
		arrays->floatInputs[1].values[2], 
		arrays->floatInputs[1].values[3]);
	::glBegin(GL_QUADS);
	::glVertex2i(outputFormat->width / 2, 0);
	::glVertex2i(outputFormat->width, outputFormat->height / 2);
	::glVertex2i(outputFormat->width / 2, outputFormat->height);
	::glVertex2i(0, outputFormat->height / 2);
	::glEnd();
	::glPopMatrix();

}


void CinderTOP::getGeneralInfo(TOP_GeneralInfo *ginfo) {
	// Uncomment this line if you want the TOP to cook every frame even
	// if none of it's inputs/parameters are changing.
	ginfo->cookEveryFrame = true;
}

bool CinderTOP::getOutputFormat(TOP_OutputFormat *format) {
	// In this function we could assign variable values to 'format' to specify
	// the pixel format/resolution etc that we want to output to.
	// If we did that, we'd want to return true to tell the TOP to use the settings we've
	// specified.
	// In this example we'll return false and use the TOP's settings
	return false;
}

int CinderTOP::getNumInfoCHOPChans() {
	// We return the number of channel we want to output to any Info CHOP
	// connected to the TOP. In this example we are just going to send one channel.
	return 1;
}

void CinderTOP::getInfoCHOPChan(int index, TOP_InfoCHOPChan *chan) {
	// This function will be called once for each channel we said we'd want to return
	// In this example it'll only be called once.

	if (index == 0)
	{
		chan->name = "executeCount";
		chan->value = myExecuteCount * 1.0f;
	}
}

bool CinderTOP::getInfoDATSize(TOP_InfoDATSize *infoSize) {
	infoSize->rows = 1;
	infoSize->cols = 2;
	// Setting this to false means we'll be assigning values to the table
	// one row at a time. True means we'll do it one column at a time.
	infoSize->byColumn = false;
	return true;
}

void CinderTOP::getInfoDATEntries(int index, int nEntries, TOP_InfoDATEntries *entries) {
	if (index == 0)
	{
		// It's safe to use static buffers here because Touch will make it's own
		// copies of the strings immediately after this call returns
		// (so the buffers can be reuse for each column/row)
		static char tempBuffer1[4096];
		static char tempBuffer2[4096];
		// Set the value for the first column
		strcpy_s(tempBuffer1, "executeCount");
		entries->values[0] = tempBuffer1;

		// Set the value for the second column
		sprintf_s(tempBuffer2, "%d", myExecuteCount);
		entries->values[1] = tempBuffer2;
	}
}

// These functions are basic C function, which the DLL loader can find
// much easier than finding a C++ Class.
// The DLLEXPORT prefix is needed so the compile exports these functions from the .dll
// you are creating
extern "C" 
{
	DLLEXPORT int GetTOPAPIVersion(void) {
		// Always return TOP_CPLUSPLUS_API_VERSION in this function.
		return TOP_CPLUSPLUS_API_VERSION;
	}
	DLLEXPORT TOP_CPlusPlusBase* CreateTOPInstance(const TOP_NodeInfo *info) {
		// Return a new instance of your class every time this is called.
		// It will be called once per TOP that is using the .dll
		return new CinderTOP(info);
	}
	DLLEXPORT void DestroyTOPInstance(TOP_CPlusPlusBase *instance) {
		// Delete the instance here, this will be called when
		// Touch is shutting down, when the TOP using that instance is deleted, or
		// if the TOP loads a different DLL
		delete (CinderTOP*)instance;
	}
};
