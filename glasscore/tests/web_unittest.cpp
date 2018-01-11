#include <gtest/gtest.h>

#include <string>
#include <memory>
#include <sstream>
#include <iostream>
#include <fstream>
#include "Node.h"
#include "Web.h"
#include "Site.h"
#include "SiteList.h"
#include "Logit.h"

#define TESTPATH "testdata"
#define STATIONFILENAME "teststationlist.json"
#define GLOBALFILENAME "testglobal.d"
#define GRIDFILENAME "testgrid.d"
#define GRIDEXPLICITFILENAME "testexplicitgrid.d"

#define NAME "TestWeb"
#define THRESH 1.4
#define NUMDETECT 5
#define NUMNUCLEATE 4
#define RESOLUTION 100.0
#define NUMROWS 3
#define NUMCOLS 4
#define NUMZ 1
#define UPDATE true
#define NOUPDATE false

#define GLOBALNAME "TestGlobal"
#define GLOBALTHRESH 2.5
#define GLOBALNUMDETECT 10
#define GLOBALNUMNUCLEATE 5
#define GLOBALRESOLUTION 250.0
#define GLOBALNUMZ 2
#define GLOBALNUMNODES 19410

#define GRIDNAME "TestGrid"
#define GRIDTHRESH 0.5
#define GRIDNUMDETECT 10
#define GRIDNUMNUCLEATE 6
#define GRIDRESOLUTION 25.0
#define GRIDNUMROWS 51
#define GRIDNUMCOLS 51
#define GRIDNUMZ 1
#define GRIDNUMNODES 2601

#define GRIDEXPLICITNAME "TestExplicitGrid"
#define GRIDEXPLICITTHRESH 0.5
#define GRIDEXPLICITNUMDETECT 14
#define GRIDEXPLICITNUMNUCLEATE 5
#define GRIDEXPLICITRESOLUTION 100.0
#define GRIDEXPLICITNUMNODES 12

#define PHASE1 "P"
#define PHASE2 "S"

// tests to see if the node can be constructed
TEST(WebTest, Construction) {
	glassutil::CLogit::disable();

	// default constructor
	glasscore::CWeb aWeb(NOUPDATE, 10, 10);

	// construct a web
	std::shared_ptr<traveltime::CTravelTime> nullTrav;
	glasscore::CWeb * testWeb = new glasscore::CWeb(std::string(NAME),
	THRESH,
													NUMDETECT,
													NUMNUCLEATE,
													RESOLUTION,
													NUMROWS,
													NUMCOLS, NUMZ,
													UPDATE,
													nullTrav, nullTrav);

	// name
	ASSERT_STREQ(std::string(NAME).c_str(), testWeb->sName.c_str())<<
	"Web sName Matches";

	// threshold
	ASSERT_EQ(THRESH, testWeb->dThresh)<< "Web dThresh Check";

	// nDetect
	ASSERT_EQ(NUMDETECT, testWeb->nDetect)<< "Web nDetect Check";

	// nNucleate
	ASSERT_EQ(NUMNUCLEATE, testWeb->nNucleate)<< "Web nNucleate Check";

	// resolution
	ASSERT_EQ(RESOLUTION, testWeb->dResolution)<< "Web resolution Check";

	// nRow
	ASSERT_EQ(NUMROWS, testWeb->nRow)<< "Web nRow Check";

	// nCol
	ASSERT_EQ(NUMCOLS, testWeb->nCol)<< "Web nCol Check";

	// nZ
	ASSERT_EQ(NUMZ, testWeb->nZ)<< "Web nZ Check";

	// bUpdate
	ASSERT_EQ(UPDATE, testWeb->bUpdate)<< "Web bUpdate Check";

	// lists
	int expectedSize = 0;
	ASSERT_EQ(expectedSize, (int)testWeb->vNode.size())<< "node list empty";
	ASSERT_EQ(expectedSize, (int)testWeb->vSitesFilter.size())<<
	"site filter list empty";
	ASSERT_EQ(expectedSize, (int)testWeb->vNetFilter.size())<<
	"net filter list empty";

	// pointers
	ASSERT_EQ(NULL, testWeb->pGlass)<< "pGlass null";

	// construct a web
	glasscore::CWeb * testWeb2 = new glasscore::CWeb(std::string(NAME),
	THRESH,
														NUMDETECT,
														NUMNUCLEATE,
														RESOLUTION,
														NUMROWS,
														NUMCOLS, NUMZ,
														NOUPDATE,
														nullTrav, nullTrav);

	// name
	ASSERT_STREQ(std::string(NAME).c_str(), testWeb2->sName.c_str())<<
	"Web sName Matches";

	// threshold
	ASSERT_EQ(THRESH, testWeb2->dThresh)<< "Web dThresh Check";

	// nDetect
	ASSERT_EQ(NUMDETECT, testWeb2->nDetect)<< "Web nDetect Check";

	// nNucleate
	ASSERT_EQ(NUMNUCLEATE, testWeb2->nNucleate)<< "Web nNucleate Check";

	// resolution
	ASSERT_EQ(RESOLUTION, testWeb2->dResolution)<< "Web resolution Check";

	// nRow
	ASSERT_EQ(NUMROWS, testWeb2->nRow)<< "Web nRow Check";

	// nCol
	ASSERT_EQ(NUMCOLS, testWeb2->nCol)<< "Web nCol Check";

	// nZ
	ASSERT_EQ(NUMZ, testWeb2->nZ)<< "Web nZ Check";

	// bUpdate
	ASSERT_EQ(NOUPDATE, testWeb2->bUpdate)<< "Web bUpdate Check";

	// lists
	ASSERT_EQ(expectedSize, (int)testWeb2->vNode.size())<< "node list empty";
	ASSERT_EQ(expectedSize, (int)testWeb2->vSitesFilter.size())<<
	"site filter list empty";
	ASSERT_EQ(expectedSize, (int)testWeb2->vNetFilter.size())<<
	"net filter list empty";

	// pointers
	ASSERT_EQ(NULL, testWeb2->pGlass)<< "pGlass null";

	delete (testWeb);
	delete (testWeb2);
}

