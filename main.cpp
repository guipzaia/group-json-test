#include <iostream>
#include <fstream>
#include <dirent.h>
#include <bits/stdc++.h> 
#include <regex>

using namespace std;

// Configuracao do sistema
struct config {

    // Diretorio dos arquivos
    string PATH;

    // Prefixo dos arquivos de entrada
    string PATTERN;

    // Nome do arquivo de saida
    string WRITERNAME;

    // Offsets

    // Definem quantos bytes serao apagados do arquivo de saida para agrupamento
    unsigned short OFFSET_CNPJ_EMISSOR;
    unsigned short OFFSET_AGENCIA;

    // Strings
    string STR_CNPJ_EMISSOR;
    string STR_AGENCIA;
};

// Carrega as constantes do programa
config loadConstants(string filepath) {

    ifstream file(filepath);

    config c;

    // Diretorio dos arquivos
    file >> c.PATH;

    // Prefixo dos arquivos de entrada
    file >> c.PATTERN;

    // Nome do arquivo de saida
    string aux;
    file >> aux;
    c.WRITERNAME = c.PATH + aux;

    // Offsets

    // Definem quantos bytes serao apagados do arquivo de saida para agrupamento
    file >> c.OFFSET_CNPJ_EMISSOR;
    file >> c.OFFSET_AGENCIA;

    // Strings
    file >> c.STR_CNPJ_EMISSOR;
    file >> c.STR_AGENCIA;

    file.close();

    return c;
}

// Grava linha no arquivo de saida
void writeFile(string line, ofstream *writer) {

    *writer << line;
}

// Agrupa conteudo dos arquivos
void groupFiles(short offset, ofstream *writer) {

    // Casting para Linux
    #ifdef linux
        long castedOffset = (long) offset;
    #endif

    // Casting para Windows
    #ifdef _WIN32
        long long castedOffset = (long long) offset;
    #endif

    // Deleta tags de fechamento do JSON
    writer->seekp(writer->tellp() - castedOffset, ios::beg);

    // Grava nova tag
    writeFile("},{", writer);
}

// Seleciona arquivos para processamento
vector<string> glob(string path, string pattern) {

    vector<string> files;

    DIR *dir;

    struct dirent *en;

    // Abre o diretorio
    if (dir = opendir(path.c_str())) {

        // Iteracao com os arquivos do diretorio
        while ((en = readdir(dir)) != NULL) {

            // Verifica se arquivo coincide com o PATTERN
            if (regex_match(en->d_name, regex(pattern))) {

                // Inclui arquivo no vetor
                files.push_back(en->d_name);
            }
        }

        // Fecha o diretorio
        closedir(dir);
    }

    return files;
}

// Separa uma string de acordo com os limitadores
vector<string> split(string str, string delim) {

    string::size_type pos;

    vector<string> tokens;

    // Retorna a posicao do caracter delimitador
    while ((pos = str.find_first_of(delim)) != std::string::npos) {

        // Adiciona string (linha) do vetor
        tokens.push_back(str.substr(0, pos ? pos : 1));

        // Apaga string (linha) gravada
        str.erase(0, pos ? pos : 1);
    }

    return tokens;
}

// Le uma linha do arquivo (vetor)
string readLine(vector<string> lines, unsigned long long& countLines) {

    countLines++;

    // Retorna proxima linha ou "" (EOF)
    return countLines < lines.size() ? lines[countLines] : "";
}

// Retira espacos em branco de uma string
string trim(const string& str) {

    size_t first = str.find_first_not_of(' ');
    size_t last = str.find_last_not_of(' ');

    return str.substr(first, (last - first + 1));
}

// Funcao "main"
int main(int argc, const char *argv[]) {

    // Obter arquivo de configuracao automaticamente caso nao tenha argumentos
    if (argc == 1) {
        argv[1] = "./args.txt";
    }

    // Carrega as constantes do programa
    config c = loadConstants(argv[1]);

    // Chaves: numero do "cnjpjEmissor"
    string cnpjEmissorAnterior = "";
    string cnpjEmissorAtual = "";

    // Contadores
    unsigned long long countLines = 0;
    unsigned long countFiles = 0;

    // Pula cabecalho
    bool jumpHeader;

    // Abre arquivo de saida para escrita
    ofstream writer(c.WRITERNAME);

    cout << "Inicio do processamento\n";

    // Seleciona arquivos para processamento
    vector<string> files = glob(c.PATH, c.PATTERN);

    cout << "\nSelecionados " << files.size() << " arquivos para processamento\n";

    // Looping dos arquivos
    for (int i = 0; i < files.size(); i++) {
        
        // Contador de linhas
        countLines = 0;

        // Contador de arquivos
        countFiles++;

        // Pula o cabecalho a partir do 2o arquivo
        jumpHeader = countFiles > 1 ? true : false;

        cout << "\nProcessando arquivo " << countFiles << " de " << files.size() << " (" << c.PATH << files[i] << ")\n";

        // Variavel para leitura de uma linha do arquivo
        string json;

        // Abre arquivo para leitura
        ifstream file(c.PATH + files[i]);

        // Le uma linha do arquivo
        getline(file, json);

        // Fecha arquivo de entrada
        file.close();

        // Separa conteudo do arquivo em linhas
        vector<string> lines = split(json, "{[]},");

        // Le uma linha
        string line = lines[countLines];

        // Looping de processamento do arquivo
        while (line != "") {

            // Verifica se linha possui a string informada
            size_t pos = line.find(c.STR_CNPJ_EMISSOR);

            // String "cnpjEmissor" encontrada
            if (pos != std::string::npos) {

                // Obtem chave: numero do "cnpjEmissor"            
                cnpjEmissorAtual = trim(line.substr(pos + c.STR_CNPJ_EMISSOR.size(), line.size()));

                // Chave atual == chave anterior
                if (cnpjEmissorAtual == cnpjEmissorAnterior) {

                    cout << "Agrupando contas para o emissor: " << cnpjEmissorAtual << endl;

                    // Pula cabecalho da empresa emissora
                    while (line.find(c.STR_AGENCIA) == std::string::npos) {

                        // Le uma linha do arquivo
                        line = readLine(lines, countLines);
                    }

                    // Agrupa arquivos JSON
                    groupFiles(c.OFFSET_AGENCIA, &writer);

                // Chaves diferentes
                } else {

                    cout << "Incluindo novo emissor: " << cnpjEmissorAtual << endl;

                    // Atualiza chave
                    cnpjEmissorAnterior = cnpjEmissorAtual;

                    // Executa comandos a partir do 2o arquivo lido
                    if (countFiles > 1) {

                        // Agrupa arquivos JSON
                        groupFiles(c.OFFSET_CNPJ_EMISSOR, &writer);
                    }
                }

                // Grava linha do arquivo de entrada para arquivo de saida
                writeFile(line, &writer);

                // Atualiza flag para pular o cabecalho
                jumpHeader = false;

            // String "cnpjEmissor" nao encontrada
            } else {

                // Grava linha do arquivo de entrada para arquivo de saida
                // se flag para pular cabecalho igual a Falso
                if (! jumpHeader) {

                    writeFile(line, &writer);
                }
            }

            // Le uma linha do arquivo
            line = readLine(lines, countLines);
        }
    }

    // Fecha arquivo de saida
    writer.close();

    cout << "\nFim do processamento\n";

    return 0;
}