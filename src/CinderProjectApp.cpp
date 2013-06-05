#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

#include "cinder/Rect.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class CinderProjectApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
};

void CinderProjectApp::setup()
{
	Rectf r;
	r.set(1.0f, 1.0f, 1.0f, 1.0f);
	cout << r.calcArea() << endl;
}

void CinderProjectApp::mouseDown( MouseEvent event )
{
}

void CinderProjectApp::update()
{
}

void CinderProjectApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP_NATIVE( CinderProjectApp, RendererGl )