// tests to see if the node can be initialized
TEST(WebTest, Initialize) {
	glassutil::CLogit::disable();

	// default constructor
	glasscore::CWeb aWeb(UPDATE, 10, 10);
	std::shared_ptr<traveltime::CTravelTime> nullTrav;

	aWeb.initialize(std::string(NAME),
	THRESH,
					NUMDETECT,
					NUMNUCLEATE,
					RESOLUTION, NUMROWS,
					NUMCOLS,
					NUMZ,
					UPDATE,
					nullTrav, nullTrav);
}

TEST(WebTest, GlobalTest) {
	glassutil::CLogit::disable();

	std::string phasename1 = std::string(PHASE1);
	std::string phasename2 = std::string(PHASE2);

	// load files
	// stationlist
	std::ifstream stationFile;
	stationFile.open(
			"./" + std::string(TESTPATH) + "/" + std::string(STATIONFILENAME),
			std::ios::in);
	std::string stationLine = "";
	std::getline(stationFile, stationLine);
	stationFile.close();

	// global config
	std::ifstream globalFile;
	globalFile.open(
			"./" + std::string(TESTPATH) + "/" + std::string(GLOBALFILENAME),
			std::ios::in);
	std::string globalLine = "";
	std::getline(globalFile, globalLine);
	globalFile.close();

	json::Object siteList = json::Deserialize(stationLine);
	json::Object globalConfig = json::Deserialize(globalLine);

	// construct a sitelist
	glasscore::CSiteList * testSiteList = new glasscore::CSiteList();
	testSiteList->dispatch(&siteList);

	// construct a web
	glasscore::CWeb testGlobalWeb(UPDATE);
	testGlobalWeb.pSiteList = testSiteList;
	testGlobalWeb.dispatch(&globalConfig);

	// name
	ASSERT_STREQ(std::string(GLOBALNAME).c_str(), testGlobalWeb.sName.c_str())<<
	"Web sName Matches";

	// threshold
	ASSERT_EQ(GLOBALTHRESH, testGlobalWeb.dThresh)<< "Web dThresh Check";

	// nDetect
	ASSERT_EQ(GLOBALNUMDETECT, testGlobalWeb.nDetect)<< "Web nDetect Check";

	// nNucleate
	ASSERT_EQ(GLOBALNUMNUCLEATE, testGlobalWeb.nNucleate)<< "Web nNucleate Check";

	// dResolution
	ASSERT_EQ(GLOBALRESOLUTION, testGlobalWeb.dResolution)<< "Web dResolution "
	"Check";

	// nRow
	ASSERT_EQ(0, testGlobalWeb.nRow)<< "Web nRow Check";

	// nCol
	ASSERT_EQ(0, testGlobalWeb.nCol)<< "Web nCol Check";

	// nCol
	ASSERT_EQ(GLOBALNUMZ, testGlobalWeb.nZ)<< "Web nZ Check";

	// bUpdate
	ASSERT_EQ(UPDATE, testGlobalWeb.bUpdate)<< "Web bUpdate Check";

	// lists
	ASSERT_EQ(GLOBALNUMNODES, (int)testGlobalWeb.vNode.size())<< "node list";
	ASSERT_EQ(0, (int)testGlobalWeb.vSitesFilter.size())<<
	"site filter list empty";
	ASSERT_EQ(0, (int)testGlobalWeb.vNetFilter.size())<<
	"net filter list empty";

	// pointers
	ASSERT_EQ(NULL, testGlobalWeb.pGlass)<< "pGlass null";
	ASSERT_TRUE(NULL != testGlobalWeb.pTrv1)<< "pTrv1 not null";
	ASSERT_TRUE(NULL != testGlobalWeb.pTrv2)<< "pTrv2 not null";

	// phase name
	ASSERT_STREQ(testGlobalWeb.pTrv1->sPhase.c_str(), phasename1.c_str());

	// phase name
	ASSERT_STREQ(testGlobalWeb.pTrv2->sPhase.c_str(), phasename2.c_str());

	// cleanup
	delete (testSiteList);
}

