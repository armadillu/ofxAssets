//
//  AssetHolder.h
//  BaseApp
//
//  Created by Oriol Ferrer Mesià on 27/03/15.
//
//

#pragma once 

#if __cplusplus>=201103L || defined(_MSC_VER)
#include <unordered_map>
#include <memory>
#else
#include <tr1/unordered_map>
using std::tr1::unordered_map;
#endif

#include "ofMain.h"
#include "ofxDownloadCentral.h"
#include "AssetHolderStructs.h"
#include "TagManager.h"
#include "ofxChecksum.h"


#define ASSET_HOLDER_SETUP_CHECK  if(!isSetup){ofLogError("Cant do! AssetHolder not setup!"); return "error!";}

//make your object subclass AssetHolder, to handle gathering of remote assets.
class AssetHolder{

	const string assetLogFile = "logs/assetStatus.log";

public:

	AssetHolder();

	//tell me when to download things that exists locally and when not to
	void setup(const string& directoryForAssets,
			   const ofxAssets::UsagePolicy & assetOkPolicy,
			   const ofxAssets::DownloadPolicy & downloadPolicy);

	//for all assets in the app
	static void setMinimumFileSize(int numBytes){minimumFileSize = numBytes;}

	//use these to add assets easily, fill in most internal structure by just providing a few things
	//return a constructed "relativePath" which will be the acces key
	//when you add an asset, ofxAsset will try its best to tag it according to file extension
	//you can "tag" each asset to get it back later (ie "primaryImage", "sizeLarge", "sizeSmall")
	string addRemoteAsset(const string& url,
						  const string& checksum, //sha1, xxhash, etc
						  const ofxChecksum::Type checksumType, //type of checksum supplied above
						  const vector<string>& tags = vector<string>(),
						  ofxAssets::Specs spec = ofxAssets::Specs(),
						  ofxAssets::Type type = ofxAssets::TYPE_UNKNOWN
						  );

	string addLocalAsset(const string& path,
						 const vector<string>& tags = vector<string>(),
						 ofxAssets::Specs spec = ofxAssets::Specs(),
						 ofxAssets::Type type = ofxAssets::TYPE_UNKNOWN
						 ); //path relative to data!

	//totally custom - up to you to fill up the required structures - u should know what you are doing if you use this
	void addAsset(const string& absoluteURL, const ofxAssets::Descriptor&);

	bool areAllAssetsOK(); //should we drop this object? if assets are wrong, yes!
	vector<ofxAssets::Descriptor> getBrokenAssets();

	// Access ...		//
	bool remoteAssetExistsInDB(const string& url);
	bool localAssetExistsInDB(const string& absPath);

		// ... by key (relative path)
	ofxAssets::Descriptor& getAssetDescForPath(const string& path); //should be a relative path to data
	ofxAssets::Descriptor& getAssetDescForURL(const string& url);
	vector<ofxAssets::Descriptor> getAssetDescriptorsForType(ofxAssets::Type);

		// ... by Index
	int getNumAssets();
	ofxAssets::Descriptor& getAssetDescAtIndex(int i);	//in add order, 0 will be the first asset you added
														//by calling addRemoteAsset or addLocalAsset

	// User info // >> this is mostly a reminder that you can add custom info to your assets
	ofxAssets::UserInfo & getUserInfoForPath(const string& path);

	// Tags //
	void addTagsforAsset(const string & relPath, vector<string> tags);
	vector<ofxAssets::Descriptor> getAssetDescsWithTag(const string & tag);

	// Stats //
	ofxAssets::Stats getAssetStats();
	static string toString(ofxAssets::Stats &s);

	// Actions //
	void updateLocalAssetsStatus(); //call this to check local filesystem and decide what is missing / needed
	vector<string> downloadMissingAssets(ofxDownloadCentral& downloader); //return urls being downloaded

	//assets that need to be downloaded
	vector<ofxAssets::Descriptor> getMissingAssets();
	vector<ofxAssets::Descriptor> getAllAssetsInDB();

	//its up to you to fill in data structures? TODO!
	//unordered_map<string, AssetDescriptor>& getAssetsMap(){return assets;}

	// CALLBACK //
	void downloadsFinished(ofxBatchDownloaderReport & report);

protected:

	ofxAssets::Type typeFromExtension(const string& extension);
	void checkLocalAssetStatus(ofxAssets::Descriptor & d);

	//the actual assets
	map<int, string> assetAddOrder;
	unordered_map<string, ofxAssets::Descriptor> assets; 	//index by relativePath
															//2 assets cant have the same path!

	string directoryForAssets;
	bool isDownloadingData;
	bool isSetup;

	ofxAssets::UsagePolicy assetOkPolicy;
	ofxAssets::DownloadPolicy downloadPolicy;

	bool shouldDownload(const ofxAssets::Descriptor &d);
	bool isReadyToUse(const ofxAssets::Descriptor &d);

	enum TagCategory{
		CATEGORY
	};

	TagManager<TagCategory> tags = TagManager<TagCategory>(1); //only one category - forcing with our custom enum

private:

	//meh
	static ofxAssets::Descriptor emptyAsset;
	static ofxAssets::UserInfo emptyUserInfo;
	static int minimumFileSize;
	static ofMutex assetMutex;
	
};
