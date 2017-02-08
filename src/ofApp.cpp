#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
  
  ofSetCircleResolution(128);
  
  load_image = false;
  
  //  to match the image in the screen
  image_view_w = image_view_h = 0;
  image_view_max_w = ofGetWidth()-100.0f;
  image_view_max_h = ofGetHeight()-100.0f;
  //  nice
  image_alpha = 0.0f;
  //  ui
  gui.setup("Params");
  levels.setup("Max Iterations",5,2,10);
  gui.add(&levels);
  noise_max.setup("Noise Range",.3f,0.0f,1.0f);
  gui.add(&noise_max);
  generate.setup("Generate",false);
  gui.add(&generate);
  preservere_source.setup("Preserver Source",false);
  gui.add(&preservere_source);
  clear_buffer.setup("Clear Buffer",true);
  gui.add(&clear_buffer);
  mode.setup("Mode",0,0,3);
  gui.add(&mode);
  save.setup("Save",false);
  gui.add(&save);
  //  use an extra boolean to show a msg a frame before the process
  prepare = working = image_ok = saving = prepare_save = false;
    
}

//--------------------------------------------------------------
void ofApp::update(){

  
  if(prepare && !working)
  {
    //  start the process
    working = true;
    rec_level = 0;
    fbo_output.begin();
    if(clear_buffer)
      ofClear(0,0);
    if(preservere_source)
    {
      ofSetColor(255);
      image.draw(0,0,image.getWidth(),image.getHeight());
    }
    do_recursion(ofRectangle(0,0,image.getWidth(),image.getHeight()),rec_level,levels);
    fbo_output.end();
  }
  if(prepare_save && !saving)
  {
    //  save the file
    saving = true;
    ofImage save_image;
    fbo_output.readToPixels(save_image.getPixels());
    string file_name = ofToString(ofGetDay())+"_"+ofToString(ofGetHours())+"_"+ofToString(ofGetMinutes())+"_"+ofToString(ofGetSeconds())+".png";
    save_image.save("output/"+file_name);
    saving = false;
    prepare_save = false;
  }
  
  // asyn image loading
  if(load_image)
  {
    image_ok = (image.isAllocated() && image.getWidth() > 0 && image.getHeight() > 0);
    if(image_ok)
    {
      //  loaded
      load_image = false;
      //  limit the image size to the windows resolution
      if(image.getWidth() > image.getHeight())
      {
        image_view_w = image_view_max_w;
        image_view_h = (image_view_max_w*image.getHeight())/image.getWidth();
      }else{
        image_view_w = (image_view_max_h*image.getWidth())/image.getHeight();
        image_view_h = image_view_max_h;
      }
      //  resize
      image.resize(image_view_w,image_view_h);
      //  allocate fbo
      ofFbo::Settings settings;
      settings.width = image.getWidth();
      settings.height = image.getHeight();
      settings.numSamples = 8;
      settings.internalformat = GL_RGBA32F;
      fbo_output.allocate(settings);
      fbo_output.begin();
      ofClear(0,0);
      fbo_output.end();
      
    }
  }else{
    if(image_ok)
    {
      //  nice
      if(image_alpha < 1.0f)
      {
        image_alpha+=(1.0f-image_alpha)*.1f;
      }
      
      if(generate && !prepare){
        work_done = false;
        prepare = true;
      }
      
      if(save)
      {
        saving = false;
        prepare_save = true;
      }
      
    }
  }
  
  
}

