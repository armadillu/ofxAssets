//
//  AssetHolderStructs.h
//  BaseApp
//
//  Created by Oriol Ferrer Mesià on 27/03/15.
//
//

#pragma once

#include "ofMain.h"

//make your object subclass AssetHolder, to handle gathering of remote assets.
namespace ofxAssets{

	enum Location{
		REMOTE,
		LOCAL,
		UNKNOWN_LOCATION
	};

	enum Type{
		VIDEO,
		AUDIO,
		IMAGE,
		JSON,
		TEXT,
		FONT,
		OTHER,
		TYPE_UNKNOWN
	};

	enum State{ //hmmm
		NOT_CHECKED,
		LOCAL_FILE_EXISTS_OK,
		LOCAL_FILE_EXISTS_BAD_SHA1,
		LOCAL_FILE_EXISTS_NO_SHA1,
		LOCAL_FILE_MISSING		
	};

	struct Policy{
		bool fileMissing;
		bool fileExistsAndNoSha1Provided;
		bool fileExistsAndProvidedSha1Missmatch;
		bool fileExistsAndProvidedSha1Match;
		bool fileTooSmall; //regardless of sha1
	};

	struct UsagePolicy: public Policy{
		//asset should be Used if...
		UsagePolicy(){
			fileMissing = false;
			fileTooSmall = false;
			fileExistsAndNoSha1Provided = true;
			fileExistsAndProvidedSha1Missmatch = false;
			fileExistsAndProvidedSha1Match = true;
		}
	};

	struct DownloadPolicy : public Policy{
		//asset should be downloaded if...
		DownloadPolicy(){
			fileMissing = true;
			fileTooSmall = true;
			fileExistsAndNoSha1Provided = true;
			fileExistsAndProvidedSha1Missmatch = true;
			fileExistsAndProvidedSha1Match = false;
		}
	};

	struct Stats{
		int numAssets;
		int numMissingFile;
		int numSha1Missmatch;
		int numFileTooSmall;
		int numOK;
		int numDownloadFailed;
		int numNoSha1Supplied;
		Stats(){
			numAssets = numMissingFile = numSha1Missmatch = numOK = 0;
			numDownloadFailed = numFileTooSmall = numNoSha1Supplied = 0;
		}
	};

	struct Specs{
		string codec; //for movies / audio
		int width;
		int height;
		Specs(){
			width = height = 0;
		}
	};

	struct UserInfo{
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

	struct Descriptor{

		string fileName;
		string extension;
		string relativePath; //this is the "unique key" of all assets, must be unique per asset

		string url;
		string sha1;

		Type type;
		Location location;
		UserInfo userInfo;
		Specs specs;
		LocalAssetStatus status;

		Descriptor(){
			type = TYPE_UNKNOWN;
			location = UNKNOWN_LOCATION;
		}

		bool hasSha1(){return sha1.size() > 0;}
	};
}
