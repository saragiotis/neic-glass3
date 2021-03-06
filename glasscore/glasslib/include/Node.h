/*****************************************
 * This file is documented for Doxygen.
 * If you modify this file please update
 * the comments so that Doxygen will still
 * be able to work.
 ****************************************/
#ifndef NODE_H
#define NODE_H

#include <geo.h>
#include <vector>
#include <memory>
#include <string>
#include <utility>
#include <mutex>
#include <tuple>
#include <atomic>
#include <set>
#include "Link.h"

namespace glasscore {

// forward declarations
class CPick;
class CPickList;
class CSite;
class CWeb;
class CTrigger;

/**
 * \brief glasscore detection node class
 *
 * The CNode class represents a single detection node in the
 * detection graph database.  A CNode is linked to one or more sites
 * (seismic stations), as part of the overall detection graph database.
 * A detection node consists of the location (latitude, longitude, and depth)
 * of the detection node, the spatial resolution of the node, a list of the
 * links to the sites, and a list of picks currently being nucleated.
 *
 * CNode uses the nucleate function to evaluate the likelihood of a hypocenter
 * existing centered on the node based on all picks each site linked
 * to the node
 *
 * CNode uses smart pointers (std::shared_ptr).
 */
class CNode {
 public:
	/**
	 * \brief CNode constructor
	 */
	CNode();

	/**
	 * \brief CNode advanced constructor
	 *
	 * Construct a node using the provided latitude, longitude,
	 * and depth.
	 *
	 * \param name - A string containing the name of the parent web
	 * for this node
	 * \param lat - A double value containing the latitude to use
	 * for this node in degrees
	 * \param lon - A double value containing the longitude to use
	 * for this node in degrees
	 * \param z - A double value containing the depth to use
	 * for this node in kilometers
	 * \param resolution - A double value containing the inter-node resolution
	 * in kilometers
	 * \param maxDepth - A double value containing the maximum trigger depth
	 * in kilometers
	 * \param aseismic - A boolean flag indicating whether this node is in an
	 * aseismic area
	 */
	CNode(std::string name, double lat, double lon, double z, double resolution,
			double maxDepth, bool aseismic);

	/**
	 * \brief CNode destructor
	 */
	~CNode();

	/**
	 * \brief CNode clear function
	 */
	void clear();

	/**
	 * \brief Delink all sites to/from this node.
	 */
	void clearSiteLinks();

	/**
	 * \brief CNode initialization funcion
	 *
	 * Initialize a node using the provided latitude, longitude,
	 * and depth.
	 *
	 * \param name - A string containing the name of the parent web
	 * for this node
	 * \param lat - A double value containing the latitude to use
	 * for this node in degrees
	 * \param lon - A double value containing the longitude to use
	 * for this node in degrees
	 * \param z - A double value containing the depth to use
	 * for this node in kilometers
	 * \param resolution - A double value containing the inter-node resolution
	 * in kilometers
	 * \param maxDepth - A double value containing the maximum trigger depth
	 * in kilometers
	 * \param aseismic - A boolean flag indicating whether this node is in an
	 * aseismic area
	 */
	bool initialize(std::string name, double lat, double lon, double z,
					double resolution, double maxDepth, bool aseismic);

	/**
	 * \brief CNode node-site and site-node linker
	 *
	 * Add a link to/from this node to the provided site
	 * using the provided travel time
	 *
	 * NOTE: The node is required as parameter because issues occurred linking
	 * and unlinking (deleting) using this-> The solution was to pass a pointer
	 * to the node in this function.
	 *
	 * \param distDeg - A double value containing the distance in degrees
	 * between the node and the site.
	 * \param travelTime1 - A double value containing the first travel time to
	 * use for the link
	 * \param phase1 - A std::string containing the first travel time phase code
	 * \param travelTime2 - A double value containing the optional second travel
	 * time to use for the link, defaults to -1 (no travel time)
	 * \param phase2 - A std::string containing the second travel time phase 
	 * code, defaults to "" (no travel time)
	 * \param site - A shared_ptr<CSite> to the site to link
	 * \param node - A shared_ptr<CNode> to the node to link (should be itself)
	 * \return - Returns true if successful, false otherwise
	 */
	bool linkSite(std::shared_ptr<CSite> site, std::shared_ptr<CNode> node,
					double distDeg, double travelTime1, std::string phase1,
					double travelTime2 = -1, std::string phase2 = "");

	/**
	 * \brief CNode node-site and site-node unlinker
	 *
	 * Remove the given site to/from this node
	 *
	 * \param site - A std::shared_ptr<CSite> to the site to remove
	 * \return - Returns true if successful, false otherwise
	 */
	bool unlinkSite(std::shared_ptr<CSite> site);

