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

    //# Quebra de linha
    signed short OFFSET_BREAKLINE;

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

    //# Quebra de linha
    file >> c.OFFSET_BREAKLINE;

    // Strings
    file >> c.STR_CNPJ_EMISSOR;
    file >> c.STR_AGENCIA;

    file.close();

    return c;
}

// Grava linha no arquivo de saida
void writeFile(string line, ofstream *writer, unsigned long long *countLines) {

    *writer << line;
    *countLines++;
}

// Agrupa conteudo dos arquivos
void groupFiles(short offset, ofstream *writer, unsigned long long *countLines) {

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
    writeFile("},{\n", writer, countLines);
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

// Retira espacos em branco de uma string
string trim(const string& str) {

    size_t first = str.find_first_not_of(' ');
    size_t last = str.find_last_not_of(' ');

    return str.substr(first, (last - first + 1));
}

// Funcao "main"
int main(int argc, const char *argv[]) {

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

        // Contador de arquivos
        countFiles++;

        // Pula o cabecalho a partir do 2o arquivo
        jumpHeader = countFiles > 1 ? true : false;

        cout << "\nProcessando arquivo " << countFiles << " de " << files.size() << " (" << c.PATH << files[i] << ")\n";

        // Variavel para leitura de uma linha do arquivo
        string line;

        // Abre arquivo para leitura
        ifstream file(c.PATH + files[i]);

        // Le uma linha do arquivo
        getline(file, line);

        // Looping de processamento do arquivo
        while (! file.eof()) {

            // Verifica se linha possui a string informada
            size_t pos = line.find(c.STR_CNPJ_EMISSOR);

            // String "cnpjEmissor" encontrada
            if (pos != std::string::npos) {

                // Obtem chave: numero do "cnpjEmissor"            
                cnpjEmissorAtual = trim(line.substr(
                    pos + c.STR_CNPJ_EMISSOR.size(),
                    line.size() - (pos + c.STR_CNPJ_EMISSOR.size()) + c.OFFSET_BREAKLINE));

                // Chave atual == chave anterior
                if (cnpjEmissorAtual == cnpjEmissorAnterior) {

                    cout << "Agrupando contas para o emissor: " << cnpjEmissorAtual << endl;

                    // Pula cabecalho da empresa emissora
                    while (line.find(c.STR_AGENCIA) == std::string::npos) {

                        // Le uma linha do arquivo
                        getline(file, line);
                    }

                    // Agrupa arquivos JSON
                    groupFiles(c.OFFSET_AGENCIA, &writer, &countLines);

                // Chaves diferentes
                } else {

                    cout << "Incluindo novo emissor: " << cnpjEmissorAtual << endl;

                    // Atualiza chave
                    cnpjEmissorAnterior = cnpjEmissorAtual;

                    // Executa comandos a partir do 2o arquivo lido
                    if (countFiles > 1) {

                        // Agrupa arquivos JSON
                        groupFiles(c.OFFSET_CNPJ_EMISSOR, &writer, &countLines);
                    }
                }

                // Grava linha do arquivo de entrada para arquivo de saida
                writeFile(line, &writer, &countLines);

                // Atualiza flag para pular o cabecalho
                jumpHeader = false;

            // String "cnpjEmissor" nao encontrada
            } else {

                // Grava linha do arquivo de entrada para arquivo de saida
                // se flag para pular cabecalho igual a Falso
                if (! jumpHeader) {

                    writeFile(line, &writer, &countLines);
                }
            }

            // Le uma linha do arquivo
            getline(file, line);
        }

        // Fecha arquivo de entrada
        file.close();
    }

    // Fecha arquivo de saida
    writer.close();

    cout << "\nFim do processamento\n";

    return 0;
}