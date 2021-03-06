#include "GraphicsEngine.h"
KeyboardEvent* defineKey(SDL_Event*);
SDL_Surface * loadImage(string);
GraphicsEngine::GraphicsEngine(int width,int height, int bpp){
       	//Initialize SDL
    	SDL_Init(SDL_INIT_EVERYTHING);
       	TTF_Init();
        //For clearing outside of the background image if it is smaller than window size
		background=NULL;
        smallBG=false;
        sideRect=NULL;
        downRect=NULL;
        //avoid divide by 0 
        updateTime=1;
        eventTime=1;
        refreshTime=1;

   		screenWidth=width;
       	screenHeight=height;
       	screenBPP=bpp;
       	createScreen();
       	//Default text color
       	textColor.setR(0xFF);
       	textColor.setG(0xFF);
       	textColor.setB(0xFF);
       	textFont=NULL;
	}
GraphicsEngine::~GraphicsEngine(){
	killSDL();
}
/**
 * Creates a drawable screen
 */
void GraphicsEngine::createScreen(){
	screen=SDL_SetVideoMode(screenWidth,screenHeight,screenBPP,SDL_SWSURFACE);
}

/**
 * The necessary things to finilize the SDL
 * adding new sdl features may lead an addition to here!
 * like sdl-mixer etc..
 */
void GraphicsEngine::killSDL(){
    SDL_FreeSurface(screen);
    if(background!=NULL) SDL_FreeSurface(background);
    if(sideRect!=NULL) delete sideRect;
    if(downRect!=NULL) delete downRect;
    TTF_Quit();
	SDL_Quit();
}
/** 
 * Sets the mouse listener of graphic engine..
 * Listener should implement the MouseListener interface
 * The given object's mouse listener functions will be called
 * when there is a mouse event. Mouse events will be ignored if
 * no mouse listeners set.
 */
void GraphicsEngine::setMouseListener(MouseListener * mListen){
	mouseListener=mListen;
}
/**
 * Same consepts as setMouseListener..
 * Keyboard events also will be ignored if no keyboard listners set
 */
void GraphicsEngine::setKeyboardListener(KeyboardListener *kListen){
	keyboardListener=kListen;
}
/**
 * This is the starter function of game loop..
 * It simply checks the event pool to get any keyboard or
 * mouse events that occured.
 * It controls both of the event listeners to call their 
 * listener functions
 * This function has its time counter and its run time can be
 * get via getEventTime function.
 * TODO collecting the currently pressed and not released keys and defining new events for them
 * may be useful.. ex: for ctrl+a or alt+e or something.. However current keyboardEvent probably
 * will need to be changed..(maybe adding a get super key and get key functions? )
 */
void GraphicsEngine::checkEvents(){
	uint start=SDL_GetTicks();
	while(SDL_PollEvent(&event)){
		if(event.type == SDL_KEYDOWN){
			if(keyboardListener==NULL)
				return;
			keyboardListener->keyPressed(defineKey(&event));
		}
		if(event.type == SDL_KEYUP){
			if(keyboardListener== NULL)
				return;
			keyboardListener->keyReleased(defineKey(&event));
		}
		if(event.type == SDL_QUIT){
			cout<<"Quit Event Catched! Killing SDL\n";
			killSDL();
			exit(0);
		}
	}
    eventTime=SDL_GetTicks()-start;
}
/**
 * It should update game objects' current location and image
 */
void GraphicsEngine::updateGame(){
	uint start=SDL_GetTicks();
	for(int i=0;i<screenObjects.size();i++){
		GameObject *temp=screenObjects[i];
		temp->updateMovement();
		if(temp->isAnimating()){
			((AnimatingGameObject *)temp)->increaseState();
		}
	}
	drawWidgets();
	updateTime=SDL_GetTicks()-start;
	return;
}
/**
 * Set Window Title
 */
void GraphicsEngine::setTitle(string title){
	SDL_WM_SetCaption(title.c_str(),NULL);
}
/**
 * Delays the screen. 
 * Events do not interrupt the delay.. so 
 * keyboard and mouse events and system events
 * are not effective during this time. So using delayScreen(1000) or
 * similar function calls can be a problem.
 * TODO events may interrupt delay? or some events may interrupt like system events
 */
void GraphicsEngine::delayScreen(int time){
	SDL_Delay(time);
}
/**
 * Redraws the game objects to the screen
 * Last part of the game loop
 */
