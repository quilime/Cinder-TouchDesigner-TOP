/*
Author: Gabriel Dunne <gdunne@quilime.com>
*/

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
		Rectf						mParticleBounds;
		int							mMaxParticles;
		ci::Colorf					mColor;

		Vec2f						mPosition;
		Vec2f						mPPosition;

		int							mNumObstacles;
		bool						mDrawParticleVectors;

		std::vector<Vec2f>			mObstacles;
		std::vector<Vec2f>			mObstaclesPP;
};


using namespace ci;
using namespace cinderfx;
using namespace std;


FluidSimTOP::FluidSimTOP(const TOP_NodeInfo *info) : myNodeInfo(info) {

	mRgbScale = 50;
	mDenScale = 50;
	mMaxParticles = 5000;
	
	mFluid2D.set( 192, 192 );
	mFluid2D.setDt( 0.1f );
	mFluid2D.enableDensity();
	mFluid2D.enableVorticityConfinement();
	mFluid2D.setBoundaryType(Fluid2D::BOUNDARY_TYPE_WRAP);

	mParticles.useParticleStreams(false);
	mVelScale = 3.0f * max( mFluid2D.resX(), mFluid2D.resY() );
	mParticleBounds.set(0, 0, 1024, 1024);
	mParticles.setup(mParticleBounds, &mFluid2D );
	myExecuteCount = 0;
	mTimer.start();

	// create obstacle positions
	mNumObstacles = 6;
	for( int i = 0; i < mNumObstacles; ++i ) {
		mObstacles.push_back(Vec2f());
		mObstaclesPP.push_back(Vec2f());
	}
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

	bool useParticleStreams = arrays->floatInputs[0].values[2] > 0 ? true : false;
	mParticles.useParticleStreams(useParticleStreams);
	mParticles.setNumParticleStreams((int) arrays->floatInputs[0].values[3]);

	mColor = ColorA(
			arrays->floatInputs[1].values[0],
			arrays->floatInputs[1].values[1],
			arrays->floatInputs[1].values[2]);

	bool showObstacles = arrays->floatInputs[1].values[3] > 0 ? true : false;

    mPPosition = mPosition;
	mPosition.set(
			arrays->floatInputs[2].values[0],
			arrays->floatInputs[2].values[1]);
	Vec2f direction(
			arrays->floatInputs[2].values[2],
			arrays->floatInputs[2].values[3]);

	mObstaclesPP.at(0).set(mObstacles.at(0));
	mObstacles.at(0).set(
			arrays->floatInputs[3].values[0],
			arrays->floatInputs[3].values[1]);
	mObstaclesPP.at(1).set(mObstacles.at(1));
	mObstacles.at(1).set(
			arrays->floatInputs[3].values[2],
			arrays->floatInputs[3].values[3]);
	mObstaclesPP.at(2).set(mObstacles.at(2));
	mObstacles.at(2).set(
			arrays->floatInputs[4].values[0],
			arrays->floatInputs[4].values[1]);
	mObstaclesPP.at(3).set(mObstacles.at(3));
	mObstacles.at(3).set(
			arrays->floatInputs[4].values[2],
			arrays->floatInputs[4].values[3]);
	mObstaclesPP.at(4).set(mObstacles.at(4));
	mObstacles.at(4).set(
			arrays->floatInputs[5].values[0],
			arrays->floatInputs[5].values[1]);
	mObstaclesPP.at(5).set(mObstacles.at(5));
	mObstacles.at(5).set(
			arrays->floatInputs[5].values[2],
			arrays->floatInputs[5].values[3]);

	Vec2f flowDirection(
			arrays->floatInputs[6].values[0],
			arrays->floatInputs[6].values[1]);
	flowDirection.normalize();
	float flowSpeed = arrays->floatInputs[6].values[2];

	bool enableBuoyancy = arrays->floatInputs[6].values[3] > 0 ? true : false;
	mFluid2D.enableBuoyancy(enableBuoyancy);

	mFluid2D.setVorticityScale(arrays->floatInputs[7].values[0]);
	mFluid2D.setBuoyancyScale(arrays->floatInputs[7].values[1]);
	mFluid2D.setGravityDir(Vec2f(arrays->floatInputs[7].values[2], arrays->floatInputs[7].values[3]));

	float obstacleRadius = arrays->floatInputs[8].values[0];
	float obstacleVelocityScale = arrays->floatInputs[8].values[1];

   	mFluid2D.setDensityDissipation( arrays->floatInputs[8].values[2] );
	mFluid2D.setVelocityDissipation( arrays->floatInputs[8].values[3] );

	if (mMaxParticles != (int) arrays->floatInputs[9].values[0]) {
		mMaxParticles = (int) arrays->floatInputs[9].values[0];
		mParticles.resize(mMaxParticles);
	}

	mDrawParticleVectors = arrays->floatInputs[9].values[2] > 0 ? true : false;

	if ((int) mParticleBounds.getWidth() != (int) outputFormat->width ||
		(int) mParticleBounds.getWidth() != (int) outputFormat->height){
		mParticleBounds.set(0, 0, outputFormat->width, outputFormat->height);
		mParticles.setBounds(mParticleBounds);
	}

	float aVectorLength = arrays->floatInputs[9].values[1];



	// UPDATE


    // create wind
    Vec2f windVec = flowDirection * flowSpeed;
	int ypos = mFluid2D.resY() - 2;
    for (int i = 0; i < mFluid2D.resX(); i++) {
            mFluid2D.addVelocity( i, ypos, windVec );
    }


	// set obstacles
	for( int i = 0; i < mNumObstacles; ++i ) {

		Vec2f& p = mObstacles.at(i);

		p.rotate( atan2( direction.y, direction.x ) - (3.14159f / 2.0f) );
		p += mPosition;

		Vec2f& pp = mObstaclesPP.at(i);

		// convert position to texturespace
		Vec2f pos  = Vec2f((p.x  / (float) outputFormat->width)  * mFluid2D.resX(),
						   (p.y  / (float) outputFormat->height) * mFluid2D.resY());
		Vec2f ppos = Vec2f((pp.x / (float) outputFormat->width)  * mFluid2D.resX(),
						   (pp.y / (float) outputFormat->height) * mFluid2D.resY());
		Vec2f dv = p - pp;
	
		float radius = (obstacleRadius / (float) outputFormat->width) * mFluid2D.resX();
		Vec2f c_center(mFluid2D.resX() / 2.0f, mFluid2D.resX() / 2.0f);
		Vec2f cc(pos);

		// get circle bounds
		cc.x = (float) max((double) cc.x, (double) radius);
		cc.x = (float) min((double) cc.x, (double) mFluid2D.resX() - radius);
		cc.y = (float) max((double) cc.y, (double) radius);
		cc.y = (float) min((double) cc.y, (double) mFluid2D.resY() - radius);

		// only check pixels in circle rec bounds
		Area ca((int32_t) (cc.x - radius - 2.0f), (int32_t) (cc.y - radius - 2.0f),
				(int32_t) (cc.x + radius + 2.0f), (int32_t) (cc.y + radius + 2.0f));
		for (int32_t y = ca.getY1(); y < ca.getY2(); ++y) {
			for (int32_t x = ca.getX1(); x < ca.getX2(); ++x) {
				Vec2f d = Vec2f((float) x, (float) y);
				Vec2f vv = d - cc;
				double dx = cc.x - x;
				double dy = cc.y - y;
				dx *= dx;
				dy *= dy;
				double distanceSquared = dx + dy;
				double radiusSquared = radius * radius;
				if(distanceSquared <= radiusSquared) {
					mFluid2D.velocityAt(x, y) = (vv * obstacleVelocityScale) + dv;
				}
			}
		}

		// create fluid splat at position
		mFluid2D.splatVelocity( pos.x, pos.y, dv * mVelScale );
		mFluid2D.splatRgb( pos.x, pos.y, mRgbScale * mColor );
		if( mFluid2D.isBuoyancyEnabled() ) {
			mFluid2D.splatDensity( pos.x, pos.y, mDenScale );
		}
	}

	mFluid2D.step();
	mParticles.setColor( mColor );
	mParticles.update( &mTimer );





	// DRAW
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE );

	// draw particles
	glPointSize( aPointSize );
	glLineWidth( aPointSize );
	glBegin( GL_POINTS );
	for( int i = 0; i < mParticles.numParticles(); ++i ) {
		const Particle& part = mParticles.at( i );
		float alpha = std::min( part.age() / 1.0f, 0.75f );
		glColor4f( ColorAf( part.color(), alpha ) );
		glVertex2f( part.pos() );
	}
	glEnd();

	// draw particle vectors
	if (mDrawParticleVectors) {
		glBegin( GL_LINES );
		for( int i = 0; i < mParticles.numParticles(); ++i ) {
			const Particle& part = mParticles.at( i );
			float alpha = std::min( part.age() / 1.0f, 0.75f );
			glColor4f( ColorAf( part.color(), alpha ) );
			
			// arrow body
			glVertex2f( part.pos() );
			Vec2f dir = part.pos() - part.prevPos();
			dir.normalize();
			glVertex2f( part.pos() + (dir *= -aVectorLength) );

			// arrowhead
			dir.rotate( 135.0 * 3.14159f / 180.0f );
			dir.normalize();
			glVertex2f( part.pos() );
			glVertex2f( part.pos() + (dir *= -(aVectorLength/2.0)) );
			
			dir.rotate( -90.0 * 3.14159f / 180.0f );
			dir.normalize();
			glVertex2f( part.pos() );
			glVertex2f( part.pos() + (dir *= -(aVectorLength/2.0)) );
		}
		glEnd();
	}

	// draw obstacles
	if (showObstacles) {
		glColor3f(mColor);
		for( int i = 0; i < mNumObstacles; ++i ) {
			drawSolidCircle(mObstacles.at( i ), obstacleRadius);
		}
	}

	// draw origin
	//drawSolidCircle(mPosition, 10);
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
