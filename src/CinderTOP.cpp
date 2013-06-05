#include "cinder/Cinder.h"
#include "cinder/gl/gl.h"

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
		int						 myExecuteCount;
};

using namespace ci;
using namespace std;

// SETUP
CinderTOP::CinderTOP(const TOP_NodeInfo *info) : myNodeInfo(info) {
	myExecuteCount = 0;
}
CinderTOP::~CinderTOP() {}


// DRAW LOOP
void CinderTOP::execute(const TOP_OutputFormatSpecs* outputFormat , const TOP_InputArrays* arrays, void* reserved) {

	myExecuteCount++;

	//gl::drawColorCube(Vec3f(0,0,0), Vec3f(30,30,30));

	// Lets just draw a small red square in the lower left quadrant of the texture
	::glColor4f(1, 0, 0, 1);
	::glMatrixMode(GL_MODELVIEW);
	::glPushMatrix();
	::glRotatef(myExecuteCount, 0.0f, 0.0f, 1.0f);
	::glBegin(GL_QUADS);
	::glVertex2i(0, 0);
	::glVertex2i(outputFormat->width / 2, 0);
	::glVertex2i(outputFormat->width / 2, outputFormat->height / 2);
	::glVertex2i(0, outputFormat->height / 2);
	::glEnd();
	::glPopMatrix();

	// Draw a diamond to test anti-aliasing (it will draw over part of the above square)
	::glPushMatrix();
	::glRotatef(-myExecuteCount, 0.0f, 0.0f, .150f);
	::glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
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
