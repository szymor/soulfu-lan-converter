#include <stdio.h>
#include <string.h>

#define PATH_LEN	(256)

enum ErrCode
{
	EC_NONE,
	EC_NOACTION,
	EC_NOFILE,
	EC_BADARGS
};

enum Action
{
	A_NONE,
	A_HELP,
	A_TO_TXT,
	A_TO_LAN
};

char output_path[PATH_LEN] = "";
char input_path[PATH_LEN] = "";
enum Action action = A_NONE;

char *my_basename(char *path)
{
    char *base = strrchr(path, '/');
    return base ? base+1 : path;
}

void write_big_endian(unsigned char dst[4], unsigned int n)
{
	dst[0] = (n >> 24) & 0xff;
	dst[1] = (n >> 16) & 0xff;
	dst[2] = (n >> 8) & 0xff;
	dst[3] = (n >> 0) & 0xff;
}

unsigned int read_big_endian(unsigned char src[4])
{
	unsigned int n = 0;
	n |= (unsigned int)src[0] << 24;
	n |= (unsigned int)src[1] << 16;
	n |= (unsigned int)src[2] << 8;
	n |= (unsigned int)src[3] << 0;
	return n;
}

void help(void);
int to_txt(void);
int to_lan(void);

int main(int argc, char *argv[])
{
	printf("SoulFu LANguage converter\n\n");

	for (int i = 1; i < argc; ++i)
	{
		if (!strcmp(argv[i], "-h"))
		{
			action = A_HELP;
		}
		else if (!strcmp(argv[i], "--to-txt"))
		{
			if (A_NONE == action)
			{
				action = A_TO_TXT;
				if ((i + 1) >= argc)
				{
					printf("Error: bad arguments.\n");
					return EC_BADARGS;
				}
				strcpy(input_path, argv[++i]);
			}
			else
			{
				printf("Error: bad arguments.\n");
				return EC_BADARGS;
			}
		}
		else if (!strcmp(argv[i], "--to-lan"))
		{
			if (A_NONE == action)
			{
				action = A_TO_LAN;
				if ((i + 1) >= argc)
				{
					printf("Error: bad arguments.\n");
					return EC_BADARGS;
				}
				strcpy(input_path, argv[++i]);
			}
			else
			{
				printf("Error: bad arguments.\n");
				return EC_BADARGS;
			}
		}
		else if (!strcmp(argv[i], "-o"))
		{
			if ((i + 1) >= argc)
			{
				printf("Error: bad arguments.\n");
				return EC_BADARGS;
			}
			strcpy(output_path, argv[++i]);
		}
	}

	switch (action)
	{
		case A_HELP:
			help();
			return EC_NONE;
		case A_TO_TXT:
			if ('\0' == output_path[0])
			{
				strcpy(output_path, my_basename(input_path));
				strcat(output_path, ".TXT");
			}
			return to_txt();
		case A_TO_LAN:
			if ('\0' == output_path[0])
			{
				strcpy(output_path, my_basename(input_path));
				strcat(output_path, ".LAN");
			}
			return to_lan();
		case A_NONE:
		default:
			printf("Error: no action provided.\n");
			printf("Type %s -h to get a quick help.\n", argv[0]);
			return EC_NOACTION;
	}

	// never executed
	return EC_NONE;
}

void help(void)
{
	static const char contents[] = "Quick help:\n"
		"  -h                   help\n"
		"  --to-txt <lan_file>  converts LAN to TXT\n"
		"  --to-lan <txt_file>  converts TXT to LAN\n"
		"  -o <file>            specifies output file\n";
	printf(contents);
}

int to_txt(void)
{
	printf("Input path: %s\nOutput path: %s\n", input_path, output_path);

	FILE *input = fopen(input_path, "rb");
	if (!input)
	{
		printf("Error: cannot open input file.\n");
		return EC_NOFILE;
	}

	unsigned char buff[4];
	fread(buff, 1, 4, input);
	unsigned int lines_num = read_big_endian(buff);
	fseek(input, lines_num * 4 + 4, SEEK_SET);
	printf("Number of lines: %u\n", lines_num);

	FILE *output = fopen(output_path, "wb");
	int data;
	while (data = fgetc(input), data != EOF)
	{
		if (data)
			fputc(data, output);
		else
			fputc('\n', output);
	}
	fclose(output);

	fclose(input);
}

int to_lan(void)
{
	printf("Input path: %s\nOutput path: %s\n", input_path, output_path);

	// count lines
	unsigned int lines_num = 1;
	FILE *input = fopen(input_path, "rb");
	if (!input)
	{
		printf("Error: cannot open input file.\n");
		return EC_NOFILE;
	}

	int data;
	while (data = fgetc(input), data != EOF)
	{
		if ('\n' == data)
			++lines_num;
	}
	fclose(input);
	printf("Number of lines: %u\n", lines_num);

	// do the rest
	unsigned char buff[4];

	write_big_endian(buff, lines_num);
	FILE *output = fopen(output_path, "wb");
	fwrite(buff, 1, 4, output);
	input = fopen(input_path, "rb");

	unsigned int offset = 4 + lines_num * 4;
	for (int i = 0; i < lines_num; ++i)
	{
		// fill index
		fseek(output, 4 + i * 4, SEEK_SET);
		write_big_endian(buff, offset);
		fwrite(buff, 1, 4, output);

		// dump a line
		fseek(output, offset, SEEK_SET);
		int data;
		while (data = fgetc(input), '\n' != data && EOF != data)
		{
			fputc(data, output);
			++offset;
		}
		if ('\n' == data)
		{
			fputc('\0', output);
			++offset;
		}
	}

	fclose(input);
	fclose(output);
}