bool GraphicsEngine::refreshScreen(){
	uint start=SDL_GetTicks();
	addSurface(0,0,background,screen);
	(this->*funcToCall)();
	refreshTime=SDL_GetTicks()-start;
	return SDL_Flip(screen)!= -1;
}
bool GraphicsEngine::setBackground(string filename){
	int sideSpace,downSpace;
	background= loadImage(filename);
	if(background==NULL)
		return false;

	//If background is smaller than the screen then set new SDL_Rects to fill
	//remaining parts of screen
    if(screenWidth>background->w){
    	sideSpace=screenWidth-background->w;
    	sideRect=new SDL_Rect();
    	sideRect->x=background->w;
    	sideRect->y=0;
    	sideRect->w=sideSpace;
    	sideRect->h=background->h;
    	smallBG=true;
	}
    if(screenHeight>background->h){
    	downSpace=screenHeight-background->h;
    	downRect=new SDL_Rect();
    	downRect->x=0;
    	downRect->y=background->h;
    	downRect->w=screenWidth;
    	downRect->h=downSpace;
    	smallBG=true;
	}
	if(smallBG)
		funcToCall=&GraphicsEngine::clearBGRemainder;
	else 
		funcToCall=&GraphicsEngine::drawGameObjects;
    return true;
}
/**
 * Clears the outside parts of the background image
 * This may be avoided by setting the window size accurately
 * or disabling this feature by setClearBGRemainder(false);
 * it may slow down the execution by %10 currently
 * TODO: this slow down may be avoided by creating a filter layer
 * >>I dont know how<<
 * TODO: background color may be changed? just black may not be enough..
 */
bool GraphicsEngine::clearBGRemainder(){
	bool result=drawGameObjects();
	//Fill rectangles with black; 0x000000
	SDL_FillRect(screen,sideRect,0x000000);
	SDL_FillRect(screen,downRect,0x000000);
	return result;
}

/**
 * Sets whether the remaining parts of background should be filled or not
 */
void GraphicsEngine::setClearBGRemainder(bool flag){
	if(flag)
		funcToCall=&GraphicsEngine::clearBGRemainder;
	else 
		funcToCall=&GraphicsEngine::drawGameObjects;
}
/**
 * Adds given surface to given positions..
 */
void GraphicsEngine::addSurface(int x, int y, SDL_Surface * source, SDL_Surface * destination){
	addSurface(x,y,source,destination,NULL);
}
void GraphicsEngine::addSurface(int x, int y,SDL_Surface * source, SDL_Surface * destination,SDL_Rect * part){

 	SDL_Rect offset;  
 	offset.x=x;
 	offset.y=y;
 	SDL_BlitSurface(source,part,destination,&offset);
}
/**
 * Loads the image and returns a surface.. It is used when creating a gameObject
 * or other objects which requires image to draw on screen
 */
SDL_Surface * loadImage(string filename){
	SDL_Surface * temp=NULL;
	SDL_Surface * opt=NULL;
	temp=IMG_Load(filename.c_str());
	if(temp!=NULL){
		opt=SDL_DisplayFormat(temp);
		SDL_FreeSurface(temp);
	}
	return opt;
}


void GraphicsEngine::setTextColor(int r,int g,int b){
	textColor.setR(r);
	textColor.setG(g);
	textColor.setB(b);
}
/** 
 * A truetype font should be given..
 * Hadn't check with non truetype fonts
 */
bool GraphicsEngine::setTextFont(string fontName,int size){
	textFont=TTF_OpenFont(fontName.c_str(),size);
	return textFont!=NULL;
}
int GraphicsEngine::addText(string text,int x,int y){
    if(textFont==NULL){
    	return -1;
	}
    SDL_Surface *textSurface=TTF_RenderText_Solid(textFont,text.c_str(),textColor.getSDLColor());
    if(textSurface==NULL){
    	return -1;
	}
    return addGameObject(new TextObject(x,y),textSurface);
}
/**
 * Draws all game objects in screenObjects to the screen
 * screenObjects vector is only the object container
 * on the other hand objectSurfaces vector holds loaded images so as to
 * reduce system load every time. 
 */
bool GraphicsEngine::drawGameObjects(){
	if(screenObjects.size()==0)
		return false;
	for(int i=0;i<screenObjects.size();i++){
    	GameObject* temp=screenObjects[i];
    	if(!temp->isAnimating()){
    		addSurface(temp->getCords()->getX(),temp->getCords()->getY(),objectSurfaces[i],screen); 	
		}
		else{
    		addSurface(temp->getCords()->getX(),temp->getCords()->getY(),&objectSurfaces[i][reinterpret_cast<AnimatingGameObject *> (temp)->getAnimation()->getCurrentState()],screen);
		}
	}
	return true;	
}

/**
 * Draws all widgets to the screen..
 */
void GraphicsEngine::drawWidgets(){
	for(int i=0;i<widgets.size();i++){
		Widget* temp=widgets[i];
		temp->updateSurface();
		addSurface(temp->getCords()->getX(),temp->getCords()->getY(),temp->getSurface(),screen);
	}
}

