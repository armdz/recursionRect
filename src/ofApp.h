#pragma once

#include "ofMain.h"
#include "ofxThreadedImageLoader.h"
#include "ofxGui.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
  
    void  load_the_image(string _path);
    void  do_recursion(ofRectangle  _rect,int _levels,int _max_levels);
    
    ofImage image;
    bool    load_image;
    bool    image_ok;
    string  load_image_path;
    ofxThreadedImageLoader  image_loader;
    
    float   image_view_w,image_view_h,image_view_max_w,image_view_max_h;
    float   image_alpha;
    
    ofxPanel  gui;
    ofxIntSlider    levels;
    ofxIntSlider    mode;

    ofxFloatSlider  noise_max;
    ofxToggle       preservere_source;
    ofxToggle       clear_buffer;
    ofxButton       generate;
    ofxButton       save;

    bool            prepare,working,work_done,saving,prepare_save;
    int             rec_level;
  
    ofFbo           fbo_output;
  
};
