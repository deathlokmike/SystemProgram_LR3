// MFCLibrary.cpp: определяет процедуры инициализации для библиотеки DLL.
//

#include "pch.h"
#include "framework.h"
#include "MFCLibrary.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//вTODO: если эта библиотека DLL динамически связана с библиотеками DLL MFC,
//		все функции, экспортированные из данной DLL-библиотеки, которые выполняют вызовы к
//		MFC, должны содержать макрос AFX_MANAGE_STATE в
//		самое начало функции.
//
//		Например:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// тело нормальной функции
//		}
//
//		Важно, чтобы данный макрос был представлен в каждой
//		функции до вызова MFC.  Это означает, что
//		должен стоять в качестве первого оператора в
//		функции и предшествовать даже любым объявлениям переменных объекта,
//		поскольку их конструкторы могут выполнять вызовы к MFC
//		DLL.
//
//		В Технических указаниях MFC 33 и 58 содержатся более
//		подробные сведения.
//

// CMFCLibraryApp

struct Header
{
	int actionCode;//номер действия, мы теперь не используем события EventStart и т.д., вместо них просто отправляем цифру необходимого действия
	int h_size;
	int h_thread;
};
PROCESS_INFORMATION pi;

HANDLE hRead, hWrite;//Хэндлы для чтения и записи сообщений и кодов действия
HANDLE hReadConfirm, hWriteConfirm;//Хэндлы для чтения и записи подтверждения



extern "C"
{
	__declspec(dllexport)
		void _stdcall setEventConfirm()//функция подтверждения действия
	{
		HANDLE hh = GetStdHandle(STD_ERROR_HANDLE);//получение HANDLE переопределенного stdError 
		DWORD dwWrite;
		WriteFile(hh, "1", strlen("1"), &dwWrite, nullptr);//запись в отдельный анонимный канал подтверждения
	}

	__declspec(dllexport)
		void _stdcall waitEventConfirm()//функция ожидания подтверждения
	{
		const int MAXLEN = 1024;
		while (true)
		{
			DWORD dwRead;
			char buff[MAXLEN + 1];
			if (ReadFile(hReadConfirm, buff, MAXLEN, &dwRead, nullptr) || !dwRead)//попытка считываения подверждения из анонимного канала, выйдет из цикла, когда отработает функция SetEventConfirm
			{
				break;
			}
		}
	}

	_declspec(dllexport)
		void _stdcall Send(int actionCode, char* pStr, int threadNumber)//функция отправки сообщений и кодов действия
	{
		DWORD dwWrite;
		Header h = { actionCode,  strlen(pStr) + 1 , threadNumber };//заполняем структуру
		WriteFile(hWrite, &h, (BYTE)sizeof(Header), &dwWrite, nullptr);//записываем структуру
		WriteFile(hWrite, pStr, h.h_size, &dwWrite, nullptr);//записываем само сообщение
	}

	__declspec(dllexport)
		void  __stdcall Init()//начальная функция создания анонимных каналов и запуск консоли
	{
		SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
		CreatePipe(&hRead, &hWrite, &sa, 0);//создаем анонимный канал, так называемую трубу. Создаются два объекта и возвращаются два HANDLE
		SetHandleInformation(hWrite, HANDLE_FLAG_INHERIT, 0);//указание, что hWrite наследовать не нужно
		CreatePipe(&hReadConfirm, &hWriteConfirm, &sa, 0);//создаем отдельный анонимный канал для подтверждения
		SetHandleInformation(hWrite, HANDLE_FLAG_INHERIT, 0);

		STARTUPINFO si = { 0 };
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESTDHANDLES;
		si.hStdInput = hRead;//заменяем стандартный ввод с консоли на ввод из анонимного канала
		si.hStdError = hWriteConfirm;//переопределяем stdError для отправки подтверждения события

		CreateProcess(NULL, (LPSTR)("Lab1.exe"), &sa, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
	}

	_declspec(dllexport)
		bool  _stdcall ProcessisOpen()//проверка открыта ли консоль
	{
		DWORD dwExitCode;
		if (GetExitCodeProcess(pi.hProcess, &dwExitCode) != 0)
		{
			return dwExitCode == STILL_ACTIVE;
		}
	}
}
void Cleanup()//очситка всех HANDLE
{
	CloseHandle(hRead);
	CloseHandle(hWrite);
	CloseHandle(hReadConfirm);
	CloseHandle(hWriteConfirm);
}

BEGIN_MESSAGE_MAP(CMFCLibraryApp, CWinApp)
END_MESSAGE_MAP()


// Создание CMFCLibraryApp

CMFCLibraryApp::CMFCLibraryApp()
{
	// TODO: добавьте код создания,
	// Размещает весь важный код инициализации в InitInstance
}

// Единственный объект CMFCLibraryApp

CMFCLibraryApp theApp;
BOOL APIENTRY MFCLibrary1(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		Cleanup();
		break;
	}
	return TRUE;
}

// Инициализация CMFCLibraryApp

BOOL CMFCLibraryApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}