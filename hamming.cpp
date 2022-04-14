#include <bitset>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <string.h>


using namespace std;

// verifica se é potência de 2
bool isPowOf2(int n) { return (n & (n - 1)) == 0; }

class hamming
{
private:
    bitset<8> M;
    bitset<4> C; 
    bitset<1> G;
    bitset<13> wham;
    int cI ;
    int mI ;

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

        G = (byte[7] ^ byte[6] ^ byte[5] ^ byte[4] ^ C[3] ^ byte[3] ^ byte[2] ^ byte[1] ^ C[2] ^ byte[0] ^ C[1] ^ C[0]);

        cI = mI = 0;
        wham[0] = G[0];
        for (int i = 1; i < 13; i++)
        {
            if (isPowOf2(i))
                wham[i] = C[cI++];
            else
                wham[i] = M[mI++];
        }

    };
    //constroi m, c e g a partir de uma palavra de 13 bits
    void decodeHamming(bitset<13> wham)
    {
        this->wham = wham;
        G[0] = wham[0];
        cI = mI = 0;
        for (int i = 1; i < 13; i++)
        {
            if (isPowOf2(i))
                C[cI++]=wham[i];
            else
                M[mI++]=wham[i];
        }
    }

    bitset<13> whamFrom2Bytes(char *bytes)
    {
        bitset<8> byte1(bytes[0]);
        bitset<8> byte2(bytes[1]);
        bitset<13> wham;

        for (int i = 0; i < 8; i++)
            wham.set(i,byte1[i]);
        int j = 0;
        for (int i = 8 ; i < 13; i++,j++)
            wham.set(i,byte2[j]);
        decodeHamming(wham);
        this->wham = wham;
        return wham;
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
    bitset<13> getWham()
    {
        return wham;
    }
};

//se bool == false, palavra correta
pair<bool, hamming> testC(bitset<13> set)
{
    hamming hw;
    hw.decodeHamming(set);
    hamming test;
    test.encodeHamming(hw.getM());

    bitset<4> syndrome = test.getC() ^ hw.getC();
    //dados M corretos
    if(syndrome.to_ulong() == 0)
        return {false, hw};
    //impossivel corrigir
    if (syndrome.to_ulong() > 12)
    {
        cout << "valor da palavra sindrome maior do que 12, impossível corrigir!" << endl;
        return {false, test};
    }
    //corige e retorna a palavra corrigida
    //flipa o bit na posiçao da palavra sindrome
    set.flip(syndrome.to_ulong());
    hamming fixedSet;
    fixedSet.decodeHamming(set);
    return {false, fixedSet};
}

//retorna true se correto
bool testG(bitset<13> set)
{
    hamming hw;
    hw.decodeHamming(set);
    hamming test;
    test.encodeHamming(hw.getM());
    bitset<1> g = hw.getG() ^ test.getG();
    return (g.to_ulong() == 0);
}

// se bool == false, palavra correta
pair<bool, bitset<8>> correction(bitset<13> set)
{
    bitset<8> aux;
    pair<bool, hamming> fixing = testC(set);

    //erro test c
    if (fixing.first == true)
        return {true,aux};
    
    hamming hw;
    hw.encodeHamming(fixing.second.getM());

    //erro no teste G
    if(testG(hw.getWham()) == false)
    {
        cout << "G incompatível!" << endl;
        return {true, aux};
    }

    return {false, hw.getM()};
}

//-w
void write(string outFilename) {
    ifstream inFile;
    ofstream outFile;
    char buffer;
    bitset<8> b;
    hamming hw;

    cout << "Começando a gravar..." << endl;

    inFile.open("teste.txt", ios::binary | ios::in);
    outFile.open(outFilename, ios::binary | ios::out);

    if (!inFile.is_open() || !outFile.is_open())
        throw "Error opening file";

    while (!inFile.eof()) {
        inFile.read(&buffer, 1);
        if (inFile.eof())
            break;

        b = buffer;

        hw.encodeHamming(b);
        bitset<13> wham = hw.getWham();
        outFile.write((char *)&wham, 2);
    }

    inFile.close();
    outFile.close();

    cout << "gravacao completa!" << endl;
}

//-r
void read(string filename)
{
    ifstream inFile;
    ofstream outFile;
    char buffer[2];
    hamming HW;

    cout << "Começando leitura..." << endl;

    inFile.open(filename, ios::binary | ios::in);
    outFile.open("teste.txt", ios::binary | ios::out);

    if (!inFile.is_open() || !outFile.is_open())
        throw "Error opening file";

    while (!inFile.eof()) {
        inFile.read((char *)&buffer, 2);
        if (inFile.eof())
            break;

        HW.whamFrom2Bytes(buffer);
        

        pair<bool, bitset<8>> correctionResult = correction(HW.getWham());
        if (correctionResult.first == true) {
            cout << "Impossível corrigir!" << endl;
            continue;
        }

        bitset<8> fixedWord = correctionResult.second;
        outFile.write((char *)&fixedWord, sizeof(char));
    }
    outFile.close();
    inFile.close();

    cout << "leitura do arquivo terminada! "<<endl << "Arquivo corrigido gerado!" << endl;
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
