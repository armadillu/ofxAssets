//
//  AssetHolder.cpp
//  BaseApp
//
//  Created by Oriol Ferrer Mesià on 27/03/15.
//
//

#include "AssetHolder.h"
#include "ofxChecksum.h"
#include "ofxThreadSafeLog.h"
#include "AssetHolderStructs.h"

using namespace ofxAssets;


//create empty objects to be able to return references on bogus calls
ofxAssets::Descriptor AssetHolder::emptyAsset;
ofxAssets::UserInfo AssetHolder::emptyUserInfo;
int AssetHolder::minimumFileSize = 1024;
ofMutex AssetHolder::assetMutex;

AssetHolder::AssetHolder(){
	isSetup = false;
	isDownloadingData = false;
}


void AssetHolder::setup(const string& directoryForAssets_, const ofxAssets::UsagePolicy & assetOkPolicy_,
						const ofxAssets::DownloadPolicy & downloadPolicy_){
	isSetup = true;
	directoryForAssets = ofFilePath::addTrailingSlash(directoryForAssets_);
	assetOkPolicy = assetOkPolicy_;
	downloadPolicy = downloadPolicy_;

	assetMutex.lock(); //ofSetLogLevel is not thread safe!
	ofSetLogLevel("ofxBatchDownloader", OF_LOG_SILENT);
	ofSetLogLevel("ofxSimpleHttp", OF_LOG_SILENT);
	assetMutex.unlock();
}


string AssetHolder::addRemoteAsset(const string& url,
								   const string& sha1,
								   const vector<string>& tags,
								   ofxAssets::Specs spec,
								   ofxAssets::Type type){

	ASSET_HOLDER_SETUP_CHECK;

	ofxAssets::Descriptor ad;
	ad.fileName = ofFilePath::getFileName(url);
	ad.relativePath = ofToDataPath(directoryForAssets + ad.fileName, false);

	unordered_map<string, ofxAssets::Descriptor>::iterator it = assets.find(ad.relativePath);
	if(it == assets.end()){ //we dont have this one
		ad.location = REMOTE;
		ad.extension = ofFilePath::getFileExt(url);
		ad.specs = spec;
		if(type == ofxAssets::TYPE_UNKNOWN){
			ad.type = typeFromExtension(ad.extension);
		}else{
			ad.type = type;
		}
		ad.url = url;
		ad.sha1 = sha1;
		if(sha1.size()) ad.status.sha1Supplied = true;
		assetAddOrder[assetAddOrder.size()] = ad.relativePath;
		assets[ad.relativePath] = ad;
		for(auto & tag : tags){
			string objectID = ad.relativePath; //assets inside an AssetHodler are indexed by they relative path
			this->tags.addTagForObject(objectID, Tag<TagCategory>(tag, CATEGORY));
		}
	}else{
		ofLogError("AssetHolder") << " Can't add this remote asset, already have it! " << ad.relativePath;
	}
	return ad.relativePath;
};


string AssetHolder::addLocalAsset(const string& localPath,
								  const vector<string>& tags,
								  ofxAssets::Specs spec,
								  ofxAssets::Type type
								  ){

	ASSET_HOLDER_SETUP_CHECK;

	ofxAssets::Descriptor ad;
	ad.relativePath = ofToDataPath(localPath, false);

	unordered_map<string, ofxAssets::Descriptor>::iterator it = assets.find(ad.relativePath);
	if(it == assets.end()){ //we dont have this one
		ad.location = LOCAL;
		ad.extension = ofFilePath::getFileExt(localPath);
		if(type == ofxAssets::TYPE_UNKNOWN){
			ad.type = typeFromExtension(ad.extension);
		}else{
			ad.type = type;
		}
		ad.specs = spec;
		ad.fileName = ofFilePath::getFileName(localPath);
		assetAddOrder[assetAddOrder.size()] = ad.relativePath;
		assets[ad.relativePath] = ad;

		for(auto & tag : tags){
			string objectID = ad.relativePath; //assets inside an AssetHodler are indexed by they relative path
			this->tags.addTagForObject(objectID, Tag<TagCategory>(tag, CATEGORY));
		}

	}else{
		ofLogError("AssetHolder") << " Can't add this remote asset, already have it! " << ad.relativePath;
	}
	return ad.relativePath;
}


bool AssetHolder::areAllAssetsOK(){
	int numOK = 0;
	unordered_map<string, ofxAssets::Descriptor>::iterator it = assets.begin();
	while( it != assets.end()){
		if (isReadyToUse(it->second)){
			numOK++;
		}
		++it;
	}
	return numOK == assets.size();
}

vector<ofxAssets::Descriptor>
AssetHolder::getBrokenAssets(){
	vector<ofxAssets::Descriptor> broken;
	unordered_map<string, ofxAssets::Descriptor>::iterator it = assets.begin();
	while( it != assets.end()){
		if (!isReadyToUse(it->second)){
			broken.push_back(it->second);
		}
		++it;
	}
	return broken;
}

