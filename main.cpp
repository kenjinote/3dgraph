#define UNICODE
#pragma comment(linker,"/opt:nowin98")
#pragma comment(lib,"opengl32")
#include<windows.h>
#include<gl\gl.h>

extern "C" int _fltused=1; 

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define WINDOW_ASPECT ((float)WINDOW_HEIGHT/(float)WINDOW_WIDTH)

HDC hDC=NULL;
HGLRC hRC=NULL;
BOOL active=TRUE;
TCHAR szClassName[]=TEXT("Window");
GLfloat rot[2];				/* current rotation */


BOOL InitGL(GLvoid)
{
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f,0.0f,0.0f,0.5f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	return TRUE;
}

VOID DrawGLScene(GLvoid)
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-1.0,1.0,-WINDOW_ASPECT,WINDOW_ASPECT,2.0,10.0);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f,0.0f,-3.0f);
	
	glRotatef(rot[0],1.0f,0.0f,0.0f);
	glRotatef(rot[1],0.0f,1.0f,0.0f);

	glPointSize(5.0);
	glBegin(GL_POINTS);
	for(float x=-1.0f;x<1.0f;x+=0.025f)
	{
		for(float y=-1.0f;y<1.0f;y+=0.025f)
		{
			glVertex3f(x , y , x*x+x*y+y*y/300.0f);
		}
	}
	glEnd();

	glFlush();
}

static void rotate(int ox,int nx,int oy,int ny)
{
	int dx=ox-nx;
	int dy=ny-oy;
	rot[0]+=(dy*180.0f)/500.0f;
	rot[1]-=(dx*180.0f)/500.0f;
#define clamp(x) x=x>360.0f?x-360.0f:x<-360.0f?x+=360.0f:x
	clamp(rot[0]);
	clamp(rot[1]);
#undef clamp
}

LRESULT CALLBACK WndProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	static PIXELFORMATDESCRIPTOR pfd={sizeof(PIXELFORMATDESCRIPTOR),1,PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER,PFD_TYPE_RGBA,32,0,0,0,0,0,0,0,0,0,0,0,0,0,16,0,0,PFD_MAIN_PLANE,0,0,0,0};
	GLuint PixelFormat;
	static BOOL bCapture=0;
	static int oldx,oldy,x,y;
	switch(msg)
	{
	case WM_CREATE:
		if(!(hDC=GetDC(hWnd)))
		{
			return -1;
		}
		if(!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))
		{
			return -1;
		}
		if(!SetPixelFormat(hDC,PixelFormat,&pfd))
		{
			return -1;
		}
		if(!(hRC=wglCreateContext(hDC)))
		{
			return -1;
		}
		if(!wglMakeCurrent(hDC,hRC))
		{
			return -1;
		}
		if(!InitGL())
		{
			return -1;
		}
		break;
	case WM_ACTIVATE:
		active=!HIWORD(wParam);
		break;
	case WM_LBUTTONDOWN:
		SetCapture(hWnd);
		x=LOWORD(lParam);
		y=HIWORD(lParam);
		bCapture=1;
		break;
	case WM_LBUTTONUP:
		ReleaseCapture();
		bCapture=0;
		break;
	case WM_MOUSEMOVE:
		if(bCapture)
		{
			oldx=x;
			oldy=y;
			x=LOWORD(lParam);
			y=HIWORD(lParam);
			if(x&1<<15)x-=(1<<16);
			if(y&1<<15)y-=(1<<16);
			rotate(oldx,x,oldy,y);
		}
		break;
	case WM_DESTROY:
		if(hRC)
		{
			wglMakeCurrent(NULL,NULL);
			wglDeleteContext(hRC);
		}
		if(hDC)
		{
			ReleaseDC(hWnd,hDC);
		}
		PostQuitMessage(0);
		break;
	case WM_SYSCOMMAND:
		switch (wParam)
		{
		case SC_SCREENSAVE:
		case SC_MONITORPOWER:
			return 0;
		}
	default:
		return DefWindowProc(hWnd,msg,wParam,lParam);
	}
	return 0;
}

EXTERN_C void __cdecl WinMainCRTStartup()
{
	MSG msg;
	HINSTANCE hInstance=GetModuleHandle(0);
	WNDCLASS wndclass={0,WndProc,0,0,hInstance,0,LoadCursor(0,IDC_ARROW),0,0,szClassName};
	RegisterClass(&wndclass);
	RECT rect={0,0,WINDOW_WIDTH,WINDOW_HEIGHT};
	AdjustWindowRect(&rect,WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,FALSE);
	HWND hWnd=CreateWindow(szClassName,TEXT("Window"),WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,CW_USEDEFAULT,0,rect.right-rect.left,rect.bottom-rect.top,0,0,hInstance,0);
	ShowWindow(hWnd,SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	BOOL done=FALSE;
	while(!done)
	{
		if(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{
			if(msg.message==WM_QUIT)
			{
				done=TRUE;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else if(active)
		{
			DrawGLScene();
			SwapBuffers(hDC);
		}
	}
	ExitProcess(0);
}

#if _DEBUG
void main(){}
#endif
