//
//  AssetChecker.cpp
//  BaseApp
//
//  Created by Oriol Ferrer Mesià on 01/04/15.
//
//

#include "AssetChecker.h"

AssetCheckThread::AssetCheckThread(){
	progress = 0;
}


void AssetCheckThread::checkAssetsInThread(const vector<AssetHolder*>& assetObjects_){
	if(isThreadRunning()){
		ofLogError("AssetCheckThread") << "thread already running!";
	}
	assetObjects = assetObjects_;
	progress = 0;
	startThread();
}


void AssetCheckThread::threadedFunction(){

	for(int i = 0; i < assetObjects.size(); i++){
		assetObjects[i]->updateLocalAssetsStatus();
		progress = i / float(assetObjects.size() - 1);
	}
	bool whatever;
	ofNotifyEvent(eventFinishedCheckingAssets, whatever, this);
}

void AssetChecker::update(){

	if (started){

		int numRunningThreads = 0;
		for(int i = 0; i < threads.size(); i++){
			if (threads[i]->isThreadRunning()){
				numRunningThreads++;
			}
		}
		if(numThreadsCompleted == threads.size() && numRunningThreads == 0){
			started = false;
			for(int i = 0; i < threads.size(); i++){
				delete threads[i];
			}
			threads.clear();

			bool whatever;
			ofNotifyEvent(eventFinishedCheckingAllAssets, whatever, this);
		}
	}
}


AssetChecker::AssetChecker(){
	started = false;
}


void AssetChecker::checkAssets(vector<AssetHolder*> assetObjects_, int numThreads){
	assetObjects = assetObjects_;
	int nPerThread = assetObjects.size() / numThreads;
	numThreadsCompleted = 0;
	started = true;
	for(int i = 0; i < numThreads; i++){

		AssetCheckThread * t = new AssetCheckThread();
		threads.push_back(t);

		//lets split the work around threads
		int start, end;
		start = floor(i * nPerThread);

		if (i < numThreads -1){
			end = floor((i+1) * nPerThread) ;
		}else{ //special case for last thread, int division might not be even
			end = assetObjects.size();
		}

		//create a subvector, give it to thread
		vector<AssetHolder*>::const_iterator first = assetObjects.begin() + start;
		vector<AssetHolder*>::const_iterator last = assetObjects.begin() + end;
		vector<AssetHolder*> objsForThisThread(first, last);

		ofAddListener(t->eventFinishedCheckingAssets, this, &AssetChecker::onAssetCheckThreadFinished);
		t->checkAssetsInThread(objsForThisThread);
	}
}


float AssetChecker::getProgress(){
	float progress = 0.0f;
	for(int i = 0; i < threads.size(); i++){
		progress += threads[i]->getProgress() / (threads.size());
	}
	return  progress;
}

void AssetChecker::onAssetCheckThreadFinished(bool &){
	mutex.lock();
	ofLogNotice("AssetChecker") << "AssetCheck Thread Finished (" << numThreadsCompleted << "/" << (int)threads.size() << ")";
	mutex.unlock();
	numThreadsCompleted++;
}

