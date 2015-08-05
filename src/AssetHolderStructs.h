//
//  AssetHolderStructs.h
//  BaseApp
//
//  Created by Oriol Ferrer Mesià on 27/03/15.
//
//

#ifndef BaseApp_AssetHolderStructs_h
#define BaseApp_AssetHolderStructs_h

#include "ofMain.h"

//make your object subclass AssetHolder, to handle gathering of remote assets.

enum AssetLocation{
	REMOTE,
	LOCAL,
	UNKNOWN_LOCATION
};

enum AssetType{
	VIDEO,
	AUDIO,
	IMAGE,
	JSON,
	TEXT,
	FONT,
	OTHER,
	TYPE_UNKNOWN
};

enum AssetState{ //hmmm
	NOT_CHECKED,
	LOCAL_FILE_EXISTS_OK,
	LOCAL_FILE_EXISTS_BAD_SHA1,
	LOCAL_FILE_EXISTS_NO_SHA1,
	LOCAL_FILE_MISSING		
};

struct AssetPolicy{
	bool fileMissing;
	bool fileExistsAndNoSha1Provided;
	bool fileExistsAndProvidedSha1Missmatch;
	bool fileExistsAndProvidedSha1Match;
	bool fileTooSmall; //regardless of sha1
};

struct AssetUsagePolicy: public AssetPolicy{
	//asset should be Used if...
	AssetUsagePolicy(){
		fileMissing = false;
		fileTooSmall = false;
		fileExistsAndNoSha1Provided = true;
		fileExistsAndProvidedSha1Missmatch = false;
		fileExistsAndProvidedSha1Match = true;
	}
};

struct AssetDownloadPolicy : public AssetPolicy{
	//asset should be downloaded if...
	AssetDownloadPolicy(){
		fileMissing = true;
		fileTooSmall = true;
		fileExistsAndNoSha1Provided = true;
		fileExistsAndProvidedSha1Missmatch = true;
		fileExistsAndProvidedSha1Match = false;
	}
};

struct AssetStats{
	int numAssets;
	int numMissingFile;
	int numSha1Missmatch;
	int numFileTooSmall;
	int numOK;
	int numDownloadFailed;
	int numNoSha1Supplied;
	AssetStats(){
		numAssets = numMissingFile = numSha1Missmatch = numOK = 0;
		numDownloadFailed = numFileTooSmall = numNoSha1Supplied = 0;
	}
};

struct AssetSpecs{
	string codec; //for movies / audio
	int width;
	int height;
	AssetSpecs(){
		width = height = 0;
	}
};

struct AssetUserInfo{
	string title;
	bool hasSubtitles;
	string description;
	string size; //ie _o for original, _b for big etc
	string ID;
	map<string, string> extra;
};

struct LocalAssetStatus{
	bool localFileExists;
	bool sha1Supplied;
	bool localFileSha1Checked;
	bool sha1Match;
	bool fileTooSmall;

	bool checked; //if checkLocalAssetStatus() was run for that asset
	bool downloaded; 
	bool downloadOK;

	LocalAssetStatus(){
		localFileSha1Checked = localFileExists = sha1Match = false;
		downloaded = downloadOK = fileTooSmall = sha1Supplied = checked = false;
	}
};

struct AssetDescriptor{

	string fileName;
	string extension;
	string relativePath; //this is the "unique key" of all assets, must be unique per asset

	string url;
	string sha1;

	AssetType type;
	AssetLocation location;
	AssetUserInfo userInfo;
	AssetSpecs specs;
	LocalAssetStatus status;

	AssetDescriptor(){
		type = TYPE_UNKNOWN;
		location = UNKNOWN_LOCATION;
	}

	bool hasSha1(){return sha1.size() > 0;}
};

#endif