#include <bitset>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <string.h>

using namespace std;

/*

EXEMPLO
./hamming teste.txt -w
(gera um arquivo binario do codigo de hamming teste.txt.wham)
(alterar valores de teste.txt.wham no editor hexadecimal)

./hamming teste.txt.wham -r
(gera um arquivo binario fix_teste.txt.wham que é a correção de teste.txt.wham)
(tambem gera um arquivo fix_teste.txt, que deve ter o mesmo conteudo de teste.txt, caso a correção tenha funcionado)

*/


//arquivo -w sem .wham
//recebe o arquivo sem .wham e devolve com .wham
string outName(string filename)
{
    string outFilename;
    outFilename.append(filename);
    outFilename.append(".wham");
    return outFilename;
}

//arquivo -r com .wham
//recebe o arquivo com .wham e devolve string sem .wham
string inName(string filename)
{
    string inFilename;
    inFilename.append(filename);
    cout << inFilename << endl;
    for (int i = filename.length()-1;  ; i--)
    {
        inFilename.pop_back();
        if(filename.at(i) == '.')
            break;
    }
    
    return inFilename;
}
//arquivo -r com .wham

// verifica se é potência de 2
bool isPowOf2(int v) 
{ 
    return v && !(v & (v - 1));
}

class hamming
{
private:
    bitset<8> M;
    bitset<4> C; 
    bitset<1> G;
    bitset<13> ham;

public:
    hamming(){};
    //constroi a palavra de 13 bits a partir de m;
    void encodeHamming(bitset<8> byte)
    {
        this->M = byte;
        C.set(0, (byte[6] ^ byte[4] ^ byte[3] ^ byte[1] ^ byte[0]));
        C.set(1, (byte[6] ^ byte[5] ^ byte[3] ^ byte[2] ^ byte[0]));
        C.set(2, (byte[7] ^ byte[3] ^ byte[2] ^ byte[1]));
        C.set(3, (byte[7] ^ byte[6] ^ byte[5] ^ byte[4]));

        newG();
        int cI ;
        int mI ;
        cI = mI = 0;
        ham[0] = G[0];
        for (int i = 1; i < 13; i++)
        {
            if (isPowOf2(i))
                ham[i] = C[cI++];
            else
                ham[i] = M[mI++];
        }

    };
    //constroi m, c e g a partir de uma palavra de 13 bits
    void decodeHamming(bitset<13> ham)
    {
        this->ham = ham;
        G[0] = ham[0];
        int cI ;
        int mI ;
        cI = mI = 0;
        for (int i = 1; i < 13; i++)
        {
            if (isPowOf2(i))
                C[cI++]=ham[i];
            else
                M[mI++]=ham[i];
        }
    }
    //calcula G
    void newG()
    {
        G[0] = (M[7] ^ M[6] ^ M[5] ^ M[4] ^ C[3] ^ M[3] ^ M[2] ^ M[1] ^ C[2] ^ M[0] ^ C[1] ^ C[0]);
        ham[0]=G[0];
    }

    bitset<13> hamFrom2Bytes(char *bytes)
    {
        bitset<8> byte1(bytes[0]);
        bitset<8> byte2(bytes[1]);
        bitset<13> ham;

        for (int i = 0; i < 8; i++)
            ham.set(i,byte1[i]);
        int j = 0;
        for (int i = 8 ; i < 13; i++,j++)
            ham.set(i,byte2[j]);
        decodeHamming(ham);
        this->ham = ham;
        return ham;
    }

    bitset<8> getM()
    {
        return M;
    }
    bitset<4> getC()
    {
        return C;
    }
    bitset<1> getG()
    {
        return G;
    }
    bitset<13> getHam()
    {
        return ham;
    }

    void print()
    {
        cout << "M = " << getM() << endl;
        cout << "C = " << getC() << endl;
        cout << "G = " << getG() << endl;
        cout << "Hamming = " << getHam() << endl;
    }
};

//se bool == false, palavra correta, retorna a palavra sindrome
pair<bool, bitset<4>> testC(bitset<13> set)
{
    //hw vem do arquivo
    hamming hw;
    hw.decodeHamming(set);
    //gera hamming novamente a partir do M de hw
    hamming test;
    test.encodeHamming(hw.getM());

    //calcula a palavra sindrome
    bitset<4> syndrome;
    syndrome.set(0, (test.getC()[0] ^ hw.getC()[0]));
    syndrome.set(1, (test.getC()[1] ^ hw.getC()[1]));
    syndrome.set(2, (test.getC()[2] ^ hw.getC()[2]));
    syndrome.set(3, (test.getC()[3] ^ hw.getC()[3]));

    //dados M corretos
    if(syndrome.to_ulong() == 0)
    {
        cout << "Palavra sindrome == 0!" << endl;
        return {false, syndrome};
    }
    //impossivel corrigir
    else if (syndrome.to_ulong() > 12)
    {
        throw "Valor da palavra sindrome maior do que 12, impossível corrigir!";
        exit(1);
    }
    //palavra sindrome diferente de 0 e < 12, necessário corrigir!
    return {false, syndrome};
}

