#include "Pick.h"
#include <json.h>
#include <date.h>
#include <logger.h>
#include <memory>
#include <string>
#include <vector>
#include <limits>
#include <algorithm>
#include "Web.h"
#include "Trigger.h"
#include "Node.h"
#include "PickList.h"
#include "HypoList.h"
#include "Hypo.h"
#include "Site.h"
#include "SiteList.h"
#include "Glass.h"

namespace glasscore {

// constants
const unsigned int CPick::k_nNucleateAnnealPasses;
const unsigned int CPick::k_nNucleateNumberOfAnnealIterations;
constexpr double CPick::k_dNucleateInitialAnnealTimeStepSize;
constexpr double CPick::k_dNucleateFinalAnnealTimeStepSize;

// ---------------------------------------------------------CPick
CPick::CPick() {
	clear();
}

// ---------------------------------------------------------CPick
CPick::CPick(std::shared_ptr<CSite> pickSite, double pickTime,
				std::string pickIdString, double backAzimuth, double slowness) {
	initialize(pickSite, pickTime, pickIdString, backAzimuth, slowness);
}

// ---------------------------------------------------------CPick
CPick::CPick(std::shared_ptr<json::Object> pick, CSiteList *pSiteList) {
	clear();

	// null check json
	if (pick == NULL) {
		glass3::util::Logger::log("error",
									"CPick::CPick: NULL json communication.");
		return;
	}

	// check type
	if (pick->HasKey("Type")
			&& ((*pick)["Type"].GetType() == json::ValueType::StringVal)) {
		std::string type = (*pick)["Type"].ToString();

		if (type != "Pick") {
			glass3::util::Logger::log(
					"warning",
					"CPick::CPick: Non-Pick message passed in: "
							+ json::Serialize(*pick));
			return;
		}
	} else {
		glass3::util::Logger::log(
				"error",
				"CPick::CPick: Missing required Type Key: "
						+ json::Serialize(*pick));
		return;
	}

	// pick definition variables
	std::string sta = "";
	std::string comp = "";
	std::string net = "";
	std::string loc = "";
	std::shared_ptr<CSite> site = NULL;
	std::string ttt = "";
	double tpick = 0;
	double backAzimuth = -1;
	double slowness = -1;
	std::string pid = "";

	// site
	if (pick->HasKey("Site")
			&& ((*pick)["Site"].GetType() == json::ValueType::ObjectVal)) {
		// site is an object, create scnl std::string from it
		// get object
		json::Object siteobj = (*pick)["Site"].ToObject();

		// scnl varibles

		// site
		if (siteobj.HasKey("Station")
				&& (siteobj["Station"].GetType() == json::ValueType::StringVal)) {
			sta = siteobj["Station"].ToString();
		} else {
			glass3::util::Logger::log(
					"error",
					"CPick::CPick: Missing required Station Key: "
							+ json::Serialize(*pick));

			return;
		}

		// comp (optional)
		if (siteobj.HasKey("Channel")
				&& (siteobj["Channel"].GetType() == json::ValueType::StringVal)) {
			comp = siteobj["Channel"].ToString();
		} else {
			comp = "";
		}

		// net
		if (siteobj.HasKey("Network")
				&& (siteobj["Network"].GetType() == json::ValueType::StringVal)) {
			net = siteobj["Network"].ToString();
		} else {
			glass3::util::Logger::log(
					"error",
					"CPick::CPick: Missing required Network Key: "
							+ json::Serialize(*pick));

			return;
		}

		// loc (optional)
		if (siteobj.HasKey("Location")
				&& (siteobj["Location"].GetType() == json::ValueType::StringVal)) {
			loc = siteobj["Location"].ToString();
		} else {
			loc = "";
		}
	} else {
		// no site key
		glass3::util::Logger::log(
				"error",
				"CPick::CPick: Missing required Site Key: "
						+ json::Serialize(*pick));

		return;
	}

	// lookup the site, if we have a sitelist available
	if (pSiteList) {
		site = pSiteList->getSite(sta, comp, net, loc);
	}

	// check to see if we got a site
	if (site == NULL) {
		glass3::util::Logger::log(
				"warning",
				"CPick::CPick: Unknown site: " + json::Serialize(*pick));

		return;
	}

	// check to see if we're using this site
	if (!site->getEnable()) {
		return;
	}

	// time
	// get the pick time based on which key we find
	if (pick->HasKey("Time")
			&& ((*pick)["Time"].GetType() == json::ValueType::StringVal)) {
		// Time is formatted in iso8601, convert to Gregorian seconds
		ttt = (*pick)["Time"].ToString();
		glass3::util::Date dt = glass3::util::Date();
		tpick = dt.decodeISO8601Time(ttt);
	} else if (pick->HasKey("T")
			&& ((*pick)["T"].GetType() == json::ValueType::StringVal)) {
		// T is formatted in datetime, convert to Gregorian seconds
		ttt = (*pick)["T"].ToString();
		glass3::util::Date dt = glass3::util::Date();
		tpick = dt.decodeDateTime(ttt);
	} else {
		glass3::util::Logger::log(
				"error",
				"CPick::CPick: Missing required Time or T Key: "
						+ json::Serialize(*pick));

		return;
	}

	// pid
	// get the pick id based on which key we find
	if (pick->HasKey("ID")
			&& ((*pick)["ID"].GetType() == json::ValueType::StringVal)) {
		pid = (*pick)["ID"].ToString();
	} else if (pick->HasKey("Pid")
			&& ((*pick)["Pid"].GetType() == json::ValueType::StringVal)) {
		pid = (*pick)["Pid"].ToString();
	} else {
		glass3::util::Logger::log(
				"warning",
				"CPick::CPick: Missing required ID or Pid Key: "
						+ json::Serialize(*pick));

		return;
	}

	// beam
	if (pick->HasKey("Beam")
			&& ((*pick)["Beam"].GetType() == json::ValueType::ObjectVal)) {
		// beam is an object
		json::Object beamobj = (*pick)["Beam"].ToObject();

		// backAzimuth
		if (beamobj.HasKey("BackAzimuth")
				&& (beamobj["BackAzimuth"].GetType()
						== json::ValueType::DoubleVal)) {
			backAzimuth = beamobj["BackAzimuth"].ToDouble();
		} else {
			glass3::util::Logger::log(
					"warning",
					"CPick::CPick: Missing Beam BackAzimuth Key: "
							+ json::Serialize(*pick));
			backAzimuth = std::numeric_limits<double>::quiet_NaN();
		}

		// slowness
		if (beamobj.HasKey("Slowness")
				&& (beamobj["Slowness"].GetType() == json::ValueType::DoubleVal)) {
			slowness = beamobj["Slowness"].ToDouble();
		} else {
			glass3::util::Logger::log(
					"warning",
					"CPick::CPick: Missing Beam Slowness Key: "
							+ json::Serialize(*pick));
			slowness = std::numeric_limits<double>::quiet_NaN();
		}
	} else {
		backAzimuth = std::numeric_limits<double>::quiet_NaN();
		slowness = std::numeric_limits<double>::quiet_NaN();
	}

	// pass to initialization function
	if (!initialize(site, tpick, pid, backAzimuth, slowness)) {
		glass3::util::Logger::log(
				"error",
				"CPick::CPick: Failed to initialize pick: "
						+ json::Serialize(*pick));
		return;
	}

	std::lock_guard<std::recursive_mutex> guard(m_PickMutex);

	// remember input json for hypo message generation
	// note, move to init?
	m_JSONPick = pick;
}

// ---------------------------------------------------------~CPick
CPick::~CPick() {
	clear();
}

// ---------------------------------------------------------clear
void CPick::clear() {
	std::lock_guard<std::recursive_mutex> guard(m_PickMutex);

	m_wpSite.reset();
	m_wpHypo.reset();
	m_JSONPick.reset();

	m_sPhaseName = "";
	m_sID = "";
	m_tPick = 0.0;
	m_dBackAzimuth = std::numeric_limits<double>::quiet_NaN();
	m_dSlowness = std::numeric_limits<double>::quiet_NaN();
	m_tInsertion = 0.0;
	m_tFirstAssociation = 0.0;
	m_tNucleation = 0.0;
}

// ---------------------------------------------------------initialize
bool CPick::initialize(std::shared_ptr<CSite> pickSite, double pickTime,
						std::string pickIdString, double backAzimuth,
						double slowness) {
	std::lock_guard<std::recursive_mutex> guard(m_PickMutex);

	clear();

	setTPick(pickTime);
	setTSort(pickTime);
	m_sID = pickIdString;
	m_dBackAzimuth = backAzimuth;
	m_dSlowness = slowness;

	// nullcheck
	if (pickSite == NULL) {
		return (false);
	} else {
		m_wpSite = pickSite;
	}

	m_tInsertion = glass3::util::Date::now();

	return (true);
}

// ---------------------------------------------------------addHypoReference
void CPick::addHypoReference(std::shared_ptr<CHypo> hyp, bool force) {
	std::lock_guard<std::recursive_mutex> guard(m_PickMutex);

	// nullcheck
	if (hyp == NULL) {
		glass3::util::Logger::log("error",
									"CPick::addHypo: NULL hypo provided.");
		return;
	}

	// Add hypo data reference to this pick
	if (force == true) {
		m_wpHypo = hyp;
	} else if (m_wpHypo.expired() == true) {
		m_wpHypo = hyp;
	}

	if (getTFirstAssociation() == 0.0) {
		setTFirstAssociation();
	}
}

// ---------------------------------------------------------removeHypoReference
void CPick::removeHypoReference(std::shared_ptr<CHypo> hyp) {
	// nullcheck
	if (hyp == NULL) {
		glass3::util::Logger::log("error",
									"CPick::remHypo: NULL hypo provided.");
		return;
	}

	removeHypoReference(hyp->getID());
}

// ---------------------------------------------------------removeHypoReference
void CPick::removeHypoReference(std::string pid) {
	std::lock_guard<std::recursive_mutex> guard(m_PickMutex);

	// is the pointer still valid
	if (auto pHypo = m_wpHypo.lock()) {
		// Remove hypo reference from this pick
		if (pHypo->getID() == pid) {
			clearHypoReference();
		}
	} else {
		// remove invalid pointer
		clearHypoReference();
	}
}

// ---------------------------------------------------------clearHypoReference
void CPick::clearHypoReference() {
	std::lock_guard<std::recursive_mutex> guard(m_PickMutex);
	m_wpHypo.reset();
}

// ---------------------------------------------------------nucleate
bool CPick::nucleate() {
	// get the site shared_ptr
	std::shared_ptr<CSite> pickSite = m_wpSite.lock();
	std::string pt = glass3::util::Date::encodeDateTime(m_tPick);
	char sLog[glass3::util::Logger::k_nMaxLogEntrySize];

	setTNucleation();

	// Use site nucleate to scan all nodes
	// linked to this pick's site and calculate
	// the stacked agoric at each node.  If the threshold
	// is exceeded, the node is added to the site's trigger list
	std::vector<std::shared_ptr<CTrigger>> vTrigger = pickSite->nucleate(
			m_tPick);

	// if there were no triggers, we're done
	if (vTrigger.size() == 0) {
		/*
		 glass3::util::Logger::log(
		 "debug",
		 "CPick::nucleate: NOTRG site:" + pickSite->getSCNL()
		 + "; tPick:" + pt + "; sID:" + m_sID);
		 */
		return (false);
	}

	for (const auto &trigger : vTrigger) {
		if (trigger->getWeb() == NULL) {
			continue;
		}

		// check to see if the pick is currently associated to a hypo
		if (m_wpHypo.expired() == false) {
			// get the hypo and compute distance between it and the
			// current trigger
			std::shared_ptr<CHypo> pHypo = m_wpHypo.lock();
			if (pHypo != NULL) {
				glass3::util::Geo geoHypo = pHypo->getGeo();

				glass3::util::Geo trigHypo = trigger->getGeo();

				double dist = (geoHypo.delta(&trigHypo)
						/ glass3::util::GlassMath::k_DegreesToRadians)
						* glass3::util::Geo::k_DegreesToKm;

				// is the associated hypo close enough to this trigger to skip
				// close enough means within the resolution of the trigger
				if (dist < trigger->getWebResolution()) {
					glass3::util::Logger::log(
							"debug",
							"CPick::nucleate: SKIPTRG because pick proximal hypo ("
									+ std::to_string(dist) + " < "
									+ std::to_string(
											trigger->getWebResolution()) + ")");
					continue;
				}
			}
		}

		// create the hypo using the node
		std::shared_ptr<CHypo> hypo = std::make_shared<CHypo>(
				trigger, CGlass::getAssociationTravelTimes());

		// set nuclation auditing info
		hypo->setNucleationAuditingInfo(glass3::util::Date::now(),
										this->getTInsertion());

		// add links to all the picks that support the hypo
		std::vector<std::shared_ptr<CPick>> vTriggerPicks = trigger->getVPick();

		for (auto pick : vTriggerPicks) {
			// they're not associated yet, just potentially
			hypo->addPickReference(pick);
		}

		// use the hypo's nucleation threshold, which is really the
		// web's nucleation threshold
		int ncut = hypo->getNucleationDataThreshold();
		double thresh = hypo->getNucleationStackThreshold();
		double maxDepth = trigger->getNodeMaxDepth();
		bool bad = false;

		// First localization attempt after nucleation
		// make 3 passes
		for (int ipass = 0; ipass < k_nNucleateAnnealPasses; ipass++) {
			// get an initial location via synthetic annealing,
			// which also prunes out any poorly fitting picks
			// the search is based on the grid resolution, and how
			// far out the ot can change without losing the initial pick
			// this all assumes that the closest grid triggers
			// values derived from testing global event association
			double bayes = hypo->anneal(
					k_nNucleateNumberOfAnnealIterations,
					trigger->getWebResolution()
							/ CHypo::k_dInitialAnnealStepReducationFactor,  // NOLINT
					trigger->getWebResolution()
							/ CHypo::k_dFinalAnnealStepReducationFactor,
					std::max(
							trigger->getWebResolution()
									/ CHypo::k_dTimeToDistanceCorrectionFactor,  // NOLINT
							k_dNucleateInitialAnnealTimeStepSize),  // NOLINT
					k_dNucleateFinalAnnealTimeStepSize);

			// get the number of picks we have now
			int npick = hypo->getPickDataSize();
			double depth = hypo->getDepth();

			/*
			 snprintf(sLog, sizeof(sLog), "CPick::nucleate: -- Pass:%d; nPick:%d"
			 "/nCut:%d; bayes:%f/thresh:%f; %s",
			 ipass, npick, ncut, bayes, thresh,
			 hypo->getID().c_str());
			 glass3::util::Logger::log(sLog);
			 */

			// check to see if we still have enough picks for this hypo to
			// survive.
			// NOTE, in Node, ncut is used as a threshold for the number of
			// *stations* here it's used for the number of *picks*, which only
			// since we only nucleate on a single phase.
			if (npick < ncut) {
				// we don't
				snprintf(sLog, sizeof(sLog),
							"CPick::nucleate: -- Abandoning solution %s "
							"due to lack of picks "
							"(npick:%d/ncut:%d)",
							hypo->getID().c_str(), npick, ncut);
				glass3::util::Logger::log(sLog);

				// don't bother making additional passes
				bad = true;
				break;
			}

			// check to see if we still have a high enough bayes value for this
			// hypo to survive.
			if (bayes < thresh) {
				// it isn't
				snprintf(sLog, sizeof(sLog),
							"CPick::nucleate: -- Abandoning solution %s "
							"due to low bayes value "
							"(bayes:%f/thresh:%f)",
							hypo->getID().c_str(), bayes, thresh);
				glass3::util::Logger::log(sLog);

				// don't bother making additional passes
				bad = true;
				break;
			}

			// check to see if we are not below the maximum allowed depth for
			// the trigger's node
			if (depth > maxDepth) {
				// it isn't
				snprintf(sLog, sizeof(sLog),
							"CPick::nucleate: -- Abandoning solution %s "
							"due to depth greater than max depth "
							"(depth:%f/maxDepth:%f)",
							hypo->getID().c_str(), depth, maxDepth);
				glass3::util::Logger::log(sLog);

				// don't bother making additional passes
				bad = true;
				break;
			}
		}

		// we've abandoned the potential hypo at this node
		if (bad) {
			// move on to the next triggering node
			continue;
		}

		// log the hypo
		std::string st = glass3::util::Date::encodeDateTime(hypo->getTOrigin());
		glass3::util::Logger::log(
				"debug",
				"CPick::nucleate: TRG site:" + pickSite->getSCNL() + "; tPick:"
						+ pt + "; sID:" + m_sID + " => web:"
						+ hypo->getWebName() + "; hyp: " + hypo->getID()
						+ "; lat:" + std::to_string(hypo->getLatitude())
						+ "; lon:" + std::to_string(hypo->getLongitude())
						+ "; z:" + std::to_string(hypo->getDepth()) + "; bayes:"
						+ std::to_string(hypo->getBayesValue()) + "; tOrg:"
						+ st);

		// if we got this far, the hypo has enough supporting data to
		// merit adding it to the hypo list
		CGlass::getHypoList()->addHypo(hypo);
	}

	// done
	return (true);
}

// ---------------------------------------------------------getJSONPick
const std::shared_ptr<json::Object>& CPick::getJSONPick() const {
	std::lock_guard<std::recursive_mutex> pickGuard(m_PickMutex);
	return (m_JSONPick);
}

// ---------------------------------------------------------getHypo
const std::shared_ptr<CHypo> CPick::getHypoReference() const {
	std::lock_guard<std::recursive_mutex> pickGuard(m_PickMutex);
	return (m_wpHypo.lock());
}

// ---------------------------------------------------------getSite
const std::shared_ptr<CSite> CPick::getSite() const {
	std::lock_guard<std::recursive_mutex> pickGuard(m_PickMutex);
	return (m_wpSite.lock());
}

// ---------------------------------------------------------getPhaseName
const std::string& CPick::getPhaseName() const {
	std::lock_guard<std::recursive_mutex> pickGuard(m_PickMutex);
	return (m_sPhaseName);
}

// ---------------------------------------------------------getID
const std::string& CPick::getID() const {
	std::lock_guard<std::recursive_mutex> pickGuard(m_PickMutex);
	return (m_sID);
}

// ---------------------------------------------------------getBackAzimuth
double CPick::getBackAzimuth() const {
	return (m_dBackAzimuth);
}

// ---------------------------------------------------------getSlowness
double CPick::getSlowness() const {
	return (m_dSlowness);
}

// ---------------------------------------------------------getTPick
double CPick::getTPick() const {
	return (m_tPick);
}

// ---------------------------------------------------------setTPick
void CPick::setTPick(double tPick) {
	m_tPick = tPick;
}

// --------------------------------------------------getTSort
double CPick::getTSort() const {
	return (m_tSort);
}

// --------------------------------------------------setTSort
void CPick::setTSort(double newTSort) {
	m_tSort = newTSort;
}

// --------------------------------------------------getTInsertion
double CPick::getTInsertion() const {
	return (m_tInsertion);
}

// --------------------------------------------------getTFirstAssociation
double CPick::getTFirstAssociation() const {
	return (m_tFirstAssociation);
}

// --------------------------------------------------setTFirstAssociation
void CPick::setTFirstAssociation() {
	m_tFirstAssociation = glass3::util::Date::now();
}

// --------------------------------------------------getTNucleation
double CPick::getTNucleation() const {
	return (m_tNucleation);
}

// --------------------------------------------------setTNucleation
void CPick::setTNucleation() {
	m_tNucleation = glass3::util::Date::now();
}

}  // namespace glasscore