#include<iostream>

#define DELIMETER '/'

using namespace std;

void hex_t_raw (FILE* input, FILE* output) {
	unsigned char in = 0;
	unsigned char out = 0;
	bool flag = false;
	while (!feof(input)) {
		in = getc(input);
		if (((in < 48 || in > 57) && (in < 65 || in > 72)))
			continue;
		in -= (( in < 58) ? 48 : 55);
		out = (out << 4) | in;
		if (flag) {
			putc(out, output);
		}
		flag = !flag;
	}
}

void raw_to_hex (FILE* input, FILE* output) {
	unsigned char in;
	unsigned char out[2];
	out[0] = 0;
	out[1] = 0;
	while (!feof(input)) {
		in = getc(input);
		out[0] = ((unsigned char) (in << 4) >> 4) + ((((unsigned char) (in << 4) >> 4) < 10) ? 48 : 55);
		out[1] = (in >> 4) + (((in >> 4) < 10) ? 48 : 55);
		putc(DELIMETER, output);
		putc(out[1], output);
		putc(out[0], output);
	}
}

int main(int argc, char* argv[]) {
	FILE* in = stdin;
	FILE* out = stdout;
	char reverse[2] = {'-', 'd'};
	if (argc == 1) 
		raw_to_hex(in, out);
	else if (argc == 2 && argv[1][0] == reverse[0] && argv[1][1] == reverse[1])
		hex_t_raw(in, out);
	return 0;	
}
