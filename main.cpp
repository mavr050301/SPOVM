#include <iostream>
#include <string>
#include "SystemFile.h"

using namespace std;
using namespace myfilesystem;

int main()
{
	string mycommand;
	SystemFile fileSystem;
	cout << "write 'help'--- to see all commands" << endl;
	while (true)
	{
		cout << " $ ";
		cin >> mycommand;
		if (mycommand == "exit") break;
		else if (mycommand == "help")
			fileSystem.Help();
		else if (mycommand == "dir")
			fileSystem.ShowFiles();
		else if (mycommand == "addfl")
			fileSystem.CreateNewFile();
		else if (mycommand == "removefl")
			fileSystem.RemoveFile();
		else if (mycommand == "resetdisk")
			fileSystem.ResetDisk();
		else if (mycommand == "openfl")
			fileSystem.OpenFile();
		else if (mycommand == "clear")
			system("cls");
		else cout << " Illegal command" << endl;
	}
	return 0;
}