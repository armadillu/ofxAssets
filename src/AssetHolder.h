//
//  AssetHolder.h
//  BaseApp
//
//  Created by Oriol Ferrer Mesià on 27/03/15.
//
//

#ifndef _AssetHolder_h
#define _AssetHolder_h

#include "ofMain.h"
#include "ofxDownloadCentral.h"

#define ASSET_HOLDER_SETUP_CHECK  if(!isSetup){ofLogError("Cant do! AssetHolder not setup!"); return "error!";}
const string assetLogFile = "logs/assetStatus.log";





//make your object subclass AssetHolder, to handle gathering of remote assets.
class AssetHolder{

public:

	#include "AssetHolderStructs.h" //hmm this is a bit unconventional... does it work? Seems to!

	AssetHolder();

	//tell me when to download things that exists locally and when not to
	void setup(const string& directoryForAssets, AssetUsagePolicy assetOkPolicy, AssetDownloadPolicy downloadPolicy);

	//for all assets in the app
	static void setMinimumFileSize(int numBytes){minimumFileSize = numBytes;}

	//use these to add assets easily, fill in most internal structure by just providing a few things
	//return a constructed "absolutePath" which will be the acces key
	string addRemoteAsset(const string& url, const string& sha1, AssetSpecs spec = AssetSpecs());
	string addLocalAsset(const string& path, AssetSpecs spec = AssetSpecs()); //path relative to data!

	//totally custom - up to you to fill up the required structures
	void addAsset(const string& absoluteURL, const AssetDescriptor&);


	bool areAllAssetsOK(); //should we drop this object? if assets are wrong, yes!

	// Access //

	bool remoteAssetExistsInDB(const string& url);
	bool localAssetExistsInDB(const string& absPath);

	AssetDescriptor& getAssetDescForAbsPath(const string& path);
	AssetDescriptor& getAssetDescForURL(const string& url);

	int getNumAssets();
	AssetDescriptor& getAssetDescAtIndex(int i);

	AssetStats getAssetStats();


	static string toString(AssetStats &s);

	// Actions //

	void updateLocalAssetsStatus(); //call this to check local filesystem and decide what is missing / needed
	vector<string> downloadMissingAssets(ofxDownloadCentral& downloader); //return urls being downloaded

	//assets that need to be downloaded

	vector<AssetDescriptor> getMissingAssets();
	vector<AssetDescriptor> getAllAssetsInDB();

	// CALLBACK //
	void downloadsFinished(ofxBatchDownloaderReport & report);

private:

	AssetType typeFromExtension(const string& extension);
	void checkLocalAssetStatus(AssetDescriptor & d);

	//the actual assets
	map<string, AssetDescriptor> assets; 	//index by absolutePath
											//2 assets cant have the same abs path!

	//policies
	AssetUsagePolicy localAssetOkPolicy;
	AssetDownloadPolicy assetShouldBeDownloadedPolicy;

	//meh
	static AssetDescriptor emptyAsset;
	static int minimumFileSize;

	string directoryForAssets;
	bool isDownloadingData;
	bool isSetup;

	AssetUsagePolicy assetOkPolicy;
	AssetDownloadPolicy downloadPolicy;

	bool shouldDownload(const AssetDescriptor &d);
	bool isReadyToUse(const AssetDescriptor &d);
};

#endif