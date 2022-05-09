#include<iostream>
#include<string>
#include<cstring>

using namespace std;

int main(int argc, char *argv[]) {
//	FILE* cct = popen(" &> ./tmpspeck", "r");
	bool decrypt = false;
		string help[2] = {"-h", "--help"};
		string decryption[2] = {"-d", "--decrypt"};
		for (int i = 1; i < argc; i ++) {
			string input_param = "";
			int j = 0;
			do {
				input_param += argv[i][j];
				j++;
			}
			while (argv[i][j] != '\0');
			if (input_param.substr(0, 2) == help[0] || input_param.substr(0, min(static_cast<unsigned long> (input_param.size()), static_cast<unsigned long> (6))) == help[1]) {
				printf("Triggered help option. Ignoring other...\n");
				printf("This is lightweight utility to encode you file via speck cipher in cbc mode.\nSyntax: speck -h | {-p=pass [-d]]}\n    -h, --help                     show this message and exit\n    -d, --decrypt                  tells that you need decrypt file instead of default encryption\n");
				return 0;
			}
			if (input_param.substr(0, 2) == decryption[0] || input_param.substr(0, min(static_cast<unsigned long> (input_param.size()), static_cast<unsigned long> (9))) == decryption[1]) {
				decrypt = true;
			}
			else {
				fprintf(stderr, "The %s option is unknown. Try -h. Exiting...\n", argv[i]);
				return 1;
			}			
		}
	volatile string passphr;
	FILE* pass;
        if ((pass = popen("dmenu -fn '' -p passphrase: </dev/null", "r")) == NULL)
		return 1;
	unsigned char curchar = '\0';
	while (curchar != (unsigned char) EOF) {
		curchar = getc(pass);
		if (curchar == '\n' || curchar == '\0')
			break;
		passphr += curchar;
	}
	curchar = '\0';
	fclose(pass);
	if (passphr == "" || passphr[0] == EOF)
		return 1;
	volatile string cmd = (decrypt ? "dmenu -fn '' -p text\\ to\\ decrypt: </dev/null | hexifator -d | speck -p=" : "dmenu -fn '' -p text\\ to\\ encrypt: </dev/null | speck -p=") + passphr + (decrypt ? " -d | xdotool type -file -" : " | hexifator | xdotool type -file -");
	passphr.resize(0);
//	FILE* result = popen(cmd.c_str(), "r");
//	fclose(result);
	system(cmd.c_str());
	memset(passphr, '\0', passphr.size());
	passphr.resize(0);
	cmd.resize(0);
	return 0;
}