	/**
	 * \brief CNode unlink last by distance site from node
	 *
	 * Remove the last site by distance to/from this node
	 *
	 * \return - Returns true if successful, false otherwise
	 */
	bool unlinkLastSite();

	/**
	 * \brief CNode Nucleation function
	 *
	 * Given an origin time, compute a number representing the stacked PDF
	 * of a hypocenter centered on this node, by computing the PDF
	 * of each pick at each site linked to this node and totaling (stacking)
	 * them up. Picks with authors that are not in the set of allowed 
	 * sources are not nuclated. Picks without authors or sources are always
	 * nuclated.
	 *
	 * \param tOrigin - A double value containing the proposed origin time
	 * to use in Gregorian seconds
	 * \param parentThread - A pointer to the parent CPickList thread to allow 
	 * nucleate to call the CPickList thread status update function, the owner
	 * of this pointer is the CPickList object
	 * \return Returns true if the node nucleated an event, false otherwise
	 */
	std::shared_ptr<CTrigger> nucleate(double tOrigin, CPickList* parentThread);

	/**
	 * \brief CNode significance function
	 *
	 * Given an observed time and a link to a site, compute the best
	 * significance value from the traveltime contained in the link to this
	 * node.
	 *
	 * \param tObservedTT - A double value containing the observed travel time
	 * \param travelTime - A double value containing the first calculated
	 * travel time
	 * \param distDeg - A double value containing the distance between the
	 * station and the node in degrees
	 * \return Returns  significance if there is a valid travel
	 * time, -1.0 otherwise
	 */
	double getSignificance(double tObservedTT, double travelTime,
							double distDeg);

	/**
	 * \brief CNode site used function
	 *
	 * Given a site id, check to see if the site is used by the node
	 *
	 * \param siteID - A string containing the id of the site to get
	 * \return Returns a shared_ptr to a the CSite used by the node, NULL if
	 * not found
	 */
	std::shared_ptr<CSite> getSite(std::string siteID);

	/**
	 * \brief CNode get last site function
	 *
	 * Get the last site linked to the node
	 *
	 * \return Returns a shared pointer to the last CSite object if found, null
	 * otherwise
	 */
	std::shared_ptr<CSite> getLastSite();

	/**
	 * \brief CNode site link sort function
	 *
	 * Sort the list of sites linked to this node by distance from node
	 */
	void sortSiteLinks();

	/**
	 * \brief Generates a string that contains each site link, including the
	 * siteID, latitude, longitude, and depth. This string is generated when
	 * optionally creating web files that are used for tuning the web using
	 * third party programs.
	 * \return the sites string
	 */
	std::string getSitesString();

	/**
	 * \brief Gets the number of sites linked to this node
	 * \return Returns an integer containing the count of sites linked to this
	 * node
	 */
	int getSiteLinksCount() const;

	/**
	 * \brief Gets a flag indicating that the node is enabled for nucleation.
	 * Typically a node is only disabled when it is being reconfigured
	 * \return Returns a boolean flag, true if the node is enabled, false
	 * otherwise
	 */
	bool getEnabled() const;

	/**
	 * \brief Sets a flag indicating that the node is enabled for nucleation.
	 * Typically a node is only disabled when it is being reconfigured
	 * \param enabled - a boolean flag, true if the node is enabled, false
	 * otherwise
	 */
	void setEnabled(bool enabled);

	/**
	 * \brief Get the latitude for this node
	 * \return Returns a double containing the node latitude in degrees
	 */
	double getLatitude() const;

	/**
	 * \brief Get the longitude for this node
	 * \return Returns a double containing the node longitude in degrees
	 */
	double getLongitude() const;

	/**
	 * \brief Get the depth for this node
	 * \return Returns a double containing the node depth in kilometers
	 */
	double getDepth() const;

	/**
	 * \brief Get the maximum depth for triggers made at this node
	 * \return Returns a double containing the maximum depth for triggers made
	 * in kilometers
	 */
	double getMaxDepth() const;

	/**
	 * \brief Gets a flag indicating that the node is in an aseismic area
	 * \return Returns a boolean flag, true if the node is aseismic, false
	 * otherwise
	 */
	bool getAseismic() const;

	/**
	 * \brief Get the combined node location (latitude, longitude, depth) as
	 * a CGeo object
	 * \return Returns a glass3::util::Geo object containing the combined location.
	 */
	glass3::util::Geo getGeo() const;

	/**
	 * \brief Gets the resolution of the web that created this node
	 * \return Returns a double value containing the resolution of the web
	 * that created this node in kilometers
	 */
	double getResolution() const;

	/**
	 * \brief Gets a pointer to the parent CWeb that created and holds this node
	 * \return Returns a pointer to the parent CWeb
	 */
	CWeb* getWeb() const;

