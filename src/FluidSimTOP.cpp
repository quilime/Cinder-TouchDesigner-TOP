#include "cinder/Cinder.h"

#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
\
#include "cinder/Rand.h"
#include "cinder/Timer.h"

#include "cinderfx/Fluid2D.h"

#include "Particles.h"

#include "TOP_CPlusPlusBase.h"



class FluidSimTOP : public TOP_CPlusPlusBase {
	public:
		FluidSimTOP (const TOP_NodeInfo *info);
		virtual			~FluidSimTOP();
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

		float						px;
		float						py;
		float						ppx;
		float						ppy;

		ci::Timer					mTimer;

		float						mVelScale;
		float						mDenScale;
		float						mRgbScale;
		ci::Vec2f					mPrevPos;
		cinderfx::Fluid2D			mFluid2D;
		ci::gl::Texture				mTex;
		ParticleSystem				mParticles;
		ci::Colorf					mColor;
		GLuint						mFluidTex;
};


using namespace ci;
using namespace cinderfx;
using namespace std;


// SETUP
FluidSimTOP::FluidSimTOP(const TOP_NodeInfo *info) : myNodeInfo(info) {

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

	mParticles.setup(Rectf(0, 0, 1024, 1024), &mFluid2D );
}
FluidSimTOP::~FluidSimTOP() {}


// DRAW LOOP
void FluidSimTOP::execute(const TOP_OutputFormatSpecs* outputFormat , const TOP_InputArrays* arrays, void* reserved) {
	
	// update
	myExecuteCount++;

	// generate some movement so we can see some particles
	ppx = px;
	ppy = py;
	px = 500 + (sin((float) myExecuteCount * 0.004) * 300);
	py = 500 + (sin((float) myExecuteCount * 0.02)  * 300);

	// create a random color every frame
	Colorf color;
	color.r = Rand::randFloat();
	color.g = Rand::randFloat();
	color.b = Rand::randFloat();

	float s = 10;
	Vec2f prevPos = Vec2f(ppx, ppy);
	Vec2f pos = Vec2f(px, py);
	float x = (pos.x / (float) outputFormat->width )  * mFluid2D.resX();
	float y = (pos.y / (float) outputFormat->height ) * mFluid2D.resY();	
	Vec2f dv = pos - prevPos;

	// create fluid splat
	mFluid2D.splatVelocity( x, y, mVelScale * dv );
	mFluid2D.splatRgb( x, y, mRgbScale * color );
	if( mFluid2D.isBuoyancyEnabled() ) {
		mFluid2D.splatDensity( x, y, mDenScale );
	}

	// generate some particles
	for( int i = 0; i < 5; ++i ) {
		Vec2f partPos = pos + Vec2f( Rand::randFloat( -s, s ), Rand::randFloat( -s, s ) );
		float life = Rand::randFloat( 3.0f, 6.0f );
		mParticles.append( Particle( partPos, life, color ) );
	}

	mFluid2D.step();
	mParticles.update( &mTimer );



	////////////////////

	// draw fluid texture
	//gl::color( ColorAf( 1.0f, 1.0f, 1.0f, 0.999f ) );
	float* data = const_cast<float*>( (float*) mFluid2D.rgb().data() );
	//Surface32f surf( data, mFluid2D.resX(), mFluid2D.resY(), mFluid2D.resX() * sizeof(Colorf), SurfaceChannelOrder::RGB );
	//if ( ! mTex ) {
	//	mTex = gl::Texture( surf );
	//} else {
	//	mTex.update( surf );
	//}
	//gl::draw( mTex, getWindowBounds() );
	//mTex.unbind();
	
	//glColor3f(1,1,1);


	

	

	
	//glBindTexture(GL_TEXTURE_2D, mFluidTex);
	//glTexImage2D(GL_TEXTURE_2D, 0 , GL_RGB, mFluid2D.resX(), mFluid2D.resY(), 0, GL_RGB,GL_FLOAT, data);
	
	//free(data);

	
	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);


	/*
	
	glColor3f(0, 0, 0);

	float hhw = (outputFormat->width / 2);
	float hhh = (outputFormat->height / 2);

	glGenTextures( 1, &mFluidTex );

	glEnable(GL_TEXTURE_2D);
	glTexImage2D(GL_TEXTURE_2D, 0 , GL_RGB, mFluid2D.resX(), mFluid2D.resY(), 0, GL_RGB,GL_FLOAT, data);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glBindTexture(GL_TEXTURE_2D, mFluidTex);

	glPushMatrix();
	glTranslatef(Vec3f(hhw, hhh, 0));
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); 
	glVertex3f(-hhw, -hhh, 0.0);
	glTexCoord2f(0.0, 1.0); 
	glVertex3f(-hhw, hhh, 0.0);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(hhw, hhh, 0.0);
	glTexCoord2f(1.0, 0.0); 
	glVertex3f(hhw, -hhh, 0.0);
	glEnd();
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);
	
	*/



	//////////////////////


	// draw incoming texture from index
	if (arrays->TOPInputs->textureIndex) {

		float hw = outputFormat->width / 4;
		float hh = outputFormat->height / 4;

		glColor3f(0,0,0);
		
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
		glBindTexture(GL_TEXTURE_2D, arrays->TOPInputs->textureIndex);

		glPushMatrix();
		glTranslatef(Vec3f(hw, hh, 0));
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);	
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); 
		glVertex3f(-hw, -hh, 0.0);
		glTexCoord2f(0.0, 1.0); 
		glVertex3f(-hw, hh, 0.0);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(hw, hh, 0.0);
		glTexCoord2f(1.0, 0.0); 
		glVertex3f(hw, -hh, 0.0);
		glEnd();
		glPopMatrix();

		glDisable(GL_TEXTURE_2D);
		glFlush();
	}

	





	// draw fluid particles
	glPointSize( arrays->floatInputs[0].values[0] );
	//glColor4f(1,1,1,1);
	glBegin( GL_POINTS );
	for( int i = 0; i < mParticles.numParticles(); ++i ) {
		const Particle& part = mParticles.at( i );
		if( ! part.alive() )
			continue;
		float alpha = part.age() * part.invLife();
		alpha = 1.0f; // - std::min( alpha, 1.0f );
		alpha = std::min( alpha, 0.8f );
		glColor4f( ColorAf( part.color(), alpha ) );
		glVertex2f( part.pos() );
	}
	glEnd();







    //// Lets just draw a small red square in the lower left quadrant of the texture
    //::glColor4f(1, 0, 0, 1);
    //::glMatrixMode(GL_MODELVIEW);
    //::glPushMatrix();
    //::glRotatef(myExecuteCount, 1.0f, 1.0f, 1.0f);
    //::glBegin(GL_QUADS);
    //::glVertex2i(0, 0);
    //::glVertex2i(outputFormat->width / 2, 0);
    //::glVertex2i(outputFormat->width / 2, outputFormat->height / 2);
    //::glVertex2i(0, outputFormat->height / 2);
    //::glEnd();
    //::glPopMatrix();


	/*
	// drawing textures

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, outputFormat->width, 0.0, outputFormat->height, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glLoadIdentity();
	glDisable(GL_LIGHTING);

	glColor3f(1,1,1);
	glEnable(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, mark_textures[0].id);

	// Draw a textured quad
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex3f(0, 0, 0);
	glTexCoord2f(0, 1); glVertex3f(0, 100, 0);
	glTexCoord2f(1, 1); glVertex3f(100, 100, 0);
	glTexCoord2f(1, 0); glVertex3f(100, 0, 0);
	glEnd();


	glDisable(GL_TEXTURE_2D);
	glPopMatrix();


	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);

	*/
}




