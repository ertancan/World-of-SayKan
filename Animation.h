#ifndef ANIMATION_H
#define ANIMATION_H
#include "GraphicsEngine.h"
#include <string>
#include <vector>
//#include <ext/hash_map>
#include <hash_map>

using namespace std;

namespace std { using namespace __gnu_cxx; }

struct eqstr{
  
  	bool operator()(const char* s1, const char* s2) const {
    		
    		return strcmp(s1,s2)==0;
  	}
};

class Animation{

	private:
	
		SDL_Surface* surfaces;
		
		SDL_Surface* runningRight;
		SDL_Surface* runningLeft;
		//SDL_Surface* jumpingRight;
		//SDL_Surface* jumpingLeft;
		
		//Firing, Crouching .. etc?
		
		int maxState,currentState;
		bool active;
		
		hash_map<const char*, SDL_Surface*, hash<const char*>, eqstr> hashedSurfaces;
		
	

	/**
	 * Loads all images of an AnimatingGameObject and returns it as an array
	 * this array goes into the "screenObjects"
	 * TODO not existing images may be removed from the gameObject? since it wont look good
	 * TODO getting lots of images from a single file may be implemented??
	 */
	SDL_Surface * loadMultipleImage(vector<string> filename){
	
		SDL_Surface *temp=NULL;
		SDL_Surface *opt=new SDL_Surface[filename.size()];
		for(int i=0;i<filename.size();i++){
		
			temp=IMG_Load(filename[i].c_str());
			if(temp!=NULL){ //may occure a problem if one of the images are not there
				opt[i]=*SDL_DisplayFormat(temp);
				SDL_FreeSurface(temp);
			}
		}
		return opt;
	}

	public:
		Animation(){
	
			surfaces=NULL;
			maxState=0;
			currentState=0;
			active=false;
		}
		
		Animation(vector<string>* filenames){
		
			maxState=filenames->size();
			currentState=0;
			active=false;
			surfaces=loadMultipleImage(*filenames);
			
		}

		
		int getMaxState(){
			return maxState;
		}
		int getCurrentState(){
			return currentState;
		}
		bool isActive(){
			return active;
		}
		void animate(){
			active=true;
		}
		void stop(){
			active=false;
		}
		void toggle(){
			active=!active;
		}
		void increaseState(){
			currentState=(currentState+1)%maxState;
		}
		bool setState(int s){
			if(s>=maxState)
				return false;
			currentState=s;
			return true;
		}
		SDL_Surface* getSurfaces(){
			return surfaces;
		}
		

};

#endif
