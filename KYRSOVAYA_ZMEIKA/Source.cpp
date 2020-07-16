#include <windows.h> // підключення бібліотеки з функціями API
#include "ctime"
#include "iostream"
#include "fstream"
#include <string>
#include "resource.h"
using namespace std;

// Глобальні змінні:
HINSTANCE hInst; 	//Дескриптор програми	
LPCTSTR szWindowClass = "QWERTY";
LPCTSTR szTitle = "Курсовая работа - ЗМЕЙКА";

HWND hWndForTimer;

int vero = 40; // вероятность выпадения бонусов
bool GameOver = false;

UINT_PTR indTimer;
int speedgame = 80;

int sizeZmei = 20;
int BorderX = (GetSystemMetrics(SM_CXSCREEN)*0.8);
int BorderY = (GetSystemMetrics(SM_CYSCREEN)*0.8);

bool lock = false;

bool narcomania = false;

int buffNaprvl; //буффер для сохранения промежуточного направления




ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);


class Telo 
{
public:
	int naprav;
	int speed = 10;
	int x, y;
	int RgB[2];

	Telo()
	{
		RgB[0] = 255;
		RgB[1] = 230;
		RgB[2] = 230;

		naprav = 0;
		x = (BorderX/2);
		y = (BorderY/2);
		x -= x % 20;
		y -= y % 20;
	}
	Telo(Telo &Temp, int *_RgB)
	{
		RgB[0] = _RgB[0];
		RgB[1] = _RgB[1];
		RgB[2] = _RgB[2];
		switch (Temp.naprav)
		{
		case 1: //Если двигается влево
			x = Temp.x+ sizeZmei;
			y = Temp.y;
			naprav = Temp.naprav;
			break;

		case 2://Если двигается вверх
			x = Temp.x;
			y = Temp.y+ sizeZmei;
			naprav = Temp.naprav;
			break;

		case 3://Если двигается вправо
			x = Temp.x - sizeZmei;
			y = Temp.y;
			naprav = Temp.naprav;
			break;

		case 4://Если двигается вниз
			x = Temp.x ;
			y = Temp.y- sizeZmei;
			naprav = Temp.naprav;
			break;
		}
		
	}
	void Show(HDC &hdc)
	{
		
		HBRUSH ColorTelo;
		ColorTelo = CreateSolidBrush(RGB(RgB[0], RgB[1], RgB[2]));
		SelectObject(hdc, ColorTelo);
		if (narcomania)
			RoundRect(hdc, x - 3 + rand() % 7, y - 3 + rand() % 7, x + sizeZmei - 3 + rand() % 7, y + sizeZmei - 3 + rand() % 7, 2000, 2000); // рисуем наркоманное тело
		else
		RoundRect(hdc, x, y, x + sizeZmei, y + sizeZmei, 2000, 2000); // рисуем тело

		DeleteObject(ColorTelo);

		
		switch (naprav)
		{
		case 1: //Если двигается влево
			x = x - sizeZmei;
			break;

		case 2://Если двигается вверх
			y = y - sizeZmei;
			break;

		case 3://Если двигается вправо
			x = x + sizeZmei;
			break;

		case 4://Если двигается вниз
			y = y + sizeZmei;
			break;
		}
		int temp = naprav;
		naprav = buffNaprvl; //направление следует за следующей частью змеи
		buffNaprvl = temp;

		if (x + sizeZmei > BorderX || x  <0 || y + sizeZmei > BorderY || y < 0)
			GameOver = true;
		return;
		
	}
};

