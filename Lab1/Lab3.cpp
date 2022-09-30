// Lab1.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"
#include "framework.h"
#include "Lab1.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Единственный объект приложения
CWinApp theApp;
vector <HANDLE> vEventMessage;
vector <HANDLE> vEventClose;
HANDLE hMutex;

struct Header
{
    int actionCode;
    int h_size;
    int h_thread;
};

extern "C" {
    __declspec(dllimport) void __stdcall setEventConfirm();
}

void ToFile(int num, string& message)
{
    string fname = to_string(num + 1) + ".txt";
    ofstream fout(fname);
    fout << message;
    fout.close();
}

Header getDescription()//фукнция получения структуры
{
    Header description;
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    while (true) {
        DWORD dwRead;
        if (ReadFile(hIn, &description, sizeof(Header), &dwRead, nullptr)) //считывание стурктуры из анонимного канала
        {
            break;
        }
    }
    return description;
}
char* getMessage(int sizeofMessage)//функция получения сообщения
{

    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    DWORD dwRead;
    char* message = new char[sizeofMessage];
    if (!ReadFile(hIn, message, sizeofMessage, &dwRead, nullptr) || !dwRead)//считывание сообщения из анонимного канала
        return NULL;
    return message;
}

string message = "";

UINT MyThread(LPVOID LPParametr)
{
    int num = (int)LPParametr;
    WaitForSingleObject(hMutex, INFINITE);
    cout << "Поток № " << num + 1 << " создан." << endl;
    ReleaseMutex(hMutex);
    HANDLE hEvents[] = { (HANDLE)vEventMessage[num], vEventClose[num] };
    bool flag = false;
    while (1)
    {
        switch (WaitForMultipleObjects(2, hEvents, FALSE, INFINITE) - WAIT_OBJECT_0) // ожидание 2-ух событий (отправка и закрытие)
        {
        case 0:
        {
            WaitForSingleObject(hMutex, INFINITE);
            ToFile(num, message); // вывод в файл
            ReleaseMutex(hMutex);
            break;
        }
        case 1:
            WaitForSingleObject(hMutex, INFINITE);
            cout << "Поток № " << num + 1 << " завершен." << endl;
            ReleaseMutex(hMutex);
            flag = true;
            break;
        }
        if (flag)
            break; // завершение бесконечного цикла
    }
    return 0;
}

void start()
{
    int i = 0;
    hMutex = CreateMutex(NULL, FALSE, "mutex");   
    bool cmdNotClosed = true;
    while (cmdNotClosed)
    {
        const Header h = getDescription();//запрос на получение структуры
        const int ev = h.actionCode;//получения из структуры кода действия
        switch (ev) // ожидание конкретного события
        {
        case 0:
        {
            thread t(MyThread, (LPVOID)(i++));
            t.detach();//другой способ создания потоков из лекции
            vEventClose.push_back(CreateEvent(NULL, FALSE, FALSE, NULL));//вектор событий для закрытия
            vEventMessage.push_back(CreateEvent(NULL, FALSE, FALSE, NULL));//вектор событий для сообщений
            break;
        }
        case 1:
        {
            if (!vEventClose.empty()) 
            {
                SetEvent(vEventClose[--i]);
                CloseHandle(vEventClose[i]);
                vEventClose.pop_back(); 
                break;
            }
        }
        case 2:
        {
            cmdNotClosed = false;
            for (auto ev : vEventClose)
            {
                CloseHandle(ev);
            }
            break;
        }
        case 3:
        {
            message = getMessage(h.h_size);
            if (h.h_thread == -1)
                for (auto ev : vEventMessage)
                {
                    SetEvent(ev);
                } // оптправка сообщения на все потоки, кроме главного
            else if (h.h_thread == -2)
                cout << message << endl; // вывод сообщения на главный поток
            else
                SetEvent(vEventMessage[h.h_thread]); // отправка сообщения на конкретный поток
        }
        }
        setEventConfirm();
    }
    for (auto ev : vEventMessage)
    {
        CloseHandle(ev);
    }
    CloseHandle(hMutex);
}

int main()
{
    setlocale(LC_ALL, "");
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    int nRetCode = 0;
    HMODULE hModule = ::GetModuleHandle(nullptr);
    cout << "Консольное приложение запущено. " << endl;
    if (hModule != nullptr)
    {
        // инициализировать MFC, а также печать и сообщения об ошибках про сбое
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            wprintf(L"Критическая ошибка: сбой при инициализации MFC\n");
            nRetCode = 1;
        }
        else
        {
            start();
        }
    }
    else
    {
        wprintf(L"Критическая ошибка: сбой GetModuleHandle\n");
        nRetCode = 1;
    }

    return nRetCode;
}