	/**
	 * \brief Sets a pointer to the parent CWeb that created and holds this node
	 * \param web - a pointer to the parent CWeb
	 */
	void setWeb(CWeb* web);

	/**
	 * \brief Get the maximum site distance for this node
	 * \return Returns a double containing the maximum site distance for this node
	 * in degrees.
	 */
	double getMaxSiteDistance() const;

	/**
	 * \brief Gets the name of the parent CWeb that created and holds this node
	 * This name is used to ensure that only one node triggers per web during
	 * site nucleation
	 * \return Returns a std::string containing the web name
	 */
	const std::string& getName() const;

	/**
	 * \brief Get the unique id for this web
	 * \return Returns a std::string containing the node web
	 */
	std::string getID() const;

	/**
	 * \brief Add to the allowed pick sources for this node.
	 * \param source - A std::string containing a pick source (author) string to
	 * add to the list of allowed sources for this node
	 */
	void addSource(std::string source);

 private:
	/**
	 * \brief A pointer to the parent CWeb class, used get configuration,
	 * values
	 */
	CWeb * m_pWeb;

	/**
	 * \brief Name of the web subnet that this node is associated with.
	 * This attribute is used for web level tracking and dynamics such
	 * as removing a named subnet with the 'RemoveWeb' command.
	 */
	std::string m_sName;

	/**
	 * \brief A double value containing this node's latitude in degrees.
	 */
	std::atomic<double> m_dLatitude;

	/**
	 * \brief A double value containing this node's longitude in degrees.
	 */
	std::atomic<double> m_dLongitude;

	/**
	 * \brief A double value containing this node's depth in kilometers.
	 */
	std::atomic<double> m_dDepth;

	/**
	 * \brief A double value containing this node's maximum trigger depth in
	 * kilometers.
	 */
	std::atomic<double> m_dMaxDepth;

	/**
	 * \brief A boolean flag indicating whether this node is in an aseismic
	 * area or not.
	 */
	std::atomic<bool> m_bAseismic;

	/**
	 * \brief A double value containing this node's maximum site distance in
	 * degrees.
	 */
	std::atomic<double> m_dMaxSiteDistance;

	/**
	 * \brief A double value containing this node's spatial resolution
	 * (to other nodes) in kilometers.
	 */
	std::atomic<double> m_dResolution;

	/**
	 * \brief A boolean flag indicating whether this node is enabled for
	 * nucleation. Nodes are disabled when they are being reconfigured.
	 */
	std::atomic<bool> m_bEnabled;

	/**
	 * \brief A std::set of strings listing the allowed pick sources for
	 * this node.
	 */
	std::set<std::string> m_SourceSet;

	/**
	 * \brief A std::vector of tuples linking node to site
	 * {shared site pointer, travel time 1, travel time 2}
	 */
	std::vector<SiteLink> m_vSiteLinkList;

	/**
	 * \brief A mutex to control threading access to vSite.
	 */
	mutable std::mutex m_SiteLinkListMutex;

	/**
	 * \brief A recursive_mutex to control threading access to CNode.
	 * NOTE: recursive mutexes are frowned upon, so maybe redesign around it
	 * see: http://www.codingstandard.com/rule/18-3-3-do-not-use-stdrecursive_mutex/
	 * However a recursive_mutex allows us to maintain the original class
	 * design as delivered by the contractor.
	 */
	mutable std::recursive_mutex m_NodeMutex;

	// constants
	/**
	 * \brief The window used with the predicted travel time to select picks for
	 * nucleation.
	 * NOTE: it seems OK to use a slightly larger window than we likely need,
	 * since it won't generate a lot of additional results, and might be
	 * necessary if we have improperly recognized the size of other potential
	 * errors.
	 */
	static constexpr double k_dTravelTimePickSelectionWindow = 120.0;

	/**
	 * \brief The depth shell resolution in kilometers used in nucleation
	 * NOTE: this could be calculated from the grid in the future.
	 */
	static constexpr double k_dDepthShellResolutionKm = 100.0;

	/**
	 * \brief The the ratio between the grid points and the grid resolution
	 * NOTE: = sqrt(2)/2)
	 */
	static constexpr double k_dGridPointVsResolutionRatio = 0.7071;

	/**
	 * \brief A factor for allowing distance of triggering relative to the distance
	 * between detection nodes. A factor of one would just allow a residual change
	 * corresponding to a center of a node cubiod. 3 allows it to span across a
	 * kitty corner node in a cubiod plus halfway to the next.
	 */
	static constexpr double k_residualDistanceAllowanceFactor = 2.;
};
}  // namespace glasscore
#endif  // NODE_H