class Konteiner
{
public:
	int number;
	Telo **pointer;
	Konteiner()
	{
		this->number = 0;
		pointer = nullptr;
	}
	void Add(Telo &Temp)
	{
		Telo **lol;
		lol = pointer;
		pointer = new Telo*[++number];
		for (int i = 0; i < number - 1; i++)
			pointer[i] = lol[i];
		pointer[number - 1] = &Temp;
		delete[] lol;
	}
	void Delite(Telo &Temp)
	{
		Telo **lol;
		lol = pointer;
		int indx;
		for (int i = 0; i < number; i++)
		{
			if (pointer[i] == &Temp)
			{
				indx = i;
				break;
			}
		}
		pointer = new Telo*[--number];
		for (int i = 0; i < indx; i++)
			pointer[i] = lol[i];
		for (int i = indx + 1; i < number + 1; i++)
			pointer[i - 1] = lol[i];
		delete[] lol;
	}
	void DeliteAll()
	{
		number = 0;
		delete[] pointer;
		pointer = nullptr;
	}

	bool Koords(int _x, int _y)
	{
		if (number == 0) return false;

		for (int i = 0; i < number; i++)
			if (pointer[i]->x == _x && pointer[i]->y == _y)
			 return false;

		return true;
	}
	void TouchMe() // Проверка столкновения змеи с собой
	{
		if (number == 0) return ;

		for (int i = 1; i < number; i++)
			if (pointer[i]->x == pointer[0]->x && pointer[i]->y == pointer[0]->y)
			{
				GameOver = true;
				return;
			}
		return ;
	}

	void Graphic(HDC &hdc) // Прописовка змеи
	{
		if (number == 0) return;
		buffNaprvl = pointer[0]->naprav;
		pointer[0]->Show(hdc);

		for (int i = 1; i < number; i++)
			pointer[i]->Show(hdc);
		buffNaprvl = pointer[0]->naprav;
		return;
	}
};
Konteiner konteiner;
static Telo *zmey;


class Drop
{
public:
	int RgB[2];
	int x, y;
	Drop() {}
	virtual void Life(HDC &hdc) = 0;
	virtual void Run() = 0;
};


class KonteinerDrop
{
public:
	int number;
	Drop **pointer;
	KonteinerDrop()
	{
		this->number = 0;
		pointer = nullptr;
	}
	void Add(Drop &Temp)
	{
		Drop **lol;
		lol = pointer;
		pointer = new Drop*[++number];
		for (int i = 0; i < number - 1; i++)
			pointer[i] = lol[i];
		pointer[number - 1] = &Temp;
		delete[] lol;
	}
	void Delite(Drop &Temp)
	{
		Drop **lol;
		lol = pointer;
		int indx;
		for (int i = 0; i < number; i++)
			if (pointer[i] == &Temp)
			{
				indx = i;
				break;
			}
		pointer = new Drop*[--number];
		for (int i = 0; i < indx; i++)
			pointer[i] = lol[i];
		for (int i = indx + 1; i < number + 1; i++)
			pointer[i - 1] = lol[i];
		delete[] lol;
	}
	bool Koords(int _x, int _y)
	{
		if (number == 0) return true;

		for (int i = 0; i < number; i++)
			if (pointer[i]->x == _x && pointer[i]->y == _y)
				return false;

		return true;
	}

	void DeliteOne()
	{
		if (number == 0) return;
		Drop **lol;
		lol = pointer;
		int indx = 0;

		pointer = new Drop*[--number];
		for (int i = 0; i < indx; i++)
			pointer[i] = lol[i];
		for (int i = indx + 1; i < number + 1; i++)
			pointer[i - 1] = lol[i];
		delete[] lol;
	}
	void DeliteAll()
	{
		number = 0;
		delete[] pointer;
		pointer = nullptr;
	}
	void Graphic(HDC &hdc)
	{
		if (number == 0) return;
		for (int i = 0; i < number; i++)
			pointer[i]->Life(hdc);
	}
	void Touch() // Проверка столкновения змеи с обьектами на игровом поле
	{
		for (int i = 0; i < number; i++)
		{
			if (pointer[i]->x == konteiner.pointer[0]->x && pointer[i]->y == konteiner.pointer[0]->y) //если голова змеи совпадает с координатами обьекта
			{
				pointer[i]->Run();
				if (number!=0)
				Delite(*pointer[i]);	
			}
		}
	}
};

KonteinerDrop KontDrop;


