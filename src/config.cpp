#include <iostream>
#include <fstream>
#include <vector>
#include <List>
using namespace std;
//using std::cout; using std::cerr;


int main()
{
	string filename("ipporta.txt");
	list <string> portas;
	list <string> ips;

	ifstream input_file(filename);

	if (!input_file.is_open()) {
		cerr << "Could not open the file - '"
			<< filename << "'" << endl;
		return EXIT_FAILURE;
	}
	
	string a, b;
	while (input_file >> a >> b)
	{
		ips.push_back(a);
		portas.push_back(b);
	}
	
	cout << "Lista dos IPs" << endl;
	for (const auto& i : ips) {
		cout << i <<endl;
	}
	cout << endl;
	cout << "Lista das portas" << endl;
	for (const auto& j : portas) {
		cout << j <<endl;
	}

	cout << endl;
	input_file.close();

	return EXIT_SUCCESS;
}