ofxAssets::Stats AssetHolder::getAssetStats(){

	ofxAssets::Stats s;
	s.numAssets = assets.size();
	unordered_map<string, ofxAssets::Descriptor>::iterator it = assets.begin();
	while(it != assets.end()){
		ofxAssets::Descriptor & ad = it->second;
		if(ad.status.checked){
			if(ad.status.fileTooSmall) s.numFileTooSmall++;
			if(ad.status.sha1Match) s.numOK++;
			if(ad.status.downloaded && !ad.status.downloadOK) s.numDownloadFailed++;
			if(!ad.status.sha1Supplied) s.numNoSha1Supplied++;
			if(!ad.status.localFileExists) s.numMissingFile++;
			if(ad.status.downloaded && !ad.status.sha1Match) s.numSha1Missmatch++;
		}else{
			ofLogError("AssetHolder") << "Requesting AssetsStats before calling updateLocalAssetsStatus()! dont do that!";
		}
		++it;
	}
	return s;
}

void AssetHolder::downloadsFinished(ofxBatchDownloaderReport & report){

	int n = report.responses.size();
	for(int i = 0; i < n; i++){

		ofxSimpleHttpResponse & r = report.responses[i];

		if (remoteAssetExistsInDB(r.url)){

			isDownloadingData = false;

			ofxAssets::Descriptor & d = getAssetDescForURL(r.url);
			d.status.downloaded = true;
			d.status.downloadOK = r.ok;
			if(d.status.downloadOK){
				d.status.localFileExists = true;
				d.status.fileTooSmall = r.downloadedBytes < minimumFileSize;
			}else{
				ofLogError("AssetHolder") << "Asset download KO! \"" << r.reasonForStatus << "\"";
			}
			if (r.expectedChecksum == d.sha1 && r.checksumOK){
				d.status.sha1Match = true;
			}else{
				d.status.sha1Match = false;
				ofLogError("AssetHolder") << "Asset downloaded but SHA1 missmatch! [" << d.url << "] expected SHA1: " << d.sha1;
			}
		}else{
			ofLogError("AssetHolder") << "Asset downloaded but I dont know about it !? " << r.url;
		}
	}
}


void AssetHolder::updateLocalAssetsStatus(){

	unordered_map<string, ofxAssets::Descriptor>::iterator it = assets.begin();

	while( it != assets.end()){
		checkLocalAssetStatus(it->second);
		++it;
	}
}


void AssetHolder::checkLocalAssetStatus(ofxAssets::Descriptor & d){

	if(d.relativePath.size() == 0){
		ofLogError("AssetHolder") << "Asset with no 'relativePath'; cant checkLocalAssetStatus!";
		return;
	}
	ofFile f;
	f.open( d.relativePath );

	if(f.exists()){

		d.status.localFileExists = true;

		if (d.hasSha1()){
			d.status.sha1Supplied = true;
			d.status.localFileSha1Checked = true;
			d.status.sha1Match = ofxChecksum::sha1(d.relativePath, d.sha1, false/*verbose*/);

			if (d.status.sha1Match){
				ofxThreadSafeLog::one()->append(assetLogFile, "'" + string(d.url) + "' EXISTS and SHA1 OK 😄");
			}else{
				if (f.getSize() < minimumFileSize){
					d.status.fileTooSmall = true;
					ofxThreadSafeLog::one()->append(assetLogFile, "'" + string(d.url) + "' file is empty!! 😨");
				}else{
					ofxThreadSafeLog::one()->append(assetLogFile, "'" + string(d.url) + "' CORRUPT! (sha1 missmatch) 💩 " + d.sha1);
				}
			}
		}else{ //no sha1 supplied!
			ofxThreadSafeLog::one()->append(assetLogFile, "'" + string(d.url) + "' (sha1 not supplied) 🌚");
			d.status.sha1Supplied = false;
			if (f.getSize() < minimumFileSize){
				d.status.fileTooSmall = true;
				ofxThreadSafeLog::one()->append(assetLogFile, "'" + string(d.url) + "' file is empty!! 😨");
			}
		}
	}else{
		d.status.localFileExists = false;
		ofxThreadSafeLog::one()->append(assetLogFile, "'" + string(d.url) + "' Does NOT EXIST! 😞");
	}
	f.close();
	d.status.checked = true;

}



vector<string> AssetHolder::downloadMissingAssets(ofxDownloadCentral& downloader){

	if(!isDownloadingData){

		vector<string> urls;
		vector<string> sha1s;

		unordered_map<string, ofxAssets::Descriptor>::iterator it = assets.begin();

		while( it != assets.end() ){

			ofxAssets::Descriptor & d = it->second;

			if(d.location == REMOTE){
				if(shouldDownload(d)){
					urls.push_back(d.url);
					sha1s.push_back(d.sha1);
				}
			}
			++it;
		}

		if(urls.size()){
			downloader.downloadResources(urls,								//list of urls
										 sha1s,								//matching list of sha1s TODO!
										 this,								//who will get notified
										 &AssetHolder::downloadsFinished,	//callback
										 directoryForAssets					//where to download the assets
										 );
			isDownloadingData = true;
		}
		return urls;
	}else{
		ofLogError("AssetHolder") << "Cant download now! Already downloading data...";
	}
	return vector<string>();
}

vector<ofxAssets::Descriptor> AssetHolder::getAllAssetsInDB(){

	vector<ofxAssets::Descriptor> allAssets;
	for(auto & it : assetAddOrder){
		string assetPath = it.second;
		allAssets.push_back(assets[assetPath]);
	}

	return allAssets;
}