class DropMine : public Drop
{
public:
	DropMine()
	{
		RgB[0] = rand() % 256;
		RgB[1] = rand() % 256;
		RgB[2] = rand() % 256;

		while (true)
		{
			x = 20 * (rand() % (BorderX / 20));
			y = 20 * (rand() % (BorderY / 20));

			if (konteiner.Koords(x, y) && KontDrop.Koords(x,y))
				break;
		}
	}
	virtual void Life(HDC &hdc) override
	{

		HBRUSH krolik;
		krolik = CreateSolidBrush(RGB(RgB[0], RgB[1], RgB[2]));
		SelectObject(hdc, krolik);
		RoundRect(hdc, x, y, x + sizeZmei, y + sizeZmei, 0, 0);
		DeleteObject(krolik);
	}
	virtual void Run() override
	{
		GameOver = true;
	}
};
static Drop *drop;

class DropApple : public Drop
{
public:
	DropApple()
	{
		RgB[0] = rand() % 256;
		RgB[1] = rand() % 256;
		RgB[2] = rand() % 256;
		while (true)
		{
			x = 20 * (rand() % (BorderX/20));
			y = 20 * (rand() % (BorderY/20));

			if (konteiner.Koords(x, y) && KontDrop.Koords(x, y))
				break;
		}
	}
	virtual void Life(HDC &hdc) override
	{

		HBRUSH krolik;
		krolik = CreateSolidBrush(RGB(RgB[0], RgB[1], RgB[2]));
		SelectObject(hdc, krolik);
		RoundRect(hdc, x, y, x + sizeZmei, y + sizeZmei, 20, 20);
		DeleteObject(krolik);
	}
	virtual void Run() override
	{
		KillTimer(hWndForTimer, indTimer);
		if (speedgame >= 6 && rand() % 4 == 1)
				speedgame -= 1;

		indTimer = SetTimer(hWndForTimer, 1, speedgame, NULL);

		zmey = new Telo(*konteiner.pointer[konteiner.number - 1], RgB);
		konteiner.Add(*zmey);
	}
};

