// WindowsProject1.cpp : Определяет точку входа для приложения.
//

#include "framework.h"
#include "WindowsProject1.h"
#include "include/json.hpp"
#include <shlobj.h>
#include <commdlg.h>
#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include <cmath>

#define MAX_LOADSTRING 100


struct Subsystem {
    std::wstring name; // Название подсистемы
    std::wstring description; // Краткое описание
    std::vector<std::wstring> elements; // Элементы подсистемы
    int x = 0;
    int y = 0;
};
struct Relation {
    std::wstring name; // Название подсистемы
    std::wstring description; // Краткое описание
    std::wstring type;
    std::wstring firstsys;
    std::wstring secondsys;
};
std::vector<std::wstring> types = { L"Согласование",
L"Зависимость",
L"Взаимодействие",
L"Обратная связь",
L"Агрегация", 
L"Обмен данными", 
L"Поддержка"};
wchar_t *name;
wchar_t *path;
std::vector<std::wstring> inputs;
std::vector<std::wstring> outputs;
std::map<std::wstring, std::wstring> elements;
std::vector<Subsystem> subsystems;
std::vector<Relation> relations;
// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Разместите код здесь.

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINDOWSPROJECT1, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSPROJECT1));

    MSG msg;

    // Цикл основного сообщения:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

std::string fromWcharToString(wchar_t* cstr) {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, cstr, -1, NULL, 0, NULL, NULL);
    std::string newStr(size_needed - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, cstr, -1, &newStr[0], size_needed, NULL, NULL);
    return newStr;
}

std::string WStringToString(const std::wstring& wstr) {
    // Определяем необходимый размер для std::string
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string str(size_needed, 0);

    // Преобразуем строку
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &str[0], size_needed, NULL, NULL);

    return str;
}

std::wstring stringToWstring(const std::string& str) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), nullptr, 0);
    std::wstring wstr(size_needed, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), &wstr[0], size_needed);
    return wstr;
}

wchar_t* stringToWchar(const std::string& str) {
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);

    // Выделяем память для wchar_t с учетом завершающего нуля
    wchar_t* wstr = new wchar_t[size];
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wstr, size);

    return wstr; // Возвращаем указатель на wchar_t
}

//
//  ФУНКЦИЯ: MyRegisterClass()
//
//  ЦЕЛЬ: Регистрирует класс окна.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW| CS_DBLCLKS;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSPROJECT1));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINDOWSPROJECT1);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}
//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   ЦЕЛЬ: Сохраняет маркер экземпляра и создает главное окно
//
//   КОММЕНТАРИИ:
//
//        В этой функции маркер экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится главное окно программы.
//
int width =0;
int height =0;

// Проверяем, является ли корень целым числом
int numColumns = 0;
int xOffset = 0; // Смещение по X
int yOffset = 0; // Смещение по Y
int verticalSpacing = 0;
int horizontalSpacing = 0;
int boxWidth = 0;
int boxHeight =0;
int step = 0;

HFONT hFont = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
    CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Arial"));
HFONT mainFont = CreateFont(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
    CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Arial"));
// Определение расположения подсистем по сетке
void UpdateRect(RECT rect) {
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;
    int size = subsystems.size();
    int sqrtSize = static_cast<int>(sqrt(size));

    // Проверяем, является ли корень целым числом
    numColumns = (sqrtSize * sqrtSize == size) ? sqrtSize : sqrtSize + 1;
    xOffset = width / 6; // Смещение по X
    yOffset = height / 6; // Смещение по Y
    verticalSpacing = height / 10;
    horizontalSpacing = width / 15;
    boxWidth = (width-xOffset*2) / numColumns - horizontalSpacing;
    boxHeight = (boxWidth) / 2;
}
void LayoutSubsystems() {
    for (size_t i = 0; i < subsystems.size(); ++i) {
        int x = xOffset + (i % numColumns) * (boxWidth + horizontalSpacing);
        int y = yOffset + (i / numColumns) * (boxHeight + verticalSpacing);
        subsystems[i].x = x + rand()% horizontalSpacing; // Координата верхнего левого угла по X
        subsystems[i].y = y + rand() % verticalSpacing; // Координата верхнего левого угла по Y
    }
}
void DrawCenteredText(HDC hdc, const wchar_t* text, int x, int y) {
    // Получаем размер текста
    SIZE textSize;
    GetTextExtentPoint32(hdc, text, wcslen(text), &textSize);

    // Вычисляем новые координаты для центрирования
    int centeredX = x - (textSize.cx / 2);
    int centeredY = y - (textSize.cy / 2);

    // Рисуем текст
    TextOut(hdc, centeredX, centeredY, text, wcslen(text));
}
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}
//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ЦЕЛЬ: Обрабатывает сообщения в главном окне.
//
//  WM_COMMAND  - обработать меню приложения
//  WM_PAINT    - Отрисовка главного окна
//  WM_DESTROY  - отправить сообщение о выходе и вернуться
//
//
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}
std::string escapeJson(const std::string& s) {
    std::string escaped = "";
    for (char c : s) {
        switch (c) {
        case '\"': escaped += "\\\""; break;
        case '\\': escaped += "\\\\"; break;
        case '/': escaped += "\\/"; break;
        case '\b': escaped += "\\b"; break;
        case '\f': escaped += "\\f"; break;
        case '\n': escaped += "\\n"; break;
        case '\r': escaped += "\\r"; break;
        case '\t': escaped += "\\t"; break;
        default:
            if (c < 0x20) {
                escaped += "\\u" + std::to_string(static_cast<int>(c));
            }
            else {
                escaped += c;
            }
        }
    }
    return escaped;
}
BOOL CALLBACK DestoryChildCallback(
    HWND   hwnd,
    LPARAM lParam
)
{
    if (hwnd != NULL) {
        DestroyWindow(hwnd);
    }

    return TRUE;
}

INT_PTR CALLBACK AddInput(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            HWND hEdit = GetDlgItem(hDlg, IDC_MODEL_INPUTS);
            wchar_t buffer[256];
            if (GetWindowTextLength(hEdit) > 0) {
                SendMessage(hEdit, WM_GETTEXT, sizeof(buffer), LPARAM(buffer));
                HWND hList = GetDlgItem(GetParent(hDlg), IDC_INPUTS_LIST);
                int count = SendMessage(hList, LB_GETCOUNT, 0, 0);
                bool already_in_use = false;
                for (int i = 0; i < count; ++i) {
                    wchar_t wname[256];
                    SendMessage(hList, LB_GETTEXT, i, (LPARAM)wname);
                    if (wcscmp(buffer, wname) == 0) {
                        MessageBox(hDlg, L"Вход с таким названием уже существует!", L"Warning", MB_OK);
                        already_in_use = true;
                        break;
                    }
                }
                if (!already_in_use) {
                    SendMessage(GetParent(hDlg), WM_USER + 1, 0, LPARAM(buffer));
                    EndDialog(hDlg, LOWORD(wParam));
                    return (INT_PTR)TRUE;
                }
            }
            else {
                MessageBox(hDlg, L"Заполните поля!", L"Warning", MB_OK);
            }

        }
        break;

    case WM_CLOSE:
        EndDialog(hDlg, LOWORD(wParam));
        break;
    }
    return (INT_PTR)FALSE;
}
INT_PTR CALLBACK AddOutput(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            HWND hEdit = GetDlgItem(hDlg, IDC_MODEL_OUTPUTS);
            wchar_t buffer[256];
            if (GetWindowTextLength(hEdit) > 0) {
                SendMessage(hEdit, WM_GETTEXT, sizeof(buffer), LPARAM(buffer));
                HWND hList = GetDlgItem(GetParent(hDlg), IDC_OUTPUTS_LIST);
                int count = SendMessage(hList, LB_GETCOUNT, 0, 0);
                bool already_in_use = false;
                for (int i = 0; i < count; ++i) {
                    wchar_t wname[256];
                    SendMessage(hList, LB_GETTEXT, i, (LPARAM)wname);
                    if (wcscmp(buffer, wname) == 0) {
                        MessageBox(hDlg, L"Выход с таким названием уже существует!", L"Warning", MB_OK);
                        already_in_use = true;
                        break;
                    }
                }
                if (!already_in_use) {
                    SendMessage(GetParent(hDlg), WM_USER + 2, 0, LPARAM(buffer));
                    EndDialog(hDlg, LOWORD(wParam));
                    return (INT_PTR)TRUE;
                }
            }
            else {
                MessageBox(hDlg, L"Заполните поля!", L"Warning", MB_OK);
            }
        }
        break;
    case WM_CLOSE: {
        EndDialog(hDlg, LOWORD(wParam));
        break;}
    
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK AddElement(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            HWND hName = GetDlgItem(hDlg, IDC_ELEMENT_NAME);
            HWND hDesc = GetDlgItem(hDlg, IDC_ELEMENT_DESC);
            bool already_in_use = false;
            if (GetWindowTextLength(hName) > 0) {
                wchar_t name[256];
                SendMessage(hName, WM_GETTEXT, sizeof(name), LPARAM(name));
                HWND hList = GetDlgItem(GetParent(hDlg), IDC_ELEMENTS_LIST);
                int count = SendMessage(hList, LB_GETCOUNT, 0, 0);
                for (int i = 0; i < count; ++i) {
                   wchar_t wname[256];
                   SendMessage(hList, LB_GETTEXT, i, (LPARAM)wname);
                   if (wcscmp(name, wname) == 0) {
                       MessageBox(hDlg, L"Элемент с таким названием уже существует!", L"Warning", MB_OK);
                       already_in_use = true;
                       break;
                   }
                }
                if (!already_in_use) {
                    wchar_t desc[1000];
                    SendMessage(hDesc, WM_GETTEXT, sizeof(desc), LPARAM(desc));
                    SendMessage(GetParent(hDlg), WM_USER + 3, WPARAM(name), LPARAM(desc));
                    EndDialog(hDlg, LOWORD(wParam));
                    return (INT_PTR)TRUE;
                }
            }
            else {
                MessageBox(hDlg, L"Введите название элемента!", L"Warning", MB_OK);
            }
        }
        break;
    case WM_CLOSE: {
        EndDialog(hDlg, LOWORD(wParam));
        break;}
    }
    return (INT_PTR)FALSE;
}

