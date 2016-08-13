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
#include "AssetHolder.h"
#include "Poco/Environment.h"

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
	
	AssetChecker();

	void checkAssets(vector<AssetHolder*> assetObjects, int numThreads = Poco::Environment::processorCount());
	void update();
	float getProgress();
	vector<float> getPerThreadProgress();

	string getDrawableState();


	//callback
	void onAssetCheckThreadFinished();


	ofEvent<void> eventFinishedCheckingAllAssets;

protected:

	bool started;
	int numThreadsCompleted;
	vector<AssetCheckThread*> threads;
	vector<AssetHolder*> assetObjects;
	ofMutex mutex;
};

#endif /* defined(__BaseApp__AssetChecker__) */