//retorna true se correto
bool testG(bitset<1> a, bitset<1> b)
{
    bitset<1> g = a ^ b;
    return (g.to_ulong() == 0);
}


//-w
//Grava o arquivo hamming
void write(string inFilename) 
{
    ifstream inFile;
    ofstream outFile;

    cout << "Começando a gravar..." << endl;

    string outFilename = outName(inFilename);

    inFile.open(inFilename, ios::binary | ios::in);
    outFile.open(outFilename, ios::binary | ios::out);

    if (!inFile.is_open() || !outFile.is_open())
        throw "Error opening file";

    char byte;
    bitset<8> b;
    hamming hw;
    int count=0;
    while (!inFile.eof()) {
        inFile.read(&byte, 1);
        if (inFile.eof())
            break;

        b = byte;
        hw.encodeHamming(b);

        cout << "\nByte " << count++ << ": " << endl;
        hw.print();

        bitset<13> ham = hw.getHam();
        outFile.write((char*)&ham, 2);
    }

    inFile.close();
    outFile.close();

    cout << "Gravacao completa!" << endl;
}

//le um arquivo wham e gera um txt
void writeTXT(string filename)
{
    ifstream whamFile;
    ofstream tocorrectFile;
    char buffer[2];
    hamming HW;
    
    string outFilename = inName(filename);

    whamFile.open(filename, ios::binary | ios::in);
    tocorrectFile.open(outFilename, ios::binary | ios::out);

    if (!whamFile.is_open() || !tocorrectFile.is_open())
        throw "Error opening file";

    while (!whamFile.eof()) 
    {
        //lê dois bytes do .wham
        whamFile.read((char *)&buffer, 2);
        if (whamFile.eof())
            break;
        //gera hamming a partir de dois bytes do arquivo
        HW.hamFrom2Bytes(buffer);
        //hamming do arquivo
        bitset<8> fixedWord = HW.getM();
        tocorrectFile.write((char *)&fixedWord, sizeof(char));
    }
    tocorrectFile.close();
    whamFile.close();

    cout << "Leitura do arquivo terminada! Arquivo txt corrigido gerado!" << endl;
}


//-r
//le o arquivo .wham e corrige caso adulterado
//salva o arquivo com a correção com o sufixo fix_
void read(string filename)
{
    ifstream whamFile;
    ofstream newWhamFile;
    char buffer[2];
    hamming HW;
    hamming fix;


    cout << "Começando leitura..." << endl;
    string outFilename;
    outFilename.append("fix_");
    outFilename.append(filename);

    whamFile.open(filename, ios::binary | ios::in);
    newWhamFile.open(outFilename, ios::binary | ios::out);

    if (!whamFile.is_open() || !newWhamFile.is_open())
        throw "Error opening file";

    while (!whamFile.eof()) 
    {
        //lê dois bytes do .wham
        whamFile.read((char *)&buffer, 2);
        if (whamFile.eof())
            break;

        //gera hamming a partir de dois bytes do arquivo
        HW.hamFrom2Bytes(buffer);

        //hamming do arquivo
        cout << "Palavra do arquivo = " << HW.getM() << endl;
        cout << "Hamming palavra do arquivo = " << HW.getHam() << endl;

        //faz o testeC
        pair<bool, bitset<4>> syndrome = testC(HW.getHam());

        cout << "Palavra sindrome = " << syndrome.second << endl;

        if (syndrome.first == true)
        {
            throw "Arquivo corrompido!";
            exit(1);
        }
        else if (syndrome.second.to_ulong() == 0)
        {
            cout << "M está correto!" << endl;
            fix.decodeHamming(HW.getHam());
        }
        else
        {
            //flipa o bit na posição da pavra sindrome
            bitset<13> fromfile = HW.getHam();
            fromfile.flip(syndrome.second.to_ulong());
            //recupera M, C e G depois da correção
            fix.decodeHamming(fromfile);
            fix.newG();
            //faz o teste G
            if (testG(fix.getG(),HW.getG())==false)
            {
                throw "Teste G rejeitado!";
                exit(1);
            }
            else
            {
                cout << "Palavra M corrigida! Gravando palavra corrigida no arquivo...\n";
                cout << "Hamming palavra corrigida = " << fix.getHam() << endl;
                cout << "Palavra corrigida = " << fix.getM() << endl;
            }
        }
        bitset<13> fixedHam = fix.getHam();
        newWhamFile.write((char*)&fixedHam, 2);

        cout << "==========================================\n"; 
    }
    newWhamFile.close();
    whamFile.close();

    writeTXT(outFilename);
    cout << "Leitura do arquivo terminada! Arquivo corrigido " << outFilename << " gerado!" << endl;
}


int main(int argc, char const *argv[])
{
    if(!strcmp(argv[2],"-r"))
    {
        read(argv[1]);
    }
    else if(!strcmp(argv[2],"-w"))
    {
        write(argv[1]);
    }
    else
        cout << "Entrada inválida!\n";

    return 0;
}
