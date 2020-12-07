#include <iostream>

#include "server.hpp"

/*void config_read() {
    std::ifstream cFile(filename);
    if (cFile.is_open()) {
        std::string line;
        while(getline(cFile, line)){
            line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());
            if(line[0] == '#' || line.empty())
		continue;
            auto delimiterPos = line.find("=");
            auto name = line.substr(0, delimiterPos);
            auto value = line.substr(delimiterPos + 1);
	    configuration[name] = std::stoi(value);
        }
        
    }
    else {
        std::cerr << "Couldn't open config file for reading.\n";
	return -1;
    }
    return 0;   

    }*/

int main () {
    Server server("./www/", "crt/crt", "crt/key");
    server.loop();
    return 0;
}
