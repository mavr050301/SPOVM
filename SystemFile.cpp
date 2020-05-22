#include "SystemFile.h"
#include <conio.h>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;
using namespace myfilesystem;


const char SystemFile::SOURCE_NAME_[] = "mydisk.bin";

void SystemFile::Help()
{
	cout << endl;
	cout << "Available commands" << endl;
	cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
	cout << "dir - show all files" << endl;
	cout << "addfl -  add a new file" << endl;
	cout << "removefl -  remove component" << endl;
	cout << "resetdisk - reset disk" << endl;
	cout << "openfl - open file" << endl;
	cout << "clear - clear screen" << endl;
	cout << "exit - exit the programm" << endl;
	cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
	cout << endl;
}

SystemFile::SystemFile()
{
	this->free_space = sizeof(Block) * AMOUNT_OF_BLOCKS_;
	SecureZeroMemory(this->fmdt, this->MAX_AMOUNT_OF_FILES_ * sizeof(FMDT));
	this->source_file.open(this->SOURCE_NAME_, ios::binary | ios::in | ios::out | ios::ate);

	if (!this->source_file.is_open())
	{
		this->source_file.open(this->SOURCE_NAME_, ios::binary | ios::out | ios::ate);

		const int empty_file_size = this->AMOUNT_OF_BLOCKS_ * sizeof(Block) + this->FMDT_SIZE;
		byte set_arr[empty_file_size];
		SecureZeroMemory(set_arr, empty_file_size);
		this->source_file.write(set_arr, empty_file_size);

		this->source_file.close();
		this->source_file.open(this->SOURCE_NAME_, ios::binary | ios::in | ios::out | ios::ate);
	}
	else
	{
		this->source_file.seekg(0);
		this->source_file.read((byte*)this->fmdt, sizeof(FMDT) * this->MAX_AMOUNT_OF_FILES_);
	}
}

SystemFile::~SystemFile()
{
	this->source_file.close();
}

void SystemFile::cpString(char* destinantion, int dest_max_size, const char* source)
{
	for (int i = 0; i < dest_max_size && source[i] != '\0'; i++)
	{
		destinantion[i] = source[i];
	}
}

void SystemFile::ResetDisk()
{
	this->free_space = sizeof(Block) * AMOUNT_OF_BLOCKS_;
	SecureZeroMemory(this->fmdt, this->MAX_AMOUNT_OF_FILES_ * sizeof(FMDT));

	this->source_file.close();
	this->source_file.open(this->SOURCE_NAME_, ios::binary | ios::out | ios::trunc);

	const int empty_file_size = this->AMOUNT_OF_BLOCKS_ * sizeof(Block) + this->FMDT_SIZE;
	byte set_arr[empty_file_size];
	SecureZeroMemory(set_arr, empty_file_size);
	this->source_file.write(set_arr, empty_file_size);

	this->source_file.close();
	this->source_file.open(this->SOURCE_NAME_, ios::binary | ios::in | ios::out | ios::ate);

	cout << endl << endl << "The disk has been reseted..." << endl << endl;
}

void SystemFile::ShowFiles()
{
	cout << endl << "All files: " << endl;
	int flag = 0;
	for (int i = 0; i < this->MAX_AMOUNT_OF_FILES_; i++)
	{
		if (this->fmdt[i].begining_offset != 0)
		{
			cout << '>' << this->fmdt[i].name << endl;
			flag = 1;
		}
	}
	if (flag == 0) { cout << "no files" << endl; }
	cout << endl;
}

void SystemFile::CreateNewFile()
{
	cout << "Enter the name of a new file" << endl;

	string new_file;
	cin >> new_file;

	for (int i = 0; i < this->MAX_AMOUNT_OF_FILES_; i++)
	{
		if (strcmp(this->fmdt[i].name, new_file.c_str()) == 0)
		{
			cout << endl << "This name is already used, please repeat..." << endl;
			return;
		}
	}

	for (int i = 0; i < this->MAX_AMOUNT_OF_FILES_; i++)
	{
		if (this->fmdt[i].begining_offset == 0)
		{
			this->fmdt[i].begining_offset = this->FindFreeBlock(false);
			if (this->fmdt[i].begining_offset == 0)
				break;

			int copy_size = this->MAX_FILE_NAME_LENGTH_ < new_file.length() ? this->MAX_FILE_NAME_LENGTH_ : new_file.length();
			cpString(this->fmdt[i].name, copy_size, new_file.c_str());

			this->FMDTSave();

			Block temp_block;
			temp_block.busy_flag = 1;
			this->BlockSave(temp_block, this->fmdt[i].begining_offset);

			return;
		}
	}
	cout << endl << endl << "Reached max amount of files! Need to free some space and repeat..." << endl << endl;
}

