//
//  AssetHolder.cpp
//  BaseApp
//
//  Created by Oriol Ferrer Mesià on 27/03/15.
//
//

#include "AssetHolder.h"
#include "ofxThreadSafeLog.h"
#include "AssetHolderStructs.h"

using namespace ofxAssets;
using namespace std;


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
//	oldSimpleHttpLevel = ofGetLogLevel("ofxSimpleHttp");
//	oldBatchDownloaderLevel = ofGetLogLevel("ofxBatchDownloader");
//	ofSetLogLevel("ofxBatchDownloader", OF_LOG_SILENT);
//	ofSetLogLevel("ofxSimpleHttp", OF_LOG_SILENT);
	assetMutex.unlock();
}


string AssetHolder::addRemoteAsset(const string& url,
								   const string& checksum,
								   const ofxChecksum::Type checksumType,
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
		ad.checksum = checksum;
		ad.checksumType = checksumType;
		if(checksum.size()) ad.status.checksumSupplied = true;
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
			if(ad.status.checksumMatch) s.numOK++;
			if(ad.status.downloaded && !ad.status.downloadOK) s.numDownloadFailed++;
			if(!ad.status.checksumSupplied) s.numNoChecksumSupplied++;
			if(!ad.status.localFileExists) s.numMissingFile++;
			if(ad.status.downloaded && !ad.status.checksumMatch) s.numChecksumMissmatch++;
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
				ofLogError("AssetHolder") << "Asset download KO! \"" << r.reasonForStatus << "\" \"" << r.url << "\"";
			}
			if(r.checksumType == d.checksumType){
				if (r.expectedChecksum == d.checksum && r.checksumOK){
					d.status.checksumMatch = true;
				}else{
					d.status.checksumMatch = false;
					ofLogError("AssetHolder") << "Asset downloaded but checksum mismatch! [" << d.url << "] expected checksum: \"" << d.checksum << "\" but got \"" << r.calculatedChecksum << "\" instead";
				}
			}else{
				ofLogError("AssetHolder") << "Asset downloaded but checksum type mismatch! Make sure checksum types match!";
				d.status.checksumMatch = false;
			}
		}else{
			ofLogError("AssetHolder") << "Asset downloaded but I dont know about it !? " << r.url;
		}
	}
//	ofSetLogLevel("ofxBatchDownloader", oldBatchDownloaderLevel);
//	ofSetLogLevel("ofxSimpleHttp", oldSimpleHttpLevel);
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

		if (d.hasChecksum()){
			d.status.checksumSupplied = true;
			d.status.localFileChecksumChecked = true;

			if(d.checksumType == ofxChecksum::Type::SHA1){
				d.status.checksumMatch = ofxChecksum::sha1(d.relativePath, d.checksum, false/*verbose*/);
			}else{ //for now only two types, so it must be xxHash
				auto sum = ofxChecksum::xxHash(d.relativePath);
				d.status.checksumMatch = sum == d.checksum;
			}

			if (d.status.checksumMatch){
				ofxThreadSafeLog::one()->append(assetLogFile, "'" + string(d.url) + "' EXISTS and Checksum OK 😄");
			}else{
				if (f.getSize() < minimumFileSize){
					d.status.fileTooSmall = true;
					ofxThreadSafeLog::one()->append(assetLogFile, "'" + string(d.url) + "' file is empty!! 😨");
				}else{
					ofxThreadSafeLog::one()->append(assetLogFile, "'" + string(d.url) + "' CORRUPT! (Checksum mismatch) 💩 expected \"" + d.checksum + "\"");
				}
			}
		}else{ //no sha1 supplied!
			ofxThreadSafeLog::one()->append(assetLogFile, "'" + string(d.url) + "' (Checksum not supplied) 🌚");
			d.status.checksumSupplied = false;
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
		vector<string> checksums; //

		unordered_map<string, ofxAssets::Descriptor>::iterator it = assets.begin();

		while( it != assets.end() ){

			ofxAssets::Descriptor & d = it->second;

			if(d.location == REMOTE){
				if(shouldDownload(d)){
					urls.push_back(d.url);
					checksums.push_back(d.checksum);
				}
			}
			++it;
		}

		if(urls.size()){
			downloader.downloadResources(urls,								//list of urls
										 checksums,								//matching list of checksums
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
