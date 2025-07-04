#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <sstream>
#include <string>

class Listen{ // to remove later!

    private:
        std::string _host;
        std::string _port;

    public:
        Listen();
        Listen(std::string&, std::string&);
        std::string getHost() const;
        std::string getPort() const;
        int getPort(int)  const;


    bool operator==(const Listen& l) const;
    bool operator<(const Listen& l) const;

};

#endif