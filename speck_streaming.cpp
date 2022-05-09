#include<iostream>
#include<cmath>
#include<cstdio>
#include<cstring>
#include<string>
#include<random>

#pragma -O3 -std=c++11

#define WORD 64
#define ROUNDS 34
#define ROUND(t1, t2, k) (t1 = (((t1 << (WORD - 8)) | (t1 >> 8)) + t2) ^ k, t2 = ((t2 << 3) | (t2 >> (WORD - 3))) ^ t1)
#define REVERSE_ROUND(t1, t2, k) (t2 = (((t1 ^ t2) >> 3) | ((t1 ^ t2) << (64 - 3))), t1 = (((t1 ^ k) - t2) << 8) | (((t1 ^ k) - t2) >> 64 - 8))
	
#define CREATE_KEY(passphrase, key) \
	for (int i = 0; i < 4; i ++) {\
		key[i] = 0;\
	}\
	for (int i = 0; i < min(static_cast<unsigned long> (passphrase.size()), static_cast<unsigned long> (32)); i ++) {\
		key[i / 8] = (key[i / 8] << 8) | passphrase[i];\
	}\
	for (int i = 0; i < ROUNDS; i ++) {\
		ROUND(key[0], key[1], i);\
		ROUND(key[2], key[3], i);\
		ROUND(key[0], key[3], key[i % 4]);\
		ROUND(key[1], key[2], key[(i + 2) % 4]);\
	}

using namespace std;

void cbc_encrypt(unsigned long long init[2], unsigned long long key[4], FILE* input, FILE* output) {
	unsigned long long block[2];
	unsigned long long block_previous[2];
	unsigned long long round_keys[ROUNDS];
	unsigned char curchar;
	unsigned char count = 0;
	for (int i = 0; i < ROUNDS; i ++) {
		round_keys[i] = key[3];
		ROUND(key[i % 3], key[3], i);
	}
	
	block_previous[0] = init[0];
	block_previous[1] = init[1];

	block[0] = 0;
	block[1] = 0;

	fwrite(block_previous, sizeof(unsigned long long), 2, output);

	while(!feof(input)) {
		curchar = getc(input);
		if (feof(input)) {
			break;
		}
		block[count / 8] = (block[count / 8] << 8) | ((unsigned long long) (curchar));

//		cout<< endl << (int) count << '\t' << count / 8 << '\t' << curchar << endl;

		count ++;
		if (count > 15) {
			block[0] ^= block_previous[0];
			block[1] ^= block_previous[1];
			for (int i = 0; i < ROUNDS; i ++) {
				ROUND(block[0], block[1], round_keys[i]);
			}
			fwrite(block, sizeof(unsigned long long), 2, output);
		//	putc('|', output);
			count = 0;
			block_previous[0] = block[0];
			block_previous[1] = block[1];
			block[0] = 0;
			block[1] = 0;
		//	cout << endl;
		}
	

	}
	if (count == 0) {
		block[1] = 1ULL << 63;
	}
	else {
		block[count / 8] <<= (8 * (8 - count % 8));
		block[(count == 7) ? 1 : (count / 8)] |= 1ULL << ((count == 7) ? 63 : (8 * (8 - count % 8) - 1));
	}

	block[0] ^= block_previous[0];
	block[1] ^= block_previous[1];
	for (int i = 0; i < ROUNDS; i ++) {
		ROUND(block[0], block[1], round_keys[i]);
	}
	fwrite(block, sizeof(unsigned long long), 2, output);
}

void cbc_decrypt(unsigned long long key[4], FILE* input, FILE* output) {
	unsigned long long block[2];
	unsigned long long block_previous[2];
	unsigned long long block_encrypted[2];
	unsigned long long round_keys[ROUNDS];
	unsigned char curchar;
	unsigned char count = 0;
	bool breaked = false;
	bool nfirst = false;
	for (int i = 0; i < ROUNDS; i ++) {
			round_keys[i] = key[3];
			ROUND(key[i % 3], key[3], i);
	}

	fread(block_previous, sizeof(unsigned long long), 2, input);

	while (!feof(input)) {
		fread(block_encrypted, sizeof(unsigned long long), 2, input);
		if (feof(input)) {
			count = 0;
			while(block[1 - count / 64] % 2 == 0) {
				block[1 - count / 64] >>= 1;
				count ++;
			}
			block[1 - count / 64] >>= 1;
			count++;
			breaked = true;
		}

	//	putc('|', output);
	//	cout << (int) count << endl;

		if (nfirst) {
			for (int i = 0; i < 16 - count / 8; i ++) {
				curchar = (unsigned char) ((block[i / 8]) >> (8 * (7 - i % 8) - ((count <= 64 && i < 8) ? 0 : count % 64)));
				fputc(curchar, output);
			}
		}
		if (breaked) {
			break;
		}
		block[0] = block_encrypted[0];
		block[1] = block_encrypted[1];
		for (int i = 0; i < ROUNDS; i ++) {
			REVERSE_ROUND(block[0], block[1], round_keys[ROUNDS - 1 - i]);
		}
		block[0] ^= block_previous[0];
		block[1] ^= block_previous[1];
		block_previous[0] = block_encrypted[0];
		block_previous[1] = block_encrypted[1];
		nfirst = true;
	}
}


void create_key_file(FILE* keyfile, string passphrase) {
	volatile unsigned long long key[4];
	CREATE_KEY(passphrase, key);
	fwrite(key, sizeof(unsigned long long), 4, keyfile);
	for (int i = 0; i < 4; i ++) {
		key[i] = 0;
	}
}
//-----------------------------------------------------------------------------------------

