#include "../include/config.hpp"

Config::Config(map<string, string> possibleServerAddresses){
    this->possibleServerAddresses = possibleServerAddresses;
}

int Config::generateIpsAndGatesLists(){
	string filename("ipporta.txt"); //é um arquivo externo, que não pertence à classe, não precisa botar no header, né?
	ifstream input_file(filename);
	
	if (!input_file.is_open()) {
		cerr << "Could not open the file - '" << filename << "'" << endl;
		return EXIT_FAILURE;
	}
	
	string a, b; //são só variáveis auxiliares para ler o arquivo, não precisa ir no header, né? Ou é melhor colocar lá?
	
	while (input_file >> a >> b){
		this->possibleServerAddresses.insert(pair<string, string>(a, b));
	}
	
	input_file.close();

	return EXIT_SUCCESS;
}


