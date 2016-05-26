#ifndef VS_RS_CONFIG_READER_H_
#define VS_RS_CONFIG_READER_H_

#include <string>
#include <map>
#include <iosfwd>
#include <mf/common.h>

namespace vs {

class rs_config_reader {
private:
	std::map<std::string, std::string> parameters_;
	
	void parse_(std::istream&);
	
public:
	explicit rs_config_reader(const std::string& filename);
	
	bool has(const std::string& key) const;
	const std::string& get_string(const std::string& key) const;
	int get_int(const std::string& key) const;
	mf::real get_real(const std::string& key) const;
};

}

#endif