class DropPrize : public Drop
{
public:
	DropPrize()
	{
		
		while (true)
		{
			x = 20 * (rand() % (BorderX / 20));
			y = 20 * (rand() % (BorderY / 20));

			if (konteiner.Koords(x, y) && KontDrop.Koords(x, y))
				break;
		}
	}
	virtual void Life(HDC &hdc) override
	{

		HBRUSH krolik;
		krolik = CreateSolidBrush(RGB(rand() % 256, rand() % 256, rand() % 256));
		SelectObject(hdc, krolik);
		RoundRect(hdc, x, y, x + sizeZmei, y + sizeZmei, 20, 20);
		DeleteObject(krolik);
	}
	virtual void Run() override
	{
		switch (rand() % 5)
		{
		case 0:
			for (int i = 0; i < 30; i++)
			{
				drop = new DropApple();
				KontDrop.Add(*drop);
			}
			break;
		case 1:
			for (int i = 0; i < 30; i++)
			{
				drop = new DropMine();
				KontDrop.Add(*drop);
			}
			break;
		case 2:
			narcomania = !narcomania;
			break;
		case 3:
			KontDrop.DeliteAll();
			break;
		case 4 :
			KillTimer(hWndForTimer, indTimer);
			speedgame = 90;

			indTimer = SetTimer(hWndForTimer, 1, speedgame, NULL);
			break;
		
		}
		
	}
};





 
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,
	int nCmdShow)
{
	MSG msg;
	// Реєстрація класу вікна 
	MyRegisterClass(hInstance);
	// Створення вікна програми
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}
	// Цикл обробки повідомлень
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = 0; 		//стиль вікна
	wcex.lpfnWndProc = (WNDPROC)WndProc; 		//віконна процедура
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance; 			//дескриптор програми
	wcex.hIcon = LoadIcon(NULL, IDI_HAND); 		//визначення іконки
	wcex.hCursor = LoadCursor(NULL, IDC_CROSS); 	//визначення курсору
	wcex.hbrBackground = CreateSolidBrush(RGB(135, 206, 235)); //установка фону
	wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1); 				//визначення меню
	wcex.lpszClassName = szWindowClass; 		//ім’я класу
	wcex.hIconSm = NULL;
	return RegisterClassEx(&wcex); 			//реєстрація класу вікна
}
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	BorderX -= BorderX % sizeZmei;
	BorderY -= BorderY % sizeZmei;

	HWND hWnd;
	hInst = hInstance; //зберігає дескриптор додатка в змінній hInst
	hWnd = CreateWindow(szWindowClass, 	// ім’я класу вікна
		szTitle, 				// назва програми
		WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,			// стиль вікна
		GetSystemMetrics(SM_CXSCREEN)*0.1, 			// положення по Х	
		GetSystemMetrics(SM_CYSCREEN)*0.1,			// положення по Y	
		BorderX, 			// розмір по Х
		BorderY, 			// розмір по Y
		NULL, 					// дескриптор батьківського вікна	
		NULL, 					// дескриптор меню вікна
		hInstance, 				// дескриптор програми
		NULL); 				// параметри створення.
	
	
	if (!hWnd) 	//Якщо вікно не творилось, функція повертає FALSE
	{
		return FALSE;
	}
	ShowWindow(hWnd, nCmdShow); 		//Показати вікно
	UpdateWindow(hWnd); 				//Оновити вікно
	return TRUE;
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	static TCHAR ch;
	RECT rt;
	HDC hdc;

	hWndForTimer = hWnd;

	switch (message)
	{
	case WM_CREATE: 				//Повідомлення приходить при створенні вік-на
		zmey = new Telo();
		konteiner.Add(*zmey);
		srand(time(0));


		indTimer = SetTimer(hWnd, 1, speedgame, NULL);
		
		break;
	case WM_CHAR:
		if (lock)
			break;

		ch = (TCHAR)wParam;
		
		if (( ((ch == 'ф')  ||  (ch == 'a')) && konteiner.pointer[0]->naprav!=3))
			konteiner.pointer[0]->naprav = 1; //голова змеи поворачивает

		if (( ((ch == 'ц') || (ch == 'w'))&& konteiner.pointer[0]->naprav != 4))
			konteiner.pointer[0]->naprav = 2; //голова змеи поворачивает

		if (( ((ch == 'в') || (ch == 'd'))&& konteiner.pointer[0]->naprav != 1))
			konteiner.pointer[0]->naprav = 3; //голова змеи поворачивает

		if (( ((ch == 'ы') || (ch == 's') || (ch == 'і'))&& konteiner.pointer[0]->naprav != 2))
			konteiner.pointer[0]->naprav = 4; //голова змеи поворачивает

		if (ch == 'l'||(ch == 'д'))
		{

			static int rgbtemp[2];
			rgbtemp[0] = rand() % 256;
			rgbtemp[1] = rand() % 256;
			rgbtemp[2] = rand() % 256;
			konteiner.Add(*(new Telo(*konteiner.pointer[konteiner.number - 1], rgbtemp)));

		}
		if (ch == 'k' || (ch == 'л'))
			KontDrop.Add(*(new DropPrize()));

		if (ch == 'j' || (ch == 'о'))
		for (int i = 0; i < 30; i++)
		{
			drop = new DropApple();
			KontDrop.Add(*drop);
		}

		lock = true;
		break;
	case WM_PAINT: 				//Перемалювати вікно
		if (GameOver)
			break;
		hdc = BeginPaint(hWnd, &ps); 	//Почати графічний вивід	
		GetClientRect(hWnd, &rt); 		//Область вікна для малювання
		BorderX = rt.right;
		BorderY = rt.bottom;

		konteiner.Graphic(hdc);
		konteiner.TouchMe();
		KontDrop.Graphic(hdc);
		KontDrop.Touch();
 
		TextOut(hdc, 0, 0, "Очки: ", strlen("Очки: "));
		TextOut(hdc, 40, 0, to_string(konteiner.number).data(), strlen(to_string(konteiner.number).data()));

		EndPaint(hWnd, &ps); 		//Закінчити графічний вивід	
		break;
	case WM_DESTROY: 				//Завершення роботи
		PostQuitMessage(0);
		break;

	
	case WM_TIMER:
		if (!GameOver)
		{
			if (rand() % vero == 1 )  // С неким шансом спавнятся негативные обьекты (огрызок)
				KontDrop.Add(*(new DropApple()));

			if (rand() % vero == 2 ) // С неким шансом спавнятся положительные обьекты (яблоко)
				KontDrop.Add(*(new DropMine()));

			if (rand() % 1000 == 3)  // С неким шансом спавнятся бонусные обьекты
				KontDrop.Add(*(new DropPrize()));
			
			if (rand() % (vero+10) == 3) // С неким шансом пропадают обьекты на поле
				KontDrop.DeliteOne();
			
			InvalidateRect(hWnd, NULL, TRUE);
			lock = false;
		}
		else
		{
			hdc = GetDC(hWnd);
			TextOut(hdc, BorderX / 2, BorderY / 2, "GAME OVER!", strlen("GAME OVER!!"));
			ReleaseDC(hWnd,hdc);
		}
		break;
	
	case WM_COMMAND:
	{
		if (LOWORD(wParam) == ID_40001)
		{
			
			KontDrop.DeliteAll();
			konteiner.DeliteAll();
			//zmey = new Telo();
			konteiner.Add(*(new Telo()));
			GameOver = false;
			narcomania = false;

			speedgame = 80;
			indTimer = SetTimer(hWnd, 1, speedgame, NULL);

			break;
		}
		if (LOWORD(wParam) == ID_40002)
		{
			if(GameOver)
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, DlgProc);
			break;
		}
		if (LOWORD(wParam) == ID_40003)
		{
			if (GameOver) {
				static STARTUPINFO tin;
				static PROCESS_INFORMATION pInfo;

				tin.cb = sizeof(STARTUPINFO);
				tin.dwFlags = STARTF_USESHOWWINDOW;
				tin.wShowWindow = SW_SHOWNORMAL;


				char boof[256] = "notepad Score.txt";
				CreateProcess(NULL, boof,
					NULL, NULL, FALSE, 0, NULL, NULL, &tin, &pInfo);
			}
			break;
		}
		break;
	}
	
	default:
		//Обробка повідомлень, які не оброблені користувачем
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

