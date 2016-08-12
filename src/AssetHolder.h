//
//  AssetHolder.h
//  BaseApp
//
//  Created by Oriol Ferrer Mesià on 27/03/15.
//
//

#ifndef _AssetHolder_h
#define _AssetHolder_h

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

#define ASSET_HOLDER_SETUP_CHECK  if(!isSetup){ofLogError("Cant do! AssetHolder not setup!"); return "error!";}
const string assetLogFile = "logs/assetStatus.log";



//make your object subclass AssetHolder, to handle gathering of remote assets.
class AssetHolder{

public:



	AssetHolder();

	//tell me when to download things that exists locally and when not to
	void setup(const string& directoryForAssets,
			   ofxAssets::UsagePolicy assetOkPolicy,
			   ofxAssets::DownloadPolicy downloadPolicy);

	//for all assets in the app
	static void setMinimumFileSize(int numBytes){minimumFileSize = numBytes;}

	//use these to add assets easily, fill in most internal structure by just providing a few things
	//return a constructed "relativePath" which will be the acces key
	string addRemoteAsset(const string& url, const string& sha1, ofxAssets::Specs spec = ofxAssets::Specs());
	string addLocalAsset(const string& path, ofxAssets::Specs spec = ofxAssets::Specs()); //path relative to data!

	//totally custom - up to you to fill up the required structures
	void addAsset(const string& absoluteURL, const ofxAssets::Descriptor&);


	bool areAllAssetsOK(); //should we drop this object? if assets are wrong, yes!

	// Access //

	bool remoteAssetExistsInDB(const string& url);
	bool localAssetExistsInDB(const string& absPath);

	ofxAssets::Descriptor& getAssetDescForAbsPath(const string& path);
	ofxAssets::Descriptor& getAssetDescForURL(const string& url);

	int getNumAssets();
	ofxAssets::Descriptor& getAssetDescAtIndex(int i); //in add order, 0 will be the first asset you added
												//by calling addRemoteAsset or addLocalAsset

	ofxAssets::Stats getAssetStats();

	static string toString(ofxAssets::Stats &s);

	// Actions //

	void updateLocalAssetsStatus(); //call this to check local filesystem and decide what is missing / needed
	vector<string> downloadMissingAssets(ofxDownloadCentral& downloader); //return urls being downloaded

	//assets that need to be downloaded

	vector<ofxAssets::Descriptor> getMissingAssets();
	vector<ofxAssets::Descriptor> getAllAssetsInDB(); //not in add order! :(

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
													//2 assets cant have the same abs path!

	string directoryForAssets;
	bool isDownloadingData;
	bool isSetup;

	ofxAssets::UsagePolicy assetOkPolicy;
	ofxAssets::DownloadPolicy downloadPolicy;

	bool shouldDownload(const ofxAssets::Descriptor &d);
	bool isReadyToUse(const ofxAssets::Descriptor &d);

private:

	//meh
	static ofxAssets::Descriptor emptyAsset;
	static int minimumFileSize;
};

#endif