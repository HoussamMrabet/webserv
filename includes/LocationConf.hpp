
#pragma once 

#include "ConfigBuilder.hpp"

class ServerConf;

class LocationConf {
    private :
		const std::string name;
        std::string root;                        
        std::vector<std::string> index;           
        bool autoIndex;
        std::vector<std::string> allowedMethods;
        size_t bodySizeLimit;
        std::string redirectUrl;
        bool    listing;
    public :
        LocationConf();
		LocationConf(const ServerConf server, std::string name);
     	virtual ~LocationConf();
        
		void setRoot(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens);
		void setIndex(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens);
		void setAutoIndex(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens);
		void setAllowedMethods(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens);
        void setBodySizeLimit(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens);
        void setRedirectUrl(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens);
        void setListing(std::vector<std::string>::const_iterator &it, std::vector<std::string> &tokens);
		
		const std::string getName() const;
        std::string getRoot() const;                        
        std::vector<std::string> getIndex() const;           
        bool getAutoIndex() const;
        std::vector<std::string> getAllowedMethods() const;
        size_t getBodySizeLimit() const;
        std::string getRedirectUrl() const;
        bool    getListing() const;
		

		void printName(std::ostream& os) const;
		void printRoot(std::ostream& os) const;
		void printIndex(std::ostream& os) const;
		void printAutoIndex(std::ostream& os) const;
		void printAllowedMethods(std::ostream& os) const;
		void printBodySizeLimit(std::ostream& os) const;
		void printRedirectUrl(std::ostream& os) const;
		void prontListing(std::ostream& os) const;
    };