TEST(WebTest, GridTest) {
	glassutil::CLogit::disable();

	std::string phasename1 = std::string(PHASE1);
	std::string phasename2 = std::string(PHASE2);

	// load files
	// stationlist
	std::ifstream stationFile;
	stationFile.open(
			"./" + std::string(TESTPATH) + "/" + std::string(STATIONFILENAME),
			std::ios::in);
	std::string stationLine = "";
	std::getline(stationFile, stationLine);
	stationFile.close();

	// grid config
	std::ifstream gridFile;
	gridFile.open(
			"./" + std::string(TESTPATH) + "/" + std::string(GRIDFILENAME),
			std::ios::in);
	std::string gridLine = "";
	std::getline(gridFile, gridLine);
	gridFile.close();

	json::Object siteList = json::Deserialize(stationLine);
	json::Object gridConfig = json::Deserialize(gridLine);

	// construct a sitelist
	glasscore::CSiteList * testSiteList = new glasscore::CSiteList();
	testSiteList->dispatch(&siteList);

	// construct a web
	glasscore::CWeb testGridWeb(UPDATE);
	testGridWeb.pSiteList = testSiteList;
	testGridWeb.dispatch(&gridConfig);

	// name
	ASSERT_STREQ(std::string(GRIDNAME).c_str(), testGridWeb.sName.c_str())<<
	"Web sName Matches";

	// threshold
	ASSERT_EQ(GRIDTHRESH, testGridWeb.dThresh)<< "Web dThresh Check";

	// nDetect
	ASSERT_EQ(GRIDNUMDETECT, testGridWeb.nDetect)<< "Web nDetect Check";

	// nNucleate
	ASSERT_EQ(GRIDNUMNUCLEATE, testGridWeb.nNucleate)<< "Web nNucleate Check";

	// dResolution
	ASSERT_EQ(GRIDRESOLUTION, testGridWeb.dResolution)<< "Web dResolution "
	"Check";

	// nRow
	ASSERT_EQ(GRIDNUMROWS, testGridWeb.nRow)<< "Web nRow Check";

	// nCol
	ASSERT_EQ(GRIDNUMROWS, testGridWeb.nCol)<< "Web nCol Check";

	// nCol
	ASSERT_EQ(GRIDNUMZ, testGridWeb.nZ)<< "Web nZ Check";

	// bUpdate
	ASSERT_EQ(UPDATE, testGridWeb.bUpdate)<< "Web bUpdate Check";

	// lists
	ASSERT_EQ(GRIDNUMNODES, (int)testGridWeb.vNode.size())<< "node list";
	ASSERT_EQ(0, (int)testGridWeb.vSitesFilter.size())<<
	"site filter list empty";
	ASSERT_EQ(0, (int)testGridWeb.vNetFilter.size())<<
	"net filter list empty";

	// pointers
	ASSERT_EQ(NULL, testGridWeb.pGlass)<< "pGlass null";
	ASSERT_TRUE(NULL != testGridWeb.pTrv1)<< "pTrv1 not null";
	ASSERT_TRUE(NULL != testGridWeb.pTrv2)<< "pTrv2 not null";

	// phase name
	ASSERT_STREQ(testGridWeb.pTrv1->sPhase.c_str(), phasename1.c_str());

	// phase name
	ASSERT_STREQ(testGridWeb.pTrv2->sPhase.c_str(), phasename2.c_str());

	// cleanup
	delete (testSiteList);
}