LRESULT CALLBACK ScrollListProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_MOUSEWHEEL: {
        // Обработка прокрутки колесиком мыши
        int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        if (zDelta > 0) {
            SendMessage(hWnd, WM_VSCROLL, SB_LINEUP, 0);
        }
        else {
            SendMessage(hWnd, WM_VSCROLL, SB_LINEDOWN, 0);
        }
        return 0; // Обработано
    }
                      // Обработка других сообщений
    default:
        break;
    }
    // Вызов старого обработчика
    return CallWindowProc((WNDPROC)GetWindowLongPtr(hWnd, GWLP_USERDATA), hWnd, message, wParam, lParam);
}

INT_PTR CALLBACK SysDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    std::string subsystemName;
    std::string subsystemDescription;
    wchar_t** selectedItems;
    HWND hSysName = GetDlgItem(hDlg, IDC_SYS_NAME);
    HWND hSysDesc = GetDlgItem(hDlg, IDC_SYS_DESC);
    HWND hAllElements = GetDlgItem(hDlg, IDC_ALL_ELEMENTS);
    HWND hSysElements = GetDlgItem(hDlg, IDC_SYS_ELEMENTS);
    Subsystem* newsub = new Subsystem();
    int count = 0;
    switch (message) {
    case WM_INITDIALOG:
        // Добавляем элементы в список
        for (const auto& elem : elements) {
            SendMessage(hAllElements, LB_ADDSTRING, 0, LPARAM(elem.first.c_str()));
        }
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:  // Кнопка "Готово"
            if (GetWindowTextLength(hSysName) > 0) {
                wchar_t wname[256], wdescription[256];
                SendMessage(hSysName, WM_GETTEXT, sizeof(wname), LPARAM(wname));
                bool already_in_use = false;
                HWND hList = GetDlgItem(GetParent(hDlg), IDC_SYS_LIST);
                int count = SendMessage(hList, LB_GETCOUNT, 0, 0);
                for (const auto &sys:subsystems) {
                    if (wcscmp(sys.name.c_str(), wname) == 0) {
                        MessageBox(hDlg, L"Подсистема с таким названием уже существует!", L"Warning", MB_OK);
                        already_in_use = true;
                        break;
                    }
                }
                if (!already_in_use) {
                    SendMessage(hSysDesc, WM_GETTEXT, sizeof(wdescription), LPARAM(wdescription));
                    count = SendMessage(hSysElements, LB_GETCOUNT, 0, 0);
                    if (count > 0) {
                        wchar_t el[256];
                        for (int i = 0; i < count; ++i) {
                            SendMessage(hSysElements, LB_GETTEXT, i, (LPARAM)el);// Получаем текст каждого элемента                          
                            newsub->elements.push_back(std::wstring(el));
                        }
                        // Сохранение введенной информации
                        newsub->name = wname;
                        newsub->description = wdescription;
                        SendMessage(GetParent(hDlg), WM_USER + 4, 0, (LPARAM)newsub);
                        EndDialog(hDlg, IDOK);
                        return TRUE;
                    }
                    else {
                        MessageBox(hDlg, L"Добавьте элементы!", L"Warning", MB_OK);
                    }
                }
            }
            else {
               MessageBox(hDlg, L"Введите название подсистемы!", L"Warning", MB_OK);
            }break;

        case IDCANCEL:  // Кнопка "Отмена"
            EndDialog(hDlg, IDCANCEL);
            return TRUE;

        case IDC_ALL_ELEMENTS:  // Список элементов
            if (HIWORD(wParam) == LBN_DBLCLK) {  // Двойной щелчок
                // Получить выбранный элемент
                int index = SendMessage(hAllElements, LB_GETCURSEL, 0, 0);
                if (index != LB_ERR) {
                    char buffer[256];
                    SendMessage(hAllElements, LB_GETTEXT, index, (LPARAM)buffer);
                    // Добавление в список выбранных элементов
                    SendMessage(hSysElements, LB_ADDSTRING, 0, (LPARAM)buffer);
                    SendMessage(hAllElements, LB_DELETESTRING, index, 0);
                }
            }
            break;
        }
    case IDC_SYS_ELEMENTS:{  // Список элементов
        if (HIWORD(wParam) == LBN_DBLCLK) {  // Двойной щелчок
            // Получить выбранный элемент
            int index = SendMessage(hSysElements, LB_GETCURSEL, 0, 0);
            if (index != LB_ERR) {
                char buffer[256];
                SendMessage(hSysElements, LB_GETTEXT, index, (LPARAM)buffer);
                // Добавление в список выбранных элементов
                SendMessage(hAllElements, LB_ADDSTRING, 0, (LPARAM)buffer);
                SendMessage(hSysElements, LB_DELETESTRING, index, 0);
            }
        }
        break;
    }

    case WM_CLOSE:
        EndDialog(hDlg, IDCANCEL);
        return TRUE;
    }
    return FALSE;
}

