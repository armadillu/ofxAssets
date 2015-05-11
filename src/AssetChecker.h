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

	AssetCheckThread();
	void checkAssetsInThread(const vector<AssetHolder*>& assetObjects);

	float getProgress(){return progress;}
	ofEvent<bool> eventFinishedCheckingAssets;

private:

	float progress;
	void threadedFunction();
	vector<AssetHolder*> assetObjects;
};


class AssetChecker{

public:
	
	AssetChecker();

	void checkAssets(vector<AssetHolder*> assetObjects, int numThreads = Poco::Environment::processorCount());
	void update();
	float getProgress();

	//callback
	void onAssetCheckThreadFinished(bool &);


	ofEvent<bool> eventFinishedCheckingAllAssets;

protected:

	bool started;
	int numThreadsCompleted;
	vector<AssetCheckThread*> threads;
	vector<AssetHolder*> assetObjects;
	ofMutex mutex;
};

#endif /* defined(__BaseApp__AssetChecker__) */