TEST(WebTest, GridExplicitTest) {
	glassutil::CLogit::disable();

	std::string phasename1 = std::string(PHASE1);

	// load files
	// stationlist
	std::ifstream stationFile;
	stationFile.open(
			"./" + std::string(TESTPATH) + "/" + std::string(STATIONFILENAME),
			std::ios::in);
	std::string stationLine = "";
	std::getline(stationFile, stationLine);
	stationFile.close();

	// grid config
	std::ifstream gridFile;
	gridFile.open(
			"./" + std::string(TESTPATH) + "/"
					+ std::string(GRIDEXPLICITFILENAME),
			std::ios::in);
	std::string gridLine = "";
	std::getline(gridFile, gridLine);
	gridFile.close();

	json::Object siteList = json::Deserialize(stationLine);
	json::Object gridConfig = json::Deserialize(gridLine);

	// construct a sitelist
	glasscore::CSiteList * testSiteList = new glasscore::CSiteList();
	testSiteList->dispatch(&siteList);

	// construct a web
	glasscore::CWeb testGridWeb(UPDATE);
	testGridWeb.pSiteList = testSiteList;
	testGridWeb.dispatch(&gridConfig);

	// name
	ASSERT_STREQ(std::string(GRIDEXPLICITNAME).c_str(),
			testGridWeb.sName.c_str())<< "Web sName Matches";

	// threshold
	ASSERT_EQ(GRIDEXPLICITTHRESH,
			  testGridWeb.dThresh)<< "Web dThresh Check";

	// nDetect
	ASSERT_EQ(GRIDEXPLICITNUMDETECT,
			  testGridWeb.nDetect)<< "Web nDetect Check";

	// nNucleate
	ASSERT_EQ(GRIDEXPLICITNUMNUCLEATE,
			  testGridWeb.nNucleate)<< "Web nNucleate Check";

	// dResolution
	ASSERT_EQ(GRIDEXPLICITRESOLUTION,
			  testGridWeb.dResolution)<< "Web dResolution Check";

	// nRow
	ASSERT_EQ(0, testGridWeb.nRow)<< "Web nRow Check";

	// nCol
	ASSERT_EQ(0, testGridWeb.nCol)<< "Web nCol Check";

	// nCol
	ASSERT_EQ(0, testGridWeb.nZ)<< "Web nZ Check";

	// bUpdate
	ASSERT_EQ(NOUPDATE, testGridWeb.bUpdate)<< "Web bUpdate Check";

	// lists
	ASSERT_EQ(GRIDEXPLICITNUMNODES, (int)testGridWeb.vNode.size())<< "node list";
	ASSERT_EQ(0, (int)testGridWeb.vSitesFilter.size())<<
	"site filter list empty";
	ASSERT_EQ(0, (int)testGridWeb.vNetFilter.size())<<
	"net filter list empty";

	// pointers
	ASSERT_EQ(NULL, testGridWeb.pGlass)<< "pGlass null";
	ASSERT_TRUE(NULL != testGridWeb.pTrv1)<< "pTrv1 not null";
	ASSERT_TRUE(NULL == testGridWeb.pTrv2)<< "pTrv2 null";

	// phase name
	ASSERT_STREQ(testGridWeb.pTrv1->sPhase.c_str(), phasename1.c_str());

	// cleanup
	delete (testSiteList);
}
