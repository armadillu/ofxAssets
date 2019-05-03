//
//  AssetHolderStructs.h
//  BaseApp
//
//  Created by Oriol Ferrer Mesià on 27/03/15.
//
//

#pragma once

#include "ofMain.h"
#include "ofxChecksum.h"

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

	struct Policy{
		bool fileMissing;
		bool fileExistsAndNoChecksumProvided;
		bool fileExistsAndProvidedChecksumMissmatch;
		bool fileExistsAndProvidedChecksumMatch;
		bool fileTooSmall; //regardless of sha1
	};

	struct UsagePolicy: public Policy{ //asset should be Used if...
		UsagePolicy(){
			fileMissing = false;
			fileTooSmall = false;
			fileExistsAndNoChecksumProvided = true;
			fileExistsAndProvidedChecksumMissmatch = false;
			fileExistsAndProvidedChecksumMatch = true;
		}
	};

	struct DownloadPolicy : public Policy{ //asset should be downloaded if...
		DownloadPolicy(){
			fileMissing = true;
			fileTooSmall = true;
			fileExistsAndNoChecksumProvided = true;
			fileExistsAndProvidedChecksumMissmatch = true;
			fileExistsAndProvidedChecksumMatch = false;
		}
	};

	struct ObjectUsagePolicy{ //an AssetHolder obj should only be used as live content if...
		bool allObjectAssetsAreOK;
		int minNumberOfImageAssets; //object will be dropped unless it has N or more valid assets (ie drop objects with 0 assets)
		int minNumberOfVideoAssets;
		int minNumberOfAudioAssets;

		ObjectUsagePolicy(){
			allObjectAssetsAreOK = true;
			minNumberOfImageAssets = 1;
			minNumberOfVideoAssets = 0;
			minNumberOfAudioAssets = 0;
		}
	};

	struct Stats{
		int numAssets;
		int numMissingFile;
		int numChecksumMissmatch;
		int numFileTooSmall;
		int numOK;
		int numDownloadFailed;
		int numNoChecksumSupplied;
		Stats(){
			numAssets = numMissingFile = numChecksumMissmatch = numOK = 0;
			numDownloadFailed = numFileTooSmall = numNoChecksumSupplied = 0;
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
		bool checksumSupplied;
		bool localFileChecksumChecked;
		bool checksumMatch;
		bool fileTooSmall;

		bool checked; //if checkLocalAssetStatus() was run for that asset
		bool downloaded; 
		bool downloadOK;

		LocalAssetStatus(){
			localFileChecksumChecked = localFileExists = checksumMatch = false;
			downloaded = downloadOK = fileTooSmall = checksumSupplied = checked = false;
		}
	};

	struct Descriptor{

		string fileName;
		string extension;
		string relativePath; //this is the "unique key" of all assets, must be unique per asset

		string url;

		string checksum;
		ofxChecksum::Type checksumType;

		Type type;
		Location location;
		UserInfo userInfo;
		Specs specs;
		LocalAssetStatus status;

		Descriptor(){
			type = TYPE_UNKNOWN;
			location = UNKNOWN_LOCATION;
		}

		bool hasChecksum(){return checksum.size() > 0;}
	};
}
