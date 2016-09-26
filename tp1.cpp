#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <cstdio>

using namespace std;

int main() {
	char estadoInicial;
	string gramatica, regra;
	vector<char> estados, alfabeto;
	multimap<char, string> regras;
	int controle=0;

	getline(cin, gramatica);
	for(int i = 0; i < gramatica.size(); i++){
		if(gramatica[i] == '{' && controle == 0){
			i++;
			while(gramatica[i] != '}'){
				if (gramatica[i] != ',') estados.push_back(gramatica[i]);
				i++;
			}
			controle++;
		}else if(gramatica[i] == '{' && controle == 1){
			i++;
			while(gramatica[i] != '}'){
				if(gramatica[i] != ',') alfabeto.push_back(gramatica[i]);
				i++;
			}
			controle++;
		}else if(gramatica[i] == '{' && controle == 2){
			i++;
			bool terminou = false;
			while (gramatica[i] != '}') {
				char variavel;
				if(isalpha(gramatica[i])) variavel = gramatica[i];
				regra = "";
				i+=2;
				while(gramatica[i] != ',' ) {
					if (gramatica[i] == '}') terminou = true;
					if(gramatica[i] != ' ' && gramatica[i] != '>' && gramatica[i] != '}') regra += gramatica[i];
					i++;
				}
				regras.insert(make_pair(variavel, regra));
				if(terminou) goto label;
				i++;
			}
			controle++;
		}else if (controle == 1000) {
			label: i+=2;
			estadoInicial = gramatica[i];
		}
	}

	puts("Variaveis:");
	for (int i = 0; i < estados.size(); i++) {
		printf("%c ", estados[i]);
	}
	puts("\nAlfabeto: ");
	for (int i = 0; i < alfabeto.size(); i++) {
		printf("%c ", alfabeto[i]);
	}
	puts("\nRegras: ");
	for(auto it : regras) {
		cout << it.first << " -> " << it.second << endl;
	}

	return 0;
}