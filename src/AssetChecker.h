//
//  AssetChecker.h
//  BaseApp
//
//  Created by Oriol Ferrer Mesià on 01/04/15.
//
//

#ifndef __BaseApp__AssetChecker__
#define __BaseApp__AssetChecker__

#include "ofMain.h"

class AssetHolder;

class AssetCheckThread : public ofThread{

public:

	void checkAssetsInThread(const vector<AssetHolder*>& assetObjects);

	float getProgress(){return progress;}
	ofEvent<void> eventFinishedCheckingAssets;

	int getNumObjectsToCheck(){return assetObjects.size();};
	int getNumObjectsChecked(){ return assetObjects.size() * progress; };

private:

	float progress = 0;
	void threadedFunction();
	vector<AssetHolder*> assetObjects;
};



class AssetChecker{

public:
	
	AssetChecker(){};

	void checkAssets(const vector<AssetHolder*> & assetObjects_, uint64_t numThreads = std::thread::hardware_concurrency());
	void update();
	float getProgress();
	vector<float> getPerThreadProgress();

	string getDrawableState();

	//callback
	void onAssetCheckThreadFinished();

	ofEvent<void> eventFinishedCheckingAllAssets;

protected:

	bool started = false;
	int numThreadsCompleted = 0;
	vector<AssetCheckThread*> threads;
	vector<AssetHolder*> assetObjects;
	ofMutex threadsMutex;
};

#endif /* defined(__BaseApp__AssetChecker__) */
