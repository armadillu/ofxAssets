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
	//return a constructed "relativePath" which will be the acces key
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
	AssetDescriptor& getAssetDescAtIndex(int i); //in add order, 0 will be the first asset you added
												//by calling addRemoteAsset or addLocalAsset

	AssetStats getAssetStats();

	static string toString(AssetStats &s);

	// Actions //

	void updateLocalAssetsStatus(); //call this to check local filesystem and decide what is missing / needed
	vector<string> downloadMissingAssets(ofxDownloadCentral& downloader); //return urls being downloaded

	//assets that need to be downloaded

	vector<AssetDescriptor> getMissingAssets();
	vector<AssetDescriptor> getAllAssetsInDB(); //not in add order! :(

	//its up to you to fill in data structures? TODO!
	//unordered_map<string, AssetDescriptor>& getAssetsMap(){return assets;}

	// CALLBACK //
	void downloadsFinished(ofxBatchDownloaderReport & report);

protected:

	AssetType typeFromExtension(const string& extension);
	void checkLocalAssetStatus(AssetDescriptor & d);

	//the actual assets
	map<int, string> assetAddOrder;
	unordered_map<string, AssetDescriptor> assets; 	//index by relativePath
													//2 assets cant have the same abs path!

	string directoryForAssets;
	bool isDownloadingData;
	bool isSetup;

	AssetUsagePolicy assetOkPolicy;
	AssetDownloadPolicy downloadPolicy;

	bool shouldDownload(const AssetDescriptor &d);
	bool isReadyToUse(const AssetDescriptor &d);

private:

	//meh
	static AssetDescriptor emptyAsset;
	static int minimumFileSize;
};

#endif