INT_PTR CALLBACK RelDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    HWND hRelName = GetDlgItem(hDlg, IDC_REL_NAME);
    HWND hRelDesc = GetDlgItem(hDlg, IDC_REL_DESC);
    HWND hRelType = GetDlgItem(hDlg, IDC_REL_TYPE);
    HWND hFirstSys = GetDlgItem(hDlg, IDC_FIRST_SYS);
    HWND hSecondSys = GetDlgItem(hDlg, IDC_SECOND_SYS);
    Relation* newrel = new Relation();
    int count = 0;
    switch (message) {
    case WM_INITDIALOG: {
        for (const auto& sys : subsystems) {
            SendMessage(hFirstSys, CB_ADDSTRING, 0, (LPARAM)sys.name.data());
            SendMessage(hSecondSys, CB_ADDSTRING, 0, (LPARAM)sys.name.data());
        }
        for (const auto& type : types) {
            SendMessage(hRelType, CB_ADDSTRING, 0, (LPARAM)type.data());
        }
        break;
    }
    case WM_COMMAND:
    {
        switch (LOWORD(wParam)) {
            case IDOK:{  // Кнопка "Готово"
                wchar_t wname[256], wdescription[256], wtype[256], wfirstsys[256], wsecondsys[256];
                if (GetWindowTextLength(hRelName) > 0) {
                    SendMessage(hRelName, WM_GETTEXT, sizeof(wname), LPARAM(wname));
                    bool already_in_use = false;
                    HWND hList = GetDlgItem(GetParent(hDlg), IDC_REL_LIST);
                    int count = SendMessage(hList, LB_GETCOUNT, 0, 0);
                    for (const auto &rel:relations) {
                        if (wcscmp(rel.name.c_str(), wname) == 0) {
                            MessageBox(hDlg, L"Отношение с таким названием уже существует!", L"Warning", MB_OK);
                            already_in_use = true;
                            break;
                        }
                    }
                    if (!already_in_use) {
                        SendMessage(hRelDesc, WM_GETTEXT, sizeof(wdescription), LPARAM(wdescription));
                        SendMessage(hRelType, WM_GETTEXT, sizeof(wtype), LPARAM(wtype));
                        SendMessage(hFirstSys, WM_GETTEXT, sizeof(wfirstsys), LPARAM(wfirstsys));
                        SendMessage(hSecondSys, WM_GETTEXT, sizeof(wsecondsys), LPARAM(wsecondsys));
                        int fs = SendMessage(hFirstSys, CB_GETCURSEL, 0, 0);
                        int ss = SendMessage(hSecondSys, CB_GETCURSEL, 0, 0);
                        if (fs != CB_ERR && ss != CB_ERR) {
                            // Сохранение введенной информации
                            newrel->name = wname;
                            newrel->description = wdescription;
                            newrel->type = wtype;
                            newrel->firstsys = wfirstsys;
                            newrel->secondsys = wsecondsys;
                            SendMessage(GetParent(hDlg), WM_USER + 5, 0, (LPARAM)newrel);
                            EndDialog(hDlg, IDOK);
                            return TRUE;
                        }
                        else {
                            MessageBox(hDlg, L"Выберите подсистемы!", L"Warning", MB_OK);
                        }
                    }
                }
                else {
                    MessageBox(hDlg, L"Введите название отношения!", L"Warning", MB_OK);
                }
            }
        }
        break;
    }
    case WM_CLOSE: {
        EndDialog(hDlg, IDCANCEL);
        break;
    }
    }
    return FALSE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HWND hEdit;
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Разобрать выбор в меню:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
            //Первый Этап
        case IDM_OPEN: {
            OPENFILENAME ofn;       // Диалоговое окно выбора файла
            WCHAR szFile[260];      // Здесь будет храниться выбранный файл

            // Обнуляем память
            ZeroMemory(&ofn, sizeof(ofn));
            ZeroMemory(szFile, sizeof(szFile));

            // Настраиваем OPENFILENAME
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = NULL; // Указываем родительское окно
            ofn.lpstrFile = szFile;
            ofn.lpstrFile[0] = '\0';
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = L"Json (*.json)\0*.json\0"; // Фильтры для файлов
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.lpstrInitialDir = NULL; // Начальная директория
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            // Открываем диалог выбора файла
            if (GetOpenFileName(&ofn) == TRUE) {
                wchar_t *openfile= ofn.lpstrFile;
                std::ifstream inputFile(openfile);
                if (!inputFile.is_open()) {
                    std::cerr << "Не удалось открыть файл!" << std::endl;
                    return 1;
                }

                // Создаем объект JSON
                nlohmann::json j;  
                // Читаем файл в JSON-объект
                try {
                    inputFile >> j;
                }
                catch (const std::exception& e) {
                    std::cerr << "Ошибка при чтении JSON: " << e.what() << std::endl;
                    break;
                }
                if (!(j.contains("name") && j.contains("inputs") && j.contains("outputs")&&\
                    j.contains("elements")&& j.contains("subsystems")&& j.contains("relations"))) {
                    MessageBox(NULL, L"Wrong file!", L"Error", MB_OK);
                    break;
                }
                name = stringToWchar(j["name"]);
                inputs.clear();
                for (const auto& inp : j["inputs"]) {
                    inputs.push_back(stringToWstring(inp["name"]));
                }
                outputs.clear();
                for (const auto& out : j["outputs"]) {
                    outputs.push_back(stringToWstring(out["name"]));
                }
                elements.clear();
                for (const auto& el : j["elements"]) {
                    elements[stringToWstring(el["name"])] = stringToWstring(el["desc"]);
                }
                subsystems.clear();
                for (const auto& subsystem : j["subsystems"]) {
                    Subsystem* newsub = new Subsystem();
                    newsub->name = stringToWstring(subsystem["name"]);
                    newsub->description = stringToWstring(subsystem["desc"]);
                    for (const auto& el : subsystem["elements"]) {
                        newsub->elements.push_back(stringToWstring(el["name"]));
                    }
                    subsystems.push_back(*newsub);
                }
                relations.clear();
                for (const auto& rel : j["relations"]) {
                    Relation* newrel = new Relation();
                    newrel->name=stringToWstring(rel["name"]);
                    newrel->description = stringToWstring(rel["desc"]);
                    newrel->type = stringToWstring(rel["type"]);
                    newrel->firstsys = stringToWstring(rel["first"]);
                    newrel->secondsys = stringToWstring(rel["second"]);
                    relations.push_back(*newrel);
                }
                SendMessage(hWnd, WM_COMMAND, IDM_SHOW, IDM_SHOW);
            }
        }break;
         
        case IDM_SHOW: {
            step = IDM_SHOW;
            RECT clientRect;
            GetClientRect(hWnd, &clientRect);
            UpdateRect(clientRect);
            LayoutSubsystems();
            InvalidateRect(hWnd, NULL, TRUE);
            EnumChildWindows(hWnd /* parent hwnd*/, DestoryChildCallback, NULL);
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            Rectangle(hdc, 0, 0, xOffset, height);
            DrawCenteredText(hdc, L"Входы", xOffset / 2, 10);
            int n = 0;
            for (const std::wstring& inp : inputs) {
                DrawCenteredText(hdc, inp.c_str(), xOffset / 2, 40 + n * 20);
                n++;
            }
            Rectangle(hdc, xOffset, 0, width - xOffset, height);
            DrawCenteredText(hdc, L"Система", width / 2, 10);
            Rectangle(hdc, width - xOffset, 0, width, height);
            DrawCenteredText(hdc, L"Выходы", width - xOffset / 2, 10);
            n = 0;
            for (const std::wstring& out : outputs) {
                DrawCenteredText(hdc, out.c_str(), width - xOffset / 2, 40 + n * 20);
                n++;
            }
            for (const Relation& relation : relations) {
                int startX = 0, startY = 0, endX = 0, endY = 0;

                // Найдем координаты первой и второй подсистемы
                for (size_t i = 0; i < subsystems.size(); ++i) {
                    if (subsystems[i].name == relation.firstsys) {
                        startX = subsystems[i].x + boxWidth / 2;
                        startY = subsystems[i].y + boxHeight / 2;
                    }
                    if (subsystems[i].name == relation.secondsys) {
                        endX = subsystems[i].x + boxWidth / 2;
                        endY = subsystems[i].y + boxHeight / 2;
                    }
                }

                // Рисуем линию между подсистемами
                MoveToEx(hdc, startX, startY, nullptr);
                LineTo(hdc, endX, endY);
            }
            std::vector<HWND> elshow;
            for (size_t i = 0; i < subsystems.size(); ++i) {
                int x = subsystems[i].x;
                int y = subsystems[i].y;
                RECT rect = { x, y, x + boxWidth, y + boxHeight };
                RECT elrect = { x + 10, y + 30, x + boxWidth / 2, y + boxHeight / 2 };
                FillRect(hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1));
                HGDIOBJ oldPen = SelectObject(hdc, GetStockObject(BLACK_PEN));
                Rectangle(hdc, x, y, x + boxWidth, y + boxHeight);
                SelectObject(hdc, oldPen);
                DrawText(hdc, subsystems[i].name.c_str(), -1, &rect, DT_CENTER | DT_TOP | DT_SINGLELINE);
                //DrawText(hdc, L"Элементы", -1, &elrect, DT_CENTER | DT_TOP | DT_SINGLELINE);
                elshow.push_back(CreateWindow(WC_COMBOBOX, NULL,
                    WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
                    x + boxWidth / 4, y + boxHeight / 3, boxWidth / 2, 60 * subsystems[i].elements.size(), hWnd, NULL, NULL, NULL));
                for (size_t j = 0; j < subsystems[i].elements.size(); ++j) {
                    SendMessage(elshow[i], CB_ADDSTRING, 0, LPARAM(subsystems[i].elements[j].c_str()));
                }
            }
            EndPaint(hWnd, &ps);
        }break;
       
        case IDM_NEW: {
            step = IDM_NEW;
            subsystems.clear();
            elements.clear();
            relations.clear();
            inputs.clear();
            outputs.clear();
            EnumChildWindows(hWnd /* parent hwnd*/, DestoryChildCallback, NULL);
            InvalidateRect(hWnd, NULL, TRUE);
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // Выбор шрифта в контексте устройства
            SelectObject(hdc, hFont);
            // Устанавливаем текст и рисуем его
            const WCHAR* header = L"Новая модель";
            const WCHAR* textModelName = L"Название модели";
            const WCHAR* textPath = L"Выберите место для сохранения файла";
            RECT titleRect = { 10, 10, 500, 50 };
            // Получаем размеры клиентской области
            SetBkMode(hdc, TRANSPARENT); // Устанавливаем прозрачный фон
            DrawTextW(hdc, header, -1, &titleRect, DT_CENTER | 0 | DT_SINGLELINE); // Рисуем текст по центру
            SelectObject(hdc, mainFont);
            RECT rectModelName = { 20,60,1000,500 };
            //GetClientRect(hWnd, &rect);
            DrawTextW(hdc, textModelName, -1, &rectModelName, DT_LEFT | DT_VCENTER | DT_WORDBREAK); // Рисуем текст по центру
            RECT rectPath = { 20,100,1000,500 };
            //GetClientRect(hWnd, &rect);
            DrawTextW(hdc, textPath, -1, &rectPath, DT_LEFT | DT_VCENTER | DT_WORDBREAK);
            EndPaint(hWnd, &ps);
            HWND hNext = CreateWindow(L"BUTTON", L"Далее",
                WS_VISIBLE | WS_CHILD,
                230, 150, 100, 20, hWnd, (HMENU)11, NULL, NULL);
            HWND hModelName = CreateWindow(L"EDIT", 0,
                WS_BORDER | WS_CHILD | WS_VISIBLE,
                20, 80, 200, 20, hWnd, (HMENU)IDC_MODEL_NAME, hInst, 0);
            hEdit = CreateWindow(L"EDIT", 0,
                WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY,
                20, 120, 300, 20, hWnd, NULL, NULL, NULL);
            HWND hButton = CreateWindow(L"BUTTON", L"Выбрать папку",
                WS_VISIBLE | WS_CHILD,
                350, 115, 120, 30, hWnd, (HMENU)1, NULL, NULL);
            HWND testButton = CreateWindow(L"BUTTON", L"Тест нейросети",
                WS_VISIBLE | WS_CHILD,
                450, 215, 120, 30, hWnd, (HMENU)99, NULL, NULL);
            if (sizeof(path) > 0) {
                SetWindowText(hEdit, (LPWSTR)path);
            }
            if (sizeof(name) > 0) {
                SetWindowText(hModelName, (LPWSTR)name);
            }
            break;}
        case 99: {
            CURL* curl;
            CURLcode res;

            std::string readBuffer;
            const char* jsonData = "{\"text\":\"Name 3 countries on Russian Language\"}";

            curl_global_init(CURL_GLOBAL_DEFAULT);
            curl = curl_easy_init();
            if (curl) {
                curl_easy_setopt(curl, CURLOPT_URL, "https://0f64-35-237-152-195.ngrok-free.app/test");

                struct curl_slist* headers = NULL;
                headers = curl_slist_append(headers, "Content-Type: application/json");
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData);

                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

                res = curl_easy_perform(curl);

                if (res != CURLE_OK) {
                    MessageBoxA(NULL, curl_easy_strerror(res), "Error", MB_OK | MB_ICONERROR);
                }
                else {
                    MessageBoxA(NULL, readBuffer.c_str(), "Response", MB_OK | MB_ICONINFORMATION);
                }

                curl_slist_free_all(headers);
                curl_easy_cleanup(curl);
            }
            curl_global_cleanup();
            break;
        }
        case 1: {
            BROWSEINFO bi = { 0 };
            path = new wchar_t[MAX_PATH];
            bi.lpszTitle = L"Выберите папку";
            bi.ulFlags = BIF_RETURNONLYFSDIRS; // Только папки
            hEdit = CreateWindow(L"EDIT", 0,
                    WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY,
                    20, 120, 300, 20, hWnd, NULL, NULL, NULL);
            LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
            if (pidl != NULL) {
                // Получаем путь
                SHGetPathFromIDList(pidl, (LPWSTR)path);
                // Отображаем путь в текстовом поле
                SetWindowText(hEdit, (LPWSTR)path);
            }
        }break;
        //Второй Этап
        case 11:{
            step = 11;
            name = new wchar_t[256];
            HWND mname = GetDlgItem(hWnd, IDC_MODEL_NAME);
            SendMessage(mname, WM_GETTEXT, sizeof(name), (LPARAM)name);
            if (*name =='\0' || *path == '\0') {
                MessageBox(hWnd, L"Заполнены не все поля", L"Ошибка", MB_OK);
                break;
            }
            EnumChildWindows(hWnd /* parent hwnd*/, DestoryChildCallback, NULL);
            InvalidateRect(hWnd, NULL, TRUE);
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // Выбор шрифта в контексте устройства
            SelectObject(hdc, hFont);
            // Устанавливаем текст и рисуем его
            const WCHAR* header = L"Определение входов системы";
            const WCHAR* textInputs = L"Входы системы — это ресурсы или данные, которые поступают в систему для её функционирования.\
Они представляют собой всё то, что необходимо системе для выполнения своих задач и достижения целей.\n\
                Примеры:\n\
                \t-В производственной системе : сырьё(металлы, пластмассы), работники, оборудование, энергия.\n\
                \t- В информационной системе : данные, полученные от пользователей или других систем.\n\
                \t- В бизнесе : финансовые ресурсы, информация о спросе, технологии.\n\
                Введите входы системы для вашего предприятия:";
            RECT titleRect = { 10, 10, 500, 50 };
            // Получаем размеры клиентской области
            SetBkMode(hdc, TRANSPARENT); // Устанавливаем прозрачный фон
            DrawTextW(hdc, header, -1, &titleRect, DT_CENTER | 0 | DT_SINGLELINE); // Рисуем текст по центру
            SelectObject(hdc, mainFont);
            RECT rectInputs = { 20,60,1000,500 };
            DrawTextW(hdc, textInputs, -1, &rectInputs, DT_LEFT | DT_VCENTER | DT_WORDBREAK); // Рисуем текст по центру
            RECT rectPath = { 20,100,1000,500 };
            EndPaint(hWnd, &ps);
            HWND hBack = CreateWindow(L"BUTTON", L"Назад",
                WS_VISIBLE | WS_CHILD,
                10, 460, 100, 20, hWnd, (HMENU)IDM_NEW, NULL, NULL);
            HWND hNext = CreateWindow(L"BUTTON", L"Далее",
                WS_VISIBLE | WS_CHILD,
                230, 460, 100, 20, hWnd, (HMENU)12, NULL, NULL);
            HWND hButtonAddInput = CreateWindow(L"BUTTON", L"Добавить вход",
                WS_VISIBLE | WS_CHILD,
                20, 265, 120, 30, hWnd, (HMENU)21, NULL, NULL);
            HWND hButtonRemoveInput = CreateWindow(L"BUTTON", L"Удалить вход",
                WS_VISIBLE | WS_CHILD,
                150, 265, 120, 30, hWnd, (HMENU)22, NULL, NULL);
            HWND hInputsList = CreateWindow(L"LISTBOX", 0,
                WS_VISIBLE | WS_CHILD| WS_VSCROLL | LBS_NOINTEGRALHEIGHT | LBS_DISABLENOSCROLL,
                20, 340, 150, 100, hWnd, (HMENU)IDC_INPUTS_LIST, NULL, NULL);
            SetWindowLongPtr(hInputsList, GWLP_USERDATA, (LONG_PTR)GetWindowLongPtr(hInputsList, GWLP_WNDPROC));
            SetWindowLongPtr(hInputsList, GWLP_WNDPROC, (LONG_PTR)ScrollListProc);
            SetWindowLongPtr(hInputsList, GWL_STYLE, GetWindowLongPtr(hInputsList, GWL_STYLE)|WS_BORDER);
            if (inputs.size()>0) {
                for (std::wstring input:inputs)
                {
                    SendMessage(hInputsList, LB_ADDSTRING, 0, (LPARAM)input.data());
                }
            }
            break;}
        case 21: {
            DialogBox(hInst, MAKEINTRESOURCE(IDD_MODEL_INPUTS), hWnd, AddInput);
            break;
        }
        case 22: {
            HWND hInputsList = GetDlgItem(hWnd, IDC_INPUTS_LIST);
            int index = SendMessage(hInputsList, LB_GETCURSEL, 0, 0);
            if (index != LB_ERR) {
                SendMessage(hInputsList, LB_DELETESTRING, index, 0);
            }
            break;
        }
        //Третий Этап
        case 12:{ 
        HWND hList = GetDlgItem(hWnd, IDC_INPUTS_LIST);
        int count = SendMessage(hList, LB_GETCOUNT, 0, 0);
        if (count > 0) {
            inputs.clear();
            wchar_t inp[256];
            for (int i = 0; i < count; ++i) { 
                SendMessage(hList, LB_GETTEXT, i, (LPARAM)inp); // Получаем текст каждого элемента
                inputs.push_back(inp);
            }
        }
        else if (inputs.size() == 0) {
            MessageBox(hWnd, L"Не добавлены входы!", L"Ошибка", MB_OK);
            break;
        }
        step = 12;
        EnumChildWindows(hWnd /* parent hwnd*/, DestoryChildCallback, NULL);
        InvalidateRect(hWnd, NULL, TRUE);
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // Выбор шрифта в контексте устройства
        SelectObject(hdc, hFont);
        // Устанавливаем текст и рисуем его
        const WCHAR* header = L"Определение выходов системы";
        const WCHAR* textInputs = L"Выходы системы — это результаты или продукты, которые система производит в результате своей работы.\
 Это то, что выходит из системы после обработки входов.\n\
            Примеры:\n\
        \t-В производственной системе : готовая продукция(детали, изделия), отчёты о производительности.\n\
        \t- В информационной системе : результаты анализа данных, отчёты.\n\
        \t- В бизнесе : товары, услуги, прибыль, удовлетворённость клиентов.\n\
        Введите выходы системы для вашего предприятия :";
        RECT titleRect = { 10, 10, 500, 50 };
        // Получаем размеры клиентской области
        SetBkMode(hdc, TRANSPARENT); // Устанавливаем прозрачный фон
        DrawTextW(hdc, header, -1, &titleRect, DT_CENTER | 0 | DT_SINGLELINE); // Рисуем текст по центру
        SelectObject(hdc, mainFont);
        RECT rectInputs = { 20,60,1000,500 };
        DrawTextW(hdc, textInputs, -1, &rectInputs, DT_LEFT | DT_VCENTER | DT_WORDBREAK); // Рисуем текст по центру
        RECT rectPath = { 20,100,1000,500 };
        EndPaint(hWnd, &ps);
        HWND hBack = CreateWindow(L"BUTTON", L"Назад",
            WS_VISIBLE | WS_CHILD,
            10, 460, 100, 20, hWnd, (HMENU)11, NULL, NULL);
        HWND hNext = CreateWindow(L"BUTTON", L"Далее",
            WS_VISIBLE | WS_CHILD,
            230, 460, 100, 20, hWnd, (HMENU)13, NULL, NULL);
        HWND hButtonAddInput = CreateWindow(L"BUTTON", L"Добавить выход",
            WS_VISIBLE | WS_CHILD,
            20, 265, 120, 30, hWnd, (HMENU)31, NULL, NULL);
        HWND hButtonRemoveInput = CreateWindow(L"BUTTON", L"Удалить выход",
            WS_VISIBLE | WS_CHILD,
            150, 265, 120, 30, hWnd, (HMENU)32, NULL, NULL);
        HWND hOutputsList = CreateWindow(L"LISTBOX", 0,
            WS_VISIBLE | WS_CHILD | WS_VSCROLL | LBS_NOINTEGRALHEIGHT | LBS_DISABLENOSCROLL,
            20, 340, 150, 100, hWnd, (HMENU)IDC_OUTPUTS_LIST, NULL, NULL);
        SetWindowLongPtr(hOutputsList, GWLP_USERDATA, (LONG_PTR)GetWindowLongPtr(hOutputsList, GWLP_WNDPROC));
        // Установка нового обработчика
        SetWindowLongPtr(hOutputsList, GWLP_WNDPROC, (LONG_PTR)ScrollListProc);
        if (outputs.size()>0) {
            for (std::wstring output:outputs)
            {
                SendMessage(hOutputsList, LB_ADDSTRING, 0, (LPARAM)output.data());
            }
        }
        break;}
        case 31: {
            DialogBox(hInst, MAKEINTRESOURCE(IDD_MODEL_OUTPUTS), hWnd, AddOutput);
            break;
        }
        case 32: {
            HWND hInputsList = GetDlgItem(hWnd, IDC_OUTPUTS_LIST);
            int index = SendMessage(hInputsList, LB_GETCURSEL, 0, 0);
            if (index != LB_ERR) {
                SendMessage(hInputsList, LB_DELETESTRING, index, 0);
            }
            break;
        }
        //Чётвертый Этап
        case 13: {
            HWND hList = GetDlgItem(hWnd, IDC_OUTPUTS_LIST);
            int count = SendMessage(hList, LB_GETCOUNT, 0, 0);
            if (count > 0) {
                outputs.clear();
                wchar_t out[256];
                for (int i = 0; i < count; ++i) {
                    SendMessage(hList, LB_GETTEXT, i, (LPARAM)out); // Получаем текст каждого элемента
                    outputs.push_back(out);
                }
            }
            else if (outputs.size() == 0) {
                MessageBox(hWnd, L"Не добавлены выходы!", L"Ошибка", MB_OK);
                break;
            }
            step = 13;
            EnumChildWindows(hWnd /* parent hwnd*/, DestoryChildCallback, NULL);
            InvalidateRect(hWnd, NULL, TRUE);
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // Выбор шрифта в контексте устройства
            SelectObject(hdc, hFont);
            // Устанавливаем текст и рисуем его
            const WCHAR* header = L"Выделение элементов системы";
            const WCHAR* textElements = L"Элемент системы – это неделимая часть системы с однозначно определенными известными свойствами.\
 Определите основные наиболее значимые элементы вашей системы, которые необходимы для её функционирования и достижения цели.";
            RECT titleRect = { 10, 10, 500, 50 };
            // Получаем размеры клиентской области
            SetBkMode(hdc, TRANSPARENT); // Устанавливаем прозрачный фон
            DrawTextW(hdc, header, -1, &titleRect, DT_CENTER | 0 | DT_SINGLELINE); // Рисуем текст по центру
            SelectObject(hdc, mainFont);
            RECT rectInputs = { 20,60,1000,500 };
            DrawTextW(hdc, textElements, -1, &rectInputs, DT_LEFT | DT_VCENTER | DT_WORDBREAK); // Рисуем текст по центру
            RECT rectPath = { 20,100,1000,500 };
            EndPaint(hWnd, &ps);
            HWND hBack = CreateWindow(L"BUTTON", L"Назад",
                WS_VISIBLE | WS_CHILD,
                10, 460, 100, 20, hWnd, (HMENU)12, NULL, NULL);
            HWND hNext = CreateWindow(L"BUTTON", L"Далее",
                WS_VISIBLE | WS_CHILD,
                500, 460, 100, 20, hWnd, (HMENU)14, NULL, NULL);
            HWND hButtonAddElement = CreateWindow(L"BUTTON", L"Добавить элемент",
                WS_VISIBLE | WS_CHILD,
                20, 265, 120, 30, hWnd, (HMENU)41, NULL, NULL);
            HWND hButtonRemoveElement = CreateWindow(L"BUTTON", L"Удалить элемент",
                WS_VISIBLE | WS_CHILD,
                150, 265, 120, 30, hWnd, (HMENU)42, NULL, NULL);
            HWND hElementsList = CreateWindow(L"LISTBOX", 0,
                WS_VISIBLE | WS_CHILD | WS_VSCROLL | LBS_NOINTEGRALHEIGHT | LBS_DISABLENOSCROLL|LBS_NOTIFY,
                20, 340, 150, 100, hWnd, (HMENU)IDC_ELEMENTS_LIST, NULL, NULL);
            HWND hDescList = CreateWindow(L"LISTBOX", 0,
                WS_VISIBLE | WS_CHILD | WS_VSCROLL | LBS_NOINTEGRALHEIGHT | LBS_DISABLENOSCROLL | LBS_NOTIFY,
                0, 0, 1, 1, hWnd, (HMENU)IDC_DESC_LIST, NULL, NULL);
            HWND hElementDesc = CreateWindow(L"EDIT", 0,
                WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY,
                200, 340, 200, 100, hWnd, (HMENU)IDC_SHOW_DESC, NULL, NULL);
            SetWindowLongPtr(hElementsList, GWLP_USERDATA, (LONG_PTR)GetWindowLongPtr(hElementsList, GWLP_WNDPROC));
            // Установка нового обработчика
            SetWindowLongPtr(hElementsList, GWLP_WNDPROC, (LONG_PTR)ScrollListProc);
            for (auto it = elements.begin(); it != elements.end(); ++it)
            {
                SendMessage(hElementsList, LB_ADDSTRING, 0, LPARAM(it->first.data()));
                SendMessage(hDescList, LB_ADDSTRING, 0, LPARAM(it->second.data()));
                
            }
            break;
        }
        case 41: {
            DialogBox(hInst, MAKEINTRESOURCE(IDD_MODEL_ELEMENT), hWnd, AddElement);
            break;
        }
        case 42: {
            HWND hElementsList = GetDlgItem(hWnd, IDC_ELEMENTS_LIST);
            HWND hDescList = GetDlgItem(hWnd, IDC_DESC_LIST);
            int index = SendMessage(hElementsList, LB_GETCURSEL, 0, 0);
            char* name = new char[256];
            if (index != LB_ERR) {
                SendMessage(hElementsList, LB_DELETESTRING, index, 0);
                SendMessage(hDescList, LB_DELETESTRING, index, 0);
            }
            break;
        }
        case IDC_ELEMENTS_LIST:{
            if (HIWORD(wParam) == LBN_DBLCLK) {
                HWND hElementsList = GetDlgItem(hWnd, IDC_ELEMENTS_LIST);
                HWND hDescList = GetDlgItem(hWnd, IDC_DESC_LIST);
                HWND hElementDesc = GetDlgItem(hWnd, IDC_SHOW_DESC);
                int index = SendMessage(hElementsList, LB_GETCURSEL, 0, 0);
                if (index != LB_ERR) {
                    char* name = new char[256];
                    char* desc = new char[1000];
                    SendMessage(hElementsList, LB_GETTEXT, index, (LPARAM)name);
                    SendMessage(hDescList, LB_GETTEXT, index, (LPARAM)desc);
                    SendMessageW(hElementDesc, WM_SETTEXT, 0, LPARAM(desc));
                }
            }
            break;
         }
        //Пятый Этап
        case 14: {
            HWND hList = GetDlgItem(hWnd, IDC_ELEMENTS_LIST);
            HWND hDesc = GetDlgItem(hWnd, IDC_DESC_LIST);
            int count = SendMessage(hList, LB_GETCOUNT, 0, 0);
            if (count > 0) {
                elements.clear();
                for (int i = 0; i < count; ++i) {
                    wchar_t wname[256];
                    wchar_t wdesc[256];
                    SendMessage(hList, LB_GETTEXT, i, (LPARAM)wname);
                    SendMessage(hDesc, LB_GETTEXT, i, (LPARAM)wdesc);
                    elements[wname] = wdesc;
                }
            }
            else if (elements.size() == 0) {
                MessageBox(hWnd, L"Не добавлены элементы!", L"Ошибка", MB_OK);
                break;
            }
            step = 14;
            EnumChildWindows(hWnd /* parent hwnd*/, DestoryChildCallback, NULL);
            InvalidateRect(hWnd, NULL, TRUE);
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // Выбор шрифта в контексте устройства
            SelectObject(hdc, hFont);
            // Устанавливаем текст и рисуем его
            const WCHAR* header = L"Выделение подсистем";
            const WCHAR* textElements = L"Подсистема– это часть системы содержащая в себе элементы системы и выполняющая определённую функцию.\
 Выделите подсистемы и определите, какие из ранее созданных элементов входят в их составы";
            RECT titleRect = { 10, 10, 500, 50 };
            // Получаем размеры клиентской области
            SetBkMode(hdc, TRANSPARENT); // Устанавливаем прозрачный фон
            DrawTextW(hdc, header, -1, &titleRect, DT_CENTER | 0 | DT_SINGLELINE); // Рисуем текст по центру
            SelectObject(hdc, mainFont);
            RECT rectInputs = { 20,60,1000,500 };
            DrawTextW(hdc, textElements, -1, &rectInputs, DT_LEFT | DT_VCENTER | DT_WORDBREAK); // Рисуем текст по центру
            RECT rectPath = { 20,100,1000,500 };
            EndPaint(hWnd, &ps);
            HWND hBack = CreateWindow(L"BUTTON", L"Назад",
                WS_VISIBLE | WS_CHILD,
                10, 460, 100, 20, hWnd, (HMENU)13, NULL, NULL);
            HWND hNext = CreateWindow(L"BUTTON", L"Далее",
                WS_VISIBLE | WS_CHILD,
                500, 460, 100, 20, hWnd, (HMENU)15, NULL, NULL);
            HWND hButtonAddSys = CreateWindow(L"BUTTON", L"Добавить подсистему",
                WS_VISIBLE | WS_CHILD,
                20, 265, 120, 30, hWnd, (HMENU)51, NULL, NULL);
            HWND hButtonRemoveSys = CreateWindow(L"BUTTON", L"Удалить подсистему",
                WS_VISIBLE | WS_CHILD,
                150, 265, 120, 30, hWnd, (HMENU)52, NULL, NULL);
            HWND hSysList = CreateWindow(L"LISTBOX", 0,
                WS_VISIBLE | WS_CHILD | WS_VSCROLL | LBS_NOINTEGRALHEIGHT | LBS_DISABLENOSCROLL | LBS_NOTIFY,
                20, 340, 150, 100, hWnd, (HMENU)IDC_SYS_LIST, NULL, NULL);
            HWND hElementsList = CreateWindow(L"LISTBOX", 0,
                WS_VISIBLE | WS_CHILD | WS_VSCROLL | LBS_NOINTEGRALHEIGHT | LBS_DISABLENOSCROLL | LBS_NOTIFY,
                400, 340, 150, 100, hWnd, (HMENU)IDC_SYSEL_LIST, NULL, NULL);
            HWND hSysDesc = CreateWindow(L"EDIT", 0,
                WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY,
                200, 340, 150, 100, hWnd, (HMENU)IDC_SHOW_DESC, NULL, NULL);
            SetWindowLongPtr(hSysList, GWLP_USERDATA, (LONG_PTR)GetWindowLongPtr(hSysList, GWLP_WNDPROC));
            // Установка нового обработчика
            SetWindowLongPtr(hSysList, GWLP_WNDPROC, (LONG_PTR)ScrollListProc);
            for (auto it = subsystems.begin(); it != subsystems.end(); ++it)
            {
                SendMessage(hSysList, LB_ADDSTRING, 0, LPARAM(it->name.data()));
            }
            break;
        }
        case IDC_SYS_LIST: {
            if (HIWORD(wParam) == LBN_DBLCLK) {
                HWND hSysList = GetDlgItem(hWnd, IDC_SYS_LIST);
                HWND hSysDesc = GetDlgItem(hWnd, IDC_SHOW_DESC);
                HWND hElList = GetDlgItem(hWnd, IDC_SYSEL_LIST);
                int index = SendMessage(hSysList, LB_GETCURSEL, 0, 0);
                if (index != LB_ERR) {
                    const wchar_t* desc = subsystems[index].description.data();
                    SendMessageW(hSysDesc, WM_SETTEXT, 0, LPARAM(desc));
                    int count = SendMessage(hElList, LB_GETCOUNT, 0, 0);
                    for (int i = 0; i <= count; i++) {
                        SendMessage(hElList, LB_DELETESTRING, 0, 0);    
                    }
                    for (std::wstring el : subsystems[index].elements) {
                        
                        SendMessage(hElList, LB_ADDSTRING, 0, (LPARAM)el.data());
                    }
                }
            }
            break;
        }
        case 51: {
            DialogBox(hInst, MAKEINTRESOURCE(IDD_SYS_DIALOG), hWnd, SysDialogProc);
            break;
        }
        case 52: {
            HWND hSysList = GetDlgItem(hWnd, IDC_SYS_LIST);
            int index = SendMessage(hSysList, LB_GETCURSEL, 0, 0);
            if (index != LB_ERR) {
                SendMessage(hSysList, LB_DELETESTRING, index, 0);
                subsystems.erase(subsystems.begin() + index);
            }
            break;
        }
        //Шестой Этап
        case 15: {
            if (subsystems.empty()) {
                MessageBox(hWnd, L"Не добавлены подсистемы!", L"Ошибка", MB_OK);
                break;
            }
            step = 15;
            EnumChildWindows(hWnd /* parent hwnd*/, DestoryChildCallback, NULL);
            InvalidateRect(hWnd, NULL, TRUE);
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // Выбор шрифта в контексте устройства
            SelectObject(hdc, hFont);
            // Устанавливаем текст и рисуем его
            const WCHAR* header = L"Определение отношений между подсистемами";
            const WCHAR* textElements = L"Отношения внутри системы – это связи и взаимодействия между частями системы.\
 Отношения также отражают функции подсистем. Определите как подсистемы взаимодействуют друг с другом, создав отношения между ними.";
            RECT titleRect = { 10, 10, 500, 50 };
            // Получаем размеры клиентской области
            SetBkMode(hdc, TRANSPARENT); // Устанавливаем прозрачный фон
            DrawTextW(hdc, header, -1, &titleRect, DT_CENTER | 0 | DT_SINGLELINE); // Рисуем текст по центру
            SelectObject(hdc, mainFont);
            RECT rectInputs = { 20,60,1000,500 };
            DrawTextW(hdc, textElements, -1, &rectInputs, DT_LEFT | DT_VCENTER | DT_WORDBREAK); // Рисуем текст по центру
            RECT rectPath = { 20,100,1000,500 };
            EndPaint(hWnd, &ps);
            HWND hBack = CreateWindow(L"BUTTON", L"Назад",
                WS_VISIBLE | WS_CHILD,
                10, 460, 100, 20, hWnd, (HMENU)14, NULL, NULL);
            HWND hNext = CreateWindow(L"BUTTON", L"Готово",
                WS_VISIBLE | WS_CHILD,
                500, 460, 100, 20, hWnd, (HMENU)16, NULL, NULL);
            HWND hButtonAddRel = CreateWindow(L"BUTTON", L"Добавить отношение",
                WS_VISIBLE | WS_CHILD,
                20, 265, 120, 30, hWnd, (HMENU)61, NULL, NULL);
            HWND hButtonRemoveRel = CreateWindow(L"BUTTON", L"Удалить отношение",
                WS_VISIBLE | WS_CHILD,
                150, 265, 120, 30, hWnd, (HMENU)62, NULL, NULL);
            HWND hRelList = CreateWindow(L"LISTBOX", 0,
                WS_VISIBLE | WS_CHILD | WS_VSCROLL | LBS_NOINTEGRALHEIGHT | LBS_DISABLENOSCROLL | LBS_NOTIFY,
                20, 340, 150, 100, hWnd, (HMENU)IDC_REL_LIST, NULL, NULL);
            HWND hSysList = CreateWindow(L"LISTBOX", 0,
                WS_VISIBLE | WS_CHILD | WS_VSCROLL | LBS_NOINTEGRALHEIGHT | LBS_DISABLENOSCROLL | LBS_NOTIFY,
                400, 340, 150, 100, hWnd, (HMENU)IDC_RELSYS_LIST, NULL, NULL);
            HWND hRelDesc = CreateWindow(L"EDIT", 0,
                WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY,
                200, 340, 150, 100, hWnd, (HMENU)IDC_SHOW_DESC, NULL, NULL);
            SetWindowLongPtr(hRelList, GWLP_USERDATA, (LONG_PTR)GetWindowLongPtr(hSysList, GWLP_WNDPROC));
            // Установка нового обработчика
            SetWindowLongPtr(hRelList, GWLP_WNDPROC, (LONG_PTR)ScrollListProc);
            break;
        }
        case IDC_REL_LIST: {
            if (HIWORD(wParam) == LBN_DBLCLK) {
                HWND hRelList = GetDlgItem(hWnd, IDC_REL_LIST);
                HWND hRelDesc = GetDlgItem(hWnd, IDC_SHOW_DESC);
                HWND hRSysList = GetDlgItem(hWnd, IDC_RELSYS_LIST);
                int index = SendMessage(hRelList, LB_GETCURSEL, 0, 0);
                if (index != LB_ERR) {
                    const wchar_t* desc = relations[index].description.data();
                    SendMessageW(hRelDesc, WM_SETTEXT, 0, LPARAM(desc));
                    int count = SendMessage(hRSysList, LB_GETCOUNT, 0, 0);
                    for (int i = 0; i <= count; i++) {
                        SendMessage(hRSysList, LB_DELETESTRING, 0, 0);
                    }
                    SendMessage(hRSysList, LB_ADDSTRING, 0, (LPARAM)relations[index].firstsys.data());
                    SendMessage(hRSysList, LB_ADDSTRING, 0, (LPARAM)relations[index].secondsys.data());
                }
            }
            break;
        }
        case 61: {
            DialogBox(hInst, MAKEINTRESOURCE(IDD_REL_DIALOG), hWnd, RelDialogProc);
            break;
        }
        case 62: {
            HWND hRelList = GetDlgItem(hWnd, IDC_REL_LIST);
            int index = SendMessage(hRelList, LB_GETCURSEL, 0, 0);
            if (index != LB_ERR) {
                SendMessage(hRelList, LB_DELETESTRING, index, 0);
                subsystems.erase(subsystems.begin() + index);
            }
            break;
        }
        case 16: {
            if (relations.empty()) {
                MessageBox(hWnd, L"Не добавлены отношения!", L"Ошибка", MB_OK);
                break;
            }
            nlohmann::json j;
            std::string nameStr = fromWcharToString(name);
            j["name"] = nameStr;

            for (const auto& inp : inputs) {
                nlohmann::json inj;
                inj["name"] = WStringToString(inp);
                j["inputs"].push_back(inj);
            }
            for (const auto& out : outputs) {
                nlohmann::json outj;
                outj["name"] = WStringToString(out);
                j["outputs"].push_back(outj);
            }
            for (const auto& el : elements) {
                nlohmann::json elj;
                elj["name"] = WStringToString(el.first);
                elj["desc"] = WStringToString(el.second);
                j["elements"].push_back(elj);
            }
            for (const auto& subsystem : subsystems) {
                nlohmann::json sysj;
                sysj["name"] = WStringToString(subsystem.name);
                sysj["desc"] = WStringToString(subsystem.description);
                for (const auto& el : subsystem.elements) {
                    nlohmann::json elj;
                    elj["name"] = WStringToString(el);
                    sysj["elements"].push_back(elj);
                }
                j["subsystems"].push_back(sysj);
            }

            for (const auto& rel : relations) {
                nlohmann::json relj;
                relj["name"] = WStringToString(rel.name);
                relj["desc"] = WStringToString(rel.description);
                relj["type"] = WStringToString(rel.type);
                relj["first"] = WStringToString(rel.firstsys);
                relj["second"] = WStringToString(rel.secondsys);
                j["relations"].push_back(relj);
            }
            // Преобразуем JSON в строку
            std::string jsonString = j.dump(4); // 4 - это количество пробелов для отступов

            std::wstring truepath = (path + std::wstring(L"\\") + name + std::wstring(L".json"));
            MessageBox(NULL, LPCWSTR(truepath.data()), L"test", MB_OK);
            // Записываем JSON в файл
            std::ofstream outFile(truepath.data());
            if (outFile.is_open()) {
                outFile << jsonString;
                outFile.close();
            }
            else {
                MessageBox(hWnd, L"Не удалось создать файл!", L"Ошибка", MB_OK | MB_ICONERROR);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_SHOW, IDM_SHOW);
        }break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
       
    }break;
    case WM_USER + 1:{
        wchar_t* listel = (wchar_t*)lParam;
        HWND hList = GetDlgItem(hWnd, IDC_INPUTS_LIST);
        if (hList != NULL) {
            SendMessage(hList, LB_ADDSTRING, 0, lParam);
        }

        break;
    }
    case WM_USER + 2:{
        wchar_t* listel = (wchar_t*)lParam;
        HWND hList = GetDlgItem(hWnd, IDC_OUTPUTS_LIST);
        if (hList != NULL) {
            SendMessage(hList, LB_ADDSTRING, 0, lParam);
        }

        break;
    }
    case WM_USER + 3:{
        HWND hList = GetDlgItem(hWnd, IDC_ELEMENTS_LIST);
        HWND hDesc = GetDlgItem(hWnd, IDC_DESC_LIST);
        if (hList != NULL && hDesc != NULL) {
            SendMessage(hList, LB_ADDSTRING, 0, wParam);
            SendMessage(hDesc, LB_ADDSTRING, 0, lParam);
        }
        break;
    }
    case WM_USER + 4: {
        HWND hList = GetDlgItem(hWnd, IDC_SYS_LIST);
        Subsystem* newsub = (Subsystem*)lParam;
        if (hList != NULL) {
            subsystems.push_back(*newsub);
            SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)newsub->name.data());
        }
        break;
        
    }
    case WM_USER + 5: {
        HWND hList = GetDlgItem(hWnd, IDC_REL_LIST);
        Relation* newrel = (Relation*)lParam;
        if (hList != NULL) {
            relations.push_back(*newrel);
            SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)newrel->name.data());
        }
        break;

    }
    case WM_SIZE: {
        if (step == IDM_SHOW && !IsIconic(hWnd)) {
            RECT clientRect;
            GetClientRect(hWnd, &clientRect);
            UpdateRect(clientRect);
            LayoutSubsystems();
            InvalidateRect(hWnd, NULL, TRUE);
            EnumChildWindows(hWnd /* parent hwnd*/, DestoryChildCallback, NULL);
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            Rectangle(hdc, 0, 0, xOffset, height);
            DrawCenteredText(hdc, L"Входы", xOffset / 2, 10);
            int n = 0;
            for (const std::wstring& inp : inputs) {
                DrawCenteredText(hdc, inp.c_str(), xOffset / 2, 40 + n * 20);
                n++;
            }
            Rectangle(hdc, xOffset, 0, width - xOffset, height);
            DrawCenteredText(hdc, L"Система", width / 2, 10);
            Rectangle(hdc, width - xOffset, 0, width, height);
            DrawCenteredText(hdc, L"Выходы", width - xOffset / 2, 10);
            n = 0;
            for (const std::wstring& out : outputs) {
                DrawCenteredText(hdc, out.c_str(), width - xOffset / 2, 40 + n * 20);
                n++;
            }
            for (const Relation& relation : relations) {
                int startX = 0, startY = 0, endX = 0, endY = 0;

                // Найдем координаты первой и второй подсистемы
                for (size_t i = 0; i < subsystems.size(); ++i) {
                    if (subsystems[i].name == relation.firstsys) {
                        startX = subsystems[i].x + boxWidth / 2;
                        startY = subsystems[i].y + boxHeight / 2;
                    }
                    if (subsystems[i].name == relation.secondsys) {
                        endX = subsystems[i].x + boxWidth / 2;
                        endY = subsystems[i].y + boxHeight / 2;
                    }
                }

                // Рисуем линию между подсистемами
                MoveToEx(hdc, startX, startY, nullptr);
                LineTo(hdc, endX, endY);
            }
            std::vector<HWND> elshow;
            for (size_t i = 0; i < subsystems.size(); ++i) {
                int x = subsystems[i].x;
                int y = subsystems[i].y;
                RECT rect = { x, y, x + boxWidth, y + boxHeight };
                RECT elrect = { x + 10, y + 30, x + boxWidth / 2, y + boxHeight / 2 };
                FillRect(hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1));
                HGDIOBJ oldPen = SelectObject(hdc, GetStockObject(BLACK_PEN));
                Rectangle(hdc, x, y, x + boxWidth, y + boxHeight);
                SelectObject(hdc, oldPen);
                DrawText(hdc, subsystems[i].name.c_str(), -1, &rect, DT_CENTER | DT_TOP | DT_SINGLELINE);
                //DrawText(hdc, L"Элементы", -1, &elrect, DT_CENTER | DT_TOP | DT_SINGLELINE);
                elshow.push_back(CreateWindow(WC_COMBOBOX, NULL,
                    WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
                    x + boxWidth / 4, y + boxHeight / 3, boxWidth / 2, 60 * subsystems[i].elements.size(), hWnd, NULL, NULL, NULL));
                for (size_t j = 0; j < subsystems[i].elements.size(); ++j) {
                    SendMessage(elshow[i], CB_ADDSTRING, 0, LPARAM(subsystems[i].elements[j].c_str()));
                }
            }
            EndPaint(hWnd, &ps);
            for (size_t i = 0; i < subsystems.size(); ++i) {
                int x = subsystems[i].x;
                int y = subsystems[i].y;
                elshow.push_back(CreateWindow(WC_COMBOBOX, NULL,
                    WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
                    x + boxWidth / 4, y + boxHeight / 3, boxWidth / 2, 60 * subsystems[i].elements.size(), hWnd, NULL, NULL, NULL));
                for (size_t j = 0; j < subsystems[i].elements.size(); ++j) {
                    SendMessage(elshow[i], CB_ADDSTRING, 0, LPARAM(subsystems[i].elements[j].c_str()));
                }
            }
        }
    }break;
  
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // Выбор шрифта в контексте устройства
        
        // Устанавливаем текст и рисуем его
        switch (step) {
        case 0: {
            RECT titleRect = { 10, 10, 500, 50 };
            // Получаем размеры клиентской области
            SetBkMode(hdc, TRANSPARENT); // Устанавливаем прозрачный фон


            RECT rect = { 20,60,1000,500 };
            const WCHAR* header = L"Добро пожаловать в Model Construct!";
            const WCHAR* text = L"Данное приложение представляет собой диалоговую систему поддержки формирования модели предприятия. За основу процесса создания модели системы взята методика Кошарского-Уёмова, базирующаяся на двойственном определении системы.\nПостроение модели разделено на следующие этапы :\n\
            \t1.	Создание новой модели\n\
            \t2.	Определение входов системы\n\
            \t3.	Определение выходов системы\n\
            \t4.	Определение связей между входами и выходами системы\n\
            \t5.	Выделение значимых элементов\n\
            \t6.	Выделение подсистем\n\
            \t7.	Определение отношений между подсистемами";
            SelectObject(hdc, hFont);
            DrawTextW(hdc, header, -1, &titleRect, DT_CENTER | 0 | DT_SINGLELINE); // Рисуем текст по центру
            SelectObject(hdc, mainFont);
            DrawTextW(hdc, text, -1, &rect, DT_LEFT | DT_VCENTER | DT_WORDBREAK); // Рисуем текст по центру
        }break;
        case IDM_NEW: {
            const WCHAR* header = L"Новая модель";
            const WCHAR* textModelName = L"Название модели";
            const WCHAR* textPath = L"Выберите место для сохранения файла";
            RECT titleRect = { 10, 10, 500, 50 };
            // Получаем размеры клиентской области
            SetBkMode(hdc, TRANSPARENT); // Устанавливаем прозрачный фон
            SelectObject(hdc, hFont);
            DrawTextW(hdc, header, -1, &titleRect, DT_CENTER | 0 | DT_SINGLELINE); // Рисуем текст по центру
            SelectObject(hdc, mainFont);
            RECT rectModelName = { 20,60,1000,500 };
            //GetClientRect(hWnd, &rect);
            DrawTextW(hdc, textModelName, -1, &rectModelName, DT_LEFT | DT_VCENTER | DT_WORDBREAK); // Рисуем текст по центру
            RECT rectPath = { 20,100,1000,500 };
            //GetClientRect(hWnd, &rect);
            DrawTextW(hdc, textPath, -1, &rectPath, DT_LEFT | DT_VCENTER | DT_WORDBREAK);
        }break;
        case 11: {
            SelectObject(hdc, hFont);
            // Устанавливаем текст и рисуем его
            const WCHAR* header = L"Определение входов системы";
            const WCHAR* textInputs = L"Входы системы — это ресурсы или данные, которые поступают в систему для её функционирования.\
Они представляют собой всё то, что необходимо системе для выполнения своих задач и достижения целей.\n\
                Примеры:\n\
                \t-В производственной системе : сырьё(металлы, пластмассы), работники, оборудование, энергия.\n\
                \t- В информационной системе : данные, полученные от пользователей или других систем.\n\
                \t- В бизнесе : финансовые ресурсы, информация о спросе, технологии.\n\
                Введите входы системы для вашего предприятия:";
            RECT titleRect = { 10, 10, 500, 50 };
            // Получаем размеры клиентской области
            SetBkMode(hdc, TRANSPARENT); // Устанавливаем прозрачный фон
            DrawTextW(hdc, header, -1, &titleRect, DT_CENTER | 0 | DT_SINGLELINE); // Рисуем текст по центру
            SelectObject(hdc, mainFont);
            RECT rectInputs = { 20,60,1000,500 };
            //GetClientRect(hWnd, &rect);
            DrawTextW(hdc, textInputs, -1, &rectInputs, DT_LEFT | DT_VCENTER | DT_WORDBREAK); // Рисуем текст по центру
            RECT rectPath = { 20,100,1000,500 };
        }break;
        case 12: {
            SelectObject(hdc, hFont);
            // Устанавливаем текст и рисуем его
            const WCHAR* header = L"Определение выходов системы";
            const WCHAR* textInputs = L"Выходы системы — это результаты или продукты, которые система производит в результате своей работы.\
 Это то, что выходит из системы после обработки входов.\n\
            Примеры:\n\
        \t-В производственной системе : готовая продукция(детали, изделия), отчёты о производительности.\n\
        \t- В информационной системе : результаты анализа данных, отчёты.\n\
        \t- В бизнесе : товары, услуги, прибыль, удовлетворённость клиентов.\n\
        Введите выходы системы для вашего предприятия :";
            RECT titleRect = { 10, 10, 500, 50 };
            // Получаем размеры клиентской области
            SetBkMode(hdc, TRANSPARENT); // Устанавливаем прозрачный фон
            DrawTextW(hdc, header, -1, &titleRect, DT_CENTER | 0 | DT_SINGLELINE); // Рисуем текст по центру
            SelectObject(hdc, mainFont);
            RECT rectInputs = { 20,60,1000,500 };
            //GetClientRect(hWnd, &rect);
            DrawTextW(hdc, textInputs, -1, &rectInputs, DT_LEFT | DT_VCENTER | DT_WORDBREAK); // Рисуем текст по центру
            RECT rectPath = { 20,100,1000,500 };
            //GetClientRect(hWnd, &rect);
        }break;
        case 13: {
            SelectObject(hdc, hFont);
            // Устанавливаем текст и рисуем его
            const WCHAR* header = L"Выделение элементов системы";
            const WCHAR* textElements = L"Элемент системы – это неделимая часть системы с однозначно определенными известными свойствами.\
 Определите основные наиболее значимые элементы вашей системы, которые необходимы для её функционирования и достижения цели.";
            RECT titleRect = { 10, 10, 500, 50 };
            // Получаем размеры клиентской области
            SetBkMode(hdc, TRANSPARENT); // Устанавливаем прозрачный фон
            DrawTextW(hdc, header, -1, &titleRect, DT_CENTER | 0 | DT_SINGLELINE); // Рисуем текст по центру
            SelectObject(hdc, mainFont);
            RECT rectInputs = { 20,60,1000,500 };
            //GetClientRect(hWnd, &rect);
            DrawTextW(hdc, textElements, -1, &rectInputs, DT_LEFT | DT_VCENTER | DT_WORDBREAK); // Рисуем текст по центру
            RECT rectPath = { 20,100,1000,500 };
        }break;
        case 14: {
            SelectObject(hdc, hFont);
            // Устанавливаем текст и рисуем его
            const WCHAR* header = L"Выделение подсистем";
            const WCHAR* textElements = L"Подсистема– это часть системы содержащая в себе элементы системы и выполняющая определённую функцию.\
 Выделите подсистемы и определите, какие из ранее созданных элементов входят в их составы";
            RECT titleRect = { 10, 10, 500, 50 };
            // Получаем размеры клиентской области
            SetBkMode(hdc, TRANSPARENT); // Устанавливаем прозрачный фон
            DrawTextW(hdc, header, -1, &titleRect, DT_CENTER | 0 | DT_SINGLELINE); // Рисуем текст по центру
            SelectObject(hdc, mainFont);
            RECT rectInputs = { 20,60,1000,500 };
            //GetClientRect(hWnd, &rect);
            DrawTextW(hdc, textElements, -1, &rectInputs, DT_LEFT | DT_VCENTER | DT_WORDBREAK); // Рисуем текст по центру
            RECT rectPath = { 20,100,1000,500 };
        }break;
        case 15: {
            SelectObject(hdc, hFont);
            // Устанавливаем текст и рисуем его
            const WCHAR* header = L"Определение отношений между подсистемами";
            const WCHAR* textElements = L"Отношения внутри системы – это связи и взаимодействия между частями системы.\
 Отношения также отражают функции подсистем. Определите как подсистемы взаимодействуют друг с другом, создав отношения между ними.";
            RECT titleRect = { 10, 10, 500, 50 };
            // Получаем размеры клиентской области
            SetBkMode(hdc, TRANSPARENT); // Устанавливаем прозрачный фон
            DrawTextW(hdc, header, -1, &titleRect, DT_CENTER | 0 | DT_SINGLELINE); // Рисуем текст по центру
            SelectObject(hdc, mainFont);
            RECT rectInputs = { 20,60,1000,500 };
            //GetClientRect(hWnd, &rect);
            DrawTextW(hdc, textElements, -1, &rectInputs, DT_LEFT | DT_VCENTER | DT_WORDBREAK); // Рисуем текст по центру
            RECT rectPath = { 20,100,1000,500 };
        }break;
        case IDM_SHOW: {

        }break;
        }

        
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    };
    return 0;
}
// Обработчик сообщений для окна "О программе".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