//--------------------------------------------------------------
void ofApp::draw(){
  
  ofBackground(0);
  
  if(!image_ok)
  {
    ofPushMatrix();
    ofTranslate(ofGetWidth()*.5f,ofGetHeight()*.5f);
    ofSetColor(255);
    ofNoFill();
    ofDrawRectangle(-100, -100, 200, 200);
    ofFill();
    string help_string = !load_image ? "DRAG AN IMAGE" : "LOADING";
    ofDrawBitmapString(help_string,-(help_string.size()/2.0f)*8.0f,0);
    ofPopMatrix();
  }else{
    ofPushMatrix();
    ofTranslate(ofGetWidth()*.5f,ofGetHeight()*.5f);
    ofTranslate(-image_view_w/2,-image_view_h/2);
    ofSetColor(255,255.0f*image_alpha);
    if(!work_done){
      //  before the process
      image.draw(0,0,image_view_w,image_view_h);
    }else{
      //  the draw stuff is done
      ofSetColor(255);
      fbo_output.draw(0,0,fbo_output.getWidth(),fbo_output.getHeight());
    }
    
    if(prepare || prepare_save)
    {
      ofSetColor(0,100);
      ofDrawRectangle(0,0,image_view_w,image_view_h);
      ofPushMatrix();
      ofTranslate(image_view_w*.5f,image_view_h*.4f);
      string prep_help = "WAIT ! (the app freezes)";
      if(prepare_save)
      {
        prep_help = "SAVING (the app freezes)";
      }
      ofSetColor(255);
      ofDrawBitmapString(prep_help,-(prep_help.size()/2.0f)*8.0f,0);
      ofPopMatrix();
    }
    
    ofPopMatrix();
    
  }
  
  gui.draw();

}

//--------------------------------------------------------------

void  ofApp::load_the_image(string _path)
{
  image.clear();
  fbo_output.clear();
  image_alpha = 0.0f;
  work_done = false;
  load_image = true;
  image_ok = false;
  load_image_path = _path;
  image_loader.stopThread();
  image.getTexture().setTextureWrap( GL_REPEAT, GL_REPEAT );
  image_loader.loadFromDisk(image,load_image_path);
}

//  do the thing

void ofApp::do_recursion(ofRectangle  _rect,int _levels,int _max_levels)
{
  float new_width = _rect.getWidth()/2.0f;
  float new_height = _rect.getHeight()/2.0f;
  if(_levels < _max_levels)
  {
    _levels++;
    for(int i=0;i<4;i++)
    {
      float x = _rect.x+(i%2)*new_width;
      float y = _rect.y+(i == 0 ? 0 : 1%i)*new_height;
      ofRectangle rect(x,y, new_width, new_height);

      //  draw  inside the rect bounds !
      
      if(ofNoise(x,y,x+y+i) > noise_max){
        
        ofColor color = image.getColor(x,y);
        
        if(mode == 0){
          ofSetColor(color);
          ofDrawRectangle(rect);
          ofNoFill();
          ofSetColor(0);
          ofDrawRectangle(rect);
          ofFill();
        }else if(mode == 1)
        {
          ofNoFill();
          ofSetColor(color);
          ofDrawCircle(x+rect.getWidth()/2, y+rect.getWidth()/2, rect.getWidth()/2);
          ofFill();
          ofSetColor(color);
          ofDrawLine(rect.x, rect.y, rect.x+rect.getWidth(), rect.y+rect.getHeight());
          ofDrawLine(rect.x+rect.getWidth(),rect.y,rect.x, rect.y+rect.getHeight());
        }else if(mode == 3)
        {
          ofSetColor(255);
          float noise = ofNoise(rect.x,rect.y,rect.getWidth());
          float x_d = rect.x+(noise*rect.x);
          float y_d = rect.y+(noise*rect.y);
          image.getTexture().drawSubsection(rect.x, rect.y, rect.getWidth(), rect.getHeight(), x_d,y_d);

        }
      }
      
      //
      
      do_recursion(rect,_levels,_max_levels);
      
    }
  }else{
    prepare = working = false;
    work_done = true;
  }

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
  if(!load_image){
    string path = dragInfo.files.at(0);
    string ext = ofToLower(path.substr(path.size()-4,path.size()));
    if(ext == ".png" || ext == ".jpg")
    {
      load_the_image(path);
    }
  }
}
