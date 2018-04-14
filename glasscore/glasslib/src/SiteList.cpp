#include <json.h>
#include <cmath>
#include <string>
#include <memory>
#include <mutex>
#include <vector>
#include "Glass.h"
#include "Site.h"
#include "WebList.h"
#include "SiteList.h"
#include "Node.h"
#include "Web.h"
#include "Logit.h"

namespace glasscore {

// ---------------------------------------------------------CSiteList
CSiteList::CSiteList() {
	clear();
}

// ---------------------------------------------------------~CSiteList
CSiteList::~CSiteList() {
	clear();
}

// ---------------------------------------------------------clear
void CSiteList::clear() {
	std::lock_guard<std::recursive_mutex> siteListGuard(m_SiteListMutex);
	pGlass = NULL;

	// clear sites
	clearSites();
}

// ---------------------------------------------------------clearSites
void CSiteList::clearSites() {
	std::lock_guard<std::mutex> guard(vSiteMutex);
	// remove all picks from sites
	for (auto site : vSite) {
		site->clearVPick();
	}

	// clear the vector and map
	vSite.clear();
	mSite.clear();
	mLookup.clear();
}

// ---------------------------------------------------------dispatch
bool CSiteList::dispatch(std::shared_ptr<json::Object> com) {
	// null check json
	if (com == NULL) {
		glassutil::CLogit::log(
				glassutil::log_level::error,
				"CSiteList::dispatch: NULL json communication.");
		return (false);
	}

	// check for a command
	if (com->HasKey("Cmd")
			&& ((*com)["Cmd"].GetType() == json::ValueType::StringVal)) {
		// dispatch to appropriate function based on Cmd value
		json::Value v = (*com)["Cmd"].ToString();

		// clear all sites
		if (v == "ClearGlass") {
			clearSites();

			// ClearGlass is also relevant to other glass
			// components, return false so they also get a
			// chance to process it
			return (false);
		}

		// get the current site list
		if (v == "ReqSiteList") {
			return (reqSiteList());
		}
	}

	// Input data can have Type keys
	if (com->HasKey("Type")
			&& ((*com)["Type"].GetType() == json::ValueType::StringVal)) {
		// dispatch to appropriate function based on Cmd value
		json::Value v = (*com)["Type"].ToString();

		// add or update a site
		if (v == "StationInfo") {
			return (addSite(com));
		}
		// add a list of sites
		if (v == "StationInfoList") {
			return (addSiteList(com));
		}
	}

	// this communication was not handled
	return (false);
}

// ---------------------------------------------------------addSite
bool CSiteList::addSite(std::shared_ptr<json::Object> com) {
	// null check json
	if (com == NULL) {
		glassutil::CLogit::log(glassutil::log_level::error,
								"CSiteList::addSite: NULL json configuration.");
		return (false);
	}

	// check type
	if (com->HasKey("Type")
			&& ((*com)["Type"].GetType() == json::ValueType::StringVal)) {
		std::string type = (*com)["Type"].ToString();

		if (type != "StationInfo") {
			glassutil::CLogit::log(
					glassutil::log_level::warn,
					"CSiteList::addSite: Non-StationInfo message passed in.");
			return (false);
		}
	} else {
		glassutil::CLogit::log(
				glassutil::log_level::error,
				"CSiteList::addSite: Missing required Cmd Key.");
	}

	// create a new a site from the json message;
	CSite * site = new CSite(com, pGlass);

	// make sure a site was actually created
	if (site->getScnl() == "") {
		glassutil::CLogit::log(glassutil::log_level::warn,
								"CSiteList::addSite: Site not created.");
		delete (site);
		return (false);
	}

	// create new shared pointer to the site
	std::shared_ptr<CSite> newSite(site);

	return (addSite(newSite));
}

// ---------------------------------------------------------addSiteList
bool CSiteList::addSiteList(std::shared_ptr<json::Object> com) {
	// null check json
	if (com == NULL) {
		glassutil::CLogit::log(
				glassutil::log_level::error,
				"CSiteList::addSiteList: NULL json configuration.");
		return (false);
	}

	// check type
	if (com->HasKey("Type")
			&& ((*com)["Type"].GetType() == json::ValueType::StringVal)) {
		std::string type = (*com)["Type"].ToString();

		if (type != "StationInfoList") {
			glassutil::CLogit::log(
					glassutil::log_level::warn,
					"CSiteList::addSite: Non-StationInfoList message passed"
					" in.");
			return (false);
		}
	} else {
		glassutil::CLogit::log(
				glassutil::log_level::error,
				"CSiteList::addSite: Missing required Type Key.");
	}

	// get the list
	if (((*com).HasKey("StationList"))
			&& ((*com)["StationList"].GetType() == json::ValueType::ArrayVal)) {
		json::Array stationList = (*com)["StationList"].ToArray();
		// for each station in the list
		for (auto v : stationList) {
			if (v.GetType() == json::ValueType::ObjectVal) {
				// convert object to json pointer
				std::shared_ptr<json::Object> siteObj = std::make_shared<
						json::Object>(v.ToObject());

				// create a new a site from the station json;
				CSite * site = new CSite(siteObj, pGlass);

				// make sure a site was actually created
				if (site->getScnl() == "") {
					glassutil::CLogit::log(
							glassutil::log_level::warn,
							"CSiteList::addSiteList: Site not created.");
					delete (site);
					continue;
				}

				// create new shared pointer to the site
				std::shared_ptr<CSite> newSite(site);

				if (addSite(newSite) == false) {
					glassutil::CLogit::log(
							glassutil::log_level::warn,
							"CSiteList::addSiteList: Site not added.");
					delete (site);
					continue;
				}
			}
		}
	}

	return (true);
}

// ---------------------------------------------------------addSite
bool CSiteList::addSite(std::shared_ptr<CSite> site) {
	// null check
	if (site == NULL) {
		glassutil::CLogit::log(glassutil::log_level::error,
								"CSiteList::addSite: NULL CSite provided.");
		return (false);
	}

	// check to see if we have an existing site
	std::shared_ptr<CSite> oldSite = getSite(site->getScnl());

	std::lock_guard<std::mutex> guard(vSiteMutex);

	// check if we already had this site
	if (oldSite) {
		// update existing site
		oldSite->update(site.get());

		// pass updated site to webs
		if (pGlass) {
			if (pGlass->getWebList()) {
				pGlass->getWebList()->addSite(oldSite);
			}
		}
	} else {
		// add new site to list and map
		vSite.push_back(site);
		mSite[site->getScnl()] = site;

		// pass new site to webs
		if (pGlass) {
			if (pGlass->getWebList()) {
				pGlass->getWebList()->addSite(site);
			}
		}
	}

	return (true);
}

// ---------------------------------------------------------getSiteCount
int CSiteList::getSiteCount() {
	std::lock_guard<std::mutex> guard(vSiteMutex);
	int size = static_cast<int>(vSite.size());
	// Return number of sites in site list (for iteration)
	return (size);
}

// ---------------------------------------------------------getSite
std::shared_ptr<CSite> CSiteList::getSite(int ix) {
	std::lock_guard<std::mutex> guard(vSiteMutex);
	// Return shared pointer to site from vector given index
	return (vSite[ix]);
}

// ---------------------------------------------------------getSite
std::shared_ptr<CSite> CSiteList::getSite(std::string scnl) {
	if (scnl == "") {
		glassutil::CLogit::log(glassutil::log_level::error,
								"CSiteList::getSite: Empty site passed in.");
		return (NULL);
	}

	std::lock_guard<std::mutex> guard(vSiteMutex);

	// lookup the site in the map by scnl
	auto itsite = mSite.find(scnl);
	if (itsite != mSite.end()) {
		return (mSite[scnl]);
	}

	// nothing found
	return (NULL);
}

// ---------------------------------------------------------getSite
std::shared_ptr<CSite> CSiteList::getSite(std::string site, std::string comp,
											std::string net, std::string loc,
											bool lookup) {
	if (site == "") {
		glassutil::CLogit::log(glassutil::log_level::error,
								"CSiteList::getSite: Empty site passed in.");
		return (NULL);
	}

	if (net == "") {
		glassutil::CLogit::log(glassutil::log_level::error,
								"CSiteList::getSite: Empty network passed in.");
		return (NULL);
	}

	// generate SCNL the same way that it is built in CSite::initialize
	std::string scnl = "";

	// station, required
	if (site != "") {
		scnl += site;
	} else {
		glassutil::CLogit::log(glassutil::log_level::error,
								"CSiteList::getSite: missing sSite.");
		return (NULL);
	}

	// component, optional
	if (comp != "") {
		scnl += "." + comp;
	}

	// network, required
	if (net != "") {
		scnl += "." + net;
	} else {
		glassutil::CLogit::log(glassutil::log_level::error,
								"CSiteList::getSite: missing sNet.");
		return (NULL);
	}

	// location, optional, -- is used by some networks to represent an empty
	// location code
	if (loc == "--") {
		loc = "";
	}
	if (loc != "") {
		scnl += "." + loc;
	}

	// lookup the site from the map
	std::shared_ptr<CSite> foundSite = getSite(scnl);
	if (foundSite != NULL) {
		return (foundSite);
	}

	// send request for information about this station
	// NOTE: If we moved this section above the getSite() call
	// we could be constantly requesting new station information,
	// which might be useful.
	if ((lookup == true) && (pGlass != NULL)) {
		// what time is it
		time_t tNow;
		std::time(&tNow);

		// get what time this station has been looked up before
		int tLookup = 0;
		auto itsite = mLookup.find(scnl);
		if (itsite != mLookup.end()) {
			tLookup = mLookup[scnl];
		}

		// only ask for a station occasionally
		// NOTE: Hardcoded to 6 hours.
		if ((tNow - tLookup) > (60 * 60 * 6)) {
			// construct request json message
			std::shared_ptr<json::Object> request = std::make_shared<
					json::Object>(json::Object());
			(*request)["Type"] = "SiteLookup";
			(*request)["Site"] = site;
			(*request)["Comp"] = comp;
			(*request)["Net"] = net;
			(*request)["Loc"] = loc;

			// send request
			pGlass->send(request);

			// log
			char sLog[1024];
			snprintf(sLog, sizeof(sLog),
						"CSiteList::getSite: SCNL:%s not on station list, "
						"requesting information.",
						scnl.c_str());
			glassutil::CLogit::log(sLog);

			// remember when we tried
			mLookup[scnl] = tNow;
		}
	}

	// site not found
	return (NULL);
}

std::vector<std::shared_ptr<CSite>> CSiteList::getSiteList() {
	std::lock_guard<std::mutex> guard(vSiteMutex);

	std::vector<std::shared_ptr<CSite>> siteList;

	// move through whole vector
	for (const auto &site : vSite) {
		siteList.push_back(site);
	}

	// return what we got
	return (siteList);
}

// ---------------------------------------------------------getSiteList
bool CSiteList::reqSiteList() {
	// construct request json message
	std::shared_ptr<json::Object> sitelistObj = std::make_shared<json::Object>(
			json::Object());
	(*sitelistObj)["Cmd"] = "SiteList";

	// array to hold data
	json::Array stationList;

	std::lock_guard<std::mutex> guard(vSiteMutex);

	// move through whole vector
	for (const auto &site : vSite) {
		json::Object stationObj;
		double lat;
		double lon;
		double z;

		// convert to geographic coordinates
		site->getGeo().getGeographic(&lat, &lon, &z);

		// convert from earth radius to elevation
		double elv = (z - 6317.0) * -1000.0;

		// construct a json message containing our site info
		stationObj["Lat"] = lat;
		stationObj["Lon"] = lon;
		stationObj["Z"] = elv;
		stationObj["Qual"] = site->getQual();
		stationObj["Use"] = site->getUse();
		stationObj["UseForTele"] = site->getUseForTele();

		stationObj["Sta"] = site->getSite();
		if (site->getComp() != "") {
			stationObj["Comp"] = site->getComp();
		}
		stationObj["Net"] = site->getNet();
		if (site->getLoc() != "") {
			stationObj["Loc"] = site->getLoc();
		}

		// add to station list
		stationList.push_back(stationObj);
	}

	(*sitelistObj)["SiteList"] = stationList;

	if (pGlass) {
		pGlass->send(sitelistObj);
	}

	return (true);
}

const CGlass* CSiteList::getGlass() const {
	std::lock_guard<std::recursive_mutex> siteListGuard(m_SiteListMutex);
	return (pGlass);
}

void CSiteList::setGlass(CGlass* glass) {
	std::lock_guard<std::recursive_mutex> siteListGuard(m_SiteListMutex);
	pGlass = glass;
}

int CSiteList::getVSiteSize() const {
	std::lock_guard<std::mutex> vSiteGuard(vSiteMutex);
	return (vSite.size());
}

}  // namespace glasscore
