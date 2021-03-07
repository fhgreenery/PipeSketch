#include<iostream>
#include<cmath>
#include <cstring>
#include <sstream>  
using namespace std;

int main(){
	// short s;
	// char ch;
	uint32_t ip;
	uint16_t port;
	string str="175535";
	stringstream ss(str);
	ss>>ip;
	cout<<ip<<endl;
	ss>>port;
	cout<<port<<endl;
	
	return 0;

}