string SystemFile::ProcFileBuffer(string fileBuffer)
{
	string processed = fileBuffer;

	while (true)
	{
		system("cls");

		cout << processed << endl << endl;

		cout << "Choose an operation:" << endl;
		cout << "1. Clear file" << endl;
		cout << "2. Add to position" << endl;
		cout << "3. Remove from position" << endl;
		cout << "0. Close file and save changes" << endl;

		switch (_getch())
		{
		case '0':
		{
			return processed;
		}
		case '1':
		{
			processed.clear();
			break;
		}
		case '2':
		{
			cout << "Select option:" << endl;
			cout << "1. Append" << endl;
			cout << "2. From position" << endl;

			char symbol = 0;
			while (symbol != '1' && symbol != '2')
			{
				symbol = _getch();
			}


			string additional;

			cout << "Continue..." << endl;

			cin.ignore();
			cin.clear();

			cout << "Please, enter the text" << endl;
			getline(cin, additional);

			if (symbol == '1')
			{
				processed.append(additional);
			}
			else
			{
				int adding_position = -1;
				while (adding_position < 0 || (adding_position > processed.length() - 1))
				{
					cout << "Please, enter the position, where to start adding text..." << endl;
					cin >> adding_position;
				}

				processed.insert(adding_position, additional);
			}
			break;
		}
		case '3':
		{
			int delete_position = -1;
			int delete_size = -1;

			while (delete_position < 0 || delete_size < 0 || ((delete_size + delete_position) > processed.length() + 1))
			{
				cout << "Enter the position, where to start deleting..." << endl;
				cin >> delete_position;

				cout << "Enter the size of a fragment..." << endl;
				cin >> delete_size;
			}

			processed.erase(delete_position, delete_size);

			break;
		}
		
		}
	}
}

void SystemFile::OpenFile()
{
	cout << "Enter the name of a file" << endl;

	string currentfile;
	cin >> currentfile;

	for (int i = 0; i < this->MAX_AMOUNT_OF_FILES_; i++)
	{
		if (strcmp(this->fmdt[i].name, currentfile.c_str()) == 0)
		{
			string fileBuffer = this->FileToBuffer(this->fmdt[i].begining_offset);
			string processed;
			processed = this->ProcFileBuffer(fileBuffer);

			if (processed.length() == 0)
			{
				int offset = this->fmdt[i].begining_offset;
				while (true)
				{
					Block temp_block = this->BlockRead(offset);

					int next_offset = temp_block.next_offset;

					this->BlockClear(temp_block);

					this->BlockSave(temp_block, offset);

					if (next_offset == 0)
						break;
					else
						offset = next_offset;
				}

				Block firstFileBlock;
				firstFileBlock.busy_flag = 1;
				this->BlockSave(firstFileBlock, this->fmdt[i].begining_offset);

				return;
			}

			this->FileSave(processed, this->fmdt[i].begining_offset);
			return;
		}
	}

	cout << endl << "Incorrect file name, please repeat..." << endl;
	return;
}

void SystemFile::RemoveFile()
{
	cout << "Please, enter the name of a file to delete" << endl;

	cin.clear();
	string delete_file;
	cin >> delete_file;

	for (int i = 0; i < this->MAX_AMOUNT_OF_FILES_; i++)
	{
		if (strcmp(this->fmdt[i].name, delete_file.c_str()) == 0)
		{
			int offset = this->fmdt[i].begining_offset;
			while (true)
			{
				Block temp_block = this->BlockRead(offset);

				int next_offset = temp_block.next_offset;

				this->BlockClear(temp_block);

				this->BlockSave(temp_block, offset);

				if (next_offset == 0)
					break;
				else
					offset = next_offset;
			}
			this->fmdt[i].begining_offset = 0;
			SecureZeroMemory(this->fmdt[i].name, this->MAX_FILE_NAME_LENGTH_);

			this->FMDTSave();

			return;
		}
	}
	cout << endl << "Incorrect file name, please repeat..." << endl;
}

