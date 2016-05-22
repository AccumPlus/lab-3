#include <stdlib.h>
#include <popt.h>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <dlfcn.h>
#include <algorithm>
#include <stdexcept>

FILE *fileout;

int main(int argc, const char* argv[])
{
	char *filename = new char[256];
	int debug = 0;
	
	/* Таблица параметров командной строки */
	poptOption optionsTable[] = {
		{"debug", '\0', POPT_ARG_VAL, &debug, 1, "Start in debug mode", NULL},
		{NULL, 'i', POPT_ARG_STRING, &filename, 'i', "Take array from file", "filename"},
		{NULL, '\0', 0, NULL, 0, NULL, NULL}
	};

	poptContext optCon;
	optCon = poptGetContext(0, argc, argv, optionsTable, 0);

	// Парсим параметры
	int res;
	while (true)
	{
		res = poptGetNextOpt(optCon);
		if (res < 0)
			break;
	}
	if (res < -1)
	{
		fprintf(stderr, "%s: %s\n", poptBadOption(optCon, 0), poptStrerror(res));
		poptFreeContext(optCon);
		delete filename;
		return EXIT_FAILURE;
	}

	poptFreeContext(optCon);

	// Если был введён параметр debug
	if (debug)
		fileout = stdout;
	else
		fileout = fopen("/dev/null", "w");

	fprintf (fileout, "Проверка дебага\n");

	// Определяем входной поток
	FILE *fileIn;
	if (strlen(filename) > 0)
	{
		fileIn = fopen(filename, "r");
		if (fileIn == NULL)
		{
			fprintf(stderr, "File %s does not exist\n", filename);
			fclose(fileout);
			delete filename;
			return EXIT_FAILURE;
		}
	}
	else
		fileIn = stdin;

	delete filename;

	printf ("Ожидание ввода массива...\n");
	char *line = NULL;
	size_t len;
	if (getline(&line, &len, fileIn) == -1)
	{
		fprintf(stderr, "Error at getting files");
		fclose(fileout);
		fclose(fileIn);
		return EXIT_FAILURE;
	}

	// Удаляем последний enter
	line[strlen(line) - 1] = '\0';
	printf("Готово!\n");

	fclose(fileIn);

	// Сплитаем по пробелам
	void *lib = dlopen("./lib/libextfunc.so", RTLD_LAZY);
	if (!lib)
	{
		fprintf(stderr, "Error at loading lib for split!\n");
		fclose(fileout);
		free(line);
		return EXIT_FAILURE;
	}

	std::vector<std::string> (*split)(const std::string &str, const char &delim) = (std::vector<std::string> (*)(const std::string &str, const char &delim))dlsym(lib, "split");
	std::vector<std::string> vec_str = (*split)(line, ' ');
	dlclose(lib);
	lib = NULL;

	free(line);

	// Удаляем пустые элементы
	fprintf(fileout, "Vec size before = %d\n", vec_str.size());
	for (auto el: vec_str)
		fprintf(fileout, "El = |%s|\n", el.c_str());
	vec_str.erase(std::remove(vec_str.begin(), vec_str.end(), ""), vec_str.end());
	fprintf(fileout, "Vec size after = %d\n", vec_str.size());

	if (vec_str.size() == 0)
		fprintf (stderr, "Array is empty!\n");
	else
	{
		// Переделываем в int
		std::vector<int> vec_int;
		try
		{
			for (auto elem: vec_str)
				vec_int.push_back(std::stoi(elem));
		}
		catch (std::invalid_argument &e)
		{
			fprintf (stderr, "Array contains invalid argument!\n");
			fclose(fileout);
			return EXIT_FAILURE;
		}

		// Выполняем функцию
		lib = dlopen("lib/libext.so", RTLD_LAZY);
		if (!lib)
		{
			fprintf(stderr, "Error at loading lib!\n");
			fclose(fileout);
			return EXIT_FAILURE;
		}
		int (*getMin)(const std::vector<int> &vec) = (int (*)(const std::vector<int> &vec))dlsym(lib, "getMinimum");
		int res = (*getMin)(vec_int);
		dlclose(lib);

		printf ("Result = %d\n", res);
	}

	fclose(fileout);

	return EXIT_SUCCESS;
}
