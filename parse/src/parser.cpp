#include <parser.h>
#include <string>

namespace parse {
Parser::Parser(const std::string &newAgencyID, const std::string &newAuthor) {
	m_AgencyID = newAgencyID;
	m_Author = newAuthor;
}

Parser::~Parser() {
}
}  // namespace parse