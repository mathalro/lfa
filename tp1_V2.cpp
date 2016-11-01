#include <iostream>
#include <string>
#include <map>													//utilizado para mapear um variavel em uma regra
#include <vector>												//utilizado para armazenar as listas que compoe o automato
#include <fstream>												//utilizado na manipulação de arquivos
#include <queue>												//utilizado na trasnformacao do afn em afd

#include "Estruturas.h"

using namespace std;

void imprimeAutomato(Automato aut, string tipo);

int main (int argc, char **argv) {
	ifstream arquivo;											//cria objeto arquivo de entrada
	string gramatica, linha;									//objetos para leitura do arquivo
	arquivo.open(argv[1]);										//le o arquivo de acordo com o nome especificado na entrada
	while (getline(arquivo, linha)) gramatica += linha;			//le o arquivo linha a linha e salva na gramatica

	/**
		Lexico:  salva as variaveis, regras e transições, de acordo com a entrada.
	**/
	string estado_inicial;										//salva o estado inicial
	vector<string> variavel;									//container para salvar as variaveis da gramatica lida
	vector<string> alfabeto;									//container para salvar o alfabeto da gramatica
	multimap<string, Transicao> regras;							//container para salvar as transicoes da gramatica

	int estado = 0;												//controla o que o lexico deve fazer
	char atual;													//armazena o caractere que esta sendo lido atualmente
	bool permiteEstado = false;									//controla o funcionamento dos estados (so funciona apos ler '{')
	string token = "";											//string que armazena as variaveis encontradas
	string nova_regra;											//string que armazena uma regra no estado 3
	Transicao nova_transicao;									//estrutura mapeada por uma regra

	//itera todos os caracteres da gramatica
	for (int i = 0; i < gramatica.size(); i++) {
		atual = gramatica[i];									//salva caractere atual
		if (atual == ' ') continue;								//ignora espacos em branco

		if (permiteEstado) {									//caso uma { tenha sido lido
			switch(estado) {									
				case 1: {										//estado de leitura de variaveis
					switch(atual) {
						case ',': case '}': {
							variavel.push_back(token);			//coloca o novo token na lista de variaveis
							token = "";							//reinicia o token
							if (atual == '}') 					//a leitura de um } significa o fim desse estado
								permiteEstado = false;
							break;
						}
						default: { 
							token += atual;						//coloca o caractere no token atual
						}
					}
					break;
				}
				case 2:	{										//estado de leitura de alfabeto
					switch(atual) {
						case ',': case '}': {
							alfabeto.push_back(token);
							token = "";
							if (atual == '}') 
								permiteEstado = false;
							break;
						}
						default: {
							token += atual;
						}
					}
					break;
				}
				case 3: {
					switch(atual) {								//estado de leitura das regras
						case '-': {
							nova_regra = token;					//significa que terminamos de ler uma variavel
							token = "";
							break;
						}
						case '>': 
							break; 
						case ',': case '}': {
							nova_transicao.consome = token[0];	//o que e consumido
							nova_transicao.destino = token[1];	//pra onde vai apos o consumo
							if (token[1]) 						//caso exista destino
								nova_transicao.final = false;	//nao e estado final
							else if(token[0] == '#'){			//caso o destino seja lambda insere uma transição vazia
								nova_transicao.destino = '#';	//lambda
								nova_transicao.consome = '#';	//lambda
								nova_transicao.final = true;	//e estado final
							}else{
								nova_transicao.destino = 'Z';	//caso nao exista destino vai para Z
								nova_transicao.final = true;	//essa transicao vai para estado final
							}
							if(atual == '}') {					//termina de ler as regras
								estado = 4;						//vai direto para o ultimo estado
							}
							regras.insert(make_pair(nova_regra, nova_transicao));
							token = "";
							break;
						}
						default:
							token += atual;
					}
					break;
				}
				case 4: {										//estado de leitura dos estados iniciais 
					if(atual == ',') break;
					if(atual == ')') break;
					estado_inicial = atual;
					break;
				}
			}
		}

		if (gramatica[i] == '{') {								//caso haja uma abertura de chave, muda de estado
			permiteEstado = true;								
			estado++;			
		}
	}//fim do lexico

	/**
		Criacao do automato. Foi feito de forma separada do lexico, para facilitar 
		o entendimento do codigo e manipulacao dos containers
	**/
	Automato afn;
	afn.inicio = estado_inicial;

	for (int i = 0; i < variavel.size(); i++) {			 		//itera o vetor de variaveis que foram lidas
		Estado novo;
		novo.nome = variavel[i];								//para cadas variavel lida cria-se um novo estado
		for (auto it : regras) {								//itera o mapeameto de regras, salvando as transicoes desse estado
			if (it.first == novo.nome) {						
				novo.transicoes.push_back(it.second);
				novo.final = it.second.final;
			}
		}

		afn.S.push_back(novo);									//coloca a nova regra no afn
	}
	
	imprimeAutomato(afn, "Automato finito nao deterministico");

	/**
		Transformacao do afn em afd: foi utilizado uma fila, em que sao colocados os novos conjuntos de estados criados, caso
		estes ainda nao estejam no automato. 
	**/
	Automato afd;
	queue<vector<string> > q;									//fila onde sao colocados os conjuntos
	vector<string> inicio;										
	inicio.push_back(afn.inicio);								
	q.push(inicio);													//itera o vetor de variaveis que foram lidas

	map<string, int> alfabetoInt;								//mapeamento de alfabeto para um inteiro
	for (int i = 0; i < alfabeto.size(); i++) 
		alfabetoInt[alfabeto[i]] = i;

	while (!q.empty()) {
		Estado novo;											//novo estado para o afd
		string aux[alfabeto.size()];							//vetor de transicoes (tamanho do alfabeto, por ser deterministico)

		vector<string> topo = q.front(); q.pop();				//desenfileira o conjunto do inicio da fila

		for (int i = 0; i < topo.size(); i++) {					//itera nas variaveis do estado retirado da pilha
			string atual = topo[i];
			novo.nome += atual;									//o nome do novo estado sera o conjunto de variaveis
			for (int j = 0; j < afn.S.size(); j++) {			//itera os estados do afn
				if (afn.S[j].nome == atual) {					//se o estado for igual a variavel atual
					for (int k = 0; k < afn.S[j].transicoes.size(); k++) { 	//varre as transicoes desse estado
						if (afn.S[j].transicoes[k].destino != "#")		   	//se for para lambda nao considera
							aux[alfabetoInt[afn.S[j].transicoes[k].consome]] += afn.S[j].transicoes[k].destino;	
					}
				}
			}
		}


		bool existe;											//determina se o novo estado ja existe no afd	
		for (int i = 0; i < alfabeto.size(); i++) {				//itera o alfabeto para incluir os novos estados no afd
			existe = false;
			Transicao t;
			t.consome = alfabeto[i];							//a transicao consome essa "letra" do alfabeto
			t.destino = aux[i];									//define o destino
			novo.transicoes.push_back(t);						//coloca essa nova transicao na lista de estados

			for (int j = 0; j < afd.S.size(); j++) {			//itera nos estados ja exitentes do afd
				if (afd.S[j].nome == aux[i]) { 					
					existe = true; 								//caso ja exista esse estado
				}
			}

			if (!existe) {										//se o estado nao existe, ele deve ser colocado na fila
				vector<string> novoElemento;					//como cada variavel e considerada separadamente, cria um vetor com as variaveis desse estado
				for (int j = 0; j < aux[i].size(); j++) {
					string inc = "";
					inc += aux[i][j];							
					novoElemento.push_back(inc);
				}
				if (novoElemento.size() > 0)					//se o novo estado nao for vazio coloca na pilha
					q.push(novoElemento);							//para nao entrar em loop
			}
		}

		if (!existe)
			afd.S.push_back(novo);								//coloca o estado no afd
	}

	imprimeAutomato(afd, "Automato finito deterministico");

	return 0;
}

void imprimeAutomato(Automato aut, string tipo) {
	cout << endl << tipo << endl;
	cout << "Estado inicial: " << aut.inicio << endl;
	for (int i = 0; i < aut.S.size(); i++) {
		cout << "[" << aut.S[i].nome << "]: ";
		for (int j = 0; j < aut.S[i].transicoes.size(); j++) {
			cout << "lendo: " << aut.S[i].transicoes[j].consome << " vai para: " << 
					 "[" << aut.S[i].transicoes[j].destino << "] || ";
		}
		cout << endl;
	}
}