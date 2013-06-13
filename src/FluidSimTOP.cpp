#include "cinder/Cinder.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
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
		void drawSolidCircle( const ci::Vec2f &center, float radius, int numSegments = 0 );
		
		const TOP_NodeInfo			*myNodeInfo;
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


		Vec2f						mPosition;
		Vec2f						mPPosition;
};


using namespace ci;
using namespace cinderfx;
using namespace std;


FluidSimTOP::FluidSimTOP(const TOP_NodeInfo *info) : myNodeInfo(info) {

	mRgbScale = 50;
	mDenScale = 50;
	
	mFluid2D.set( 64, 64 );

	// from particlesystem
	//mFluid2D.setDt( 0.1f );
	
	mFluid2D.enableDensity();
	mFluid2D.enableRgb();
	mFluid2D.enableVorticityConfinement();
	mFluid2D.enableBuoyancy();

	mFluid2D.setBoundaryType(Fluid2D::BOUNDARY_TYPE_WRAP);
   	mFluid2D.setDensityDissipation( 0.99f );
	mFluid2D.setRgbDissipation( 0.99f ); 
	mFluid2D.setVelocityDissipation( 1.0f );
	mFluid2D.setGravityDir(Vec2f(0, -1.0));
	mFluid2D.setBuoyancyScale(5.0);
	mFluid2D.setVorticityScale(0.5);

	//mParticles.useParticleStreams(false);
	//mParticles.setNumParticleStreams(20);

	mVelScale = 3.0f * max( mFluid2D.resX(), mFluid2D.resY() );

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

	mParticles.setup(Rectf(0, 0, 1024, 1024), &mFluid2D );

	myExecuteCount = 0;
	mTimer.start();
}
FluidSimTOP::~FluidSimTOP() {}


void FluidSimTOP::execute(
		const TOP_OutputFormatSpecs* outputFormat , 
		const TOP_InputArrays* arrays, void* reserved) {

	myExecuteCount++;


	////////////////
	// SET VARIABLES
	float aPointSize = arrays->floatInputs[0].values[0];

	mVelScale = arrays->floatInputs[0].values[1] * max( mFluid2D.resX(), mFluid2D.resY() );

	float useParticleStreams = arrays->floatInputs[0].values[2] > 0 ? true : false;
	mParticles.useParticleStreams(useParticleStreams);
	mParticles.setNumParticleStreams(arrays->floatInputs[0].values[3]);

	mColor = ColorA(
			arrays->floatInputs[1].values[0],
			arrays->floatInputs[1].values[1],
			arrays->floatInputs[1].values[2]);

    mPPosition = mPosition;
	mPosition.set(
			arrays->floatInputs[2].values[0],
			arrays->floatInputs[2].values[1]);
	Vec2f orientation(
			arrays->floatInputs[2].values[2],
			arrays->floatInputs[2].values[3]);
	float directon = orientation.x;

	Vec2f obstacle1(
			arrays->floatInputs[3].values[0],
			arrays->floatInputs[3].values[1]);
	Vec2f obstacle2(
			arrays->floatInputs[3].values[2],
			arrays->floatInputs[3].values[3]);
	Vec2f obstacle3(
			arrays->floatInputs[4].values[0],
			arrays->floatInputs[4].values[1]);
	Vec2f obstacle4(
			arrays->floatInputs[4].values[2],
			arrays->floatInputs[4].values[3]);
	Vec2f obstacle5(
			arrays->floatInputs[5].values[0],
			arrays->floatInputs[5].values[1]);
	Vec2f obstacle6(
			arrays->floatInputs[5].values[2],
			arrays->floatInputs[5].values[3]);

	Vec2f flowDirection(
			arrays->floatInputs[6].values[0],
			arrays->floatInputs[6].values[1]);
	flowDirection.normalize();
	float flowSpeed = arrays->floatInputs[6].values[2];


	// UPDATE


	// create a random color every frame
	Colorf color;
	color.r = Rand::randFloat();
	color.g = Rand::randFloat();
	color.b = Rand::randFloat();

	// createa movement that will disrupt the fluid field
	Vec2f pos = Vec2f(	(mPosition.x / (float) outputFormat->width)  * mFluid2D.resX(),
						(mPosition.y / (float) outputFormat->height) * mFluid2D.resY());
	
	
	Vec2f dv = mPosition - mPPosition;

	// create fluid splat
	mFluid2D.splatVelocity( pos.x, pos.y, dv * mVelScale );
	mFluid2D.splatRgb( pos.x, pos.y, mRgbScale * color );
	if( mFluid2D.isBuoyancyEnabled() ) {
		mFluid2D.splatDensity( pos.x, pos.y, mDenScale );
	}

	// generate some particles at the position
	float s = 10;
	for( int i = 0; i < 5; ++i ) {
		Vec2f partPos = mPosition + Vec2f( 
			Rand::randFloat( -s, s ), 
			Rand::randFloat( -s, s ));
		float life = Rand::randFloat( 3.0f, 6.0f );
		mParticles.append( Particle( partPos, life, color ) );
	}





    // create wind
    Vec2f wind_vec = flowDirection * flowSpeed;
	float ypos = mFluid2D.resY() - 2;
    for (int i = 0; i < mFluid2D.resX(); i++) {
            mFluid2D.addVelocity( i, ypos, wind_vec );
    }

	mFluid2D.step();
	mParticles.setColor( mColor );
	mParticles.update( &mTimer );



	// get fluidsim texturedata
	// float* data = const_cast<float*>( (float*) mFluid2D.rgb().data() );



	// DRAW

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE );

	// draw particles
	glPointSize( aPointSize );
	glBegin( GL_POINTS );
	for( int i = 0; i < mParticles.numParticles(); ++i ) {
		const Particle& part = mParticles.at( i );
		float alpha = std::min( part.age() / 1.0f, 0.75f );
		glColor4f( ColorAf( part.color(), alpha ) );
		glVertex2f( part.pos() );
	}
	glEnd();

	// draw obstacles
	glColor3f(mColor);
	drawSolidCircle(mPosition, 20);
}


void FluidSimTOP::drawSolidCircle( const Vec2f &center, float radius, int numSegments )
{
        // determine the number of segments from the circumference
        if( numSegments <= 0 ) {
                numSegments = (int)math<double>::floor( radius * M_PI * 2 );
        }
        if( numSegments < 2 ) numSegments = 2;
        GLfloat *verts = new float[(numSegments+2)*2];
        verts[0] = center.x;
        verts[1] = center.y;
        for( int s = 0; s <= numSegments; s++ ) {
                float t = s / (float)numSegments * 2.0f * 3.14159f;
                verts[(s+1)*2+0] = center.x + math<float>::cos( t ) * radius;
                verts[(s+1)*2+1] = center.y + math<float>::sin( t ) * radius;
        }
        glEnableClientState( GL_VERTEX_ARRAY );
        glVertexPointer( 2, GL_FLOAT, 0, verts );
        glDrawArrays( GL_TRIANGLE_FAN, 0, numSegments + 2 );
        glDisableClientState( GL_VERTEX_ARRAY );
        delete [] verts;
}


void FluidSimTOP::getGeneralInfo(TOP_GeneralInfo *ginfo) {
	ginfo->cookEveryFrame = false;
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
		delete (FluidSimTOP*) instance;
	}
};





















/*

NOTES
*/



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