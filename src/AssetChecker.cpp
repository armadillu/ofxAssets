//
//  AssetChecker.cpp
//  BaseApp
//
//  Created by Oriol Ferrer Mesià on 01/04/15.
//
//

#include "AssetChecker.h"
#include "AssetHolder.h"


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
	#elif defined(TARGET_LINUX)
	pthread_setname_np(pthread_self(), "AssetCheckThread");
	#else
	pthread_setname_np("AssetCheckThread");
	#endif

	for(int i = 0; i < assetObjects.size(); i++){
		assetObjects[i]->updateLocalAssetsStatus();
		progress = (i + 1) / float(assetObjects.size());
	}
	progress = 1.0;
	ofNotifyEvent(eventFinishedCheckingAssets, this);
	ofSleepMillis(24);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void AssetChecker::update(){

	if (started){
		threadsMutex.lock();
		int numRunningThreads = 0;
		for(int i = 0; i < threads.size(); i++){
			if (threads[i]->isThreadRunning()){
				numRunningThreads++;
			}
		}
		bool finishedWork = false;
		if(numThreadsCompleted == threads.size() && numRunningThreads == 0){
			for(int i = 0; i < threads.size(); i++){
				delete threads[i];
			}
			threads.clear();
			finishedWork = true;
		}
		threadsMutex.unlock();
		if(finishedWork){
			started = false;
			ofNotifyEvent(eventFinishedCheckingAllAssets, this);
		}
	}
}


string AssetChecker::getDrawableState(){

	if(started){
		string msg;
		msg += "AssetChecker : checking assets integrity.";
		msg += "\n\n";
		vector<float> progress = getPerThreadProgress();
		for(int i = 0; i < progress.size(); i++){
			char aux[5]; sprintf(aux, "%03d", i);
			msg += "  Thread (" + string(aux) + "): " + ofToString(100 * progress[i], 1) +
			"% done. (" + ofToString((int)threads[i]->getNumObjectsChecked()) + " of " +
			ofToString((int)threads[i]->getNumObjectsToCheck()) + " Assets Checked)\n";
		}
		return msg;
	}else{
		return "AssetChecker : Idle";
	}

}

void AssetChecker::checkAssets(const vector<AssetHolder*> & assetObjects_, uint64_t numThreads){

	if(numThreads <= 0) numThreads = 1;

	assetObjects = assetObjects_;
	float nPerThread = float(assetObjects.size()) / float(numThreads);
	numThreadsCompleted = 0;
	started = true;

	uint64_t numAssets = 0;
	for(auto & obj : assetObjects){
		numAssets += obj->getNumAssets();
	}

	for(int i = 0; i < numThreads; i++){

		AssetCheckThread * t = new AssetCheckThread();
		threadsMutex.lock();
		threads.push_back(t);
		threadsMutex.unlock();

		//lets split the work around threads
		uint64_t start, end;
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

		if(end - start > 0 && numAssets > 0){
			ofLogNotice("AssetChecker") << "Start CheckAssets Thread! (" << i << "/" << numThreads << ") - checking objects " << start << " to " << end;
		}

		ofAddListener(t->eventFinishedCheckingAssets, this, &AssetChecker::onAssetCheckThreadFinished);
		t->checkAssetsInThread(objsForThisThread);
	}
}


float AssetChecker::getProgress(){
	float progress = 0.0f;
	threadsMutex.lock();
	for(int i = 0; i < threads.size(); i++){
		progress += threads[i]->getProgress() / (threads.size());
	}
	threadsMutex.unlock();
	return  progress;
}


vector<float> AssetChecker::getPerThreadProgress(){
	vector<float> p;
	threadsMutex.lock();
	for(int i = 0; i < threads.size(); i++){
		p.push_back(threads[i]->getProgress());
	}
	threadsMutex.unlock();
	return p;
}


void AssetChecker::onAssetCheckThreadFinished() {
	threadsMutex.lock();
	numThreadsCompleted++;
	if (numThreadsCompleted == threads.size()) {
		ofLogNotice("AssetChecker") << "All AssetCheck Threads Finished (" << numThreadsCompleted << ")";
	}
	threadsMutex.unlock();
}

