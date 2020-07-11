import glob

#############################################
##              Configuracao               ##
#############################################

# Diretorio dos arquivos
path = './'

# Prefixo dos arquivos de entrada
pattern = '{}teste*.json'.format(path)

# Nome do arquivo de saida
writername = '{}saida.json'.format(path)

# Offsets

# Definem quantos bytes serao apagados do arquivo de saida para agrupamento
offsetCnpjEmissor = 18
offsetAgencia = 38

# Quebra de linha
offsetBreakline = -3

# Strings
strCnpjEmissor = '\"cnpjEmissor\":'
strAgencia = '\"agencia\":'

#############################################
#############################################
#############################################

# Chaves: numero do "cnjpjEmissor"
cnpjEmissorAnterior = None
cnpjEmissorAtual = None

# Contadores
countLines = 0
countFiles = 0

# Abre arquivo de saida para escrita
writer = open(writername, 'w+')

# Grava linha no arquivo de saida
def writeFile(line):

    global countLines

    writer.write(line)
    countLines += 1

# Agrupa conteudo dos arquivos
def groupFiles(offset):

    # Deleta tags de fechamento do JSON
    writer.seek(writer.tell() - offset, 0)

    # Grava nova tag
    writeFile('},{\n')

print('Inicio do processamento\n')

# Seleciona arquivos para processamento
files = glob.glob(pattern)

print('Selecionados {} arquivos para processamento'.format(len(files)))

# Looping dos arquivos
for filename in files:

    # Contador de arquivos
    countFiles += 1

    # Pula o cabecalho a partir do 2o arquivo
    jumpHeader = True if countFiles > 1 else False

    print('\nProcessando arquivo {} de {} ({})'.format(countFiles, len(files), filename))

    # Abre arquivo para leitura
    file = open(filename, 'r')

    # Le uma linha do arquivo
    line = file.readline()

    # Looping de processamento do arquivo
    while line:

        # Verifica se linha possui a string informada
        pos = line.find(strCnpjEmissor)

        # String "cnpjEmissor" encontrada
        if pos > -1:

            # Obtem chave: numero do "cnpjEmissor"            
            cnpjEmissorAtual = str(line)[pos + len(strCnpjEmissor):offsetBreakline].strip()

            # Chave atual == chave anterior
            if cnpjEmissorAtual == cnpjEmissorAnterior:

                print('Agrupando contas para o emissor: {}'.format(cnpjEmissorAtual))

                # Pula cabecalho da empresa emissora
                while line.find(strAgencia) < 0:
                    line = file.readline()

                # Agrupa arquivos JSON
                groupFiles(offsetAgencia)

            # Chaves diferentes
            else:

                print('Incluindo novo emissor: {}'.format(cnpjEmissorAtual))

                # Atualiza chave
                cnpjEmissorAnterior = cnpjEmissorAtual

                # Executa comandos a partir do 2o arquivo lido
                if countFiles > 1:

                    # Agrupa arquivos JSON
                    groupFiles(offsetCnpjEmissor)

            # Grava linha do arquivo de entrada para arquivo de saida
            writeFile(line)

            # Atualiza flag para pular o cabecalho
            jumpHeader = False

        # String "cnpjEmissor" nao encontrada
        else:

            # Grava linha do arquivo de entrada para arquivo de saida
            # se flag para pular cabecalho igual a Falso
            if not jumpHeader:
                writeFile(line)

        # Le uma linha do arquivo de entrada
        line = file.readline()

    # Fecha arquivo de entrada
    file.close()

# Fecha arquivo de saida
writer.close()

print('\nFim do processamento')