int main(int argc, char *argv[]) {
	bool key_given = false;
	bool flag = false;
	string keyfilename = "";
	string new_keyfilename = "";
	volatile string pass = "";
	if (argc > 1) {
		string help[2] = {"-h", "--help"};
		string decryption[2] = {"-d", "--decrypt"};
		string key[2] = {"-k=", "--key="};
		string passphrase[2] = {"-p=", "--passphrase="};
		string create[2] = {"-c=", "--create="};
		for (int i = 1; i < argc; i ++) {
		//	cout << mode[0].substr(0, 3).c_str() << '\t' << strcmp(argv[i], mode[0].substr(0, 3).c_str()) << '\t' << strstr(argv[i], mode[0].c_str()) << endl;
			string input_param = "";
		//	cout << sizeof(*argv[i]) << '\t' << '\0' << endl;
			int j = 0;
			do {
			//	cout << argv[i][j] << endl;
				input_param += argv[i][j];
				j++;
			}
			while (argv[i][j] != '\0');
		//cout << input_param.size() << endl;
		//	cout << input_param.substr(0, 2) << endl;
		//	cout <<  input_param.substr(0, min(input_param.size(), (unsigned long) 7)) << endl;
			if (input_param.substr(0, 2) == help[0] || input_param.substr(0, min(static_cast<unsigned long> (input_param.size()), static_cast<unsigned long> (6))) == help[1]) {
				cout << "Triggered help option. Ignoring other..." << endl;
				cout << "This is lightweight utility to encode you file via speck cipher in cbc mode.\nSyntax: speck -h | {{-p=pass [-c=output/key/file/name]} | -k=key/file/name [-d]]}\n    -h, --help                     show this message and exit\n    -d, --decrypt                  tells that you need decrypt file instead of default encryption\n    -p, --passphrase=pass          specify passphrase\n    -c, --create=output/file/name  creates a keyfile out of passphrase (Ignored if passphrase is not specified)\n    -k, --key=key/file/name        specify key file" << endl;
				return 0;
			}
			if (input_param.substr(0, 2) == decryption[0] || input_param.substr(0, min(static_cast<unsigned long> (input_param.size()), static_cast<unsigned long> (9))) == decryption[1]) {
				flag = true;
			}
			else if (input_param.substr(0, 3) == key[0] || input_param.substr(0, min(static_cast<unsigned long> (input_param.size()), static_cast<unsigned long> (6))) == key[1]) {
				key_given = true;
				int eq = input_param.find('=');
				string tmp = input_param.substr(eq + 1, input_param.size());
				//cout << tmp << endl;
				keyfilename += tmp;
			}
			else if (input_param.substr(0, 3) == passphrase[0] || input_param.substr(0, min(static_cast<unsigned long> (input_param.size()), static_cast<unsigned long> (13))) == passphrase[1]) {
				int eq = input_param.find('=');
				string tmp = input_param.substr(eq + 1, input_param.size());
				//cout << tmp << endl;
				pass += tmp;
				tmp.resize(0);
			}
			else if (input_param.substr(0, 3) == create[0] || input_param.substr(static_cast<unsigned long> (0), min(static_cast<unsigned long> (input_param.size()), static_cast<unsigned long> (13))) == create[1]) {
				int eq = input_param.find('=');
				string tmp = input_param.substr(eq + 1, input_param.size());
				//cout << tmp << endl;
				new_keyfilename += tmp;
			}
			else {
				cerr << "The " << argv[i] << " option is unknown. Try -h. Exiting..." << endl;
				return 1;
			}			
		}
	}
	else {
		cerr << "You have not specify anything. Exiting..." << endl;
		return 1;
	}

	if (!key_given && pass == "") {
		cerr << "You give not enought information. Try -h. Exiting..." << endl;
		return 1;
	}

	if (new_keyfilename != "") {
		if (pass == "") {
			cerr << "You have not give passphrase. Exiting..." << endl;
			return 1;
		}
		else {
			FILE* new_keyfile; 
        		if ((new_keyfile = fopen(new_keyfilename.c_str(), "wb")) == NULL) {
				cerr << "Error openeing new keyfile. Exiting..." << endl;
				return 1;
			}
			unsigned long long key[4];
			CREATE_KEY(pass, key);
			fwrite(key, sizeof(unsigned long long), 4, new_keyfile);
			fclose(new_keyfile);
		}
	}

	unsigned long long key[4];

	if (pass != "") {
		CREATE_KEY(pass, key);
	}
	else {
		FILE* keyfile;
        	if ((keyfile = fopen(keyfilename.c_str(), "rb")) == NULL) {
			cerr << "Error openeing keyfile. Exiting..." << endl;
			return 1;
		}
		fread(key, sizeof(unsigned long long), 4, keyfile);
		fclose(keyfile);
	}
//cout << key[0] << '\t' << key[1] << '\t' << key[2] << '\t' << key[3] << endl;
	if (!flag) {
		FILE* random;
		if ((random = fopen("/dev/urandom", "rb")) == NULL) {
			cerr << "Error opening urandom. Exiting..." << endl;
			return 1;
		}
		unsigned long long init[2]
			;
		// = {1ULL << 63, 1ULL};
		fread(init, sizeof(unsigned long long), 2, random);
		cbc_encrypt(init, key, stdin, stdout);
		fclose(random);
	}
	else {
		cbc_decrypt(key, stdin, stdout);
	}

	pass.resize(0);
	return 0;
}
