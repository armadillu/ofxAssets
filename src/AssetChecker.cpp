//
//  AssetChecker.cpp
//  BaseApp
//
//  Created by Oriol Ferrer Mesià on 01/04/15.
//
//

#include "AssetChecker.h"


void AssetCheckThread::checkAssetsInThread(const vector<AssetHolder*>& assetObjects_){
	if(isThreadRunning()){
		ofLogError("AssetCheckThread") << "thread already running!";
	}
	assetObjects = assetObjects_;
	progress = 0;
	startThread();
}


void AssetCheckThread::threadedFunction(){

	#ifdef TARGET_WIN32
	#else
	pthread_setname_np("AssetCheckThread");
	#endif

	for(int i = 0; i < assetObjects.size(); i++){
		assetObjects[i]->updateLocalAssetsStatus();
		progress = i / float(assetObjects.size() - 1);
	}
	ofNotifyEvent(eventFinishedCheckingAssets, this);
	ofSleepMillis(16);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void AssetChecker::update(){

	if (started){
		int numRunningThreads = 0;
		for(int i = 0; i < threads.size(); i++){
			if (threads[i]->isThreadRunning()){
				numRunningThreads++;
			}
		}
		if(numThreadsCompleted == threads.size() && numRunningThreads == 0){
			for(int i = 0; i < threads.size(); i++){
				delete threads[i];
			}
			threads.clear();
			started = false;
			ofNotifyEvent(eventFinishedCheckingAllAssets, this);
		}
	}
}


AssetChecker::AssetChecker(){
	started = false;
}

string AssetChecker::getDrawableState(){

	if(started){
		string msg;
		msg += "AssetChecker : checking assets integrity.";
		msg += "\n\n";
		vector<float> progress = getPerThreadProgress();
		for(int i = 0; i < progress.size(); i++){
			msg += "  Thread (" + ofToString(i) + "): " + ofToString(100 * progress[i], 1) +
			"% done. (" + ofToString((int)threads[i]->getNumObjectsChecked()) + " of " +
			ofToString((int)threads[i]->getNumObjectsToCheck()) + " Assets Checked)\n";
		}
		return msg;
	}else{
		return "AssetChecker : Idle";
	}

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

		mutex.lock();
		ofLogNotice("AssetChecker") << "Start CheckAssets Thread! (" << i << "/" << numThreads << ") - checking " << end - start << " objects";
		mutex.unlock();

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


vector<float> AssetChecker::getPerThreadProgress(){
	vector<float> p;
	for(int i = 0; i < threads.size(); i++){
		p.push_back(threads[i]->getProgress());
	}
	return p;
}


void AssetChecker::onAssetCheckThreadFinished() {
	mutex.lock();
	numThreadsCompleted++;
	if (numThreadsCompleted == threads.size()) {
		ofLogNotice("AssetChecker") << "All AssetCheck Threads Finished (" << numThreadsCompleted << ")";
	}
	mutex.unlock();
}