int GraphicsEngine::addGameObject(GameObject * obj,SDL_Surface* surface){
	screenObjects.push_back(obj);
    objectSurfaces.push_back(surface);
	return screenObjects.size()-1;
}
int GraphicsEngine::addGameObject(GameObject* obj){
	//If the game object is an animating game object; there will be more than one
	//Surfaces per object so we have to add them to the vector as a array of SDL_Surface
	//Since our vector holds pointers, we just give a pointer which refers to an array.
	if(obj->isAnimating()) {
		AnimatingGameObject * tmp=reinterpret_cast<AnimatingGameObject*> (obj);
		return addGameObject(tmp,tmp->getAnimation()->getSurfaces());
	}

	return addGameObject(obj,loadImage(obj->getImage()->getFilename()));
}

int GraphicsEngine::addWidget(Widget* wid){
	widgets.push_back(wid);
	return widgets.size()-1;
}
void GraphicsEngine::removeWidget(Widget * wid){
	for(int i=0;i<widgets.size();i++){
		if(widgets[i]==wid){
			widgets.erase(widgets.begin()+i);
		}
	}
}

void GraphicsEngine::removeWidget(int id){
	if(id>=widgets.size())
		return;
	widgets.erase(widgets.begin()+id);
}

void GraphicsEngine::removeGameObject(int id){
	if(id>=screenObjects.size())
		return;
	screenObjects.erase(screenObjects.begin()+id);
	objectSurfaces.erase(objectSurfaces.begin()+id);
}
void GraphicsEngine::removeGameObject(GameObject* obj){
	for(int i=0;i<screenObjects.size();i++){
		if(screenObjects[i]==obj){
			screenObjects.erase(screenObjects.begin()+i);
			objectSurfaces.erase(objectSurfaces.begin()+i);
		}
	}
}
void GraphicsEngine::clearGameObjects(){
	objectSurfaces.clear();
	screenObjects.clear();
}

uint GraphicsEngine::getUpdateTime(){
	return updateTime;
}
uint GraphicsEngine::getEventTime(){
	return eventTime;                
}
uint GraphicsEngine::getRefreshTime(){
	return refreshTime;
}
/**gameLoopStart and gameLoopEnd is to calculate time pass for
 * one loop..Only one frame should be processed in order to
 * use getFPS.. otherwise calling getFPS would return useless results..
 * using get**Time functions may be usefull for calculating fps otherwise
 */
void GraphicsEngine::gameLoopStart(){
	loopStartTime=SDL_GetTicks();
}
void GraphicsEngine::gameLoopEnd(){
	loopEndTime=SDL_GetTicks();
}
/**
 * gameLoopStart and gameLoopEnd
 * should have been called before getting the FPS
 * this result may be wrong otherwise..
 * use getUpdateTime,getEventTime and getRefreshTime if you wont
 * call all functions
 * !!Returns maximum fps possible.. not the actual FPS!!
 * TODO: real FPS may be? :)
 */
int GraphicsEngine::getFPS(){
	int totalTime=loopEndTime-loopStartTime;
if(totalTime==0)
		return -1;
	return 1000/totalTime;
}

//non-class functions

/**
 * This functions tries to change SDL key event to
 * GraphicsEngine key event.. 
 */
KeyboardEvent* defineKey(SDL_Event* ev){
 	switch(ev->key.keysym.sym){
    	case SDLK_LEFT:
    		return new KeyboardEvent(KeyboardEvent::LEFT);
    		break;
    	case SDLK_RIGHT:
    		return new KeyboardEvent(KeyboardEvent::RIGHT);
    		break;
    	case SDLK_UP:
			return new KeyboardEvent(KeyboardEvent::UP);
    		break;
    	case SDLK_DOWN:
    		return new KeyboardEvent(KeyboardEvent::DOWN);
    		break;
    	case SDLK_0:
    		return new KeyboardEvent(KeyboardEvent::NUM0);
    		break;
    	case SDLK_1:
    		return new KeyboardEvent(KeyboardEvent::NUM1);
    		break;
    	case SDLK_2:
    		return new KeyboardEvent(KeyboardEvent::NUM2);
    		break;
    	case SDLK_3:
    		return new KeyboardEvent(KeyboardEvent::NUM3);
    		break;
    	case SDLK_4:
    		return new KeyboardEvent(KeyboardEvent::NUM4);
    		break;
    	case SDLK_5:
    		return new KeyboardEvent(KeyboardEvent::NUM5);
    		break;
    	case SDLK_6:
    		return new KeyboardEvent(KeyboardEvent::NUM6);
    		break;
    	case SDLK_7:
    		return new KeyboardEvent(KeyboardEvent::NUM7);
    		break;
    	case SDLK_8:
    		return new KeyboardEvent(KeyboardEvent::NUM8);
    		break;
    	case SDLK_9:
    		return new KeyboardEvent(KeyboardEvent::NUM9);
    		break;
    	case SDLK_ASTERISK:
    		return new KeyboardEvent(KeyboardEvent::LSTAR);
    		break;
    	case SDLK_MINUS:
    		return new KeyboardEvent(KeyboardEvent::LMINUS);
    		break;
    	default:
    		return new KeyboardEvent();
	}
}

