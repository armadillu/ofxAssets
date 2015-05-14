//
//  AssetHolderUtils.cpp
//  BaseApp
//
//  Created by Oriol Ferrer Mesià on 27/03/15.
//
//

#include "AssetHolder.h"

bool AssetHolder::localAssetExistsInDB(const string& absPath){
	unordered_map<string, AssetDescriptor>::iterator it = assets.find(absPath);
	return it != assets.end();
}



bool AssetHolder::remoteAssetExistsInDB(const string& url){

	unordered_map<string, AssetDescriptor>::iterator it = assets.begin();
	while( it != assets.end()){
		if(it->second.url == url){
			return true;
		}
		++it;
	}
	return false;
}


AssetHolder::AssetDescriptor&
AssetHolder::getAssetDescForAbsPath(const string& absPath){
	unordered_map<string, AssetDescriptor>::iterator it = assets.find(absPath);
	if(it != assets.end()){
		return it->second;
	}
	return emptyAsset;
}


AssetHolder::AssetDescriptor&
AssetHolder::getAssetDescForURL(const string& url){
	unordered_map<string, AssetDescriptor>::iterator it = assets.begin();
	while( it != assets.end()){
		if(it->second.url == url){
			return it->second;
		}
		++it;
	}
	return emptyAsset;
}


string AssetHolder::toString(AssetStats &s){

	string msg = "NumAssets: " + ofToString(s.numAssets) +
	" NumMissingLocalFile: " + ofToString(s.numMissingFile) +
	" numSha1Missmatch: " + ofToString(s.numSha1Missmatch) +
	" NumFileTooSmall: " + ofToString(s.numFileTooSmall) +
	" NumOK: " + ofToString(s.numOK) +
	" NumDownloadFailed: " + ofToString(s.numDownloadFailed) +
	" NumNoSha1Supplied: " + ofToString(s.numNoSha1Supplied);
	return msg;
}


AssetHolder::AssetDescriptor&
AssetHolder::getAssetDescAtIndex(int i){

	if(i >= 0 && i < assetAddOrder.size()){
		return assets[assetAddOrder[i]];
	}
	return  emptyAsset;
}


int AssetHolder::getNumAssets(){
	return assets.size();
}

AssetHolder::AssetType
AssetHolder::typeFromExtension(const string& extension){

	AssetType type = TYPE_UNKNOWN;

	string lce = ofToLower(extension);
	if(lce == "jpg" || lce == "jpeg" || lce == "png" || lce == "gif" || lce == "tiff" ||
	   lce == "tga" || lce == "bmp"){
		type = IMAGE;
	}
	else if(lce == "mov" || lce == "mp4" || lce == "avi" || lce == "mpg" || lce == "mpeg" ||
			lce == "mkv" || lce == "vob" || lce == "qt" || lce == "wmv" || lce == "m4v" ||
			lce == "mp2" || lce == "m2v" || lce == "m4v"){
		type = VIDEO;
	}
	else if (lce == "aif" || lce == "aiff" || lce == "mp3" || lce == "wav" || lce == "mp4" ||
			 lce == "aac" || lce == "flac" || lce == "m4a" || lce == "wma"){
		type = AUDIO;
	}
	else if (lce == "json"){
		type = JSON;
	}
	else if (lce == "txt" || lce == "log"){
		type = TEXT;
	}
	else if (lce == "ttf" || lce == "otf"){
		type = FONT;
	}
	return type;
}


bool AssetHolder::shouldDownload(const AssetDescriptor &d){

	bool shouldDownload = false;
	//lets see if we should download this asset
	if(d.status.checked){
		if(d.status.localFileExists){
			if(!d.status.fileTooSmall){ //file big enough
				if(d.status.sha1Supplied){
					if(d.status.sha1Match){
						shouldDownload = downloadPolicy.fileExistsAndProvidedSha1Match;
					}else{
						shouldDownload = downloadPolicy.fileExistsAndProvidedSha1Missmatch;
					}
				}else{ //no sha1 supplied, but file exists and its not small
					shouldDownload = downloadPolicy.fileExistsAndNoSha1Provided;
				}
			}else{ //file too small
				shouldDownload = downloadPolicy.fileTooSmall;;
			}
		}else{
			shouldDownload = downloadPolicy.fileMissing;
		}
	}else{
		ofLogError("AssetHolder") << "cant decide wether to download or not - havent checked for local files yet!";
	}
	return shouldDownload;
}

bool AssetHolder::isReadyToUse(const AssetDescriptor &d){

	bool isReady = false;
	//lets see if we should download this asset
	if(d.status.checked){
		if(d.status.localFileExists){
			if(!d.status.fileTooSmall){ //file big enough
				if(d.status.sha1Supplied){
					if(d.status.sha1Match){
						isReady = assetOkPolicy.fileExistsAndProvidedSha1Match;
					}else{
						isReady = assetOkPolicy.fileExistsAndProvidedSha1Missmatch;
					}
				}else{ //no sha1 supplied, but file exists and its not small
					isReady = assetOkPolicy.fileExistsAndNoSha1Provided;
				}
			}else{ //file too small
				isReady = assetOkPolicy.fileTooSmall;
			}
		}else{
			isReady = assetOkPolicy.fileMissing;
		}
	}else{
		ofLogError("AssetHolder") << "cant decide wether to USE or not - havent checked for local files yet!";
	}
	return isReady;
}