#define ID_EDITNAME 844
int CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hName;
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rt;

	switch (message)
	{
	case WM_INITDIALOG:  //ініціалізація функціоналу керування діалоговим вікном

		SetTimer(hDlg, 1, 30, NULL);
	 hName = CreateWindow("edit", "Игрок ", WS_VISIBLE | WS_CHILD, 10, 40, 200, 10, hDlg, (HMENU)ID_EDITNAME, hInst, NULL);

		return TRUE;
		break;

	case WM_TIMER:
	{
		
		InvalidateRect(hName, NULL,TRUE);
		break;
	}
	case WM_PAINT: 				//Перемалювати вікно
		hdc = BeginPaint(hDlg, &ps); 	//Почати графічний вивід	
		GetClientRect(hDlg, &rt); 		//Область вікна для малювання

		TextOut(hdc, 10, 0, "Количество очков:", strlen("Количество очков:"));
		TextOut(hdc, 140, 0, to_string(konteiner.number).data(), strlen(to_string(konteiner.number).data()));
		TextOut(hdc, 10, 25, "Введите ваше имя:", strlen("Введите ваше имя:"));

		


		EndPaint(hDlg, &ps);


	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			char nick[256];
			GetWindowText(hName, nick, 255);
			
			ofstream score;
			score.open("Score.txt", ofstream::app);
			score << nick<<" - " << konteiner.number<< endl;
			score.close();

			EndDialog(hDlg, LOWORD(wParam));//знищення модального діалогового вікна
			return TRUE;
		}
		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));//знищення модального діалогового вікна
			return TRUE;
		}
		break;
	}
	return FALSE;
}