void FluidSimTOP::getGeneralInfo(TOP_GeneralInfo *ginfo) {
	ginfo->cookEveryFrame = true;
}

bool FluidSimTOP::getOutputFormat(TOP_OutputFormat *format) {
	// In this function we could assign variable values to 'format' to specify
	// the pixel format/resolution etc that we want to output to.
	// If we did that, we'd want to return true to tell the TOP to use the settings we've
	// specified.
	// In this example we'll return false and use the TOP's settings
	return false;
}

int FluidSimTOP::getNumInfoCHOPChans() {
	// We return the number of channel we want to output to any Info CHOP
	// connected to the TOP. In this example we are just going to send one channel.
	return 1;
}

void FluidSimTOP::getInfoCHOPChan(int index, TOP_InfoCHOPChan *chan) {
	// This function will be called once for each channel we said we'd want to return
	// In this example it'll only be called once.
	if (index == 0) {
		chan->name = "executeCount";
		chan->value = myExecuteCount * 1.0f;
	}
}

bool FluidSimTOP::getInfoDATSize(TOP_InfoDATSize *infoSize) {
	infoSize->rows = 1;
	infoSize->cols = 2;
	// Setting this to false means we'll be assigning values to the table
	// one row at a time. True means we'll do it one column at a time.
	infoSize->byColumn = false;
	return true;
}

void FluidSimTOP::getInfoDATEntries(int index, int nEntries, TOP_InfoDATEntries *entries) {
	if (index == 0) {
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
		return new FluidSimTOP(info);
	}
	DLLEXPORT void DestroyTOPInstance(TOP_CPlusPlusBase *instance) {
		// Delete the instance here, this will be called when
		// Touch is shutting down, when the TOP using that instance is deleted, or
		// if the TOP loads a different DLL
		delete (FluidSimTOP*)instance;
	}
};
