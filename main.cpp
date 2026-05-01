#include <windows.h>
#include <string>
#include <cctype>
using namespace std;

//--------------------------------------------------
// GLOBALS (GUI HANDLES)
//--------------------------------------------------
HWND hInput, hOutput;
HWND hCaesarRadio, hVigenereRadio;
HWND hShiftBox, hKeyBox;
HWND hEncryptBtn, hDecryptBtn;

//--------------------------------------------------
// HELPER: Caesar cipher
//--------------------------------------------------
string caesar(const string &text, int shift, bool encrypt)
{
    string result = text;
    if (!encrypt) shift = -shift;

    for (char &c : result)
    {
        if (isalpha(c))
        {
            char base = islower(c) ? 'a' : 'A';
            c = (char)(base + (c - base + shift + 26) % 26);
        }
    }
    return result;
}

//--------------------------------------------------
// HELPER: Vigenere cipher
//--------------------------------------------------
string vigenere(const string &text, const string &key, bool encrypt)
{
    string result = text;
    string cleanKey = "";

    for (char c : key)
        if (isalpha(c))
            cleanKey += toupper(c);

    if (cleanKey.empty())
        return "Error: Key must contain letters.";

    int k = 0;
    for (int i = 0; i < (int)text.size(); i++)
    {
        if (!isalpha(text[i])) {
            result[i] = text[i];
            continue;
        }

        char base = islower(text[i]) ? 'a' : 'A';
        int keyShift = cleanKey[k % cleanKey.size()] - 'A';

        if (!encrypt) keyShift = -keyShift;

        result[i] = (char)(base + (text[i] - base + keyShift + 26) % 26);
        k++;
    }
    return result;
}

//--------------------------------------------------
// PROCESS TEXT
//--------------------------------------------------
void ProcessText(bool encrypt)
{
    char inputText[500], shiftText[10], keyText[100];
    GetWindowTextA(hInput, inputText, 500);
    GetWindowTextA(hShiftBox, shiftText, 10);
    GetWindowTextA(hKeyBox, keyText, 100);

    string text = inputText;
    string shiftStr = shiftText;
    string keyStr = keyText;

    string result;

    if (SendMessage(hCaesarRadio, BM_GETCHECK, 0, 0) == BST_CHECKED)
    {
        if (shiftStr.empty() || !isdigit(shiftStr[0]))
        {
            result = "Error: Shift must be a number.";
        }
        else
        {
            int shift = stoi(shiftStr) % 26;
            result = caesar(text, shift, encrypt);
        }
    }
    else
    {
        if (keyStr.empty())
        {
            result = "Error: Enter a key.";
        }
        else
        {
            result = vigenere(text, keyStr, encrypt);
        }
    }

    SetWindowTextA(hOutput, result.c_str());
}

//--------------------------------------------------
// ENABLE / DISABLE INPUT FIELDS
//--------------------------------------------------
void UpdateInputFields()
{
    bool caesar = SendMessage(hCaesarRadio, BM_GETCHECK, 0, 0) == BST_CHECKED;

    EnableWindow(hShiftBox, caesar);        // enable only for caesar
    EnableWindow(hKeyBox, !caesar);         // enable only for vigenere
}

//--------------------------------------------------
// WINDOW PROCEDURE
//--------------------------------------------------
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
        // Input text box
        hInput = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE,
                              20, 20, 820, 120, hwnd, NULL, NULL, NULL);

        // Radio buttons
        hCaesarRadio = CreateWindow("BUTTON", "Caesar Cipher", WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON,
                                    20, 160, 200, 25, hwnd, (HMENU)1, NULL, NULL);
        hVigenereRadio = CreateWindow("BUTTON", "Vigenere Cipher", WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON,
                                      240, 160, 200, 25, hwnd, (HMENU)2, NULL, NULL);

        SendMessage(hCaesarRadio, BM_SETCHECK, BST_CHECKED, 0);

        // Shift input
        CreateWindow("STATIC", "Shift:", WS_CHILD | WS_VISIBLE,
                     20, 200, 60, 25, hwnd, NULL, NULL, NULL);
        hShiftBox = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER,
                                 90, 200, 80, 25, hwnd, NULL, NULL, NULL);

        // Key input
        CreateWindow("STATIC", "Key:", WS_CHILD | WS_VISIBLE,
                     240, 200, 60, 25, hwnd, NULL, NULL, NULL);
        hKeyBox = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER,
                               300, 200, 200, 25, hwnd, NULL, NULL, NULL);

        // Buttons
        hEncryptBtn = CreateWindow("BUTTON", "Encrypt", WS_CHILD | WS_VISIBLE,
                                   20, 240, 150, 35, hwnd, (HMENU)3, NULL, NULL);
        hDecryptBtn = CreateWindow("BUTTON", "Decrypt", WS_CHILD | WS_VISIBLE,
                                   200, 240, 150, 35, hwnd, (HMENU)4, NULL, NULL);

        // Output box
        hOutput = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_READONLY,
                               20, 290, 820, 200, hwnd, NULL, NULL, NULL);

        UpdateInputFields();
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case 1: // Caesar radio
        case 2: // Vigenere radio
            SendMessage(hCaesarRadio, BM_SETCHECK, LOWORD(wParam) == 1, 0);
            SendMessage(hVigenereRadio, BM_SETCHECK, LOWORD(wParam) == 2, 0);
            UpdateInputFields();
            break;

        case 3: // Encrypt
            ProcessText(true);
            break;

        case 4: // Decrypt
            ProcessText(false);
            break;
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

//--------------------------------------------------
// MAIN
//--------------------------------------------------
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int)
{
    const char CLASS_NAME[] = "CipherApp";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // MUCH BIGGER TABLET-SIZED WINDOW
    HWND hwnd = CreateWindow(CLASS_NAME, "Cipher Encryption App",
                             WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT, CW_USEDEFAULT,
                             900, 600,        // <<< bigger window
                             NULL, NULL, hInst, NULL);

    ShowWindow(hwnd, SW_SHOW);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