string SystemFile::FileToBuffer(int begining_offset)
{
	string resultstr;
	int offset = begining_offset;
	while (true)
	{
		Block temp_block = this->BlockRead(offset);

		char temp_buffer[this->BLOCK_DATA_SIZE + 1] = { 0 };
		this->cpString(temp_buffer, this->BLOCK_DATA_SIZE, temp_block.data);

		resultstr.append(temp_buffer);

		if (temp_block.next_offset == 0)
			return resultstr;
		else
			offset = temp_block.next_offset;
	}
}

void SystemFile::FMDTSave()
{
	this->source_file.seekp(0);
	this->source_file.write((byte*)this->fmdt, sizeof(FMDT) * this->MAX_AMOUNT_OF_FILES_);
	this->source_file.flush();
}


void SystemFile::BlockSave(Block block, int offset)
{
	this->source_file.seekp(offset);
	this->source_file.write((byte*)&block, sizeof(Block));
	this->source_file.clear();
	this->source_file.flush();
}


void SystemFile::BlockClear(Block &block)
{
	block.next_offset = 0;
	block.busy_flag = 0;
	SecureZeroMemory(block.data, this->BLOCK_DATA_SIZE);
}

SystemFile::Block SystemFile::BlockRead(int offset)
{
	Block temp_block;
	this->source_file.seekg(offset);
	this->source_file.read((byte*)&temp_block, sizeof(Block));
	this->source_file.clear();
	return temp_block;
}

int SystemFile::FindFreeBlock(bool next)
{
	byte current_flag = 0;
	int offset = this->FIRST_BLOCK_OFFSET_;
	for (int i = 0; i < this->AMOUNT_OF_BLOCKS_; i++)
	{
		this->source_file.seekg(offset);
		source_file.read(&current_flag, sizeof(byte));

		if (current_flag == 0)
		{
			if (next)
			{
				next = false;
				continue;
			}
			return offset;
		}
		offset += sizeof(Block);
	}
	return 0;
}













void SystemFile::FileSave(string processed, int begining_offset)
{
	int unprocessed_length = processed.length();
	int offset = begining_offset;
	while (true)
	{
		int min_size = unprocessed_length < this->BLOCK_DATA_SIZE ? unprocessed_length : this->BLOCK_DATA_SIZE;

		Block temp_block = this->BlockRead(offset);
		SecureZeroMemory(temp_block.data, this->BLOCK_DATA_SIZE);
		temp_block.busy_flag = 1;

		this->cpString(temp_block.data, min_size, processed.c_str());

		this->BlockSave(temp_block, offset);

		unprocessed_length -= min_size;
		processed.erase(0, min_size);

		if (unprocessed_length == 0)
		{
			if (temp_block.next_offset != 0)
			{
				int clear_offset = temp_block.next_offset;
				temp_block.next_offset = 0;
				this->BlockSave(temp_block, offset);
				while (true)
				{
					Block clearblock = this->BlockRead(clear_offset);

					int old_offset = clear_offset;
					clear_offset = clearblock.next_offset;

					this->BlockClear(clearblock);
					this->BlockSave(clearblock, old_offset);

					if (clear_offset == 0)
						break;
				}
			}
			else return;
		}

		if (temp_block.next_offset == 0)
		{
			if (unprocessed_length != 0)
			{
				temp_block.next_offset = this->FindFreeBlock(false);

				if (temp_block.next_offset == 0)
				{
					cout << endl << endl << "Not enough memory on disk! Some information was lost!" << endl << endl;
					return;
				}

				this->BlockSave(temp_block, offset);
			}
			else
			{
				return;
			}
		}

		offset = temp_block.next_offset;
	}
}


