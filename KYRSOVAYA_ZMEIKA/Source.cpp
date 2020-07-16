#include <windows.h> // ���������� �������� � ��������� API
#include "ctime"
#include "iostream"
#include "fstream"
#include <string>
#include "resource.h"
using namespace std;

// �������� ����:
HINSTANCE hInst; 	//���������� ��������	
LPCTSTR szWindowClass = "QWERTY";
LPCTSTR szTitle = "�������� ������ - ������";

HWND hWndForTimer;

int vero = 40; // ����������� ��������� �������
bool GameOver = false;

UINT_PTR indTimer;
int speedgame = 80;

int sizeZmei = 20;
int BorderX = (GetSystemMetrics(SM_CXSCREEN)*0.8);
int BorderY = (GetSystemMetrics(SM_CYSCREEN)*0.8);

bool lock = false;

bool narcomania = false;

int buffNaprvl; //������ ��� ���������� �������������� �����������




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
		case 1: //���� ��������� �����
			x = Temp.x+ sizeZmei;
			y = Temp.y;
			naprav = Temp.naprav;
			break;

		case 2://���� ��������� �����
			x = Temp.x;
			y = Temp.y+ sizeZmei;
			naprav = Temp.naprav;
			break;

		case 3://���� ��������� ������
			x = Temp.x - sizeZmei;
			y = Temp.y;
			naprav = Temp.naprav;
			break;

		case 4://���� ��������� ����
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
			RoundRect(hdc, x - 3 + rand() % 7, y - 3 + rand() % 7, x + sizeZmei - 3 + rand() % 7, y + sizeZmei - 3 + rand() % 7, 2000, 2000); // ������ ����������� ����
		else
		RoundRect(hdc, x, y, x + sizeZmei, y + sizeZmei, 2000, 2000); // ������ ����

		DeleteObject(ColorTelo);

		
		switch (naprav)
		{
		case 1: //���� ��������� �����
			x = x - sizeZmei;
			break;

		case 2://���� ��������� �����
			y = y - sizeZmei;
			break;

		case 3://���� ��������� ������
			x = x + sizeZmei;
			break;

		case 4://���� ��������� ����
			y = y + sizeZmei;
			break;
		}
		int temp = naprav;
		naprav = buffNaprvl; //����������� ������� �� ��������� ������ ����
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
	void TouchMe() // �������� ������������ ���� � �����
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

	void Graphic(HDC &hdc) // ���������� ����
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
	void Touch() // �������� ������������ ���� � ��������� �� ������� ����
	{
		for (int i = 0; i < number; i++)
		{
			if (pointer[i]->x == konteiner.pointer[0]->x && pointer[i]->y == konteiner.pointer[0]->y) //���� ������ ���� ��������� � ������������ �������
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
	// ��������� ����� ���� 
	MyRegisterClass(hInstance);
	// ��������� ���� ��������
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}
	// ���� ������� ����������
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
	wcex.style = 0; 		//����� ����
	wcex.lpfnWndProc = (WNDPROC)WndProc; 		//������ ���������
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance; 			//���������� ��������
	wcex.hIcon = LoadIcon(NULL, IDI_HAND); 		//���������� ������
	wcex.hCursor = LoadCursor(NULL, IDC_CROSS); 	//���������� �������
	wcex.hbrBackground = CreateSolidBrush(RGB(135, 206, 235)); //��������� ����
	wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1); 				//���������� ����
	wcex.lpszClassName = szWindowClass; 		//��� �����
	wcex.hIconSm = NULL;
	return RegisterClassEx(&wcex); 			//��������� ����� ����
}
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	BorderX -= BorderX % sizeZmei;
	BorderY -= BorderY % sizeZmei;

	HWND hWnd;
	hInst = hInstance; //������ ���������� ������� � ����� hInst
	hWnd = CreateWindow(szWindowClass, 	// ��� ����� ����
		szTitle, 				// ����� ��������
		WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,			// ����� ����
		GetSystemMetrics(SM_CXSCREEN)*0.1, 			// ��������� �� �	
		GetSystemMetrics(SM_CYSCREEN)*0.1,			// ��������� �� Y	
		BorderX, 			// ����� �� �
		BorderY, 			// ����� �� Y
		NULL, 					// ���������� ������������ ����	
		NULL, 					// ���������� ���� ����
		hInstance, 				// ���������� ��������
		NULL); 				// ��������� ���������.
	
	
	if (!hWnd) 	//���� ���� �� ���������, ������� ������� FALSE
	{
		return FALSE;
	}
	ShowWindow(hWnd, nCmdShow); 		//�������� ����
	UpdateWindow(hWnd); 				//������� ����
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
	case WM_CREATE: 				//����������� ��������� ��� �������� ��-��
		zmey = new Telo();
		konteiner.Add(*zmey);
		srand(time(0));


		indTimer = SetTimer(hWnd, 1, speedgame, NULL);
		
		break;
	case WM_CHAR:
		if (lock)
			break;

		ch = (TCHAR)wParam;
		
		if (( ((ch == '�')  ||  (ch == 'a')) && konteiner.pointer[0]->naprav!=3))
			konteiner.pointer[0]->naprav = 1; //������ ���� ������������

		if (( ((ch == '�') || (ch == 'w'))&& konteiner.pointer[0]->naprav != 4))
			konteiner.pointer[0]->naprav = 2; //������ ���� ������������

		if (( ((ch == '�') || (ch == 'd'))&& konteiner.pointer[0]->naprav != 1))
			konteiner.pointer[0]->naprav = 3; //������ ���� ������������

		if (( ((ch == '�') || (ch == 's') || (ch == '�'))&& konteiner.pointer[0]->naprav != 2))
			konteiner.pointer[0]->naprav = 4; //������ ���� ������������

		if (ch == 'l'||(ch == '�'))
		{

			static int rgbtemp[2];
			rgbtemp[0] = rand() % 256;
			rgbtemp[1] = rand() % 256;
			rgbtemp[2] = rand() % 256;
			konteiner.Add(*(new Telo(*konteiner.pointer[konteiner.number - 1], rgbtemp)));

		}
		if (ch == 'k' || (ch == '�'))
			KontDrop.Add(*(new DropPrize()));

		if (ch == 'j' || (ch == '�'))
		for (int i = 0; i < 30; i++)
		{
			drop = new DropApple();
			KontDrop.Add(*drop);
		}

		lock = true;
		break;
	case WM_PAINT: 				//������������ ����
		if (GameOver)
			break;
		hdc = BeginPaint(hWnd, &ps); 	//������ ��������� ����	
		GetClientRect(hWnd, &rt); 		//������� ���� ��� ���������
		BorderX = rt.right;
		BorderY = rt.bottom;

		konteiner.Graphic(hdc);
		konteiner.TouchMe();
		KontDrop.Graphic(hdc);
		KontDrop.Touch();
 
		TextOut(hdc, 0, 0, "����: ", strlen("����: "));
		TextOut(hdc, 40, 0, to_string(konteiner.number).data(), strlen(to_string(konteiner.number).data()));

		EndPaint(hWnd, &ps); 		//�������� ��������� ����	
		break;
	case WM_DESTROY: 				//���������� ������
		PostQuitMessage(0);
		break;

	
	case WM_TIMER:
		if (!GameOver)
		{
			if (rand() % vero == 1 )  // � ����� ������ ��������� ���������� ������� (�������)
				KontDrop.Add(*(new DropApple()));

			if (rand() % vero == 2 ) // � ����� ������ ��������� ������������� ������� (������)
				KontDrop.Add(*(new DropMine()));

			if (rand() % 1000 == 3)  // � ����� ������ ��������� �������� �������
				KontDrop.Add(*(new DropPrize()));
			
			if (rand() % (vero+10) == 3) // � ����� ������ ��������� ������� �� ����
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
		//������� ����������, �� �� �������� ������������
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
	case WM_INITDIALOG:  //����������� ����������� ��������� ��������� �����

		SetTimer(hDlg, 1, 30, NULL);
	 hName = CreateWindow("edit", "����� ", WS_VISIBLE | WS_CHILD, 10, 40, 200, 10, hDlg, (HMENU)ID_EDITNAME, hInst, NULL);

		return TRUE;
		break;

	case WM_TIMER:
	{
		
		InvalidateRect(hName, NULL,TRUE);
		break;
	}
	case WM_PAINT: 				//������������ ����
		hdc = BeginPaint(hDlg, &ps); 	//������ ��������� ����	
		GetClientRect(hDlg, &rt); 		//������� ���� ��� ���������

		TextOut(hdc, 10, 0, "���������� �����:", strlen("���������� �����:"));
		TextOut(hdc, 140, 0, to_string(konteiner.number).data(), strlen(to_string(konteiner.number).data()));
		TextOut(hdc, 10, 25, "������� ���� ���:", strlen("������� ���� ���:"));

		


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

			EndDialog(hDlg, LOWORD(wParam));//�������� ���������� ���������� ����
			return TRUE;
		}
		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));//�������� ���������� ���������� ����
			return TRUE;
		}
		break;
	}
	return FALSE;
}
